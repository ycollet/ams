#ifndef MIDIGUICOMPONENT_H
#define MIDIGUICOMPONENT_H

#include <QWidget>
#include <qlist.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midicontrollerlist.h"

/**
  *@author Matthias Nagorni
  */

enum GUIcomponentType { GUIcomponentType_slider, GUIcomponentType_intslider, 
                        GUIcomponentType_checkbox, GUIcomponentType_combobox, 
                        GUIcomponentType_pushbutton, GUIcomponentType_floatintslider };

class MidiGUIcomponent : public QWidget {
Q_OBJECT

public:
  Module *parentModule; 
  int midiGUIcomponentListIndex;
  int midiSign; 
  GUIcomponentType componentType;
  bool controllerOK;
        
public:
  MidiGUIcomponent(Module *p_parentModule, QWidget * parent, const QString &name);
  ~MidiGUIcomponent();
  QList<MidiControllerKey> midiControllerList;
  void connectToController(MidiControllerKey midiController);
  void disconnectController(MidiControllerKey midiController);
  virtual void setMidiValue(int value);
  virtual int getMidiValue();
  void invalidateController();

signals:
  void guiComponentTouched();
  void sigResetController();
  
public slots:
  void midiValueChanged(int);
  void resetControllerOK();
};
  
#endif
