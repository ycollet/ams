#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <stdio.h>
#include <math.h>
#include <qstrlist.h>
#include "midipushbutton.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

MidiPushButton::MidiPushButton(QObject *parentModule, QWidget * parent, const char * name, SynthData *p_synthdata)
                           : MidiGUIcomponent(parentModule, p_synthdata, parent, name) {

  componentType = GUIcomponentType_pushbutton;
  setMargin(5);
  QHBox *buttonBox = new QHBox(this);
  new QWidget(buttonBox);
  pushButton = new QPushButton(name, buttonBox);  
  new QWidget(buttonBox);
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
