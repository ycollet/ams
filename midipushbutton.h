#ifndef MIDIPUSHBUTTON_H
#define MIDIPUSHBUTTON_H

#include <qpushbutton.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

class MidiPushButton : public MidiGUIcomponent {

Q_OBJECT

private:

public:
  QPushButton *pushButton;
    
public:
  MidiPushButton(QObject *parentModule, QWidget * parent, const char * name=0, SynthData *p_synthdata=0);
  ~MidiPushButton();
  virtual void setMidiValue(int value);

signals: 
  void clicked();

public slots:
  void buttonClicked();

};  
#endif
