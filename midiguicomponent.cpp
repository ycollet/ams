#include <qcheckbox.h>
#include <qpushbutton.h>
#include <stdio.h>
#include "synthdata.h"
#include "midicontroller.h"
#include "midiguicomponent.h"
#include "midicontrollable.h"


MidiGUIcomponent::MidiGUIcomponent(MidiControllableBase &mcAble)
  : mcAble(mcAble)
{
  setObjectName(mcAble.name);
  nameLabel.setText(mcAble.name);
  mcAble.connectTo(this);
  controllerOK = false;
}

MidiGUIcomponent::~MidiGUIcomponent()
{
  mcAble.disconnect(this);
}


void MidiGUIcomponent::resetControllerOK() {

  controllerOK = false;
}

void MidiGUIcomponent::invalidateController() {

  emit sigResetController();
}
