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
#include "m_advenv.h"
#include "port.h"
#include "multi_envelope.h"

M_advenv::M_advenv(QWidget* parent, const char *name, SynthData *p_synthdata)
              : Module(2, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_advenv;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ADVENV_WIDTH, MODULE_ADVENV_HEIGHT);
  attack[0] = 0;
  attack[1] = 0.05;
  attack[2] = 0.5;
  attack[3] = 0.05;
  attack[4] = 1.0;
  attack[5] = 0.05;
  attack[6] = 0.9;
  attack[7] = 0.1;
  sustain = 0.7;
  release[0] = 0.05;
  release[1] = 0.5;
  release[2] = 0.05;
  release[3] = 0.2;
  release[4] = 0.05;
  timeScale = 1.0;
  port_gate = new Port("Gate", PORT_IN, 0, this, synthdata);
  port_gate->move(0, 35);
  port_gate->outTypeAcceptList.append(outType_audio);
  portList.append(port_gate);
  port_retrigger = new Port("Retrigger", PORT_IN, 1, this, synthdata);
  port_retrigger->move(0, 55);
  port_retrigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_retrigger);
  port_gain_out = new Port("Out", PORT_OUT, 0, this, synthdata);
  port_gain_out->move(width() - port_gain_out->width(), 75);
  port_gain_out->outType = outType_audio;
  portList.append(port_gain_out);
  port_inverse_out = new Port("Inverse Out", PORT_OUT, 1, this, synthdata);
  port_inverse_out->move(width() - port_inverse_out->width(), 95);
  port_inverse_out->outType = outType_audio;
  portList.append(port_inverse_out);
  qs.sprintf("Advanced ENV ID %d", moduleID);
  configDialog->setCaption(qs);

  MultiEnvelope *menv = new MultiEnvelope(1, &timeScale, &attack[0], &sustain, &release[0],configDialog->headerBox);

  FloatParameter * p;
  qs.sprintf("Time Scale / Sustain / Delay");
  p = new FloatParameter(this,"Time Scale","",0.1,10,&timeScale);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Sustain", "",0.0,1.0, &sustain);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Delay", "",0.0,1.0, &attack[0]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

  qs.sprintf("Attack Time");
  p = new FloatParameter(this, "Attack Time 0", "",0.0,1.0, &attack[1]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Attack Time 1", "",0.0,1.0, &attack[3]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Attack Time 2", "",0.0,1.0, &attack[5]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Attack Time 3", "",0.0,1.0, &attack[7]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

  qs.sprintf("Attack Level");
  p = new FloatParameter(this, "Attack Level 0", "",0.0,1.0, &attack[2]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Attack Level 1", "",0.0,1.0, &attack[4]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Attack Level 2", "",0.0,1.0, &attack[6]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

  qs.sprintf("Release Time");
  p = new FloatParameter(this, "Release Time 0", "",0.0,1.0, &release[0]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Release Time 1", "",0.0,1.0, &release[2]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Release Time 2", "",0.0,1.0, &release[4]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

  qs.sprintf("Release Level");
  p = new FloatParameter(this, "Release Level 0", "",0.0,1.0, &release[1]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  p = new FloatParameter(this, "Release Level 1", "",0.0,1.0, &release[3]);
  configDialog->addParameter(p,qs);
  connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    noteActive[l1] = false;
    gate[l1] = false;
    retrigger[l1] = false;
    noteOnOfs[l1] = 0;
    noteOffOfs[l1] = 0;
    e[l1] = 0;
    de[l1] = 0;
  }
}

M_advenv::~M_advenv() {

}

void M_advenv::paintEvent(QPaintEvent *ev) {

  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "Advanced ENV");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_advenv::noteOnEvent(int osc) {

}

void M_advenv::noteOffEvent(int osc) {

}

void M_advenv::generateCycle() {

  int l1, l2, status;
  float tscale, de_a[4], de_d[3];
  float t[8];

  if (!cycleReady) {
    cycleProcessing = true;
    for (l2 = 0; l2 < 2; l2++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[l2][l1], data[l2][l1], synthdata->cyclesize * sizeof(float));
      }
    }
    if (port_gate->connectedPortList.count()) {
      in_M_gate = (Module *)port_gate->connectedPortList.at(0)->parentModule;
      if (!in_M_gate->cycleProcessing) {
        in_M_gate->generateCycle();
        gateData = in_M_gate->data[port_gate->connectedPortList.at(0)->index];
      } else {
        gateData = in_M_gate->lastdata[port_gate->connectedPortList.at(0)->index];
      }
    } else {
      in_M_gate = NULL;
      gateData = synthdata->zeroModuleData;
    }
    if (port_retrigger->connectedPortList.count()) {
      in_M_retrigger = (Module *)port_retrigger->connectedPortList.at(0)->parentModule;
      if (!in_M_retrigger->cycleProcessing) {
        in_M_retrigger->generateCycle();
        retriggerData = in_M_retrigger->data[port_retrigger->connectedPortList.at(0)->index];
      } else {
        retriggerData = in_M_retrigger->lastdata[port_retrigger->connectedPortList.at(0)->index];
      }
    } else {
      in_M_retrigger = NULL;
      retriggerData = synthdata->zeroModuleData;
    }
    tscale = timeScale * synthdata->rate;
    de_a[0] = (attack[1] > 0) ? attack[2] / (tscale * attack[1]) : 0;
    de_a[1] = (attack[3] > 0) ? (attack[4] - attack[2]) / (tscale * attack[3]) : 0;
    de_a[2] = (attack[5] > 0) ? (attack[6] - attack[4]) / (tscale * attack[5]) : 0;
    de_a[3] = (attack[7] > 0) ? (sustain - attack[6]) / (tscale * attack[7]) : 0;
    de_d[0] = (release[0] > 0) ? (release[1] - sustain) / (tscale * release[0]) : 0;
    de_d[1] = (release[2] > 0) ? (release[3] - release[1]) / (tscale * release[2]) : 0;
    de_d[2] = (release[4] > 0) ? - release[3] / (tscale * release[4]) : 0;
    t[0] = tscale * attack[0];
    t[1] = t[0] + tscale * attack[1];
    t[2] = t[1] + tscale * attack[3];
    t[3] = t[2] + tscale * attack[5];
    t[4] = t[3] + tscale * attack[7];
    t[5] = tscale * release[0];
    t[6] = t[5] + tscale * release[2];
    t[7] = t[6] + tscale * release[4];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!gate[l1] && (gateData[l1][l2] > 0.5)) {
          gate[l1] = true;
          noteActive[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ADVENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ADVENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (gate[l1] && (gateData[l1][l2] < 0.5)) {
          gate[l1] = false;
          noteOffOfs[l1] = 0;
          e_noteOff[l1] = e[l1];
          de_release[l1] = (release[0] > 0) ? (release[1] - e_noteOff[l1]) / (tscale * release[0]) : 0;
  //        fprintf(stderr, "de_release[%d]: %f, e_noteOff: %f\n", l1, de_release[l1], e_noteOff[l1]);
        }
        if (!retrigger[l1] && (retriggerData[l1][l2] > 0.5)) { 
          retrigger[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ADVENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ADVENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (retrigger[l1] && (retriggerData[l1][l2] < 0.5)) {
          retrigger[l1] = false;
        }
        if (gate[l1]) {
            status = 1;
            if (noteOnOfs[l1] < 0) status = 0;
            if (noteOnOfs[l1] >= long(t[0])) status = 2; 
            if (noteOnOfs[l1] >= long(t[1])) status = 3;
            if (noteOnOfs[l1] >= long(t[2])) status = 4;
            if (noteOnOfs[l1] >= long(t[3])) status = 5;
            if (noteOnOfs[l1] >= long(t[4])) status = 6;
            switch (status) {
              case 0: e[l1] -= de[l1];
                      break;
              case 1: e[l1] = 0;
                      break;
              case 2: e[l1] += de_a[0];
                      break;
              case 3: e[l1] += de_a[1];
                      break;
              case 4: e[l1] += de_a[2];
                      break;
              case 5: e[l1] += de_a[3];
                      break;
              case 6: e[l1] = sustain;
                      break;
              default: e[l1] = 0;
                       break;
            }
            if (e[l1] < 0) e[l1] = 0;
            data[0][l1][l2] = e[l1];
            data[1][l1][l2] = -e[l1];
            noteOnOfs[l1]++;
        } else {                          // Release      
          if (noteActive[l1]) {
            status = 1;
            if (noteOffOfs[l1] < 0) status = 0;
            if (noteOffOfs[l1] >= long(t[5])) status = 2;
            if (noteOffOfs[l1] >= long(t[6])) status = 3;
            if (noteOffOfs[l1] >= long(t[7])) status = 4;
            switch (status) {
              case 0: e[l1] = 0;
                      break;
              case 1: e[l1] += de_release[l1];
                      break;
              case 2: e[l1] += de_d[1];
                      break;
              case 3: e[l1] += de_d[2];
                      break;
              case 4: e[l1] = 0;
                      break;
              default: e[l1] = 0;
                     break;
            }
            if (e[l1] < 0) {
//              fprintf(stderr, "status: %d e[%d] < 0: %f\n", status, l1, e[l1]);
              e[l1] = 0;
            }
            noteOffOfs[l1]++;
            if (noteOffOfs[l1] >= int(t[7])) {
//              fprintf(stderr, "noteOffOfs[%d] = %d >= t[7] = %f; e[%d] = %f\n", l1, noteOffOfs[l1], t[7], l1, e[l1]);
              noteActive[l1] = false;
            }
          }
          data[0][l1][l2] = e[l1];
          data[1][l1][l2] = -e[l1];
        }
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_advenv::showConfigDialog() {
}
