#include <qhbox.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <stdio.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "midiguicomponent.h"
#include "midiwidget.h"

MidiGUIcomponent::MidiGUIcomponent(QObject *p_parentModule, SynthData *p_synthdata, QWidget * parent, const char * name)
           : QHBox(parent, name) {

  parentModule = p_parentModule;
  synthdata = p_synthdata;
  controllerOK = false;
  midiSign = 1;
  listViewItem = NULL;
  QObject::connect(this, SIGNAL(guiComponentTouched()),
                   (MidiWidget *)synthdata->midiWidget, SLOT(updateGuiComponent())); 
}

MidiGUIcomponent::~MidiGUIcomponent() {
}

void MidiGUIcomponent::connectToController(MidiController *p_midiController) {

  if (!midiControllerList.contains(p_midiController)) {
    QObject::connect(p_midiController, SIGNAL(midiValueChanged(int)),
                     this, SLOT(midiValueChanged(int)));  
    midiControllerList.append(p_midiController);
    ((MidiWidget *)synthdata->midiWidget)->addMidiGuiComponent(p_midiController, (QObject *)this);
    controllerOK = false;
  }
}
   
void MidiGUIcomponent::disconnectController(MidiController *p_midiController) {

  QObject::disconnect(p_midiController, SIGNAL(midiValueChanged(int)),
                      this, SLOT(midiValueChanged(int)));
  midiControllerList.remove(p_midiController);
}  

void MidiGUIcomponent::connectToController() {

  MidiController *midiController;
  
  if ((midiController = ((MidiWidget *)synthdata->midiWidget)->getSelectedController())) {
    if (!midiControllerList.contains(midiController)) {
      QObject::connect(midiController, SIGNAL(midiValueChanged(int)),
                       this, SLOT(midiValueChanged(int)));
      midiControllerList.append(midiController);
      ((MidiWidget *)synthdata->midiWidget)->addMidiGuiComponent(midiController, (QObject *)this);
      controllerOK = false;
    }
  }
} 

void MidiGUIcomponent::disconnectController() {
   
  if (((MidiWidget *)synthdata->midiWidget)->getSelectedController()) {
    QObject::disconnect(((MidiWidget *)synthdata->midiWidget)->getSelectedController(), SIGNAL(midiValueChanged(int)),
                       this, SLOT(midiValueChanged(int)));
    midiControllerList.remove(((MidiWidget *)synthdata->midiWidget)->getSelectedController());
  }
}

void MidiGUIcomponent::disconnectController(int index) {

  if (midiControllerList.count()) {
    QObject::disconnect(midiControllerList.at(index), SIGNAL(midiValueChanged(int)),
                       this, SLOT(midiValueChanged(int)));
    ((MidiWidget *)synthdata->midiWidget)->deleteMidiGuiComponent(midiControllerList.at(index), (QObject *)this);
    midiControllerList.remove(midiControllerList.at(index));
  }
}

void MidiGUIcomponent::setMidiValue(int value) {

}

int MidiGUIcomponent::getMidiValue() {

  return(0);
}

void MidiGUIcomponent::midiValueChanged(int value) {

  setMidiValue(value);
}

