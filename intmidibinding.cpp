#include <qobject.h>
#include <qstring.h>

#include "midicontroller.h"

#include "intmidibinding.h"
#include "intparameter.h"

/*! @file intmidibinding.cpp
 *
 *  @brief
 *  Implementation of class IntMidiBinding
 *  @author Lukas Degener
 */

IntMidiBinding::IntMidiBinding(IntParameter * param, MidiController * ctrl, bool autoDelete, const char * name )
:MidiBinding(param,ctrl,autoDelete,name){
   mParameter=param;
   controllerOK=true;
}

IntMidiBinding::~IntMidiBinding(){
}




void IntMidiBinding::setMidiValue(int val) {
//! @todo implement LogarithmicMap

   switch(mMapType){
      default:
      case LinearMap:
      case LogarithmicMap:
      if (mSign == 1) {
         mParameter->setValue((int)((float)mParameter->minValue() + (float)(mParameter->maxValue() - mParameter->minValue()) * (float)val / 127.0));
      } else {
         mParameter->setValue( (int)((float)mParameter->maxValue() -(float)(mParameter->maxValue() - mParameter->minValue()) * (float)val / 127.0));
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

      case OnCycleMap:
      int v;
      if (val>=mOnThreshold){
         if (mSign>0){
           v = (mParameter->value()==mParameter->maxValue()) ? mParameter->minValue()
                                                                  : mParameter->value()+1;

         }
         else{
           v = (mParameter->value()==mParameter->minValue()) ? mParameter->maxValue()
                                                                  : mParameter->value()-1;

         }
         mParameter->setValue(v);
      }

      break;
  }
}

MidiBinding::CtrlMapType IntMidiBinding::defaultMapType(){
   return LinearMap;
}

int IntMidiBinding::mapTypes(){
   return LinearMap|LogarithmicMap|OnToggleMap|OnOffToggleMap|OnCycleMap;
}
