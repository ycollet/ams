#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include "envelope.h"


Envelope::Envelope(FloatParameter *p_delay,
                   FloatParameter *p_attack,
                   FloatParameter *p_hold,
                   FloatParameter *p_decay,
                   FloatParameter *p_sustain,
                   FloatParameter *p_release,
                   QWidget* parent, const char *name, SynthData *p_synthdata)
: QWidget (parent, name)
{
  synthdata = p_synthdata;
  delay = p_delay;
  attack = p_attack;
  hold = p_hold;
  decay = p_decay;
  sustain = p_sustain;
  release = p_release;
  connect(delay,SIGNAL(valueChanged(float)), this,SLOT(updateEnvelope(float)));
  connect(attack,SIGNAL(valueChanged(float)), this,SLOT(updateEnvelope(float)));
  connect(hold,SIGNAL(valueChanged(float)), this,SLOT(updateEnvelope(float)));
  connect(decay,SIGNAL(valueChanged(float)), this,SLOT(updateEnvelope(float)));
  connect(sustain,SIGNAL(valueChanged(float)), this,SLOT(updateEnvelope(float)));
  connect(release,SIGNAL(valueChanged(float)), this,SLOT(updateEnvelope(float)));
  setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
  setMinimumHeight(140);
}

Envelope::~Envelope()
{
}

void Envelope::paintEvent(QPaintEvent *) {
   const float * delayRef=delay->valuePtr();
   const float * attackRef=attack->valuePtr();
   const float * holdRef=hold->valuePtr();
   const float * decayRef=decay->valuePtr();
   const float * sustainRef=sustain->valuePtr();
   const float * releaseRef=release->valuePtr();
  QPixmap pm(width(), height());
  QPainter p(&pm);
  QPointArray points(7);
  QPen pen;
  float len, x, y, xscale, yscale;

  pm.fill(QColor(10, 50, 10));
  p.setViewport(0, 0, width(), height());
  p.setWindow(0, 0, width(), height());
  len = *delayRef + *attackRef + *holdRef + *decayRef + SUSTAIN_LEN + *releaseRef;
  xscale = (float)width() / len;
  yscale = (float)(height()-6);
  x = *delayRef * xscale;
  points.setPoint(0, (int)x, height());
  x += *attackRef * xscale;
  points.setPoint(1, (int)x, 6);
  x += *holdRef * xscale;
  points.setPoint(2, (int)x, 6);
  x += *decayRef * xscale;
  y = *sustainRef * yscale;
  points.setPoint(3, (int)x, height() - (int)y);
  x += SUSTAIN_LEN * xscale;
  points.setPoint(4, (int)x, height() - (int)y);
  x += *releaseRef * xscale;
  points.setPoint(5, (int)x, height() - 1);
  x = *delayRef * xscale;
  points.setPoint(6, (int)x, height() - 1);
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
  bitBlt(this, 0, 0, &pm);
}

void Envelope::updateEnvelope(float value) {

  repaint(false);
}

QSize Envelope::sizeHint() const {

  return QSize(ENVELOPE_MINIMUM_WIDTH, ENVELOPE_MINIMUM_HEIGHT);
}

QSizePolicy Envelope::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void Envelope::resizeEvent (QResizeEvent* )
{
  repaint(true);
}
