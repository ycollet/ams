#include<qobject.h>
#include<qstring.h>

#include "actionparameter.h"
#include "parameter.h"

/*! @file actionparameter.cpp
 *
 *  @brief
 *  Implementation of class ActionParameter
 *  @author Lukas Degener
 */


ActionParameter::ActionParameter(QObject * parent, QString name, QString descr, bool p_enabled)
:Parameter(parent,name,descr){

   mEnabled=p_enabled;
}


ActionParameter::~ActionParameter(){
}

void ActionParameter::setEnabled(bool val){
   mEnabled=val;
   emit stateChanged(mEnabled);
}

bool ActionParameter::enabled(){
   return mEnabled;
}

void ActionParameter::trigger(){
   if(mEnabled) emit triggered();
}

