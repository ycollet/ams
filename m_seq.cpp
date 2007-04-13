#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_seq.h"
#include "port.h"

M_seq::M_seq(int p_seqLen, QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(4, parent, name, p_synthdata) {

  QString qs;
  char str[1024];
  int l1;
  QVBox *pitchTab[4], *gateTab[4], *velocityTab[4];

  M_type = M_type_seq;
  seqLen = p_seqLen;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SEQ_WIDTH, MODULE_SEQ_HEIGHT);
  port_trigger = new Port("Trigger", PORT_IN, 0, this, synthdata);
  port_trigger->move(0, 35);
  port_trigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_trigger);
  port_gate_out = new Port("Gate", PORT_OUT, 0, this, synthdata);          
  port_gate_out->move(width() - port_gate_out->width(), 55);
  port_gate_out->outType = outType_audio;
  portList.append(port_gate_out);
  port_note_out = new Port("Freq", PORT_OUT, 1, this, synthdata);          
  port_note_out->move(width() - port_note_out->width(), 75);
  port_note_out->outType = outType_audio;
  portList.append(port_note_out);
  port_velocity_out = new Port("Velocity", PORT_OUT, 2, this, synthdata);          
  port_velocity_out->move(width() - port_velocity_out->width(), 95);
  port_velocity_out->outType = outType_audio;
  portList.append(port_velocity_out);
  port_trigger_out = new Port("Trigger", PORT_OUT, 3, this, synthdata);          
  port_trigger_out->move(width() - port_trigger_out->width(), 115);
  port_trigger_out->outType = outType_audio;
  portList.append(port_trigger_out);
  qs.sprintf("SEQ ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->initTabWidget();
  QVBox *generalTab = new QVBox(configDialog->tabWidget);
  int seqLen_8 = (seqLen + 7) / 8;
  for (l1 = 0; l1 < seqLen_8; l1++) {
    pitchTab[l1] = new QVBox(configDialog->tabWidget);
    gateTab[l1] = new QVBox(configDialog->tabWidget);
    velocityTab[l1] = new QVBox(configDialog->tabWidget);
  }
  seq_gate = 0;
  seq_freq = 0;
  seq_velocity = 0;
  seq_pos = 0;
  tick = 0;
  osc = 0;
  trigger = false;
  triggerCount = 0;
  triggerOut = false;
  bpm = 120;
  pitch_ofs = 32;
  updateTimerFlag = false;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(nextStep()));
  configDialog->addIntSlider(0, 63, pitch_ofs, "Pitch Offset", &pitch_ofs, generalTab);
  configDialog->addIntSlider(3, 300, bpm, "Beats per minute", &bpm, generalTab);
  QStrList *noteLenNames = new QStrList(true);
  noteLenNames->append("1");
  noteLenNames->append("3/4");
  noteLenNames->append("1/2");
  noteLenNames->append("1/4");
  configDialog->addComboBox(1, "Gate time", (int *)&note_len, noteLenNames->count(), noteLenNames, generalTab);
  for (l1 = 0; l1 < seqLen; l1++) {
    pitch[l1] = 31;
    velocity[l1] = 63;
    gate[l1] = 0;
  }
  for (l1 = 0; l1 < seqLen; l1++) {
    sprintf(str, "Gate %d", l1);
    configDialog->addCheckBox(gate[l1], str, &gate[l1], gateTab[l1 / 8]);
    sprintf(str, "Pitch %d", l1);
    configDialog->addIntSlider(0, 64, pitch[l1], str, &pitch[l1], pitchTab[l1 / 8]);
    sprintf(str, "Velocity %d", l1);
    configDialog->addIntSlider(0, 127, velocity[l1], str, &velocity[l1], velocityTab[l1 / 8]);
  } 
  configDialog->addTab(generalTab, "Pitch Offset / Tempo / Gate Time");
  for (l1 = 0; l1 < seqLen_8; l1++) {
    qs.sprintf("Pitch %d", l1);
    configDialog->addTab(pitchTab[l1], qs);
  }
  for (l1 = 0; l1 < seqLen_8; l1++) {
    qs.sprintf("Gate %d", l1);
    configDialog->addTab(gateTab[l1], qs);
  }
  for (l1 = 0; l1 < seqLen_8; l1++) {
    qs.sprintf("Velocity %d", l1);
    configDialog->addTab(velocityTab[l1], qs);
  }
  QObject::connect(configDialog->intMidiSliderList.at(1), SIGNAL(valueChanged(int)),
                   this, SLOT(updateTimer(int)));
  timer->start(int(3750.0 / (float)bpm), false);
}

M_seq::~M_seq() {

}

void M_seq::noteOnEvent(int osc) {

}

void M_seq::noteOffEvent(int osc) {

}

void M_seq::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;

    triggerData = port_trigger->getinputdata ();

    if (triggerCount) {
      triggerCount--;
    } else {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!trigger && (triggerData[0][l2] > 0.5)) {
          trigger = true;
          triggerCount = 32;
        }
      }
    }
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      data[3][0][l2] = (triggerOut) ? 1.0 : 0;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (l1 == osc) {
          data[0][l1][l2] = seq_gate;
          data[1][l1][l2] = seq_freq;
          data[2][l1][l2] = seq_velocity;
        } else {
          data[0][l1][l2] = 0;
        }
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_seq::nextStep() {

  int len, l2;
  long noteCount;

  if (updateTimerFlag) {
    timer->stop();
    timer->start(int(3750.0 / (float)bpm), false);
    updateTimerFlag = false;
  }
  if (trigger) {
    tick = 0;
    seq_pos = 0;
    trigger = false;
  }
  if (seq_pos == 0) {
    triggerOut = true;
  } else {
    triggerOut = false;
  }
  len = 4 - note_len;
  if (tick == 0) {
    seq_freq = float(pitch[seq_pos] + pitch_ofs) / 12.0;

// Search for next free voice    
    osc = -1;
    noteCount = 0;
    for (l2 = 0; l2 < synthdata->poly; l2++) {
      if (synthdata->noteCounter[l2] > noteCount) {
        noteCount = synthdata->noteCounter[l2];
        osc = l2;
      }
    }  
    
    seq_gate = (osc < 0) ? 0 : (float)gate[seq_pos];
    seq_velocity = float(velocity[seq_pos]) / 127.0;
    seq_pos++;
    if (seq_pos >= seqLen) {
      seq_pos = 0;
    }
  } 
  if (tick == len) {
    seq_gate = 0;
  }
  tick++;
  if (tick >= 4) {
    tick = 0;
  }
}

void M_seq::showConfigDialog() {
}

void M_seq::updateTimer(int p_bpm) {

  bpm = p_bpm;
  updateTimerFlag = true;
}
