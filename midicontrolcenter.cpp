
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qsplitter.h>
#include <qcheckbox.h>
#include <qlistview.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include "midicontroller.h"
#include "parameter.h"
#include "midicontroller.h"
#include "midibinding.h"
#include "floatmidibinding.h"
#include "intmidibinding.h"
#include "enummidibinding.h"
#include "boolmidibinding.h"
#include "actionmidibinding.h"

#include "midicontrolcenter.h"
#include "parametereditorfactory.h"
#include "parameterpanel.h"

/*! @file midicontrolcenter.cpp
 *
 *  @brief
 *  Implementation of class MidiControlCenter
 *  @author Lukas Degener, Matthias Nagorni
 */
MidiControlCenter * MidiControlCenter::mInstance = NULL;
MidiControlCenter * MidiControlCenter::getInstance(){
  if(!mInstance) mInstance = new MidiControlCenter();
  return mInstance;
}

MidiControlCenter::MidiControlCenter(QWidget* parent=0, const char *name=0)
:QVBox(parent,name){
   setMargin(10);
   setSpacing(5);

   QSplitter *listViewBox = new QSplitter(this);
   setStretchFactor(listViewBox, 10);
   mControllerListView = new QListView(listViewBox);
   mControllerListView->setRootIsDecorated(true);
   mControllerListView->addColumn("MIDI Controller / Binding");
   mControllerListView->addColumn("MIDI Sign");
   mControllerListView->addColumn("Module");
   mControllerListView->setAllColumnsShowFocus(true);
   mParameterListView = new QListView(listViewBox);
   mParameterListView->setRootIsDecorated(true);
   mParameterListView->addColumn("Module / Parameter");
   mParameterListView->setAllColumnsShowFocus(true);
   QObject::connect(mParameterListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(parameterListViewSelectionChanged(QListViewItem *)));
   QObject::connect(mControllerListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(controllerListViewSelectionChanged(QListViewItem *)));
   QVBox *controlFrame = new QVBox(this);
   controlFrame->setSpacing(5);

   setStretchFactor(controlFrame, 1);
   mEditorBox = new QVBox(controlFrame);
   mEditorBox->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   new QLabel("hier sollte was sein...",mEditorBox);
   mCurrentEditor = NULL;
   QHBox *checkbuttonBox = new QHBox(controlFrame);
   checkbuttonBox->setSpacing(10);
   checkbuttonBox->setMargin(5);
   QHBox *buttonBox = new QHBox(controlFrame);
   buttonBox->setSpacing(5);
   buttonBox->setMargin(5);
   new QWidget(buttonBox);
   mNoteCheckBox= new QCheckBox("Enable note events", checkbuttonBox);
   mNoteCheckBox->setChecked(false);
   mConfigCheckBox = new QCheckBox("Follow Configuration Dialog", checkbuttonBox);
   mConfigCheckBox->setChecked(false);
   mMidiCheckBox = new QCheckBox("Follow MIDI", checkbuttonBox);
   mMidiCheckBox->setChecked(false);
   QObject::connect(mNoteCheckBox, SIGNAL(toggled(bool)), this, SLOT(noteCheckBoxToggled(bool)));
   QObject::connect(mConfigCheckBox, SIGNAL(toggled(bool)), this, SLOT(configCheckBoxToggled(bool)));
   QObject::connect(mMidiCheckBox, SIGNAL(toggled(bool)), this, SLOT(midiCheckBoxToggled(bool)));
   new QWidget(buttonBox);
   QPushButton *mBindButton = new QPushButton("Bind", buttonBox);
   new QWidget(buttonBox);
   QPushButton *mClearButton = new QPushButton("Clear Binding", buttonBox);
   new QWidget(buttonBox);
   new QLabel("Map Type :",buttonBox);
   mBindingCombo = new QComboBox("Map Type",buttonBox);
   mBindingCombo->setEnabled(false);
   //QPushButton *clearAllButton = new QPushButton("Clear All", buttonBox);
   new QWidget(buttonBox);
   mSignCheckBox = new QCheckBox("Reverse",buttonBox);
   //QPushButton *midiSignButton = new QPushButton("Toggle MIDI Sign", buttonBox);
   new QWidget(buttonBox);
   QObject::connect(mBindButton, SIGNAL(clicked()), this, SLOT(bindButtonClicked()));
   QObject::connect(mClearButton, SIGNAL(clicked()), this, SLOT(clearButtonClicked()));
   //QObject::connect(clearAllButton, SIGNAL(clicked()), this, SLOT(clearAllClicked()));
   //QObject::connect(midiSignButton, SIGNAL(clicked()), this, SLOT(toggleMidiSign()));
   mSelectedParameter=NULL;
   mSelectedController=NULL;
   mSelectedBinding=NULL;
}

MidiControlCenter::~MidiControlCenter(){
}

Parameter * MidiControlCenter::selectedParameter(){
   return mSelectedParameter;
}

MidiBinding * MidiControlCenter::selectedBinding(){
   return mSelectedBinding;
}

MidiController * MidiControlCenter::selectedController(){
   return mSelectedController;
}

MidiControllerList * MidiControlCenter::controllers(){
   return &mControllers;
}

void MidiControlCenter::registerParameter(Parameter * newParam){
   //return if newParam is already registered
   if(mListViewItems.find(newParam)) return;
   //look for the Params parent, to see if
   //an listviewitem does exist for it
   ObjectListViewItem * newNode;
   QObject * p = newParam->parent();
   //cout<<p->name()<<"::"<<newParam->name()<<endl;
   ObjectListViewItem * parentNode = mListViewItems.find(p);

   //if no parent node exists, create it
   if(!parentNode){
      parentNode = new ObjectListViewItem(mParameterListView, p);
      mListViewItems.replace(p,parentNode);
      parentNode->setText(0,p->name());
      if(p->isA("ParameterPanel")){
        ParameterPanel *pp = (ParameterPanel *)p;
        connect(pp,SIGNAL(nameChanged(int, const QString &)),this,SLOT(panelNameChanged(int,const QString &)));
      }
      cout<<"Parent node created: "<<parentNode->text(0)<<endl;
   }
   //now create a node for newParam and make it a child of parentNode
   newNode = new ObjectListViewItem(parentNode, newParam);
   mListViewItems.replace(newParam,newNode);
   newNode->setText(0,newParam->name());
   connect(newParam,SIGNAL(destroyed(QObject *)), this, SLOT(parameterDestroyed(QObject *)));

   //voila :-)
}

bool MidiControlCenter::registerController(MidiController * newCtrl){
   if(newCtrl->parent()!=this) return false;
   if(!mControllers.containsRef(newCtrl)){
      mControllers.append(newCtrl);
      connect(newCtrl,SIGNAL(destroyed(QObject *)), this, SLOT(controllerDestroyed(QObject *)));
      connect(newCtrl,SIGNAL(bindingAdded(MidiController*, MidiBinding *)), this, SLOT(bindingAdded(MidiController*, MidiBinding *)));
      connect(newCtrl,SIGNAL(bindingRemoved(MidiController*, MidiBinding *)), this, SLOT(bindingRemoved(MidiController*, MidiBinding *)));
      ObjectListViewItem *lvi = new ObjectListViewItem(mControllerListView, newCtrl);
      lvi->setText(0, newCtrl->name());
      mListViewItems.replace(newCtrl,lvi);
   }
   return true;
}

void MidiControlCenter::selectBinding(MidiBinding * binding){
   ObjectListViewItem * lvi = mListViewItems.find(binding);
   if(lvi) mControllerListView->setSelected(lvi,true);
}

void MidiControlCenter::selectParameter(Parameter * param){
   ObjectListViewItem * lvi = mListViewItems.find(param);
   if(lvi) mParameterListView->setSelected(lvi,true);
}

void MidiControlCenter::selectController(MidiController * ctrl){
   ObjectListViewItem * lvi = mListViewItems.find(ctrl);
   if(lvi) mControllerListView->setSelected(lvi,true);
}

void MidiControlCenter::parameterDestroyed(QObject * param){
   ObjectListViewItem * lvi = mListViewItems.find(param);
   QListViewItem *p = lvi->parent();
   if(lvi){
      mListViewItems.remove(param);
      delete lvi;
   }
   if(!p->childCount()){
      mListViewItems.remove(p);
      delete p;
   }
}

void MidiControlCenter::bindingDestroyed(QObject * binding){
   ObjectListViewItem * lvi = mListViewItems.find(binding);
   if(lvi){
      mListViewItems.remove(binding);
      delete lvi;
   }
}

void MidiControlCenter::controllerDestroyed(QObject * ctrl){
   ObjectListViewItem * lvi = mListViewItems.find(ctrl);
   if(lvi){
      mListViewItems.remove(ctrl);
      delete lvi;
   }
   mControllers.remove((MidiController *)ctrl);
}

void MidiControlCenter::bindingAdded(MidiController * ctrl, MidiBinding * binding){
   ObjectListViewItem * parentNode = mListViewItems[ctrl];
   ObjectListViewItem * newNode = new ObjectListViewItem(parentNode, binding);
   newNode->setText(0,binding->parent()->name()); //the name of the parameter
   mListViewItems.replace(binding,newNode);
}

void MidiControlCenter::bindingRemoved(MidiController * ctrl, MidiBinding * binding){
   ObjectListViewItem * lvi = mListViewItems[binding];
   mListViewItems.remove(binding);
   delete lvi;
}

void MidiControlCenter::noteCheckBoxToggled(bool val){
//NOP
}

void MidiControlCenter::configCheckBoxToggled(bool val){
//NOP
}

void MidiControlCenter::midiCheckBoxToggled(bool val){
//NOP
}

void MidiControlCenter::controllerListViewSelectionChanged( QListViewItem * item){
   QObject * o = ((ObjectListViewItem*)item)->object;
   if(mSelectedBinding){
      disconnect(mBindingCombo,SIGNAL(highlighted(const QString&)),
                 mSelectedBinding,SLOT(setMapType(const QString&)));
      disconnect(mSignCheckBox,SIGNAL(toggled(bool)),
                 mSelectedBinding,SLOT(setMidiSign(bool)));
      disconnect(mSelectedBinding,SIGNAL(mapTypeChanged(const QString&)),
                 this,SLOT(mapTypeChanged(const QString&)));
      disconnect(mSelectedBinding,SIGNAL(midiSignChanged(int)),
                 this,SLOT(midiSignChanged(int)));
   }
   if(item->isSelected()){
      if(o->isA("MidiController")) {
         mSelectedController = (MidiController *)o;
         mSelectedBinding = NULL;
         mBindingCombo->setEnabled(false);
         mBindingCombo->clear();
         mSignCheckBox->setEnabled(false);
      }
      else if(o->inherits("MidiBinding")) {
         mSelectedBinding = (MidiBinding *)o;
         mSelectedController = mSelectedBinding->controller();
         mBindingCombo->setEnabled(true);
         mBindingCombo->insertStringList(mSelectedBinding->mapTypeNames());
         mapTypeChanged(mSelectedBinding->mapTypeNames()[mSelectedBinding->mapType()]);
         mSignCheckBox->setEnabled(true);
         mSignCheckBox->setChecked(mSelectedBinding->midiSign()<0);
         connect(mBindingCombo,SIGNAL(highlighted(const QString&)),
                 mSelectedBinding,SLOT(setMapType(const QString&)));
         connect(mSignCheckBox,SIGNAL(toggled(bool)),
                 mSelectedBinding,SLOT(setMidiSign(bool)));
         connect(mSelectedBinding,SIGNAL(mapTypeChanged(const QString &)),
                 this,SLOT(mapTypeChanged(const QString&)));
         connect(mSelectedBinding,SIGNAL(midiSignChanged(int)),
                 this,SLOT(midiSignChanged(int)));
      }
   }
   else{
      mSelectedController = NULL;
      mSelectedBinding = NULL;
      mBindingCombo->setEnabled(false);
      mBindingCombo->clear();
      mSignCheckBox->setEnabled(false);
   }
}

void MidiControlCenter::parameterListViewSelectionChanged( QListViewItem * item){
   QObject * o = ((ObjectListViewItem*)item)->object;
   if(item->isSelected() && o->inherits("Parameter")){
      cout<<"Parameter "<<o->name()<<" selected"<<endl;
      mSelectedParameter=(Parameter *)o;
      if(mCurrentEditor){
         delete mCurrentEditor;
         mCurrentEditor=NULL;
      }
      mCurrentEditor = ParameterEditorFactory::createEditor(mEditorBox,
                                                            mSelectedParameter->name(),
                                                            mSelectedParameter);
      if(!mCurrentEditor) cout<<"scheißdreck!"<<endl;
      mCurrentEditor->show();
   }
   else{
      mSelectedParameter=NULL;
      if(mCurrentEditor){
         delete mCurrentEditor;
         mCurrentEditor=NULL;
      }

   }
};

void MidiControlCenter::clearButtonClicked(){
   if(mSelectedBinding){
      delete mSelectedBinding;
      mSelectedBinding=NULL;
   }
   else if (mSelectedController){
      delete mSelectedController;
      mSelectedController=NULL;
   }
   /*
    *  The following code could be used to do a "cleann-up", i.e. remove all
    *  MidiController instances for which no binding exists.
    *
   else{
      ObjectListViewItem * item = (ObjectListViewItem*)(mControllerListView->firstChild());
      ObjectListViewItem *lastItem=NULL;
      while(item){
        if(! item->childCount()){
          delete item->object;
          lastItem=item;
          item=(ObjectListViewItem*)(lastItem->nextSibling());
          delete lastItem;lastItem=NULL;
        }
      }
   }
   */
}

void MidiControlCenter::bindButtonClicked(){
   if((mSelectedController!=NULL)&&(mSelectedParameter!=NULL)){
      if(mSelectedParameter->isA("FloatParameter")){
         new FloatMidiBinding((FloatParameter *)mSelectedParameter,mSelectedController);
      }
      else if(mSelectedParameter->isA("IntParameter")){
         new IntMidiBinding((IntParameter *)mSelectedParameter,mSelectedController);
      }
      else if(mSelectedParameter->isA("EnumParameter")){
         new EnumMidiBinding((EnumParameter *)mSelectedParameter,mSelectedController);
      }
      else if(mSelectedParameter->isA("BoolParameter")){
         new BoolMidiBinding((BoolParameter *)mSelectedParameter,mSelectedController);
      }
      else if(mSelectedParameter->isA("ActionParameter")){
         new ActionMidiBinding((ActionParameter *)mSelectedParameter,mSelectedController);
      }
   }
}

void MidiControlCenter::mapTypeChanged(const QString & name){

  for(int i=0;i<mBindingCombo->count();++i){
    if(name==mBindingCombo->text(i)){
      mBindingCombo->blockSignals(true);
      mBindingCombo->setCurrentItem(i);
      mBindingCombo->blockSignals(false);
      break;
    }
  }
}

void MidiControlCenter::midiSignChanged(int sign){
  mSignCheckBox->blockSignals(true);
  mSignCheckBox->setChecked(sign<0);
  mSignCheckBox->blockSignals(false);
}

bool MidiControlCenter::noteEventsEnabled(){
   return mNoteCheckBox->isChecked();
}
bool MidiControlCenter::followMidiEnabled(){
   return mMidiCheckBox->isChecked();
}
bool MidiControlCenter::followConfigEnabled(){
   return mConfigCheckBox->isChecked();
}

void MidiControlCenter::panelNameChanged(int id,const QString& name){
  QObject *o = (QObject*)sender();
  mListViewItems[o]->setText(0,name);
}