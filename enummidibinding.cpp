#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>
#include <qvaluelist.h>
#include <qmap.h>
#include "enummidibinding.h"
#include "midicontroller.h"

/*! @file enummidibinding.cpp
 *
 *  @brief
 *  Implementation of class EnumMidiBinding
 *  @author Lukas Degener
 */


EnumMidiBinding::EnumMidiBinding(EnumParameter * param,MidiController * ctrl, bool autoDelete, const char * name)
:MidiBinding(param,ctrl,autoDelete,name){
   mParameter=param;
   connect(mParameter,SIGNAL(itemAdded(int)),this,SLOT(itemAdded(int)));
   connect(mParameter,SIGNAL(itemRemoved(int,QString)),this,SLOT(itemRemoved(int,QString)));
   connect(mParameter,SIGNAL(itemStateChanged(int)),this,SLOT(itemStateChanged(int)));
   controllerOK=true;
   mEnabledItems=mParameter->enabledItems();
}

EnumMidiBinding::~EnumMidiBinding(){
}


int EnumMidiBinding::mapTypes(){
   return ModuloMap|OnCycleMap;
}

MidiBinding::CtrlMapType EnumMidiBinding::defaultMapType(){
   return ModuloMap;
}


void EnumMidiBinding::itemAdded(int key){
   mEnabledItems=mParameter->enabledItems();
}

void EnumMidiBinding::itemRemoved(int key, QString name){
   mEnabledItems=mParameter->enabledItems();
}

void EnumMidiBinding::itemStateChanged(int key){
   mEnabledItems=mParameter->enabledItems();
}

void EnumMidiBinding::setMidiValue(int value){
   switch(mMapType){
      default:
      case ModuloMap:
      mParameter->selectItem(mEnabledItems[(mSign*value) % mEnabledItems.count()]);
      break;
      case OnCycleMap:
      if (value>=mOnThreshold){
         int current = mEnabledItems.findIndex(mParameter->selectedItem());
         if (mSign>0) current++;
         else current--;
         if (current<0)current=mEnabledItems.count()-1;
         if (current>=mEnabledItems.count()) current = 0;
         mParameter->selectItem(mEnabledItems[current]);
      }
      break;
   }
}
