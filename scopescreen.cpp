#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qscrollview.h>
#include <qevent.h>
#include <qpixmap.h>
#include "synthdata.h"
#include "scopescreen.h"

ScopeScreen::ScopeScreen(QWidget* parent, const char *name, SynthData *p_synthdata) 
                   : QWidget(parent, name)
{
  synthdata = p_synthdata;
  scopebuf = (float *)malloc(SCOPE_BUFSIZE * sizeof(float));
  scopedata= (float *)malloc(SCOPE_BUFSIZE * sizeof(float));
  scopebufValidFrames = 0;
  mode = MODE_NORMAL;
  edge = EDGE_RISING;
  triggerMode = TRIGGERMODE_CONTINUOUS;
  triggerThrs = 0;
  triggered = false;
  ch1 = 0;
  ch2 = 1;
  zoom = 1;
  timeScale = 100;
  timeScaleLastTrigger = timeScale;
  timeScaleFrames = (int)((float)synthdata->rate * timeScale / 1000.0);
  readofs = 0;
  writeofs = 0;
  setPalette(QPalette(QColor(0, 80, 0), QColor(0, 80, 0)));
}

ScopeScreen::~ScopeScreen()
{
  free(scopebuf);
  free(scopedata);
}

void ScopeScreen::paintEvent(QPaintEvent *) {
  
  int l1;
  float xscale, yscale;
  int x1, x2, y1ch1, y1ch2, y2ch1, y2ch2, thrs, vx, vy, vw, vh;
  float s1, s2;
  QPixmap pm(width(), height());
  QPainter p(&pm);

  pm.fill(QColor(0, 80, 0));
  vx = 0; vy = 0; vw = width(); vh = height();
  xscale = (float)vw / (float)timeScaleFrames;
  yscale = zoom * (float)height() / 65536.0;
  thrs = int(yscale * triggerThrs * 32767.0);
  if (triggerMode == TRIGGERMODE_TRIGGERED) {
    p.setPen(QColor(0, 255, 255));
    p.drawLine(vx, height() - (vy + (vh >> 1) + thrs), vx + 10, height() - (vy + (vh >> 1) + thrs));
    p.setPen(QColor(0, 220, 0));
  }
  for (l1 = 0; l1 < timeScaleFrames - 1; l1++) {
    x1 = int((float)l1 * xscale);
    x2 = int((float)(l1+1) * xscale);
    if (ch1 < 0) {
      s1 = 0;
    } else {
      s1 = scopebuf[2 * l1 + ch1];
    }
    if (ch2 < 0) {
      s2 = 0;
    } else {
      s2 = scopebuf[2 * l1 +ch2]; 
    }
    switch (mode) {
    case MODE_NORMAL:
      y1ch1 = int(yscale * float(s1));
      y1ch2 = int(yscale * float(s2));
      break;    
    case MODE_SUM:
      y1ch1 = int(yscale * float(s1+s2));
      break;
    case MODE_DIFF:
      y1ch1 = int(yscale * float(s1-s2));
      break;
    }
    if (ch1 < 0) {
      s1 = 0;
    } else { 
      s1 = scopebuf[2 * (l1+1) + ch1];
    }
    if (ch2 < 0) {
      s2 = 0;
    } else {
      s2 = scopebuf[2 * (l1+1) + ch2];
    }
    switch (mode) {  
    case MODE_NORMAL:
      y2ch1 = int(yscale * float(s1));
      y2ch2 = int(yscale * float(s2));
      if (ch1 >= 0) {
        p.setPen(QColor(0, 220, 0));
        p.drawLine(vx + x1, height() - ((vh >> 1) + y1ch1), vx + x2, height() - ((vh >> 1) + y2ch1));
      }
      if (ch2 >= 0) {
        p.setPen(QColor(255, 255, 0));
        p.drawLine(vx + x1, height() - ((vh >> 1) + y1ch2), vx + x2, height() - ((vh >> 1) + y2ch2));
      }
      break;      
    case MODE_SUM:
      y2ch1 = int(yscale * float(s1+s2));
      if ((ch1 >= 0) && (ch2 >= 0)) {
        p.setPen(QColor(0, 220, 0));
        p.drawLine(vx + x1, height() - ((vh >> 1) + y1ch1), vx + x2, height() - ((vh >> 1) + y2ch1));
      }
      break;
    case MODE_DIFF:
      y2ch1 = int(yscale * float(s1-s2));
      if ((ch1 >= 0) && (ch2 >= 0)) {
        p.setPen(QColor(0, 220, 0));
        p.drawLine(vx + x1, height() - ((vh >> 1) + y1ch1), vx + x2, height() - ((vh >> 1) + y2ch1));
      }
      break;
    }
  }
  p.setPen(QColor(0, 220, 0));
  p.drawRect(vx, vy, vx + vw, vy + vh);
  p.drawLine(vx, vy + (vh >> 1), vx + vw - 1, vy + (vh >> 1));
  p.drawLine(vx + (vw >> 1), vx, vx + (vw >> 1), vy + vh);
  p.drawText(vx + 5, vy + 20, QString::number((int)timeScale)+" ms");
  bitBlt(this, 0, 0, &pm);
}

void ScopeScreen::refreshScope() {

  int l1, ofs;
  float s1, s2;

  readofs = writeofs - synthdata->cyclesize - timeScaleFrames;
  if (readofs < 0 ) {  
    readofs+=SCOPE_BUFSIZE >> 1;
  }
  ofs = readofs;
  if ((triggerMode == TRIGGERMODE_TRIGGERED) && (ch1 >=0)) {
    l1 = 0;
    triggered = false;
    while(l1 < TRIGGER_RANGE) {
      s1 = scopedata[2 * ofs + ch1];
      ofs--;
      if (ofs < 0 ) {  
        ofs += SCOPE_BUFSIZE >> 1;
      }
      s2 = scopedata[2 * ofs +ch1];
      if (edge == EDGE_FALLING) {
        if ((s1 < triggerThrs * 32767.0) && (s2 > triggerThrs * 32767.0)) {
          readofs = ofs;
          triggered = true;
          break;
        }
      } else {
        if ((s1 > triggerThrs * 32767.0) && (s2 < triggerThrs * 32767.0)) {
          readofs = ofs;
          triggered = true;
          break;
        }
      }
      ofs--;
      if (ofs < 0 ) {
        ofs += SCOPE_BUFSIZE >> 1;  
      }
      l1++;
    }
  }
  if ((triggerMode != TRIGGERMODE_TRIGGERED) || triggered) {
//    fprintf(stderr, "M1\n");
    for (l1 = 0; l1 < timeScaleFrames; l1++) {
//      fprintf(stderr, "l1: %d ofs: %d\n", l1, ofs);
      scopebuf[2 * l1] = scopedata[2 * ofs];
      scopebuf[2 * l1 + 1] = scopedata[2 * ofs + 1];
      ofs++;
      if (ofs >= SCOPE_BUFSIZE >> 1) {
        ofs -= SCOPE_BUFSIZE >> 1;
      }
    }
    scopebufValidFrames = timeScaleFrames;
//    fprintf(stderr, "M2\n");
  }
  repaint(false);
}

void ScopeScreen::singleShot() {

  timeScaleLastTrigger = timeScale;
  refreshScope();
}

QSize ScopeScreen::sizeHint() const {

  return QSize(MINIMUM_WIDTH, MINIMUM_HEIGHT); 
}

QSizePolicy ScopeScreen::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

modeType ScopeScreen::setMode(modeType p_mode) {

  mode = p_mode;
  repaint(false);
  return(mode);
}

edgeType ScopeScreen::setEdge(edgeType p_edge) {

  edge = p_edge;
  return(edge);
}

triggerModeType ScopeScreen::setTriggerMode(triggerModeType p_triggerMode) {

  triggerMode = p_triggerMode;

  if ((triggerMode == TRIGGERMODE_SINGLE) || (triggerMode == TRIGGERMODE_MIDI)) {  
    timeScaleLastTrigger = timeScale;
  }
  return(triggerMode);
}

int ScopeScreen::setCh1(int p_ch1) {

  ch1 = p_ch1;
  repaint(false);
  return(ch1);
}

int ScopeScreen::setCh2(int p_ch2) {       

  ch2 = p_ch2;  
  repaint(false);
  return(ch2); 
}              

float ScopeScreen::setZoom(float p_zoom) {

  zoom = p_zoom;
  repaint(false);
  return(zoom);
}

float ScopeScreen::setTriggerThrs(float p_triggerThrs) {

  triggerThrs = p_triggerThrs;
  return(triggerThrs);
}
 
float ScopeScreen::setTimeScale(float p_timeScale) {

  timeScale = p_timeScale;
  timeScaleFrames = (int)((float)synthdata->rate * timeScale / 1000.0);
  if (timeScaleFrames <= scopebufValidFrames) {
    repaint(false);
  }
  return(timeScale);
}

modeType ScopeScreen::getMode() {

  return(mode); 
}
 
edgeType ScopeScreen::getEdge() {

  return(edge); 
}
 
triggerModeType ScopeScreen::getTriggerMode() {

  return(triggerMode);
}
 
int ScopeScreen::getCh1() {

  return(ch1);
}
 
int ScopeScreen::getCh2() {

  return(ch2);
}             
              
float ScopeScreen::getZoom() {

  return(zoom); 
}

float ScopeScreen::getTriggerThrs() {

  return(triggerThrs);
}

float ScopeScreen::getTimeScale() {
 
  return(timeScale);
}

void ScopeScreen::viewportResizeEvent(QResizeEvent *ev)
{
  repaint(false);
}
