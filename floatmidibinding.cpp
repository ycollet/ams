#include <qobject.h>
#include <qstring.h>

#include "midibinding.h"
#include "midicontroller.h"
#include "floatmidibinding.h"
#include "floatparameter.h"

/*! @file floatmidibinding.cpp
 *
 *  @brief
 *  Implementation of class FloatMidiBinding
 *  @author Lukas Degener
 */

FloatMidiBinding::FloatMidiBinding(FloatParameter * param, MidiController * ctrl, bool autoDelete, const char * name )
:MidiBinding(param,ctrl,autoDelete,name){
   mParameter=param;
   controllerOK=true;
}


FloatMidiBinding::~FloatMidiBinding(){
}




void FloatMidiBinding::setMidiValue(int val) {
//! @todo: implement LogarithmicMap

   switch(mMapType){
      default:
      case LinearMap:
      case LogarithmicMap:
      if (mSign == 1) {
         mParameter->setValue( mParameter->minValue() + (mParameter->maxValue() - mParameter->minValue()) * (float)val / 127.0);
      } else {
         mParameter->setValue( mParameter->maxValue() - (mParameter->maxValue() - mParameter->minValue()) * (float)val / 127.0);
      }
      break;

      case OnToggleMap:
      if (val>=mOnThreshold){
         if (mParameter->value()==mParameter->maxValue()){
            mParameter->setValue(mParameter->minValue());
         }
         else{
            mParameter->setValue(mParameter->maxValue());
         }
      }
      break;

      case OnOffToggleMap:
      if (val>=mOnThreshold && mSign>0){
         mParameter->setValue(mParameter->maxValue());
      }
      else if (val>=mOnThreshold && mSign<0){
         mParameter->setValue(mParameter->minValue());
      }
      else if (val<=mOffThreshold && mSign>0){
         mParameter->setValue(mParameter->minValue());
      }
      else if (val<=mOffThreshold && mSign<0){
         mParameter->setValue(mParameter->maxValue());
      }
      break;
  }
}

MidiBinding::CtrlMapType FloatMidiBinding::defaultMapType(){
   return LinearMap;
}

int FloatMidiBinding::mapTypes(){
   return LinearMap|LogarithmicMap|OnToggleMap|OnOffToggleMap;
}
