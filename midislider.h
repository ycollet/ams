#ifndef MIDISLIDER_H
#define MIDISLIDER_H

#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

#define SLIDER_SCALE 16384.0

class MidiSlider : public MidiGUIcomponent {

Q_OBJECT

public:
  bool isLog, isMaster;
  QSlider *slider;
  QLabel *minLabel, *maxLabel, *valueLabel;
  float *valueRef;
  float min, max, initial_min, initial_max;
  
public:
  MidiSlider(QObject *parentModule, float minValue, float maxValue, float pageStep, float value, QSlider::Orientation orientation,
             QWidget * parent, const char * name=0, SynthData *p_synthdata=0, float *valueRef=0, bool p_isLog=false);
  ~MidiSlider();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();
  float getValue();
  
signals:
  void valueChanged(int value);
  void logModeChanged(bool on);
	
public slots:
  void updateValue(int);
  void updateSlider(int);
  void setLogMode(bool on);
  void setNewMin();
  void setNewMax();
  void setNewMin(int p_min);
  void setNewMax(int p_max);
  void setInitialMinMax();
};

#endif
