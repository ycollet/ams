#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include <qstrlist.h>
#include "midicheckbox.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

MidiCheckBox::MidiCheckBox(QObject *parentModule, float value, QWidget * parent, const char * name, SynthData
                           *p_synthdata, float *p_valueRef)
                           : MidiGUIcomponent(parentModule, p_synthdata, parent, name) {

  componentType = GUIcomponentType_checkbox;
  valueRef = p_valueRef;
//  setSpacing(5);
//  setMargin(5);
  QWidget *dummy1 = new QWidget(this);
  QHBox *checkFrame = new QHBox(this);
  QWidget *dummy2 = new QWidget(this);
  setStretchFactor(dummy1, 3);
  setStretchFactor(checkFrame, 1);
  setStretchFactor(dummy2, 3);
  checkBox = new QCheckBox(checkFrame);  
  QLabel *nameLabel = new QLabel(checkFrame);
  if (name) {
    nameLabel->setText("  "+QString(name));
  }
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
