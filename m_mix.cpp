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
#include "m_mix.h"
#include "port.h"

M_mix::M_mix(int p_in_channels, QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_mix;
  in_channels = p_in_channels;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_MIX_WIDTH, 
              MODULE_MIX_HEIGHT + 20 + 20 * in_channels);
  gain = 1.0;
  configDialog->addSlider(0, 2, gain, "Gain", &gain);
  for (l1 = 0; l1 < in_channels; l1++) {
    qs.sprintf("In %d", l1);
    Port *audio_in_port = new Port(qs, PORT_IN, in_port_list.count(), this, synthdata);
    audio_in_port->move(0, 40 + 20 * in_port_list.count());
    audio_in_port->outTypeAcceptList.append(outType_audio);
    in_port_list.append(audio_in_port);
    portList.append(audio_in_port);
    mixer_gain[l1] = 1.0;    
    qs.sprintf("Volume %d", l1);
    configDialog->addSlider(0, 2, mixer_gain[l1], qs, &mixer_gain[l1]);
  }
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);
  port_out->move(MODULE_MIX_WIDTH - port_out->width(),
                 35 + 20 * in_channels);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Mixer %d -> 1 ID %d", in_channels, moduleID);
  configDialog->setCaption(qs);
}

M_mix::~M_mix() {
}

void M_mix::generateCycle() {

  int l1, l2, l3;
  float mixgain;

  if (!cycleReady) {
    cycleProcessing = true;

    for (l3 = 0; l3 < in_port_list.count(); l3++) inData [l3] = in_port_list.at(l3)->getinputdata();
    
    mixgain = gain * mixer_gain[0];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = mixgain * inData[0][l1][l2];     
      }
    }  
    for (l3 = 1; l3 < in_port_list.count(); l3++) {
      mixgain = gain * mixer_gain[l3];
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] += mixgain * inData[l3][l1][l2];
        }
      }
    }
    cycleProcessing = false;
    cycleReady = true;
  }
}

void M_mix::showConfigDialog() {
}
