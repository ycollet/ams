#include <qslider.h>
#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "midicombobox.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

MidiComboBox::MidiComboBox(Module *parentModule, int value, QWidget * parent,
			   const QString &name, int *p_valueRef, QStringList *itemNames)
  : MidiGUIcomponent(parentModule, parent, name) {

  QString qs;

  componentType = GUIcomponentType_combobox;
  valueRef = p_valueRef;

  QVBoxLayout *comboFrame = new QVBoxLayout(this);
  comboFrame->setSpacing(5);
  comboFrame->setMargin(5);

  QLabel *nameLabel = new QLabel();
  //!!  if (name)
    nameLabel->setText(name);
  comboFrame->addWidget(nameLabel);
  nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  comboBox = new QComboBox();  
  comboBox->addItems(*itemNames);
  comboBox->setFixedSize(comboBox->sizeHint());
  comboFrame->addWidget(comboBox);
  QObject::connect(comboBox, SIGNAL(activated(int)),
		   this, SLOT(updateValue(int)));
  updateValue(value);
}

MidiComboBox::~MidiComboBox(){
}

void MidiComboBox::setMidiValue(int value) {

  if (!controllerOK)
    controllerOK = abs(getMidiValue() - value) < 4;
  else
    if (controllerOK)
      if (midiSign == 1)
	comboBox->setCurrentIndex(int((float)(comboBox->count()-1) / 127.0 * (float)value));
      else
	comboBox->setCurrentIndex(int((float)(comboBox->count()-1) / 127.0 * (float)(127-value)));
}

void MidiComboBox::updateValue(int value) {

  *valueRef = value;    
  comboBox->setCurrentIndex(value);
  emit guiComponentTouched();
}

int MidiComboBox::getMidiValue() {

  int x;  
 
  x = (int)rint(127.0 * comboBox->currentIndex() / (comboBox->count()-1));
  return(x);
}
