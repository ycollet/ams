#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include "multi_envelope.h"


MultiEnvelope::MultiEnvelope(int p_envCount, float *p_timeScaleRef, float *p_attackRef, float *p_sustainRef, float *p_releaseRef,
                   QWidget* parent, const char *name, SynthData *p_synthdata) 
             : QWidget (parent, name)
{
  envCount = p_envCount;
  synthdata = p_synthdata;
  timeScaleRef = p_timeScaleRef;
  attackRef = p_attackRef;
  sustainRef = p_sustainRef;
  releaseRef = p_releaseRef;
  setPalette(QPalette(QColor(20, 20, 80), QColor(20, 20, 80)));
  setMinimumHeight(125);
  colorTable[1].setRgb(255, 0, 0);
  colorTable[2].setRgb(0, 255, 0);
  colorTable[3].setRgb(50, 150, 255); 
  colorTable[4].setRgb(255, 255, 0);
  colorTable[0].setRgb(255, 255, 255);
  colorTable[5].setRgb(0, 255, 255);
  colorTable[6].setRgb(255, 100, 255);
  colorTable[7].setRgb(255, 200, 50);
}

MultiEnvelope::~MultiEnvelope()
{
}

void MultiEnvelope::paintEvent(QPaintEvent *) {

  QPixmap pm(width(), height());  
  QPainter p(&pm);
  QPointArray points(10);
  QPen pen;
  QString qs;
  int l1;
  float len, maxlen, x, y, xscale, yscale;

  p.setViewport(0, 0, width(), height());
  p.setWindow(0, 0, width(), height());
  if (envCount > 1) {
    pm.fill(QColor(20, 20, 80));
    maxlen = 0;
    for (l1 = 0; l1 < envCount; l1++ ) { 
      len = timeScaleRef[l1] * (attackRef[l1] + attackRef[8+l1] + attackRef[24+l1] + attackRef[40+l1] + attackRef[56+l1]  
                             + releaseRef[l1] + releaseRef[16+l1] + releaseRef[32+l1]) + SUSTAIN_LEN;
      if (len > maxlen) maxlen = len;
    }
    for (l1 = 0; l1 < envCount; l1++ ) { 
      xscale = timeScaleRef[l1] * (float)width() / maxlen;
      yscale = (float)(height()-26);
      x = attackRef[l1] * xscale;   
      points.setPoint(0, (int)x, height());
      x += attackRef[8+l1] * xscale; 
      y = attackRef[16+l1] * yscale;
      points.setPoint(1, (int)x, height() - (int)y);
      x += attackRef[24+l1] * xscale; 
      y = attackRef[32+l1] * yscale;
      points.setPoint(2, (int)x, height() - (int)y);
      x += attackRef[40+l1] * xscale; 
      y = attackRef[48+l1] * yscale;
      points.setPoint(3, (int)x, height() - (int)y);
      x += attackRef[56+l1] * xscale; 
      y = sustainRef[l1] * yscale;
      points.setPoint(4, (int)x, height() - (int)y);
      x += SUSTAIN_LEN * xscale / timeScaleRef[l1];
      points.setPoint(5, (int)x, height() - (int)y);
      x += releaseRef[l1] * xscale;
      y = releaseRef[8+l1] * yscale;
      points.setPoint(6, (int)x, height() - (int)y);
      x += releaseRef[16+l1] * xscale;
      y = releaseRef[24+l1] * yscale;
      points.setPoint(7, (int)x, height() - (int)y);
      x += releaseRef[32+l1] * xscale;
      points.setPoint(8, (int)x, height());
      x = attackRef[l1] * xscale;
      points.setPoint(9, (int)x, height());
      pen.setColor(colorTable[l1]);
      pen.setWidth(2);
      p.setPen(pen);
      p.drawPolyline(points);
      qs.sprintf("Env. %d", l1);
      p.drawText(40 * l1 + 5, 15, qs);   
    }
  } else {
    pm.fill(QColor(10, 50, 10));
    len = attackRef[0] + attackRef[1] + attackRef[3] + attackRef[5] + attackRef[7]  
        + releaseRef[0] + releaseRef[2] + releaseRef[4] + SUSTAIN_LEN;
    xscale = (float)width() / len;
    yscale = (float)(height()-6);
    x = attackRef[0] * xscale;   
    points.setPoint(0, (int)x, height());
    x += attackRef[1] * xscale; 
    y = attackRef[2] * yscale;
    points.setPoint(1, (int)x, height() - (int)y);
    x += attackRef[3] * xscale; 
    y = attackRef[4] * yscale;
    points.setPoint(2, (int)x, height() - (int)y);
    x += attackRef[5] * xscale; 
    y = attackRef[6] * yscale;
    points.setPoint(3, (int)x, height() - (int)y);
    x += attackRef[7] * xscale; 
    y = *sustainRef * yscale;
    points.setPoint(4, (int)x, height() - (int)y);
    x += SUSTAIN_LEN * xscale;
    points.setPoint(5, (int)x, height() - (int)y);
    x += releaseRef[0] * xscale;
    y = releaseRef[1] * yscale;
    points.setPoint(6, (int)x, height() - (int)y);
    x += releaseRef[2] * xscale;
    y = releaseRef[3] * yscale;
    points.setPoint(7, (int)x, height() - (int)y);
    x += releaseRef[4] * xscale;
    points.setPoint(8, (int)x, height());
    x = attackRef[0] * xscale;
    points.setPoint(9, (int)x, height());
    p.setBrush(QBrush(QColor(10, 80, 10)));
    p.drawPolygon(points);
    pen.setColor(QColor(10, 110, 10));
    pen.setWidth(3);
    p.setPen(pen);
    p.drawPolyline(points);  
    pen.setColor(QColor(20, 160, 20));
    pen.setWidth(1);
    p.setPen(pen);
    p.drawPolyline(points); 
  }
  bitBlt(this, 0, 0, &pm);
}

void MultiEnvelope::updateMultiEnvelope(int value) {

  repaint(false);
}

QSize MultiEnvelope::sizeHint() const {

  return QSize(MULTI_ENVELOPE_MINIMUM_WIDTH, MULTI_ENVELOPE_MINIMUM_HEIGHT); 
}

QSizePolicy MultiEnvelope::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void MultiEnvelope::resizeEvent (QResizeEvent* )
{
  repaint(true);
}

