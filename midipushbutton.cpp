#include <qlabel.h>
#include <qlabel.h>
#include <QHBoxLayout>
#include "midipushbutton.h"
#include "midicontrollable.h"

MidiPushButton::MidiPushButton(MidiControllableDoOnce &mcAble)
  : MidiGUIcomponent(mcAble)
{
  componentType = GUIcomponentType_pushbutton;

  QHBoxLayout *buttonBox = new QHBoxLayout(this);
  buttonBox->setMargin(5);
  buttonBox->addStretch(0);
  pushButton = new QPushButton(mcAble.name);
  buttonBox->addWidget(pushButton);
  buttonBox->addStretch(0);
  QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(clicked()));
}

MidiGUIcomponent *MidiPushButton::createTwin()
{
  return new MidiPushButton(*dynamic_cast<MidiControllableDoOnce *>(&mcAble));
}

MidiPushButton::~MidiPushButton() {
}
/*
void MidiPushButton::setMidiValueRT(int value)
{
}

void MidiPushButton::setMidiValue(int value) {

  if (midiSign == 1) {
    if (value > 124) emit clicked();
  } else {
    if (value <= 124) emit clicked();
  }
}
*/
void MidiPushButton::clicked()
{

  //  emit clicked();
  //  emit guiComponentTouched();
}
