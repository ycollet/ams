#ifndef INTMIDISLIDER_H
#define INTMIDISLIDER_H

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

class IntMidiSlider : public MidiGUIcomponent {

Q_OBJECT

public:
  QSlider *slider;
  QLabel *valueLabel;
  int *valueRef;
     
public:
  IntMidiSlider(QObject *parentModule, int minValue, int maxValue, int pageStep, int value, QSlider::Orientation orientation,
             QWidget * parent, const char * name=0, SynthData *p_synthdata=0, int *valueRef=0);
  ~IntMidiSlider();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();

signals:
  void valueChanged(int value);
	
public slots:
  void updateValue(int);
  void updateSlider(int);
};

#endif
