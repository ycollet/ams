#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>


#include "parameter.h"
#include "floatparameter.h"

/*! @file floatparameter.cpp
 *
 *  @brief
 *  Implementation of class FloatParameter
 *  @author Lukas Degener
 */


FloatParameter::FloatParameter(QObject * parent, QString name, QString descr,
                               float p_min, float p_max, float p_value)
:Parameter(parent,name,descr),mValue (mLocalValue){

   mMin=mRealMin=p_min;
   mMax=mRealMax=p_max;
   mValue=p_value;
}

FloatParameter::FloatParameter(QObject * parent, QString name, QString descr,
                               float p_min, float p_max, float * p_ref)
:Parameter(parent,name,descr),mValue ( *p_ref){

   mMin=mRealMin=p_min;
   mMax=mRealMax=p_max;
   //mValue=p_value;
}


FloatParameter::~FloatParameter(){}


float FloatParameter::setValue(const float & val){
   if(val==mValue) return mValue; //avoid endless loops in case of cyclic connections
   mValue=min(mMax,val);
   mValue=max(mMin,mValue);
   emit valueChanged(mValue);
   return mValue;
}

float FloatParameter::setMinValue(const float & val){
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

float FloatParameter::setMaxValue(const float & val){
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

const float * FloatParameter::valuePtr(){return &mValue;}
FloatParameter::operator const float &(){return mValue;}
const float & FloatParameter::operator=(const float & orig){
   return setValue(orig);
}
float FloatParameter::value(){return mValue;}

float FloatParameter::minValue(){return mMin;}

float FloatParameter::maxValue(){return mMax;}

float FloatParameter::realMinValue(){return mRealMin;}

float FloatParameter::realMaxValue(){return mRealMax;}


