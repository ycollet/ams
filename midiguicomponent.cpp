#include <qcheckbox.h>
#include <qpushbutton.h>
#include <stdio.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"
#include "midiwidget.h"

MidiGUIcomponent::MidiGUIcomponent(Module *p_parentModule, QWidget * parent, const QString &name)
  : QWidget(parent)
{
  setObjectName(name);
  parentModule = p_parentModule;
  
  controllerOK = false;
  midiSign = 1;
  midiGUIcomponentListIndex = 0;
  QObject::connect(this, SIGNAL(guiComponentTouched()),
                   (MidiWidget *)synthdata->midiWidget, SLOT(updateGuiComponent())); 
}

MidiGUIcomponent::~MidiGUIcomponent() {
  while (midiControllerList.count())
    disconnectController(midiControllerList.at(0));
}

void MidiGUIcomponent::connectToController(MidiControllerKey midiController)
{
  if (!midiControllerList.contains(midiController)) {
    midiControllerList.append(midiController);
    synthdata->midiWidget->addMidiGuiComponent(midiController, this);
    controllerOK = false;
  }
}
   
void MidiGUIcomponent::disconnectController(MidiControllerKey midiController)
{
  midiControllerList.removeAll(midiController);
  synthdata->midiWidget->removeMidiGuiComponent(midiController, this);
}  

// void MidiGUIcomponent::connectToController() {

//   const MidiController *midiController;
  
//   if ((midiController = synthdata->midiWidget->getSelectedController())) {
//     if (!midiControllerList.contains(midiController)) {
//       QObject::connect(midiController->context, SIGNAL(midiValueChanged(int)),
//                        this, SLOT(midiValueChanged(int)));
//       midiController->context->mgcList.append(this);
//       midiControllerList.append(midiController);
//       ((MidiWidget *)synthdata->midiWidget)->addMidiGuiComponent(*midiController, (QObject *)this);
//       controllerOK = false;
//     }
//   }
// } 

// void MidiGUIcomponent::disconnectController()
// {
//   const MidiController *midiController = synthdata->midiWidget->getSelectedController();
//   if (midiController) {
//     QObject::disconnect(midiController->context, SIGNAL(midiValueChanged(int)),
//                        this, SLOT(midiValueChanged(int)));
//     midiController->context->mgcList.remove(this);
//     midiControllerList.remove(((MidiWidget *)synthdata->midiWidget)->getSelectedController());
//   }
// }

// void MidiGUIcomponent::disconnectController(int index) {

//   if (midiControllerList.count()) {
//     QObject::disconnect(midiControllerList.at(index)->context, SIGNAL(midiValueChanged(int)),
//                        this, SLOT(midiValueChanged(int)));
//     ((MidiWidget *)synthdata->midiWidget)->deleteMidiGuiComponent(*midiControllerList.at(index), (QObject *)this);
//     midiControllerList.at(index)->context->mgcList.remove(this);
//     midiControllerList.remove(midiControllerList.at(index));
//   }
// }

void MidiGUIcomponent::setMidiValue(int) {

}

int MidiGUIcomponent::getMidiValue() {

  return(0);
}

void MidiGUIcomponent::midiValueChanged(int value) {

  int type, ch, param;
  snd_seq_event_t ev;

//   if (!controllerOK) {
//     if ((synthdata->midiControllerMode == 1) && midiControllerList.count()) {
//       type = midiControllerList.at(0)->type();
//       if (type == SND_SEQ_EVENT_CONTROLLER) {
//         ch = midiControllerList.at(0)->ch();
//         param = midiControllerList.at(0)->param();
//         snd_seq_ev_clear(&ev);
//         snd_seq_ev_set_subs(&ev);
//         snd_seq_ev_set_direct(&ev);
//         ev.type = SND_SEQ_EVENT_CONTROLLER;
//         ev.data.control.channel = ch;
//         ev.data.control.param = param;
//         ev.data.control.value = getMidiValue();
//         snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
//         snd_seq_event_output_direct(synthdata->seq_handle, &ev);
// //        fprintf(stderr, "--> %d %d %d\n", type, ch, param);
//       }
//       if (synthdata->midiControllerMode > 0) {
//         controllerOK = true;
//       }  
//     }  
//   }  
  setMidiValue(value);
}

void MidiGUIcomponent::resetControllerOK() {

  controllerOK = false;
}

void MidiGUIcomponent::invalidateController() {

  emit sigResetController();
}
