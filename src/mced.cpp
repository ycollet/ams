#include "module.h"
#include "mced.h"
#include "midicontrollable.h"


/**
  *@author Karsten Wiese
  */


MCedThing::~MCedThing()
{
  if (module) {
    QList<MidiControllableBase*>::const_iterator it;
    for (it = module->midiControllables.constBegin();
	 it != module->midiControllables.constEnd(); ++it)
      (*it)->disconnect(this);
  }
}

void MCedThing::listenTo(Module *m)
{
  module = m;
  QList<MidiControllableBase*>::const_iterator it;
  for (it = module->midiControllables.constBegin();
       it != module->midiControllables.constEnd(); ++it)
    (*it)->connectTo(this);
}

void MCedThing::listenTo(Module *m, int from)
{
  module = m;
  for (; from < module->midiControllables.count(); ++from)
    module->midiControllables.at(from)->connectTo(this);
}
