#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qevent.h>
#include "synthdata.h"
#include "spectrumscreen.h"
#include <rfftw.h>
#include <math.h>

SpectrumScreen::SpectrumScreen(QWidget* parent, const char *name, SynthData *p_synthdata) : QWidget (parent, name)
{
  synthdata = p_synthdata;
  spectrumdata= (float *)malloc(SPECTRUM_BUFSIZE * sizeof(float));
  spectrumbuf_ch1 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  spectrumbuf_ch2 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  spectrumbuf_sum = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  mode = SPECTRUM_MODE_NORMAL;
  fftMode = FFT_MODE_ABS;
  viewMode = VIEW_MODE_FIF;
  normMode = NORM_MODE_EACH;
  triggerMode = SPECTRUM_TRIGGERMODE_CONTINUOUS;
  window = WINDOW_HANN;
  ch1 = 0;
  ch2 = 1;
  zoom = 1;
  f_min = 0;
  f_max = (double)synthdata->rate / 2.0;
  f = f_max / 2.0;
  max1 = 0;
  max2 = 0;
  maxsum = 0;
  fftFrames = 1024;
  freqZoom = false;
  setPalette(QPalette(QColor(50, 50, 100), QColor(50, 50, 100)));
  initPalette(0);
  initPalette(1);
  plan = rfftw_create_plan(fftFrames, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
  fftPixmap1 = new QPixmap(SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
  fftPixmap2 = new QPixmap(SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
  fftPixmap1->fill(QColor(50, 50, 100));
  fftPixmap2->fill(QColor(50, 50, 100));
}

SpectrumScreen::~SpectrumScreen()
{
  free(spectrumdata);
  free(spectrumbuf_ch1);
  free(spectrumbuf_ch2);
}

void SpectrumScreen::paintEvent(QPaintEvent *) {
  
  int l1, l2;
  double xscale, xscale1, xscale2, yscale, max, ych1Max, ych2Max, fch1Max, fch2Max;
  int x1, x2, y1ch1, y1ch2, y2ch1, y2ch2, tmp, r, g, b, f1, f2, frame1, frame2, xch1Max, xch2Max;
  QPixmap pm(width(), height());
  QPainter p(&pm);

  pm.fill(QColor(50, 50, 100));
  p.setPen(QColor(0, 220, 0));
  if ((ch1 < 0) || (ch2 < 0)) {
    max = (ch1 < 0) ? max2 : max1;
  } else {
    max = (max1 > max2) ? max1 : max2;
  }
  f1 = int(2.0 * f_min / (double)synthdata->rate * width());
  f2 = int(2.0 * f_max / (double)synthdata->rate * width());
  if (!freqZoom) {
    p.setPen(QColor(255, 255, 0));
    p.drawLine(f1, 0, f1, height());
    p.drawLine(f2, 0, f2, height());
    p.setPen(QColor(0, 220, 0));
    f1 = 0;
    f2 = width();
  }
  frame1 = int((double)f1 / width() * (double)fftFrames / 2.0);
  frame2 = int((double)f2 / width() * (double)fftFrames / 2.0);
  if (viewMode == VIEW_MODE_FIF) {
    xscale = (double)width() / (double)(frame2 - frame1);
    yscale = - 0.9 * zoom * (double)height() / max;
    ych1Max = 0;
    ych2Max = 0;
    fch1Max = 0;
    fch2Max = 0;
    xch1Max = 0;
    xch2Max = 0;
    for (l1 = frame1; l1 < frame2 - 1; l1++) {
      x1 = int((double)(l1 - frame1) * xscale);
      x2 = int((double)(l1 + 1 - frame1) * xscale);
      switch (mode) {
      case SPECTRUM_MODE_NORMAL:
        y1ch1 = int(yscale * spectrumbuf_ch1[l1]);
        y1ch2 = int(yscale * spectrumbuf_ch2[l1]);
        break;    
      case SPECTRUM_MODE_SUM:
        y1ch1 = int(yscale * spectrumbuf_sum[l1]);
        break;
      }
      switch (mode) {  
      case SPECTRUM_MODE_NORMAL: 
        y2ch1 = int(yscale * spectrumbuf_ch1[l1 + 1]);
        y2ch2 = int(yscale * spectrumbuf_ch2[l1 + 1]);
        if (ch1 >= 0) {
          p.setPen(QColor(0, 220, 0));
          p.drawLine(x1, height() + y1ch1, x2, height() + y2ch1);
        }
        if (ch2 >= 0) { 
          p.setPen(QColor(255, 255, 0));
          p.drawLine(x1, height() + y1ch2, x2, height() + y2ch2);
        }
        break;      
      case SPECTRUM_MODE_SUM:
        y2ch1 = int(yscale * spectrumbuf_sum[l1 + 1]);
        if ((ch1 >= 0) && (ch2 >= 0)) {
          p.setPen(QColor(0, 220, 0));
          p.drawLine(x1, height() + y1ch1, x2, height() + y2ch1);
        }
        break;
      }
      if (-y2ch1 > ych1Max) {
        ych1Max = -y2ch1;
        fch1Max = (double)(l1 + 1) / (double)fftFrames * (double)synthdata->rate;
        xch1Max = x2;
      }
      if (-y2ch2 > ych2Max) {
        ych2Max = -y2ch2;
        fch2Max = (double)(l1 + 1) / (double)fftFrames * (double)synthdata->rate;
        xch2Max = x2;
      }
    }
    if ((ch1 >= 0) && (fch1Max > 0)) {
      p.setPen(QColor(0, 220, 0));
      p.drawText(xch1Max, height() - ych1Max, QString::number(fch1Max));
    }
    if ((ch2 >= 0) && (fch2Max > 0)) {
      p.setPen(QColor(255, 255, 0));
      p.drawText(xch2Max, height() - ych2Max, QString::number(fch2Max));
    }
    bitBlt(this, 0, 0, &pm);
  } else {
    bitBlt(fftPixmap1, 0, -1, fftPixmap1);
    bitBlt(fftPixmap2, 0, -1, fftPixmap2);
    QPainter p1(fftPixmap1); 
    QPainter p2(fftPixmap2);
    yscale = 764.0 / max;
    xscale1 = (double)fftPixmap1->width() / (double)(frame2 - frame1);
    xscale2 = (double)fftPixmap2->width() / (double)(frame2 - frame1);
    switch (mode) {
    case SPECTRUM_MODE_NORMAL:
      for (l1 = 0; l1 < fftPixmap1->width(); l1++) {
        x1 = int((double)l1 / xscale1) + frame1;
        x2 = int((double)(l1 + 1) / xscale1) + frame1;
        y1ch1 = 0;
        y1ch2 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * spectrumbuf_ch1[l2]);
            if (tmp > y1ch1) y1ch1 = tmp;
            tmp = int(0.9 * zoom * yscale * spectrumbuf_ch2[l2]);
            if (tmp > y1ch2) y1ch2 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * spectrumbuf_ch1[x1]);
          y1ch2 = int(0.9 * zoom * yscale * spectrumbuf_ch2[x1]);
        }
        if (y1ch1 < 0) y1ch1 = 0;
        if (y1ch2 < 0) y1ch2 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        if (y1ch2 > 764) y1ch2 = 764;
        r = palette1[y1ch1].red() + palette2[y1ch2].red();
        g = palette1[y1ch1].green() + palette2[y1ch2].green();
        b = palette1[y1ch1].blue() + palette2[y1ch2].blue();
        if (r > 255) r = 255; 
        if (g > 255) g = 255; 
        if (b > 255) b = 255; 
        p1.setPen(QColor(r, g, b));
        p1.drawPoint(l1, fftPixmap1->height() - 1);
      }
      for (l1 = 0; l1 < fftPixmap2->width(); l1++) {
        x1 = int((double)l1 / xscale2) + frame1;
        x2 = int((double)(l1 + 1) / xscale2) + frame1;
        y1ch1 = 0;
        y1ch2 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * spectrumbuf_ch1[l2]);
            if (tmp > y1ch1) y1ch1 = tmp;
            tmp = int(0.9 * zoom * yscale * spectrumbuf_ch2[l2]);
            if (tmp > y1ch2) y1ch2 = tmp;   
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * spectrumbuf_ch1[x1]);
          y1ch2 = int(0.9 * zoom * yscale * spectrumbuf_ch2[x1]);
        }
        if (y1ch1 < 0) y1ch1 = 0;
        if (y1ch2 < 0) y1ch2 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        if (y1ch2 > 764) y1ch2 = 764;
        r = palette1[y1ch1].red() + palette2[y1ch2].red();
        g = palette1[y1ch1].green() + palette2[y1ch2].green();
        b = palette1[y1ch1].blue() + palette2[y1ch2].blue();
        if (r > 255) r = 255; 
        if (g > 255) g = 255; 
        if (b > 255) b = 255; 
        p2.setPen(QColor(r, g, b));
        p2.drawPoint(l1, fftPixmap2->height() - 1);
      }
      break;
    case SPECTRUM_MODE_SUM:
      yscale = 764.0 / maxsum;
      for (l1 = 0; l1 < fftPixmap1->width(); l1++) {
        x1 = int((double)l1 / xscale1) + frame1;
        x2 = int((double)(l1 + 1) / xscale1) + frame1;
        y1ch1 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * spectrumbuf_sum[l2]);
            if (tmp > y1ch1) y1ch1 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * spectrumbuf_sum[x1]);
        }
        if ((ch1 < 0) || (ch2 < 0)) y1ch1 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        p1.setPen(palette1[y1ch1]);
        p1.drawPoint(l1, fftPixmap1->height() - 1);
      }
      for (l1 = 0; l1 < fftPixmap2->width(); l1++) {
        x1 = int((double)l1 / xscale2) + frame1;
        x2 = int((double)(l1 + 1) / xscale2) + frame1;
        y1ch1 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * spectrumbuf_sum[l2]);
            if (tmp > y1ch1) y1ch1 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * spectrumbuf_sum[x1]);
        }
        if ((ch1 < 0) || (ch2 < 0)) y1ch1 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        p2.setPen(palette1[y1ch1]);
        p2.drawPoint(l1, fftPixmap2->height() - 1);
      }
      break;
    }
    bitBlt(this, 0, 0, fftPixmap1);
    if (!freqZoom) {
      f1 = int(2.0 * f_min / (double)synthdata->rate * width());
      f2 = int(2.0 * f_max / (double)synthdata->rate * width());
      p.setPen(QColor(255, 255, 0));
      p.drawLine(f1, 0, f1, height());
      p.drawLine(f2, 0, f2, height());
    }
  }
}

inline float sqr(float x) {
  return(x*x);
}

void SpectrumScreen::refreshSpectrum() {

  int l1, ofs, n;
  float s1, s2;
  fftw_real *tmpbuf_ch1, *tmpbuf_ch2;
  double w;

  tmpbuf_ch1 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  tmpbuf_ch2 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  readofs = writeofs - synthdata->cyclesize - fftFrames;
  if (readofs < 0 ) {  
    readofs += SPECTRUM_BUFSIZE >> 1;
  }
  ofs = readofs;
  if (window == WINDOW_RECT) {
    for (l1 = 0; l1 < fftFrames; l1++) {
      s1 = spectrumdata[2 * ofs + ch1];
      s2 = spectrumdata[2 * ofs + ch2];
      spectrumbuf_ch1[l1] = s1;
      spectrumbuf_ch2[l1] = s2; 
      ofs++;
      if (ofs >= SPECTRUM_BUFSIZE >> 1) {
        ofs -= SPECTRUM_BUFSIZE >> 1;
      }
    }
  } else {
    for (l1 = 0; l1 < fftFrames; l1++) {
      switch (window) {
      case WINDOW_BARTLETT:
        w = 1.0 - fabs(double(l1 - fftFrames>>1)/double(fftFrames>>1));
        break;
      case WINDOW_HANN:
         w = 0.5 - 0.5 * cos(PI * (double)l1 /  double(fftFrames>>1));
        break; 
      case WINDOW_WELCH:
         w = 1.0 - sqr(double(l1 - fftFrames>>1)/double(fftFrames>>1));
        break; 
      default:
        w = 1;
        break;
      }
      s1 = spectrumdata[2 * ofs + ch1];
      s2 = spectrumdata[2 * ofs + ch2];
      spectrumbuf_ch1[l1] = int((double)s1 * w);
      spectrumbuf_ch2[l1] = int((double)s2 * w); 
      ofs++;
      if (ofs >= SPECTRUM_BUFSIZE >> 1) {
        ofs -= SPECTRUM_BUFSIZE >> 1;
      }
    }
  }
  rfftw_one(plan, spectrumbuf_ch1, tmpbuf_ch1);
  rfftw_one(plan, spectrumbuf_ch2, tmpbuf_ch2);
  if (normMode == NORM_MODE_EACH) {
    max1 = 0;
    max2 = 0;
    maxsum = 0;
  }
  n = fftFrames;
  switch (fftMode) {
  case FFT_MODE_POW:
    for (l1 = 1; l1 < (n+1)/2 - 1; l1++) {
      spectrumbuf_ch1[l1] = sqr(tmpbuf_ch1[l1]) + sqr(tmpbuf_ch1[n - l1]);
      if ((spectrumbuf_ch1[l1] > max1) && (normMode != NORM_MODE_FIXED)) {
        max1 = spectrumbuf_ch1[l1];
      }
      spectrumbuf_ch2[l1] = sqr(tmpbuf_ch2[l1]) + sqr(tmpbuf_ch2[n - l1]);
      spectrumbuf_sum[l1] = spectrumbuf_ch1[l1] + spectrumbuf_ch2[l1];
      if ((spectrumbuf_ch2[l1] > max2) && (normMode != NORM_MODE_FIXED)) {
        max2 = spectrumbuf_ch2[l1];
      }
      if ((spectrumbuf_sum[l1] > maxsum) && (normMode != NORM_MODE_FIXED)) {
        maxsum = spectrumbuf_sum[l1];
      }
    }
    break;
  case FFT_MODE_ABS:
    for (l1 = 1; l1 < (n+1)/2 - 1; l1++) {
      spectrumbuf_ch1[l1] = sqrt(sqr(tmpbuf_ch1[l1]) + sqr(tmpbuf_ch1[n - l1]));
      if ((spectrumbuf_ch1[l1] > max1) && (normMode != NORM_MODE_FIXED)) {
        max1 = spectrumbuf_ch1[l1];
      }
      spectrumbuf_ch2[l1] = sqrt(sqr(tmpbuf_ch2[l1]) + sqr(tmpbuf_ch2[n - l1]));
      spectrumbuf_sum[l1] = sqrt(sqr(tmpbuf_ch1[l1]) + sqr(tmpbuf_ch1[n - l1])
                                +sqr(tmpbuf_ch2[l1]) + sqr(tmpbuf_ch2[n - l1]));
      if ((spectrumbuf_ch2[l1] > max2) && (normMode != NORM_MODE_FIXED)) {
        max2 = spectrumbuf_ch2[l1];
      }
      if ((spectrumbuf_sum[l1] > maxsum) && (normMode != NORM_MODE_FIXED)) {
        maxsum = spectrumbuf_sum[l1];
      }
    }  
    break;
  case FFT_MODE_LOG:
    for (l1 = 1; l1 < (n+1)/2 - 1; l1++) {
      spectrumbuf_ch1[l1] = log(sqr(tmpbuf_ch1[l1]) + sqr(tmpbuf_ch1[n - l1]));
      if ((spectrumbuf_ch1[l1] > max1) && (normMode != NORM_MODE_FIXED)) {
        max1 = spectrumbuf_ch1[l1];
      }
      spectrumbuf_ch2[l1] = log(sqr(tmpbuf_ch2[l1]) + sqr(tmpbuf_ch2[n - l1]));
      spectrumbuf_sum[l1] = log(sqr(tmpbuf_ch1[l1]) + sqr(tmpbuf_ch1[n - l1])   
                               +sqr(tmpbuf_ch2[l1]) + sqr(tmpbuf_ch2[n - l1]));
      if ((spectrumbuf_ch2[l1] > max2) && (normMode != NORM_MODE_FIXED)) {
        max2 = spectrumbuf_ch2[l1];
      }
      if ((spectrumbuf_sum[l1] > maxsum) && (normMode != NORM_MODE_FIXED)) {
        maxsum = spectrumbuf_sum[l1];
      }
    }
    break;
  }
  free(tmpbuf_ch1);
  free(tmpbuf_ch2);
  repaint(false);
}

void SpectrumScreen::singleShot() {
 
  refreshSpectrum();
}

QSize SpectrumScreen::sizeHint() const {

  return QSize(MINIMUM_WIDTH, MINIMUM_HEIGHT); 
}

QSizePolicy SpectrumScreen::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

spectrumModeType SpectrumScreen::setMode(spectrumModeType p_mode) {

  mode = p_mode;
  repaint(false);
  return(mode);
}

fftModeType SpectrumScreen::setFFTMode(fftModeType p_fftMode) {
 
  fftMode = p_fftMode;
  max1 = 0;
  max2 = 0;
  maxsum = 0; 
  return(fftMode);
}

viewModeType SpectrumScreen::setViewMode(viewModeType p_viewMode) {
 
  viewMode = p_viewMode;
  return(viewMode);
}

normModeType SpectrumScreen::setNormMode(normModeType p_normMode) {
 
  normMode = p_normMode;
  return(normMode);
}

spectrumTriggerModeType SpectrumScreen::setTriggerMode(spectrumTriggerModeType p_triggerMode) {

  triggerMode = p_triggerMode;
  return(triggerMode);
}

int SpectrumScreen::setCh1(int p_ch1) {

  ch1 = p_ch1;
  repaint(false);
  return(ch1);
}

int SpectrumScreen::setCh2(int p_ch2) {       

  ch2 = p_ch2;  
  repaint(false);
  return(ch2); 
}              

windowType SpectrumScreen::setWindow(windowType p_window) {
 
  window = p_window;
  return(window); 
}

double SpectrumScreen::setZoom(double p_zoom) {

  zoom = p_zoom;
  repaint(false);
  return(zoom);
}

bool SpectrumScreen::toggleFreqZoom(bool p_freqZoom) {

  freqZoom = p_freqZoom;
  return(freqZoom);
}

float SpectrumScreen::set_f_min(float p_f_min) {

  f_min = p_f_min;
  return(f_min);
}

float SpectrumScreen::set_f_max(float p_f_max) {

  f_max = p_f_max;
  return(f_max);
}

int SpectrumScreen::setFFTFrames(int p_fftFrames) {

  fftFrames = p_fftFrames;
  rfftw_destroy_plan(plan);
  plan = rfftw_create_plan(fftFrames, FFTW_REAL_TO_COMPLEX, FFTW_IN_PLACE);
  return(fftFrames);
}

spectrumModeType SpectrumScreen::getMode() {

  return(mode); 
}

fftModeType SpectrumScreen::getFFTMode() {    
 
  return(fftMode);
}
 

viewModeType SpectrumScreen::getViewMode() {
 
  return(viewMode);
}

normModeType SpectrumScreen::getNormMode() {
 
  return(normMode);
}

spectrumTriggerModeType SpectrumScreen::getTriggerMode() {

  return(triggerMode);
}
 
int SpectrumScreen::getCh1() {

  return(ch1);
}
 
int SpectrumScreen::getCh2() {

  return(ch2);
}               

windowType SpectrumScreen::getWindow() {
 
  return(window);   
}

double SpectrumScreen::get_f_min() {

  if (freqZoom) 
    return(f_min);
  else
    return(0);
}             

double SpectrumScreen::get_f_max() {

  if (freqZoom) 
    return(f_max);
  else 
    return((double)synthdata->rate / 2.0);
}             

double SpectrumScreen::get_f() {

  return(f);
}
              
double SpectrumScreen::getZoom() {

  return(zoom); 
}

int SpectrumScreen::getFFTFrames() {
 
  return(fftFrames);
}

void SpectrumScreen::resizeEvent (QResizeEvent* )
{
  fftPixmap1->resize(width(), height());
  QPainter p(fftPixmap1); 
  p.setWindow(0, 0, fftPixmap2->width(), fftPixmap2->height());
  p.drawPixmap(0, 0, *fftPixmap2);
  repaint(false);
}

void SpectrumScreen::initPalette (int index) {             
  int l1;
  
  switch (index == 0) {
  case 0:
    for (l1 = 0; l1 < 256; l1++) {
      palette1[l1].setRgb(l1, 0, 0);
    }
    for (l1 = 1; l1 < 256; l1++) {
      palette1[l1+255].setRgb(255, l1, 0);
    } 
    for (l1 = 1; l1 < 256; l1++) {
      palette1[l1+510].setRgb(255, 255, l1);
    } 
    break;
  case 1:
    for (l1 = 0; l1 < 256; l1++) {
      palette2[l1].setRgb(0, l1, 0);
    }
    for (l1 = 1; l1 < 256; l1++) {
      palette2[l1+255].setRgb(0, 255 - l1, l1);
    } 
    for (l1 = 1; l1 < 256; l1++) {
      palette2[l1+510].setRgb(l1, l1, 255);
    } 
    break;
  }
}

/*
void SpectrumScreen::mousePressEvent(QMouseEvent *ev) {
  
  double tmp;

  tmp = (double)ev->x() / (double)width() * (double)synthdata->rate / 2.0;
  switch (ev->button()) {
  case Qt::LeftButton: 
    if (!freqZoom && ((tmp < f_max) || (f_min < 0))) f_min = tmp;
    break;
  case Qt::RightButton: 
    if (!freqZoom && ((tmp > f_min) || (f_max < 0))) f_max = tmp;
    break;
  case Qt::MidButton: 
    freqZoom = !freqZoom;
    emit f_minmaxChanged();
    break;
  default:
    break;
  }
}

void SpectrumScreen::mouseMoveEvent(QMouseEvent *ev) {

  if (freqZoom) {
    f = (double)ev->x() / (double)width() * (f_max - f_min) + f_min;
  } else {
    f = (double)ev->x() / (double)width() * (double)synthdata->rate / 2.0;
  }
  if (f < 0) f = 0;
  emit freqChanged();
}  
*/