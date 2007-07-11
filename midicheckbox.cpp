#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "midicheckbox.h"
#include "synthdata.h"

MidiCheckBox::MidiCheckBox(MidiControllable<float> &mcAble)
  : MidiGUIcomponent(mcAble)
{

  componentType = GUIcomponentType_checkbox;

  //  QWidget *dummy1 = new QWidget(this);
  QHBoxLayout *checkFrame = new QHBoxLayout(this);
  checkFrame->setSpacing(5);
  checkFrame->setMargin(5);

  //  QWidget *dummy2 = new QWidget(this);
//FIXME   setStretchFactor(dummy1, 3);
//   setStretchFactor(checkFrame, 1);
//   setStretchFactor(dummy2, 3);
  checkBox = new QCheckBox();
  checkFrame->addWidget(checkBox);  
  checkFrame->addStretch();  
  QLabel *nameLabel = new QLabel(mcAble.name);

  checkFrame->addWidget(nameLabel);  
  checkBox->setChecked(mcAble.getValue() > 0);
  QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
  mcAbleChanged(// value > 0
	      );
}

MidiGUIcomponent *MidiCheckBox::createTwin()
{
  return new MidiCheckBox(*dynamic_cast<MidiControllable<float> *>(&mcAble));
}

MidiCheckBox::~MidiCheckBox(){
}

void MidiCheckBox::toggled(bool)
{
}

void MidiCheckBox::mcAbleChanged()
{
}

/*
void MidiCheckBox::setMidiValueRT(int value)
{
}

void MidiCheckBox::setMidiValue(int value) {

  if (midiSign == 1) {
    checkBox->setChecked(value > 124);
  } else {
    checkBox->setChecked(value <= 124);
  }
}

int MidiCheckBox::getMidiValue() {

  return((checkBox->isChecked()) ? 127 : 0);
}

void MidiCheckBox::updateCheck(bool on) {

  checkBox->setChecked(on);
  emit guiComponentTouched();
}
*/
