#include <qwhatsthis.h>

#include "enumparametereditor.h"
#include "midiin.xpm"


/*! @file enumparametereditor.cpp
 *
 *  @brief
 *  Implementation of class EnumParameterEditor
 *  @author Lukas Degener
 */


EnumParameterEditor::EnumParameterEditor(EnumParameter * model, QWidget * parent, const char * name, WFlags f)
:ParameterEditor(parent, name, f){

   mParam = model;
   QPixmap midiIcon(midiin_xpm);
   //TODO: hmm...maybe we should do something more sophisticated to check this
   if(mParam==0) return;

   this->setFrameStyle(QFrame::Panel|QFrame::Raised);
   this->setSpacing(4);
   mMidiButton = new QPushButton(this);
   mMidiButton->setPixmap(midiIcon);
   mMidiButton->setFocusPolicy(QWidget::NoFocus);
   mMidiButton->setMinimumSize(midiIcon.width()+5,midiIcon.height()+5);
   mMidiButton->setMaximumSize(midiIcon.width()+5,midiIcon.height()+5);
   connect(mMidiButton,SIGNAL(clicked()),this,SLOT(midiButtonClicked()));
   connect(mMidiButton,SIGNAL(clicked()),mParam,SLOT(touchedByGui()));
   mNameLabel = new QLabel(mParam->name(),this);
   mNameLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));

   mComboBox = new QComboBox(this);
   mComboBox->setEditable(false);
   mComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
   //fill combo with enabled items:
   updateComboItems();
   connect(mComboBox,SIGNAL(highlighted(int)),this,SLOT(itemHighlighted(int)));

   mHelpButton = new QPushButton("?",this);
   mHelpButton->setMinimumSize(20,20);
   mHelpButton->setMaximumSize(20,20);
   connect(mHelpButton,SIGNAL(clicked()),this,SLOT(helpButtonClicked()));
   connect(mHelpButton,SIGNAL(clicked()),mParam,SLOT(touchedByGui()));
   //connect to the model
   connect(mParam,SIGNAL(selectionChanged(int)),this,SLOT(selectionChanged(int)));
   connect(mParam,SIGNAL(itemAdded(int)),this,SLOT(itemAdded(int)));
   connect(mParam,SIGNAL(itemRemoved(int,const QString&)),this,SLOT(itemRemoved(int,const QString&)));
   connect(mParam,SIGNAL(itemStateChanged(int)),this,SLOT(itemStateChanged(int)));
   connect(this,SIGNAL(touched()),mParam,SLOT(touchedByGui()));
   this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
}

EnumParameterEditor::~EnumParameterEditor(){
}

void EnumParameterEditor::updateComboItems(){
   QValueList<int> qvl = mParam->enabledItems();
   mComboBox->clear();
   for(QValueList<int>::iterator it = qvl.begin();it!=qvl.end();++it){
      mComboBox->insertItem(mParam->itemName(*it));
   }
   mComboBox->blockSignals(true);
   mComboBox->setCurrentItem(qvl.findIndex(mParam->selectedItem()));
   mComboBox->blockSignals(false);
}

void EnumParameterEditor::parameterDeleted(){
   //my,my. What shall i do here?
}

void EnumParameterEditor::selectionChanged(int key){
   mComboBox->blockSignals(true);
   QValueList<int> qvl = mParam->enabledItems();
   mComboBox->setCurrentItem(qvl.findIndex(mParam->selectedItem()));
   mComboBox->blockSignals(false);
}

void EnumParameterEditor::itemAdded(int key){
   updateComboItems();
}

void EnumParameterEditor::itemRemoved(int key,const QString& name){
   updateComboItems();
}

void EnumParameterEditor::itemStateChanged(int key){
   updateComboItems();
}

void EnumParameterEditor::helpButtonClicked(){
   QWhatsThis::display(mParam->description());
}
void EnumParameterEditor::midiButtonClicked(){
   //TODO: Do something cunningly clever here
}

void EnumParameterEditor::itemHighlighted(int index){
   QValueList<int> qvl = mParam->enabledItems();
   emit touched();
   mParam->selectItem(qvl[index]);

}

Parameter * EnumParameterEditor::getParameter(){
   return mParam;
}
