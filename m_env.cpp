#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <alsa/asoundlib.h>
#include "envelope.h"
#include "synthdata.h"
#include "midislider.h"
#include "m_env.h"
#include "port.h"

M_env::M_env(QWidget* parent) 
  : Module(M_type_env, 2, parent, "ENV")
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ENV_WIDTH, MODULE_ENV_HEIGHT);
  delay = 0;
  attack = 0.05;
  hold = 0.02;
  decay = 0.1;
  sustain = 0.7;
  release = 0.05;
  timeScale = 1.0;
  port_gate = new Port("Gate", PORT_IN, 0, this);
  port_gate->move(0, 35);
  port_gate->outTypeAcceptList.append(outType_audio);
  portList.append(port_gate);
  port_retrigger = new Port("Retrigger", PORT_IN, 1, this);
  port_retrigger->move(0, 55);
  port_retrigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_retrigger);
  port_gain_out = new Port("Out", PORT_OUT, 0, this);          
  port_gain_out->move(width() - port_gain_out->width(), 75);
  port_gain_out->outType = outType_audio;
  portList.append(port_gain_out);
  port_inverse_out = new Port("Inverse Out", PORT_OUT, 1, this);          
  port_inverse_out->move(width() - port_inverse_out->width(), 95);
  port_inverse_out->outType = outType_audio;
  portList.append(port_inverse_out);

  configDialog->addEnvelope(&delay, &attack, &hold, &decay, &sustain, &release);
  configDialog->initTabWidget();
  QVBoxLayout *adsrTab = configDialog->addVBoxTab("ADSR");
  QVBoxLayout *delayTab = configDialog->addVBoxTab("Delay / Hold / Time Scale");
  configDialog->addSlider(0, 1, delay, "Delay", &delay, false, delayTab);
  configDialog->addSlider(0, 1, attack, "Attack", &attack, false, adsrTab);
  configDialog->addSlider(0, 1, hold, "Hold", &hold, false, delayTab);
  configDialog->addSlider(0, 1, decay, "Decay", &decay, false, adsrTab);
  configDialog->addSlider(0, 1, sustain, "Sustain", &sustain, false, adsrTab);
  configDialog->addSlider(0, 1, release, "Release", &release, false, adsrTab);
  configDialog->addSlider(0.1, 10, timeScale, "Time Scale", &timeScale, false, delayTab);

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); l1++) {
    QObject::connect(configDialog->midiSliderList.at(l1), SIGNAL(valueChanged(int)), 
                     configDialog->envelopeList.at(0), SLOT(updateEnvelope(int)));
  }
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

M_env::~M_env()
{
  synthdata->listM_env.removeAll(this);
}

void M_env::generateCycle() {

  int l1, l2, status;
  float tscale, de_attack, de_decay, de_release;
  float a, dl, dc, h, dla, dlah, dlahdc;

  if (!cycleReady) {
    cycleProcessing = true;

    gateData = port_gate->getinputdata();
    retriggerData = port_retrigger->getinputdata();

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
            noteOnOfs[l1] = (de_attack > 0) ? e[l1] / de_attack : 0;
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

