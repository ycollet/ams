#include <qcollection.h>
#include "midicontrolcenter.h"
#include "midicontrollerlist.h"


MidiControllerList::MidiControllerList(){
}

MidiControllerList::~MidiControllerList(){
}

int MidiControllerList::compareItems(QCollection::Item item1, QCollection::Item item2) {
    if(*(MidiController *)item1 ==*(MidiController *)item2){
      return(0);
    } else {
      return(1);
    }
}
