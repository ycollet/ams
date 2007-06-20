#ifndef MIDIPUSHBUTTON_H
#define MIDIPUSHBUTTON_H

#include <qpushbutton.h>
#include <qlabel.h>
#include "synthdata.h"
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
  MidiPushButton(Module *parentModule, const QString &name);
  ~MidiPushButton();
  virtual void setMidiValue(int value);

signals: 
  void clicked();

public slots:
  void buttonClicked();

};  
#endif
