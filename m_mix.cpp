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
  FloatParameter *fp = new FloatParameter(this,"Gain","",0.0,2.0,&gain);
  configDialog->addParameter(fp);
  for (l1 = 0; l1 < in_channels; l1++) {
    qs.sprintf("In %d", l1);
    Port *audio_in_port = new Port(qs, PORT_IN, in_port_list.count(), this, synthdata);
    audio_in_port->move(0, 40 + 20 * in_port_list.count());
    audio_in_port->outTypeAcceptList.append(outType_audio);
    in_port_list.append(audio_in_port);
    portList.append(audio_in_port);
    mixer_gain[l1] = 1.0;
    qs.sprintf("Volume %d", l1);
    fp=new FloatParameter(this, qs,"",0.0,2.0,&mixer_gain[l1]);
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

void M_mix::paintEvent(QPaintEvent *ev) {

  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(12, 20, "Mixer");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_mix::generateCycle() {

  int l1, l2, l3;
  float mixgain;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }
    for (l3 = 0; l3 < in_port_list.count(); l3++) {
      if (in_port_list.at(l3)->connectedPortList.count()) {
        in_M[l3] = (Module *)in_port_list.at(l3)->connectedPortList.at(0)->parentModule;
        if (!in_M[l3]->cycleProcessing) {
          in_M[l3]->generateCycle();
          inData[l3] = in_M[l3]->data[in_port_list.at(l3)->connectedPortList.at(0)->index];
        } else {
          inData[l3] = in_M[l3]->lastdata[in_port_list.at(l3)->connectedPortList.at(0)->index];
        }
      } else {
        in_M[l3] = NULL;
        inData[l3] = synthdata->zeroModuleData;
      }
    }
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
