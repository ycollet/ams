#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include <qstrlist.h>
#include "midicombobox.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

MidiComboBox::MidiComboBox(QObject *parentModule, int value, QWidget * parent, const char * name, SynthData
                           *p_synthdata, int *p_valueRef, QStrList *itemNames)
                           : MidiGUIcomponent(parentModule, p_synthdata, parent, name) {

  QString qs;

  componentType = GUIcomponentType_combobox;
  valueRef = p_valueRef;
  setSpacing(5);
  setMargin(5);
  new QWidget(this);
  QVBox *comboFrame = new QVBox(this);
  comboFrame->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
  QHBox *comboNameBox = new QHBox(comboFrame);
  new QWidget(comboNameBox);
  comboNameBox->setMargin(5);
  QLabel *nameLabel = new QLabel(comboNameBox);
  new QWidget(comboNameBox);
  if (name) {
    nameLabel->setFixedHeight(nameLabel->sizeHint().height());
    nameLabel->setText(name);
  }
  nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  comboBox = new QComboBox(comboFrame);  
  comboBox->insertStrList(itemNames);
  comboBox->setFixedSize(comboBox->sizeHint());
  QObject::connect(comboBox, SIGNAL(highlighted(int)), this, SLOT(updateValue(int)));
  updateValue(value);
}

MidiComboBox::~MidiComboBox(){
}

void MidiComboBox::setMidiValue(int value) {

  if (!controllerOK) {
    controllerOK = abs(getMidiValue() - value) < 4;
  }
  if (controllerOK) {
    if (midiSign == 1) {
      comboBox->setCurrentItem(int((float)(comboBox->count()-1) / 127.0 * (float)value));
    } else {
      comboBox->setCurrentItem(int((float)(comboBox->count()-1) / 127.0 * (float)(127-value)));
    }
  }
}

void MidiComboBox::updateValue(int value) {

  *valueRef = value;    
  comboBox->setCurrentItem(value);
  emit guiComponentTouched();
}

int MidiComboBox::getMidiValue() {

  int x;  
 
  x = rint(127.0 * comboBox->currentItem() / (comboBox->count()-1));
  return(x);
}
