#include <qobject.h>
#include <qstring.h>

#include "actionmidibinding.h"
#include "actionparameter.h"
#include "midicontroller.h"

/*! @file actionmidibinding.cpp
 *
 *  @brief
 *  Implementataion of class ActionMidiBinding
 *  @author Lukas Degener
 */


ActionMidiBinding::ActionMidiBinding(ActionParameter * param, MidiController * ctrl,bool autoDelete, const char * name )
:MidiBinding(param,ctrl,autoDelete,name){
   mParameter=param;
   controllerOK=false;
}

ActionMidiBinding::~ActionMidiBinding(){
}

int ActionMidiBinding::mapTypes(){
   return OnToggleMap|OnOffToggleMap;
}

MidiBinding::CtrlMapType ActionMidiBinding::defaultMapType(){
   return OnToggleMap;
}


void ActionMidiBinding::setMidiValue(int val){
   switch(mMapType){
      default:
      case OnOffToggleMap:
      if (val>=mOnThreshold){
         mParameter->trigger();
      }
      else if (val<=mOffThreshold){
         mParameter->trigger();
      }
      break;
      case OnToggleMap:
      if (val>=mOnThreshold){
         mParameter->trigger();
      }
      break;
  }
}
