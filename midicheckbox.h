#ifndef MIDICHECKBOX_H
#define MIDICHECKBOX_H

#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

class MidiCheckBox : public MidiGUIcomponent {

Q_OBJECT

private:
  QLabel *valueLabel;
  float *valueRef;

public:
  QCheckBox *checkBox;
    
public:
  MidiCheckBox(QObject *parentModule, float value, QWidget * parent, const char * name=0, SynthData *p_synthdata=0, 
               float *p_valueRef=0);
  ~MidiCheckBox();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();
      
public slots:
  void updateValue(bool on);
  void updateCheck(bool on);
};

#endif
