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
#include "m_vcenv.h"
#include "port.h"

M_vcenv::M_vcenv(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_vcenv;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCENV_WIDTH, MODULE_VCENV_HEIGHT);
  port_M_gate = new Port("Gate", PORT_IN, 0, this, synthdata); 
  port_M_gate->move(0, 35);
  port_M_gate->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_gate);
  port_M_retrigger = new Port("Retrigger", PORT_IN, 1, this, synthdata); 
  port_M_retrigger->move(0, 55);
  port_M_retrigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_retrigger);
  port_M_attack = new Port("Attack", PORT_IN, 2, this, synthdata); 
  port_M_attack->move(0, 75);
  port_M_attack->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_attack);
  port_M_decay = new Port("Decay", PORT_IN, 3, this, synthdata); 
  port_M_decay->move(0, 95);
  port_M_decay->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_decay);
  port_M_sustain = new Port("Sustain", PORT_IN, 4, this, synthdata); 
  port_M_sustain->move(0, 115);
  port_M_sustain->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_sustain);
  port_M_release = new Port("Release", PORT_IN, 5, this, synthdata); 
  port_M_release->move(0, 135);
  port_M_release->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_release);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 155);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("VC Envelope ID %d", moduleID);
  a0 = 0.01;
  d0 = 0.3;
  s0 = 0.7;
  r0 = 0.1;
  aGain = 0.0;
  dGain = 0.0;
  sGain = 0.0;
  rGain = 0.0;
  timeScale = 1;
  decayReleaseMode = 1;
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    gate[l1] = false;
    retrigger[l1] = false;
    state[l1] = 0;
    noteActive[l1] = false;
    e[l1] = 0;
  }
  configDialog->addSlider(0, 1, a0, "Attack Offset", &a0);
  configDialog->addSlider(0, 1, d0, "Decay Offset", &d0);
  configDialog->addSlider(0, 1, s0, "Sustain Offset", &s0);
  configDialog->addSlider(0, 1, r0, "Release Offset", &r0);
  configDialog->addSlider(-1, 1, aGain, "Attack Gain", &aGain);
  configDialog->addSlider(-1, 1, dGain, "Decay Gain", &dGain);
  configDialog->addSlider(-1, 1, sGain, "Sustain Gain", &sGain);
  configDialog->addSlider(-1, 1, rGain, "Release Gain", &rGain);
  QStrList *timeScaleNames = new QStrList(true);
  timeScaleNames->append(" 0.1 s");
  timeScaleNames->append(" 1.0 s");
  timeScaleNames->append("10.0 s");
  configDialog->addComboBox(1, "Time Scale", &timeScale, timeScaleNames->count(), timeScaleNames);
  QStrList *decayReleaseModeNames = new QStrList(true);
  decayReleaseModeNames->append("Linear");
  decayReleaseModeNames->append("Exponential");
  configDialog->addComboBox(1, "Decay/Release Mode", &decayReleaseMode, decayReleaseModeNames->count(), decayReleaseModeNames);
  configDialog->setCaption(qs);
}

M_vcenv::~M_vcenv() {
}

void M_vcenv::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "VC Envelope");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_vcenv::generateCycle() {

  int l1, l2;
  double ts, tsr, tsn, tmp, c, n;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }
    if (port_M_gate->connectedPortList.count()) {
      in_M_gate = (Module *)port_M_gate->connectedPortList.at(0)->parentModule;
      if (!in_M_gate->cycleProcessing) {
        in_M_gate->generateCycle();
        gateData = in_M_gate->data[port_M_gate->connectedPortList.at(0)->index];
      } else {
        gateData = in_M_gate->lastdata[port_M_gate->connectedPortList.at(0)->index];
      }
    } else {
      in_M_gate = NULL;
      gateData = synthdata->zeroModuleData;
    }
    if (port_M_retrigger->connectedPortList.count()) {
      in_M_retrigger = (Module *)port_M_retrigger->connectedPortList.at(0)->parentModule;
      if (!in_M_retrigger->cycleProcessing) {
        in_M_retrigger->generateCycle();
        retriggerData = in_M_retrigger->data[port_M_retrigger->connectedPortList.at(0)->index];
      } else {
        retriggerData = in_M_retrigger->lastdata[port_M_retrigger->connectedPortList.at(0)->index];
      }
    } else {
      in_M_retrigger = NULL;
      retriggerData = synthdata->zeroModuleData;
    }
    if (port_M_attack->connectedPortList.count()) {
      in_M_attack = (Module *)port_M_attack->connectedPortList.at(0)->parentModule;
      if (!in_M_attack->cycleProcessing) {
        in_M_attack->generateCycle();
        attackData = in_M_attack->data[port_M_attack->connectedPortList.at(0)->index];
      } else {
        attackData = in_M_attack->lastdata[port_M_attack->connectedPortList.at(0)->index];
      }
    } else {
      in_M_attack = NULL;
      attackData = synthdata->zeroModuleData;
    }
    if (port_M_decay->connectedPortList.count()) {
      in_M_decay = (Module *)port_M_decay->connectedPortList.at(0)->parentModule;
      if (!in_M_decay->cycleProcessing) {
        in_M_decay->generateCycle();
        decayData = in_M_decay->data[port_M_decay->connectedPortList.at(0)->index];
      } else {
        decayData = in_M_decay->lastdata[port_M_decay->connectedPortList.at(0)->index];
      }
    } else {
      in_M_decay = NULL;
      decayData = synthdata->zeroModuleData;
    }
    if (port_M_sustain->connectedPortList.count()) {
      in_M_sustain = (Module *)port_M_sustain->connectedPortList.at(0)->parentModule;
      if (!in_M_sustain->cycleProcessing) {
        in_M_sustain->generateCycle();
        sustainData = in_M_sustain->data[port_M_sustain->connectedPortList.at(0)->index];
      } else {
        sustainData = in_M_sustain->lastdata[port_M_sustain->connectedPortList.at(0)->index];
      }
    } else {
      in_M_sustain = NULL;
      sustainData = synthdata->zeroModuleData;
    }
    if (port_M_release->connectedPortList.count()) {
      in_M_release = (Module *)port_M_release->connectedPortList.at(0)->parentModule;
      if (!in_M_release->cycleProcessing) {
        in_M_release->generateCycle();
        releaseData = in_M_release->data[port_M_release->connectedPortList.at(0)->index];
      } else {
        releaseData = in_M_release->lastdata[port_M_release->connectedPortList.at(0)->index];
      }
    } else {
      in_M_release = NULL;
      releaseData = synthdata->zeroModuleData;
    }
    switch(timeScale) {
      case 0: ts = 0.1;
              break;
      case 1: ts = 1.0;
              break;
      case 2: ts = 10.0; 
              break;
      default: tsr = 1.0;
    }
    tsr = ts / (double)synthdata->rate;
    tsn = ts * (double)synthdata->rate;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
//      fprintf(stderr, "l1: %d gate:%d retrigger:%d noteActive:%d state: %d e: %f\n", l1, gate[l1], retrigger[l1], noteActive[l1], state[l1], e[l1]);
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!gate[l1] && gateData[l1][l2] > 0.5) {
          gate[l1] = true;
          noteActive[l1] = true;
          state[l1] = 1;
        }
        if (gate[l1] && gateData[l1][l2] < 0.5) {
          gate[l1] = false;
          state[l1] = 4;
        }
        if (!retrigger[l1] && retriggerData[l1][l2] > 0.5) {
          retrigger[l1] = true; 
          if (gate[l1]) {
            state[l1] = 1;
          }
        }
        if (retrigger[l1] && retriggerData[l1][l2] < 0.5) {
          retrigger[l1] = false;
        }
        switch (state[l1]) {
          case 0: e[l1] = 0;
                  break;
          case 1: e[l1] += ((tmp = a0 + aGain * attackData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  if (e[l1] >= 1.0) {
                    state[l1] = 2;
                  }
                  break;
          case 2: if (decayReleaseMode) {
                    n = tsn * (d0 + dGain * decayData[l1][l2]);
                    if (n < 0) n = 0;
                    c = 2.3 / n; 
                    e[l1] *= exp(-c);            
                  } else {
                    e[l1] -= ((tmp = d0 + dGain * decayData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  }
                  if (e[l1] <= s0 + sGain * sustainData[l1][l2]) {
                    state[l1] = 3;
                  }
                  break;
          case 3: e[l1] = s0 + sGain * sustainData[l1][l2];
                  break;
          case 4: if (decayReleaseMode) {
                    n = tsn * (r0 + rGain * releaseData[l1][l2]);
                    if (n < 0) n = 0;
                    c = 2.3 / n; 
                    e[l1] *= exp(-c);      
                  } else {
                    e[l1] -= ((tmp = r0 + rGain * releaseData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  }
                  if (e[l1] <= 0.000001) {
                    e[l1] = 0;
                    state[l1] = 0;
                    noteActive[l1] = false;
                  }
                  break;
          default: e[l1] = 0;
        }
        data[0][l1][l2] = e[l1];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vcenv::showConfigDialog() {
}
