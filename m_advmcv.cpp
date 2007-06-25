#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_advmcv.h"
#include "port.h"

M_advmcv::M_advmcv(QWidget* parent) 
  : Module(M_type_advmcv, 10, parent, "Advanced MCV")
{
  QString qs;
  int l1, l2;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ADVMCV_WIDTH, MODULE_ADVMCV_HEIGHT);
  port_gate_out = new Port("Gate", PORT_OUT, 0, this);          
  port_gate_out->move(width() - port_gate_out->width(), 35);
  port_gate_out->outType = outType_audio;
  portList.append(port_gate_out);
  port_note_out = new Port("Freq", PORT_OUT, 1, this);          
  port_note_out->move(width() - port_note_out->width(), 55);
  port_note_out->outType = outType_audio;
  portList.append(port_note_out);
  port_velocity_out = new Port("Velocity", PORT_OUT, 2, this);          
  port_velocity_out->move(width() - port_velocity_out->width(), 75);
  port_velocity_out->outType = outType_audio;
  portList.append(port_velocity_out);
  port_trig_out = new Port("Trigger", PORT_OUT, 3, this);          
  port_trig_out->move(width() - port_trig_out->width(), 95);
  port_trig_out->outType = outType_audio;
  portList.append(port_trig_out);
  port_aftertouch_out = new Port("Aftertouch", PORT_OUT, 4, this);          
  port_aftertouch_out->move(width() - port_aftertouch_out->width(), 115);
  port_aftertouch_out->outType = outType_audio;
  portList.append(port_aftertouch_out);
  port_pitchbend_out = new Port("Pitchbend", PORT_OUT, 5, this);          
  port_pitchbend_out->move(width() - port_pitchbend_out->width(), 135);
  port_pitchbend_out->outType = outType_audio;
  portList.append(port_pitchbend_out);
  for (l1 = 0; l1 < MODULE_ADVMCV_CONTROLLER_PORTS; l1++) {
    qs.sprintf("Controller %d", l1);
    port_controller_out[l1] = new Port(qs, PORT_OUT, 6+l1, this);          
    port_controller_out[l1]->move(width() - port_controller_out[l1]->width(), 155 + 20 * l1);
    port_controller_out[l1]->outType = outType_audio;
    portList.append(port_controller_out[l1]);
  }

//  QStrList *channelNames = new QStrList(true);
//  channelNames->append("RESERVED FOR LATER USE");
//  for (l1 = 1; l1 < 17; l1++) {
//    qs.sprintf("RESERVED FOR LATER USE", l1);
//    channelNames->append(qs);
//  }
  channel = 0;
  pitch = 0;
  pitchbend = 0;
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    freq[l1] = 0;
    trig[l1] = 0;
    aftertouch_cv[l1] = 0;
    pitchbend_cv[l1] = 0;
    for(l2 = 0; l2 < MODULE_ADVMCV_CONTROLLER_PORTS; l2++) {
      controller_cv[l2][l1] = 0;
      controller_num[l2] = 0;
    }
  }
//  configDialog->addComboBox(0, " ", &channel, channelNames->count(), channelNames);
  configDialog->addIntSlider(-36, 36, pitch, "Note Offset", &pitch);
  configDialog->addSlider(-1, 1, pitchbend, "Pitch", &pitchbend);
  for (l1 = 0; l1 < MODULE_ADVMCV_CONTROLLER_PORTS; l1++) {
    qs.sprintf("Controller %d", l1);
    configDialog->addIntSlider(0, 127, controller_num[l1], qs, &controller_num[l1]);
  }
}

M_advmcv::~M_advmcv()
{
  synthdata->listM_advmcv.removeAll(this);
}

void M_advmcv::noteOnEvent(int osc) {

  trig[osc] = 1;
}

void M_advmcv::noteOffEvent(int) {
}

void M_advmcv::generateCycle() {

  int l1, l2, l3;
  float gate, velocity;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      gate = ((synthdata->channel[l1] == channel-1)||(channel == 0)) && (synthdata->noteCounter[l1] < 1000000);
      freq[l1] = pitchbend + float(synthdata->notes[l1]+pitch-60) / 12.0;
      velocity = (float)synthdata->velocity[l1] / 127.0;
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = gate;
        data[1][l1][l2] = freq[l1];
        data[2][l1][l2] = velocity;
        data[4][l1][l2] = aftertouch_cv[l1];
        data[5][l1][l2] = pitchbend_cv[l1];
        for (l3 = 0; l3 < MODULE_ADVMCV_CONTROLLER_PORTS; l3++) {
          data[6+l3][l1][l2] = controller_cv[l3][l1];
        }
      } 
      memset(data[3][l1], 0, synthdata->cyclesize * sizeof(float));
//      data[3][l1][0] = trig[l1];
      data[3][l1][15] = trig[l1]; // Added for interpolated input ports (e.g. m_vcenv.cpp)
      trig[l1] = 0;
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_advmcv::showConfigDialog() {
}

void M_advmcv::aftertouchEvent(int, int value, int) {

  int l1;
  
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    aftertouch_cv[l1] = (double)value / 127.0;
  }
}

void M_advmcv::controllerEvent(int, int controlNum, int value, bool) {

  int l1, l2;

//  fprintf(stderr, "controllerEvent %d %d\n", controlNum, value);
  for(l2 = 0; l2 < MODULE_ADVMCV_CONTROLLER_PORTS; l2++) {
    if (controlNum == controller_num[l2]) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        controller_cv[l2][l1] = (double)value / 127.0;
      }
    }
  }
}

void M_advmcv::pitchbendEvent(int, int value) {

  int l1;

//  fprintf(stderr, "pitchbendEvent %d\n", value);
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    pitchbend_cv[l1] = (float)value / 8192.0;
  }
}
