#ifndef MIDIGUICOMPONENT_H
#define MIDIGUICOMPONENT_H

#include <qpopupmenu.h>
#include <qlist.h>
#include <qhbox.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midicontrollerlist.h"

/**
  *@author Matthias Nagorni
  */

enum GUIcomponentType { GUIcomponentType_slider, GUIcomponentType_intslider, 
                        GUIcomponentType_checkbox, GUIcomponentType_combobox, 
                        GUIcomponentType_pushbutton };

class MidiGUIcomponent : public QHBox {

Q_OBJECT

protected:
  SynthData *synthdata;

public:
  QObject *parentModule; 
  QList<QListViewItem> listViewItemList;
  QListViewItem *listViewItem;
  int midiSign; 
  GUIcomponentType componentType;
  bool controllerOK;
        
public:
  MidiGUIcomponent(QObject *p_parentModule, SynthData *p_synthdata, QWidget * parent, const char * name=0);
  ~MidiGUIcomponent();
  MidiControllerList midiControllerList;
  void connectToController(MidiController *p_midiController);
  void disconnectController(MidiController *p_midiController);
  virtual void setMidiValue(int value);
  virtual int getMidiValue();

signals:
  void guiComponentTouched();
  
public slots:
  void midiValueChanged(int);
  void connectToController();
  void disconnectController();
  void disconnectController(int);
};
  
#endif
