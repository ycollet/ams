#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>


#include "parameter.h"
#include "intparameter.h"



/*! @file intparameter.cpp
 *
 *  @brief
 *  Implementation of class IntParameter
 *  @author Lukas Degener
 */


IntParameter::IntParameter(QObject * parent, QString name, QString descr,
                               int p_min, int p_max, int p_value)
:Parameter(parent,name,descr),mValue(mLocalValue){

   mMin=mRealMin=p_min;
   mMax=mRealMax=p_max;
   mValue=p_value;
}
IntParameter::IntParameter(QObject * parent, QString name, QString descr,
                               int p_min, int p_max, int * p_ref)
:Parameter(parent,name,descr),mValue(*p_ref){

   mMin=mRealMin=p_min;
   mMax=mRealMax=p_max;
   
}


IntParameter::~IntParameter(){}


int IntParameter::setValue(const int & val){
   if(val==mValue) return mValue; //avoid endless loops in case of cyclic connections
   mValue=min(mMax,val);
   mValue=max(mMin,mValue);
   emit valueChanged(mValue);
   return mValue;
}

int IntParameter::setMinValue(const int & val){
   if(val==mMin) return mMin; //avoid endless loops in case of cyclic connections
   mMin=min(mMax,val);
   mMin=max(mRealMin,mMin);
   if(mValue<mMin){
      mValue=mMin;
      emit valueChanged(mValue);
   }
   emit minChanged(mMin);
   return mMin;
}

int IntParameter::setMaxValue(const int & val){
   if(val==mMax) return mMax; //avoid endless loops in case of cyclic connections
   mMax=min(mRealMax,val);
   mMax=max(mMax,mMin);
   if(mValue>mMax){
      mValue=mMax;
      emit valueChanged(mValue);
   }
   emit maxChanged(mMax);
   return mMax;
}

const int * IntParameter::valuePtr(){return &mValue;}
IntParameter::operator const int &(){
   return mValue;
}
const int & IntParameter::operator=(const int & orig){
   return setValue(orig);
}
int IntParameter::value(){return mValue;}

int IntParameter::minValue(){return mMin;}

int IntParameter::maxValue(){return mMax;}

int IntParameter::realMinValue(){return mRealMin;}

int IntParameter::realMaxValue(){return mRealMax;}

/*
void IntParameter::setMidiValue(int val) {

  if (!controllerOK) {
    controllerOK = abs(getMidiValue() - val) < 4;
  }
  if (controllerOK) {
    if (midiSign == 1) {
      setValue((int)( (float)mMin + (float)(mMax - mMin) * (float)val / 127.0));
    } else {
      setValue((int)( (float)mMax - (float)(mMax - mMin) * (float)val / 127.0));
    }
  }
}

int IntParameter::getMidiValue() {


   if(mMin==mMax) return 0;
   float x;
   x = (float)(mValue -  mMin) * 127.0 / (float)(mMax - mMin);
   return(int(rint(x)));
}
*/
