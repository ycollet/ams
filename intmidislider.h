#ifndef INTMIDISLIDER_H
#define INTMIDISLIDER_H

#include "midisliderbase.h"

/**
  *@author Matthias Nagorni
  */

class IntMidiSlider : public MidiSliderBase {
Q_OBJECT

public:
  int *valueRef;
     
public:
  IntMidiSlider(Module *parentModule, int minValue, int maxValue, int pageStep, int value, Qt::Orientation orientation,
		QWidget *parent, const QString &name, int *valueRef=0);

  virtual void setMidiValue(int value);
  virtual int getMidiValue();

signals:
  void valueChanged(int value);
	
public slots:
  void updateValue(int);
};

#endif
