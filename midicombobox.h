#ifndef MIDICOMBOBOX_H
#define MIDICOMBOBOX_H

#include <qcombobox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

class MidiComboBox : public MidiGUIcomponent {

Q_OBJECT

private:
  QLabel *valueLabel;
  int *valueRef;

public:
  QComboBox *comboBox;
  
public:
  MidiComboBox(QObject *parentModule, int value, QWidget * parent, const char * name=0, SynthData *p_synthdata=0, 
               int *p_valueRef=0, QStrList *itemNames=0);
  ~MidiComboBox();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();
      
public slots:
  void updateValue(int);
};

#endif
