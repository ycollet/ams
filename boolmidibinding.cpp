#include <qobject.h>
#include <qstring.h>

#include "boolmidibinding.h"
#include "boolparameter.h"
#include "midicontroller.h"

/*! @file boolmidibinding.cpp
 *
 *  @brief
 *  Implementation of class BoolMidiBinding
 *  @author Lukas Degener
 */


BoolMidiBinding::BoolMidiBinding(BoolParameter * param, MidiController * ctrl,bool autoDelete, const char * name )
:MidiBinding(param,ctrl,autoDelete,name){
   mParameter=param;
   controllerOK=true;
}


BoolMidiBinding::~BoolMidiBinding(){
}

int BoolMidiBinding::mapTypes(){
   return OnToggleMap|OnOffToggleMap;
}

MidiBinding::CtrlMapType BoolMidiBinding::defaultMapType(){
   return OnOffToggleMap;
}


void BoolMidiBinding::setMidiValue(int val){

   switch(mMapType){
      default:
      case OnOffToggleMap:
      if (val>=mOnThreshold){
         mParameter->setOn(mSign>0);
      }
      else if (val<=mOffThreshold){
         mParameter->setOn(mSign<0);
      }
      break;
      case OnToggleMap:
      if (val>=mOnThreshold){
         mParameter->setOn(!mParameter->isOn());
      }
      break;
  }
}
