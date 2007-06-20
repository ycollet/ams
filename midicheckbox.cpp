#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "midicheckbox.h"
#include "synthdata.h"
#include "midiwidget.h"
//#include <QHBoxLayout>

MidiCheckBox::MidiCheckBox(Module *parentModule, float value, QWidget * parent,
			   const QString &name, float *p_valueRef)
  : MidiGUIcomponent(parentModule, parent, name) {

  componentType = GUIcomponentType_checkbox;
  valueRef = p_valueRef;

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
  QLabel *nameLabel = new QLabel(name);

  checkFrame->addWidget(nameLabel);  
  checkBox->setChecked(value > 0);
  QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(updateValue(bool)));
  updateValue(value > 0);
}

MidiCheckBox::~MidiCheckBox(){
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

void MidiCheckBox::updateValue(bool on) {

  *valueRef = on ? 1 : 0;    
}

void MidiCheckBox::updateCheck(bool on) {

  checkBox->setChecked(on);
  emit guiComponentTouched();
}
