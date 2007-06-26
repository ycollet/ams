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
#include "m_mcv.h"
#include "port.h"

M_mcv::M_mcv(QWidget* parent) 
  : Module(M_type_mcv, 4, parent, "MCV")
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_MCV_WIDTH, MODULE_MCV_HEIGHT);
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

  QStringList channelNames;
  channelNames << "RESERVED FOR LATER USE";
  for (l1 = 1; l1 < 17; l1++) {
    qs.sprintf("RESERVED FOR LATER USE");
    channelNames << qs;
  }
  channel = 0;
  pitch = 0;
  pitchbend = 0;
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    freq[l1] = 0;
    trig[l1] = 0;
  }
  configDialog->addComboBox(0, " ", &channel, channelNames.count(), &channelNames);
  configDialog->addIntSlider(-36, 36, pitch, "Note Offset", &pitch);
  configDialog->addSlider(-1, 1, pitchbend, "Pitch", &pitchbend);
}

M_mcv::~M_mcv()
{
  synthdata->listM_mcv.removeAll(this);
}

void M_mcv::noteOnEvent(int osc) {

  trig[osc] = 1;
}

void M_mcv::noteOffEvent(int) {

}

void M_mcv::generateCycle() {

  int l1, l2;
  float gate, velocity;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      gate = ((synthdata->channel[l1] == channel-1)||(channel == 0)) && (synthdata->noteCounter[l1] < 1000000);
      freq[l1] = pitchbend + float(synthdata->notes[l1]+pitch-60) / 12.0;
//      if (freq[l1] < 0) freq[l1] = 0;
      velocity = (float)synthdata->velocity[l1] / 127.0;
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = gate;
        data[1][l1][l2] = freq[l1];
        data[2][l1][l2] = velocity;
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

