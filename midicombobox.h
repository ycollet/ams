#ifndef MIDICOMBOBOX_H
#define MIDICOMBOBOX_H

#include <qcombobox.h>
#include <qlabel.h>
#include "synthdata.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

class MidiComboBox : public MidiGUIcomponent {

Q_OBJECT

public:
  QComboBox *comboBox;
  QLabel *valueLabel;
  int *valueRef;
      
public:
  MidiComboBox(Module *parentModule, int value, QWidget * parent, const QString &, 
               int *p_valueRef=0, QStringList *itemNames = NULL);
  ~MidiComboBox();
  virtual void setMidiValue(int value);
  virtual int getMidiValue();
      
public slots:
  void updateValue(int);
};

#endif
