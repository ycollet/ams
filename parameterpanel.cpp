

#include <qwidget.h>
#include <qobject.h>
#include <qlayout.h>
#include <qevent.h>
#include <qdragobject.h>

#include "parameter.h"
#include "parametereditor.h"
#include "parametereditorfactory.h"
#include "parameterpanel.h"
#include "intparameter.h"
#include "floatparameter.h"
#include "enumparameter.h"
#include "boolparameter.h"
/*!
 * @todo fix the segfault
 */
ParameterPanel::ParameterPanel(QWidget * parent, const char * name, bool editable,bool havePresets,int id=-1)
:QWidget(parent,name){
   mID=id;
   mIsNameChange=false;
   mEditable=editable;
   mHavePresets=havePresets;
   mDraggedWidget = NULL;
   mLayout = new QVBoxLayout(this);
   setAcceptDrops(editable);
   this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
   if(editable){
     topBox = new QHBox(this);
     topBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
     topBox->show();
     mLayout->insertWidget(-1,topBox);
     new QLabel("Panel name:",topBox);
     mNameEdit= new QLineEdit(name,topBox);
     QLabel *label = new QLabel("drop your parameters here",this);
     label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
     mLayout->insertWidget(-1,label);
     label->setPaletteBackgroundColor (cyan);

   }
   if(havePresets){
     bottomBox=new QVBox(this);
     bottomBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
     bottomBox->show();
     mLayout->insertWidget(-1,bottomBox);
     QHBox *hbox = new QHBox(bottomBox);
     QLabel *label = new QLabel("Presets:",hbox);
     mPresetCombo = new QComboBox(hbox);
     mPresetCombo->setEditable(true);
     mPresetCombo->setInsertionPolicy(QComboBox::AtCurrent);
     hbox = new QHBox(bottomBox);
     QVBox *vbox =new QVBox(hbox);
     mStoreButton = new QPushButton("Store",vbox);
     mRestoreButton = new QPushButton("Restore",vbox);
     vbox =new QVBox(hbox);
     mNewButton = new QPushButton("New",vbox);
     mDeleteButton = new QPushButton("Delete",vbox);
     new QWidget(bottomBox);
     mRemovePanelButton = new QPushButton("Remove Panel",bottomBox);

     mPreset=new EnumParameter(this,"Presets","");

     //Parameters ---> this
     connect(mPreset,SIGNAL(selectionChanged(int)),this,SLOT(selectionChanged(int)));
     connect(mPreset,SIGNAL(itemAdded(int)),this,SLOT(itemAdded(int)));
     connect(mPreset,SIGNAL(itemRemoved(int,const QString&)),this,SLOT(itemRemoved(int,const QString&)));
     connect(mPreset,SIGNAL(itemStateChanged(int)),this,SLOT(itemStateChanged(int)));
     mStore=new ActionParameter(this,"Store","");
     connect(mStore,SIGNAL(triggered()),this,SLOT(savePreset()));
     mRestore=new ActionParameter(this,"Restore","");
     connect(mRestore,SIGNAL(triggered()),this,SLOT(loadPreset()));
     mNew=new ActionParameter(this,"New","");
     connect(mNew,SIGNAL(triggered()),this,SLOT(addPreset()));
     mDelete=new ActionParameter(this,"Delete","");
     connect(mDelete,SIGNAL(triggered()),this,SLOT(removePreset()));

     //GUI ---> this
     connect (mPresetCombo,SIGNAL(highlighted(const QString&)),this,SLOT(highlighted(const QString &)));
     connect (mPresetCombo,SIGNAL(textChanged(const QString&)),this,SLOT(textChanged(const QString &)));
     connect (mRemovePanelButton,SIGNAL(clicked()),this,SLOT(removeClicked()));
     connect (mNameEdit,SIGNAL(textChanged(const QString &)),this,SLOT(setName(const QString &)));
     //GUI ---> Parameters
     connect (mNewButton,SIGNAL(clicked()),mNew,SLOT(trigger()));
     connect (mDeleteButton,SIGNAL(clicked()),mDelete,SLOT(trigger()));
     connect (mStoreButton,SIGNAL(clicked()),mStore,SLOT(trigger()));
     connect (mRestoreButton,SIGNAL(clicked()),mRestore,SLOT(trigger()));
     setCaption(name);
     updatePresets();
   }
}

ParameterPanel::~ParameterPanel(){
}

void ParameterPanel::mousePressEvent ( QMouseEvent * e ){
   cout<<"mouse pressed"<<endl;
}

void ParameterPanel::mouseReleaseEvent ( QMouseEvent * e ){
   mDraggedWidget = NULL;
   cout<<"mouse released"<<endl;
}

void ParameterPanel::mouseMoveEvent ( QMouseEvent * e ){
   mDraggedWidget = directChildAt(e->pos());
   if(mDraggedWidget==NULL) return;
   if(! mDraggedWidget->inherits("ParameterEditor")){

      return;
   }
/*! @bug there is some mysterious segfaulting going on around here, and
 * i suspect it has something to do wirh drag&drop.
 *
 */
   QString qs;
   qs=((ParameterEditor *)mDraggedWidget)->getParameter()->longName();
   QDragObject *d = new QTextDrag(qs,this);
   d->dragMove();
}

void ParameterPanel::dragMoveEvent ( QDragMoveEvent * e ){
    QWidget *w=directChildAt(e->pos());
    e->accept( QTextDrag::canDecode(e) 
            && e->source()->isA("ParameterPanel")
            && w!=topBox
            && w!=bottomBox);
}

void ParameterPanel::dropEvent ( QDropEvent * e ){
   cout<<"drop"<<endl;
   if( QTextDrag::canDecode(e) && e->source()->isA("ParameterPanel")){
      ParameterPanel * src = (ParameterPanel *) e->source();
      QString qs;
      QTextDrag::decode(e,qs);

      Parameter *param =Parameter::instance(qs);
      QWidget *w=directChildAt(e->pos());
      int index = mLayout->findWidget(w);
      if(w->isA("QLabel")&&index>1){
        --index;
      }
      if(mEditors[param]){
         removeParameter(param);
      }
      addParameter(param,index);
   }
}
QWidget * ParameterPanel::directChildAt(const QPoint & pos){
   QWidget * child = childAt(pos);
   while(child!=NULL && child->parentWidget()!=this){
      child=child->parentWidget();
   }
   return child;
}
void ParameterPanel::addParameter(Parameter * param, int position=-1){
   QWidget * newwidget =ParameterEditorFactory::createEditor(this,"",param);
   newwidget->show();
   mLayout->insertWidget(position,newwidget );
   mEditors.replace(param,newwidget);
   connect(param,SIGNAL(destroyed(QObject *)),this,SLOT(parameterDestroyed(QObject *)));
   cout<<param->name()<<" added at "<<position<<endl;
   emit parameterAdded(param);
   mParameters.append(param);
}

void ParameterPanel::removeParameter(Parameter * param){
   QWidget * editor = mEditors[param];
   if(editor){
      mEditors.remove(param);
      delete(editor);
      cout<<param->name()<<" removed."<<endl;
      emit parameterRemoved(param);
   }
   mParameters.remove(param);
}

void ParameterPanel::parameterDestroyed(QObject * param){
   removeParameter((Parameter *)param);
}

void ParameterPanel::addPreset(){
  mLastPreset++;
  QString qs;
  qs.sprintf("preset %i",mLastPreset);
  mPreset->addItem(mLastPreset,qs);
  mPreset->selectItem(mLastPreset);
  savePreset();
}

void ParameterPanel::removePreset(){
  mPresets.erase(mPreset->selectedItem());
  mPreset->removeItem(mPreset->selectedItem());
}
void ParameterPanel::savePreset(){
  QMap<QString,QString> preset;
  QString name,value;
  for(int i=0;i<mParameters.count();++i){
   Parameter * param = mParameters.at(i);
   name=param->longName();
   if (param->isA("FloatParameter")) value.setNum(((FloatParameter *)param)->value());
   else if (param->isA("IntParameter")) value.setNum(((IntParameter *)param)->value());
   else if (param->isA("EnumParameter")) value.setNum(((EnumParameter *)param)->selectedItem());
   else if (param->isA("BoolParameter")) value.setNum(((BoolParameter *)param)->isOn());
   preset.replace(name,value);
  }
  mPresets.replace(mPreset->selectedItem(),preset);
}
void ParameterPanel::loadPreset(int index){
  QMap<QString,QString> preset = mPresets[index];
  QMap<QString,QString>::iterator it = preset.begin();
  QString name,value;
  Parameter * param;
  while (it!=preset.end()){
    name=it.key();
    value=it.data();
    param = Parameter::instance(name);
    name=param->longName();
    if (param->isA("FloatParameter")) ((FloatParameter *)param)->setValue(value.toFloat());
    else if (param->isA("IntParameter"))((IntParameter *)param)->setValue(value.toInt());
    else if (param->isA("EnumParameter")) ((EnumParameter *)param)->selectItem(value.toInt());
    else if (param->isA("BoolParameter")) ((BoolParameter *)param)->setOn(value.toInt());
    it++;
  }
}

void ParameterPanel::loadPreset(){
  loadPreset(mPresetCombo->currentItem());
}
void ParameterPanel::setName(const QString & name){
  QWidget::setName(name);
  setCaption(name);
  emit nameChanged(mID,name);
}

void ParameterPanel::updatePresets(){
   mPresetCombo->blockSignals(true);
   QValueList<int> qvl = mPreset->enabledItems();
   mPresetCombo->setEnabled(qvl.count()>0);
   mStoreButton->setEnabled(qvl.count()>0);
   mRestoreButton->setEnabled(qvl.count()>0);
   mDeleteButton->setEnabled(qvl.count()>0);
   mPresetCombo->clear();
   for(QValueList<int>::iterator it = qvl.begin();it!=qvl.end();++it){
      mPresetCombo->insertItem(mPreset->itemName(*it));
   }

   mPresetCombo->setCurrentItem(qvl.findIndex(mPreset->selectedItem()));
   mPresetCombo->blockSignals(false);
}

void ParameterPanel::selectionChanged(int key){
  if(mIsNameChange) return;
  mPresetCombo->blockSignals(true);
  QValueList<int> qvl = mPreset->enabledItems();
  mPresetCombo->setCurrentItem(qvl.findIndex(key));
  mPresetCombo->blockSignals(false);
  loadPreset();
}
void ParameterPanel::itemAdded(int key){
  if(mIsNameChange) return;
  updatePresets();
}
void ParameterPanel::itemRemoved(int key,const QString & name){
  if(mIsNameChange) return;
  updatePresets();
}
void ParameterPanel::itemStateChanged(int key){
  if(mIsNameChange) return;
  updatePresets();
}
void ParameterPanel::highlighted(const QString & p){
  if(mIsNameChange) return;
  mPresetCombo->blockSignals(true);
  mPreset->selectItem(p);
  mPresetCombo->blockSignals(false);
}
void ParameterPanel::textChanged(const QString & qs){
  mIsNameChange=true;
  int key = mPreset->selectedItem();
  mPreset->removeItem(key);
  mPreset->addItem(key,qs);
  mPreset->selectItem(key);
  mIsNameChange=false;
  updatePresets();
}

void ParameterPanel::removeClicked(){
  emit removePanelButtonClicked(mID);
}
