#include <qobject.h>
#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>
#include "enumparameter.h"

/*! @file enumparameter.cpp
 *
 *  @brief
 *  Implementation of class EnumParameter
 *  @author Lukas Degener
 */


EnumParameter::EnumParameter(QObject * parent, QString name, QString descr)
:Parameter(parent,name,descr),mCurrentKey(mLocalValue){
   //hmm? nothing to do here?? ahhh yes! initialize members.
   mCurrentKey=-1;//aka undefined.
}
EnumParameter::EnumParameter(QObject * parent, QString name, QString descr, int * p_ref)
:Parameter(parent,name,descr),mCurrentKey ( *p_ref){
   //hmm? nothing to do here?? ahhh yes! initialize members.
   mCurrentKey=-1;//aka undefined.
}

EnumParameter::~EnumParameter(){}

int EnumParameter::selectedItem(){return mCurrentKey;}
EnumParameter::operator const int &(){
   return mCurrentKey;
}
const int & EnumParameter::operator=(const int & orig){
   selectItem(orig);
   return mCurrentKey;
}


const int * EnumParameter::valuePtr(){return &mCurrentKey;}

int EnumParameter::itemKey(const QString& name){
   for(QMap<int,QString>::iterator it = mItemNames.begin();it!=mItemNames.end();++it){
      if(it.data() == name) return it.key();
   }
   return -1;
}

QString EnumParameter::itemName(const int key){
   if (! mItemNames.contains(key)) return "";
   return mItemNames[key];
}

bool EnumParameter::itemEnabled(const int key){
   if (! mItemStates.contains(key)) return false;
   return mItemStates[key];
}

QValueList<int> EnumParameter::enabledItems(){
   QValueList<int> vl;
   for(QMap<int,bool>::iterator it = mItemStates.begin();it!=mItemStates.end();++it){
      if(it.data()) vl.append(it.key());
   }
   return vl;
}

void EnumParameter::addItem(const int key, const QString& name, bool enabled=true){
   if(key==-1||mItemNames.contains(key)) return;
   mItemNames[key]=name;
   mItemStates[key]=enabled;
   emit itemAdded(key);
}

void EnumParameter::removeItem(const int key){
   if(key==-1|| ! mItemNames.contains(key)) return;
   if(mCurrentKey==key) selectItem(mItemNames.begin().key());
   QString name = mItemNames[key];
   mItemNames.remove(key);
   mItemStates.remove(key);
   emit itemRemoved(key,name);
}

void EnumParameter::removeItem(const QString& name){
   removeItem(itemKey(name));
}

void EnumParameter::setItemEnabled(const int key, bool val){
   if( ! mItemStates.contains(key)) return;
   mItemStates[key] = val;
   emit itemStateChanged (key);
}

void EnumParameter::selectItem(const int & key){
   if( ! mItemNames.contains(key)) return;
   mCurrentKey=key;
   emit selectionChanged(key);
}

void EnumParameter::selectItem(const QString& name){
   selectItem(itemKey(name));
}
