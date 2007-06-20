#ifndef FLOATINTMIDISLIDER_H
#define FLOATINTMIDISLIDER_H

#include "midisliderbase.h"

/**
  *@author Matthias Nagorni
  */

class FloatIntMidiSlider : public MidiSliderBase {
Q_OBJECT

public:
  float *valueRef;
     
public:
  FloatIntMidiSlider(Module *parentModule, float minValue, float maxValue, int pageStep, float value, Qt::Orientation orientation,
                     QWidget * parent, const QString &parameterName, float *valueRef=0);

  virtual void setMidiValue(int value);
  virtual int getMidiValue();

signals:
  void valueChanged(int value);
	
public slots:
  void updateValue(int);
};

#endif
