#include <qobject.h>
#include <qvbox.h>
#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <iostream.h>

#include "parameter.h"
#include "intparametereditor.h"
#include "midiin.xpm"


/*! @file intparametereditor.cpp
 *
 *  @brief
 *  Implementation of class IntParameterEditor
 *  @author Lukas Degener
 */



IntParameterEditor::IntParameterEditor(IntParameter * model, QWidget * parent, const char * name, WFlags f)
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

   mGrid = new QGrid(2,Qt::Vertical,this);
   mGrid->setSpacing(2);

   mMinHBox = new QHBox(mGrid);

   mMinLabel = new QLabel("Min.",mMinHBox);
   mMinLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   mMinLabel->setMinimumSize(1,1);
   mMinLabel->setMaximumSize(55,40);

   mResetMinButton = new QPushButton("x",mMinHBox);
   mResetMinButton->setMinimumSize(20,20);
   mResetMinButton->setMaximumSize(20,20);
   connect(mResetMinButton,SIGNAL(clicked()),this,SLOT(resetMinButtonClicked()));


   mMinButton = new QPushButton("23.12",mGrid);
   mMinButton->setMinimumSize(55,20);
   mMinButton->setMaximumSize(55,20);
   connect(mMinButton,SIGNAL(clicked()),this,SLOT(minButtonClicked()));

   mHBox = new QHBox(mGrid);

   mNameLabel = new QLabel(mParam->name(),mHBox);
   mNameLabel->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum));

   mValueLabel = new QLabel("20",mHBox);
   mValueLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));


   mSlider = new QSlider(Qt::Horizontal,mGrid);
   mSlider->setMinimumSize(150,1);
   mSlider->setMinValue(mParam->minValue());
   mSlider->setMaxValue(mParam->maxValue());
   mSlider->setSteps(1,1);
   mSlider->setTickmarks(QSlider::Both);
   connect(mSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderMoved(int)));

   mMaxHBox = new QHBox(mGrid);
   mMaxLabel = new QLabel("Max.",mMaxHBox);
   mMaxLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   mMaxLabel->setMinimumSize(1,1);
   mMaxLabel->setMaximumSize(55,40);

   mResetMaxButton = new QPushButton("x",mMaxHBox);
   mResetMaxButton->setMinimumSize(20,20);
   mResetMaxButton->setMaximumSize(20,20);
   connect(mResetMaxButton,SIGNAL(clicked()),this,SLOT(resetMaxButtonClicked()));


   mMaxButton = new QPushButton("23",mGrid);
   mMaxButton->setMinimumSize(55,20);
   mMaxButton->setMaximumSize(55,20);
   connect(mMaxButton,SIGNAL(clicked()),this,SLOT(maxButtonClicked()));

   mHelpButton = new QPushButton("?",this);
   mHelpButton->setMinimumSize(20,20);
   mHelpButton->setMaximumSize(20,20);
   connect(mHelpButton,SIGNAL(clicked()),this,SLOT(helpButtonClicked()));
   //initialize values by simulating incoming signals from the model
   valueChanged(mParam->value());
   minChanged(mParam->minValue());
   maxChanged(mParam->maxValue());

   //connect to the model
   connect(mParam,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
   connect(mParam,SIGNAL(minChanged(int)),this,SLOT(minChanged(int)));
   connect(mParam,SIGNAL(maxChanged(int)),this,SLOT(maxChanged(int)));
   connect(this,SIGNAL(touched()),mParam,SLOT(touchedByGui()));
   this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
}

IntParameterEditor::~IntParameterEditor(){
}

void IntParameterEditor::valueChanged(int val){
   int range = mParam->maxValue()-mParam->minValue();
   if(range==0)mSlider->setValue(0);
   else {
      mSlider->blockSignals(true);
      //mSlider->setValue((int)((SLIDER_SCALE*(float)(val - mParam->minValue())/(float)range)));
      mSlider->setValue(val);
      mSlider->blockSignals(false);
   }
   QString qs;
   qs.sprintf("%i", val);
   mValueLabel->setText(qs);
}
void IntParameterEditor::minChanged(int val){
   QString qs;
   qs.sprintf("%i", val);
   mMinButton->setText(qs);
   mSlider->setMinValue(val);
   mSlider->setTickInterval(1);


   //we have to issue an extra valueChanged
   //so that the slider position is correct
   valueChanged(mParam->value());
}

void IntParameterEditor::maxChanged(int val){
   QString qs;
   qs.sprintf("%i", val);
   mMaxButton->setText(qs);
   mSlider->setMaxValue(val);
   mSlider->setTickInterval(1);

   //we have to issue an extra valueChanged
   //so that the slider position is correct
   valueChanged(mParam->value());
}

void IntParameterEditor::parameterDeleted(){
   //TODO: Not quiet sure what to do here yet
}

void IntParameterEditor::sliderMoved(int val){
   //mParam->setValue((mParam->maxValue()-mParam->minValue()) * (int)val / SLIDER_SCALE + mParam->minValue());
   mParam->setValue(val);
   emit touched();
}
void IntParameterEditor::maxButtonClicked(){
   mParam->setMaxValue(mParam->value());
   emit touched();
}
void IntParameterEditor::minButtonClicked(){
   mParam->setMinValue(mParam->value());
   emit touched();
}

void IntParameterEditor::resetMaxButtonClicked(){
   mParam->setMaxValue(mParam->realMaxValue());
   emit touched();
}
void IntParameterEditor::resetMinButtonClicked(){
   mParam->setMinValue(mParam->realMinValue());
   emit touched();
}
void IntParameterEditor::helpButtonClicked(){
   QWhatsThis::display(mParam->description());
   emit touched();
}

void IntParameterEditor::midiButtonClicked(){
   //TODO: Do something very clever here.
   emit touched();
}

Parameter * IntParameterEditor::getParameter(){
   return mParam;
}
