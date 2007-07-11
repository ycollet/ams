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

private slots:
  void valueChanged(int value);

public:
  QComboBox *comboBox;
  QLabel *valueLabel;
      
public:
  MidiComboBox(class MidiControllableNames &mcAble);
  virtual MidiGUIcomponent *createTwin();
  ~MidiComboBox();

  void mcAbleChanged();
     
};

#endif
