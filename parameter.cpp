#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>
#include <qlistview.h>


#include "parameter.h"
#include "midicontrolcenter.h"

/*! @file parameter.cpp
 *
 *  @brief
 *  Implementation of class Parameter
 *  @author Lukas Degener
 */
QDict<Parameter> Parameter::mInstances;
QMap<Parameter*,QString> Parameter::mNames;

void Parameter::registerInstance(Parameter * par){
   mInstances.replace(par->longName(),par);
   mNames.replace(par,par->longName());
}

void Parameter::unregisterInstance(Parameter * par){
   mInstances.remove(mNames[par]);
   mNames.remove(par);
}

Parameter * Parameter::instance(const QString & longName){
   return mInstances[longName];
}

Parameter::Parameter(QObject * parent, QString name, QString descr)
         :QObject(parent,name){

   mDescription=descr;
   registerInstance(this);
   MidiControlCenter::getInstance()->registerParameter(this);
}
Parameter::~Parameter(){
   unregisterInstance(this);
}
QString Parameter::longName(){
   QString qs(parent()->name());
   qs.append(":");
   qs.append(name());
   return qs;
}

QString Parameter::description(){return mDescription;}

void Parameter::setDescription(QString descr){mDescription=descr;}
void Parameter::touchedByGui(){
  MidiControlCenter *mcc=MidiControlCenter::getInstance();
  if(mcc->followConfigEnabled()) mcc->selectParameter(this);
}

