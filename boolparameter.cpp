#include<qobject.h>
#include<qstring.h>

#include "boolparameter.h"
#include "parameter.h"

/*! @file boolparameter.cpp
 *
 *  @brief
 *  Implementation of class BoolParameter
 *  @author Lukas Degener
 */


BoolParameter::BoolParameter(QObject * parent, QString name, QString descr, bool p_value)
 :Parameter(parent,name,descr),mValue(mLocalValue){

   mValue=p_value; // :-))))))
}

BoolParameter::BoolParameter(QObject * parent, QString name, QString descr, bool * p_ref)
:Parameter(parent,name,descr),mValue(*p_ref){

   //mValue=p_value; // :-))))))
}


BoolParameter::~BoolParameter(){
}

void BoolParameter::setOn(const bool & val){
   mValue=val;
   emit toggled(mValue);
}

const bool * BoolParameter::valuePtr(){
   return &mValue;
}

BoolParameter::operator const bool & (){
   return mValue;
};

const bool & BoolParameter::operator=(const bool & orig){
   setOn(orig);
   return mValue;
}

bool BoolParameter::isOn(){
   return mValue;
}

