#ifndef FLOATINTMIDISLIDER_H
#define FLOATINTMIDISLIDER_H

#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

#define SLIDER_SCALE 16384.0

class FloatIntMidiSlider : public MidiGUIcomponent {

Q_OBJECT

public:
  QSlider *slider;
  QLabel *valueLabel;
  float *valueRef;
     
public:
  FloatIntMidiSlider(QObject *parentModule, float minValue, float maxValue, int pageStep, float value, QSlider::Orientation orientation,
                     QWidget * parent, const char * name=0, SynthData *p_synthdata=0, float *valueRef=0);
  ~FloatIntMidiSlider();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();

signals:
  void valueChanged(int value);
	
public slots:
  void updateValue(int);
  void updateSlider(int);
};

#endif
