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
#include "m_env.h"
#include "port.h"
#include "envelope.h"

M_env::M_env(QWidget* parent, const char *name, SynthData *p_synthdata)
              : Module(2, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_env;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ENV_WIDTH, MODULE_ENV_HEIGHT);
  delay = 0;
  attack = 0.05;
  hold = 0.02;
  decay = 0.1;
  sustain = 0.7;
  release = 0.05;
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
  qs.sprintf("ENV ID %d", moduleID);
  configDialog->setCaption(qs);

  mDelay = new FloatParameter(this, "Delay", "",0.0,1.0, &delay);
  mHold = new FloatParameter(this, "Hold", "",0.0,1.0, &hold);
  mTimeScale = new FloatParameter(this, "Time Scale", "",0.1,10.0, &timeScale);
  mAttack = new FloatParameter(this, "Attack", "",0.0,1.0, &attack);
  mDecay = new FloatParameter(this, "Decay", "",0.0,1.0, &decay);
  mSustain = new FloatParameter(this, "Sustain", "",0.0,1.0, &sustain);
  mRelease = new FloatParameter(this, "Release", "",0.0,1.0, &release);

  Envelope * env = new Envelope(mDelay,mAttack,mHold,mDecay,mSustain,mRelease,configDialog->headerBox);
  qs="ADSR";
  configDialog->addParameter(mAttack,qs);
  configDialog->addParameter(mDecay,qs);
  configDialog->addParameter(mSustain,qs);
  configDialog->addParameter(mRelease,qs);
  qs="Time Scale / Delay / Hold";
  configDialog->addParameter(mTimeScale,qs);
  configDialog->addParameter(mDelay,qs);
  configDialog->addParameter(mHold,qs);

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

M_env::~M_env() {

}

void M_env::paintEvent(QPaintEvent *ev) {

  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "ENV");
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_env::noteOnEvent(int osc) {

}

void M_env::noteOffEvent(int osc) {

}

void M_env::generateCycle() {

  int l1, l2, status;
  float tscale, de_attack, de_decay, de_release;
  float a, dl, dc, h, dla, dlah, dlahdc;

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
    tscale = timeScale * (float)synthdata->rate;
    de_attack = (attack > 0) ? 1.0 / (attack * tscale) : 0;
    de_decay = (decay > 0) ? (1.0 - sustain) / (decay * tscale) : 0;
    a = tscale * attack;    
    dl = tscale * delay;    
    h = tscale * hold;         
    dc = tscale * decay;    
    dla = dl + a;    
    dlah = dla + h;    
    dlahdc = dlah + dc;    
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!gate[l1] && (gateData[l1][l2] > 0.5)) {
          gate[l1] = true;
          noteActive[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (gate[l1] && (gateData[l1][l2] < 0.5)) {
          gate[l1] = false;
          noteOffOfs[l1] = 0;
          e_noteOff[l1] = e[l1];
        }
        if (!retrigger[l1] && (retriggerData[l1][l2] > 0.5)) { 
          retrigger[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ENVELOPE_RESPONSE;
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
            if (noteOnOfs[l1] >= int(dl)) status = 2; 
            if (noteOnOfs[l1] >= int(dla)) status = 3;
            if (noteOnOfs[l1] >= int(dlah)) status = 4;
            if (noteOnOfs[l1] >= int(dlahdc)) status = 5; 
            switch (status) {
              case 0: e[l1] -= de[l1];
                      break;
              case 1: e[l1] = 0;
                      break;
              case 2: e[l1] += de_attack;
                      break;
              case 3: e[l1] = 1.0;
                      break;
              case 4: e[l1] -= de_decay;
                      break;
              case 5: e[l1] = sustain;
                      break;
              default: e[l1] = 0;
                       break;
            }
            if (e[l1] < 0) e[l1] = 0;
            data[0][l1][l2] = e[l1];
            data[1][l1][l2] = -e[l1];
            noteOnOfs[l1]++;
        } else {                          // Release      
          de_release = (release > 0) ? e_noteOff[l1] / (release * tscale) : 0;
          e[l1] -= de_release;
          noteOffOfs[l1]++;
          if (noteOffOfs[l1] >= int(tscale * release)) {
            noteActive[l1] = false;
          }
          if ((release == 0) || (e[l1] < 0)) {
            e[l1] = 0;
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

void M_env::showConfigDialog() {
}
