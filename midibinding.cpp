
#include "midibinding.h"


/*! @file mmidibinding.cpp
 *
 *  @brief
 *  Implementation of class MidiBinding
 *  @author Lukas Degener
 */

MidiBinding::MidiBinding(Parameter * parent, MidiController * controller, 
                         bool autodelete, const char * name)
:QObject(parent, name){
  mController = controller;
  mAutoDelete=autodelete;
  mOnThreshold=127;
  mOffThreshold=0;
  mMapType = defaultMapType();
  controllerOK=true;
  setController (controller);
  mSign=1;
}



MidiBinding::~MidiBinding(){
   setController(NULL);
}

void MidiBinding::setController(MidiController * ctrl){
   //disconnect from the old controller

   if(mController){
      disconnect(mController,SIGNAL(midiValueChanged(int)),this,SLOT(setMidiValue(int)));
      disconnect(mController,SIGNAL(destroyed()),this,SLOT(controllerDestroyed()));
      mController->removeBinding(this);
   }
   mController=ctrl;
   if(mController){
      connect(mController,SIGNAL(midiValueChanged(int)),this,SLOT(setMidiValue(int)));
      connect(mController,SIGNAL(destroyed()),this,SLOT(controllerDestroyed()));
      mController->addBinding(this);
   }
}

MidiController * MidiBinding::controller(){
   return mController;
}

void MidiBinding::setMidiSign(int sign){
   if (sign==0||sign*mSign>0) return;
   mSign = sign/abs(sign);
   emit midiSignChanged(mSign);
}
void MidiBinding::setMidiSign(bool negative){
  setMidiSign(negative ? -1 : 1);
}

void MidiBinding::toggleMidiSign(){
   mSign*=-1;
   emit midiSignChanged(mSign);
}

int MidiBinding::midiSign(){
   return mSign;
}

void MidiBinding::setMapType(CtrlMapType maptype){
   if(maptype==mMapType) return;

   if(mapTypes()&maptype) mMapType=maptype;
   else mMapType = defaultMapType();
   emit mapTypeChanged(mMapType);
   emit mapTypeChanged(mapTypeName(mMapType));
}

QString MidiBinding::mapTypeName(CtrlMapType maptype){
  switch(maptype){
    case LinearMap: return "LinearMap";
    case LogarithmicMap: return "LogarithmicMap";
    case ModuloMap: return "ModuloMap";
    case OnToggleMap: return "OnToggleMap";
    case OnOffToggleMap: return "OnOffToggleMap";
    case OnCycleMap: return "OnCycleMap";
    case OnOffCycleMap: return "OnOffCycleMap";
    case None: return "None";
  }
}

void MidiBinding::setMapType(const QString & maptypename){
   if(maptypename=="LinearMap") setMapType(LinearMap);
   else if(maptypename=="LogarithmicMap") setMapType(LogarithmicMap);
   else if(maptypename=="ModuloMap") setMapType(ModuloMap);
   else if(maptypename=="OnToggleMap") setMapType(OnToggleMap);
   else if(maptypename=="OnOffToggleMap") setMapType(OnOffToggleMap);
   else if(maptypename=="OnCycleMap") setMapType(OnCycleMap);
   else if(maptypename=="OnOffCycleMap") setMapType(OnOffCycleMap);
}

MidiBinding::CtrlMapType MidiBinding::mapType(){
   return mMapType;
}
MidiBinding::CtrlMapType MidiBinding::defaultMapType(){
   return None;
}
QStringList MidiBinding::mapTypeNames(){
   QStringList qsl;
   int valid=mapTypes();
   if(valid & LinearMap) qsl.append("LinearMap");
   if(valid & LogarithmicMap) qsl.append("LogarithmicMap");
   if(valid & ModuloMap) qsl.append("ModuloMap");
   if(valid & OnToggleMap) qsl.append("OnToggleMap");
   if(valid & OnOffToggleMap) qsl.append("OnOffToggleMap");
   if(valid & OnCycleMap) qsl.append("OnCycleMap");
   if(valid & OnOffCycleMap) qsl.append("OnOffCycleMap");

   return qsl;
}

void MidiBinding::controllerDestroyed(){
   if (mAutoDelete) deleteLater();
}
