#ifndef MIDICONTROLLERLIST_H
#define MIDICONTROLLERLIST_H

#include <qlist.h>
#include <qcollection.h>

#include "midicontroller.h"

/**
  *@author Matthias Nagorni
  */
    
class MidiControllerList : public QList<MidiController> {

  public:
    MidiControllerList();
    ~MidiControllerList();
    int compareItems(QCollection::Item item1, QCollection::Item item2 );
};
                     
#endif
                      