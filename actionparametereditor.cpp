#include <qhbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include "actionparameter.h"
#include "actionparametereditor.h"
#include "midiin.xpm"

/*! @file actionparametereditor.cpp
 *
 *  @brief
 *  Implementation of class ActionParameterEditor
 *  @author Lukas Degener
 */

ActionParameterEditor::ActionParameterEditor(ActionParameter * model, QWidget * parent, const char * name, WFlags f)
:ParameterEditor(parent, name, f){
   mAnimate=true;
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

   mTriggerButton = new QPushButton(mParam->name(),this);
   mTriggerButton->setEnabled(mParam->enabled());
   connect(mTriggerButton,SIGNAL(clicked()),this,SLOT(triggerButtonClicked()));
   connect(mTriggerButton,SIGNAL(clicked()),mParam,SLOT(touchedByGui()));


   mCheckBox = new QCheckBox("Enabled",this);
   mCheckBox->setChecked(mParam->enabled());
   connect(mCheckBox,SIGNAL(toggled(bool)),this,SLOT(checkBoxToggled(bool)));
   connect(mCheckBox,SIGNAL(clicked()),mParam,SLOT(touchedByGui()));

   mHelpButton = new QPushButton("?",this);
   mHelpButton->setMinimumSize(20,20);
   mHelpButton->setMaximumSize(20,20);
   connect(mHelpButton,SIGNAL(clicked()),this,SLOT(helpButtonClicked()));
   connect(mHelpButton,SIGNAL(clicked()),mParam,SLOT(touchedByGui()));

   //connect to the model
   connect(mParam,SIGNAL(stateChanged(bool)),this,SLOT(stateChanged(bool)));
   connect(mParam,SIGNAL(triggered()),this,SLOT(triggered()));
   this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
}

ActionParameterEditor::~ActionParameterEditor(){
}

void ActionParameterEditor::parameterDeleted(){
   //TODO: Die another day (but soon!)
}

void ActionParameterEditor::stateChanged(bool val){
   mCheckBox->blockSignals(true);
   mCheckBox->setChecked(val);
   mTriggerButton->setEnabled(val);
   mCheckBox->blockSignals(false);
}
void ActionParameterEditor::checkBoxToggled(bool val){
   mParam->setEnabled(val);
}

void ActionParameterEditor::helpButtonClicked(){
   QWhatsThis::display(mParam->description());
}

void ActionParameterEditor::midiButtonClicked(){
   //TODO: Let's all be totaly clever when the user clicked this button.
}

void ActionParameterEditor::triggerButtonClicked(){

   mParam->trigger();
   
}

void ActionParameterEditor::triggered(){

}

Parameter * ActionParameterEditor::getParameter(){
   return mParam;
}

