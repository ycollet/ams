#include <qhbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include "boolparameter.h"
#include "boolparametereditor.h"
#include "midiin.xpm"

/*! @file boolparametereditor.cpp
 *
 *  @brief
 *  Implementation of class BoolParameterEditor
 *  @author Lukas Degener
 */



BoolParameterEditor::BoolParameterEditor(BoolParameter * model, QWidget * parent, const char * name, WFlags f)
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
   connect(mMidiButton,SIGNAL(clicked()),mParam,SIGNAL(touchedByGui()));
   mCheckBox = new QCheckBox(mParam->name(),this);
   mCheckBox->setChecked(mParam->isOn());
   connect(mCheckBox,SIGNAL(toggled(bool)),this,SLOT(checkBoxToggled(bool)));
   connect(mCheckBox,SIGNAL(clicked()),mParam,SIGNAL(touchedByGui()));

   mHelpButton = new QPushButton("?",this);
   mHelpButton->setMinimumSize(20,20);
   mHelpButton->setMaximumSize(20,20);
   connect(mHelpButton,SIGNAL(clicked()),this,SLOT(helpButtonClicked()));
   connect(mHelpButton,SIGNAL(clicked()),mParam,SLOT(touchedByGui()));

   //connect to the model
   connect(mParam,SIGNAL(toggled(bool)),this,SLOT(toggled(bool)));
   this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
}

BoolParameterEditor::~BoolParameterEditor(){
}

void BoolParameterEditor::parameterDeleted(){
   //TODO: Die another day (but soon!)
}

void BoolParameterEditor::toggled(bool val){
   mCheckBox->blockSignals(true);
   mCheckBox->setChecked(val);
   mCheckBox->blockSignals(false);
}
void BoolParameterEditor::checkBoxToggled(bool val){
   mParam->setOn(val);
}

void BoolParameterEditor::helpButtonClicked(){
   QWhatsThis::display(mParam->description());
}

void BoolParameterEditor::midiButtonClicked(){
   //TODO: Let's all be totaly clever when the user clicked this button.
}

Parameter * BoolParameterEditor::getParameter(){
   return mParam;
}

