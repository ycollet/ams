#include <alsa/asoundlib.h>

#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "midibinding.h"

MidiController * MidiController::instance(MidiControlCenter * center, int p_type, int p_ch, int p_param){
   MidiControllerList * list = center->controllers();
   //If the requested controller is already registered, return
   //a pointer to it, even if it is a note controller.
   //If there are any bindings to an already registered note controller,
   //it has to be possible to send events through such a controller, even
   //if the controlcenter does not want any new note controllers

   for(MidiController * c = list->first();c;c=list->next()){

      if ((c->type == p_type)
        &&(c->ch == p_ch)
        &&(c->param == p_param)){

        return c;
      }
   }
   //don't create/register a new controller if it is a note controller and
   //the controlcenter does not want any new note controllers
   if(! center->noteEventsEnabled()){
      if ((p_type==SND_SEQ_EVENT_NOTEOFF)||(p_type==SND_SEQ_EVENT_NOTEON)){
         //cout<<"will not create any NOTE controllers right now"<<endl;
         return NULL;
      }
   }

   QString label;
   label.sprintf("type: %i, channel: %i, param: %i", p_type,p_ch,p_param);
   MidiController * ctrl = new MidiController(p_type, p_ch,p_param,center,label);
   //cout<<"temporary controller created: "<<ctrl->name()<<endl;
   //the control center might still reject the controller.
   if(!center->registerController(ctrl)){
     // cout<<"ControlCenter does not like it. deleted."<<endl;
      delete ctrl;
      return NULL;
   }
   //cout<<"controller registered: "<< ctrl->name()<<endl;
   return ctrl;
}

bool MidiController::sendMidiEvent(snd_seq_event_t *ev, MidiControlCenter *controlCenter){
   int type = ev->type;
   int ch = ev->data.control.channel;
   int param=0;
   int value=0;
   switch (type){
      case SND_SEQ_EVENT_PITCHBEND:
      value=(ev->data.control.value + 8192) / 128;
      break;
      case SND_SEQ_EVENT_CONTROL14:
      param=ev->data.control.param;
      value=ev->data.control.value / 128;
      break;
      case SND_SEQ_EVENT_CONTROLLER:
      param=ev->data.control.param;
      value=ev->data.control.value;
      break;
      case SND_SEQ_EVENT_NOTEON:
      param=ev->data.note.note;
      value=ev->data.note.velocity;
      break;
      case SND_SEQ_EVENT_NOTEOFF:
      param=ev->data.note.note;
      break;
      default:
      //unsupported eventtype
      /*! @todo do something here to handle
          unsupported event types (Aftertouch isn't handled yet, i think)
      */
      return false;
      break;
   }
   //try to get an instance of MidiController for the event
   MidiController * ctrl = MidiController::instance(controlCenter,type,ch,param);

   //this might be impossible for several reasons.
   if(! ctrl) return false;

   //now send the value und everything else should work ok
   ctrl->sendMidiValue(value);
   return true;
}


MidiController::MidiController(int p_type, int p_ch, int p_param, MidiControlCenter * parent, const char * name)
:QObject(parent,name){
  mControlCenter = parent;
  type = p_type;
  ch = p_ch;
  param = p_param;


}


MidiController::~MidiController(){
}



bool MidiController::operator==(const MidiController & orig){
return (type == orig.type
      ||ch == orig.ch
      ||param == orig.param);

}

void MidiController::sendMidiValue(int value) {
   emit midiValueChanged(value);
   if(mControlCenter->followMidiEnabled()
     &&(mControlCenter->selectedController()!=this)){
         mControlCenter->selectController(this);
   }
}


void MidiController::addBinding(MidiBinding * binding){
   emit bindingAdded(this,binding);
}
void MidiController::removeBinding(MidiBinding * binding){
   emit bindingRemoved(this,binding);
}

