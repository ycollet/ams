#include "midicontroller.h"

MidiController::MidiController(){

  type = -1;
  ch = -1;
  param = -1;
}

MidiController::MidiController(int p_type, int p_ch, int p_param) {

  type = p_type;
  ch = p_ch;
  param = p_param;
}

MidiController::~MidiController(){
}

void MidiController::sendMidiValue(int value) {

  emit midiValueChanged(value);
}

void  MidiController::setListViewItem(QListViewItem *p_listViewItem) {
 
  listViewItem = p_listViewItem;
}