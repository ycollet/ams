#ifndef MIDISLIDER_H
#define MIDISLIDER_H

#include "midisliderbase.h"

/**
  *@author Matthias Nagorni
  */

class MidiSlider : public MidiSliderBase {
Q_OBJECT

public:
  bool isLog, isMaster;
  float *valueRef;
  float min, max, initial_min, initial_max;
  
public:
  MidiSlider(Module *parentModule, float minValue, float maxValue, float pageStep, float value, Qt::Orientation orientation,
             QWidget * parent, const QString &name, float *valueRef=0, bool p_isLog=false);

  virtual void setMidiValue(int value);
  virtual int getMidiValue();
  float getValue();
  
signals:
  void valueChanged(int value);
  void logModeChanged(bool on);
	
public slots:
  void updateValue(int);
  void setLogMode(bool on);
  void setNewMin();
  void setNewMax();
  void setNewMin(int p_min);
  void setNewMax(int p_max);
  void setInitialMinMax();
};

#endif
