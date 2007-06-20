#include <qlabel.h>
#include <qlabel.h>
#include <QHBoxLayout>
#include "midipushbutton.h"

MidiPushButton::MidiPushButton(Module *parentModule, const QString &name)
                           : MidiGUIcomponent(parentModule, NULL, name) {

  componentType = GUIcomponentType_pushbutton;

  QHBoxLayout *buttonBox = new QHBoxLayout(this);
  buttonBox->setMargin(5);
  buttonBox->addStretch(0);
  pushButton = new QPushButton(name);
  buttonBox->addWidget(pushButton);
  buttonBox->addStretch(0);
  QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

MidiPushButton::~MidiPushButton() {
}

void MidiPushButton::setMidiValue(int value) {

  if (midiSign == 1) {
    if (value > 124) emit clicked();
  } else {
    if (value <= 124) emit clicked();
  }
}

void MidiPushButton::buttonClicked() {

  emit clicked();
  emit guiComponentTouched();
}
