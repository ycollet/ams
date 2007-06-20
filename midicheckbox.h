#ifndef MIDICHECKBOX_H
#define MIDICHECKBOX_H

#include <qcheckbox.h>
#include <qlabel.h>
#include "synthdata.h"
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
  MidiCheckBox(Module *parentModule, float value, QWidget * parent, const QString &name, 
               float *p_valueRef=0);
  ~MidiCheckBox();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();
      
public slots:
  void updateValue(bool on);
  void updateCheck(bool on);
};

#endif
