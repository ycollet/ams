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
    old_e[l1] = 0;
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

void M_vcenv::generateCycle() {

  int l1, l2, l2_out;
  double ts, tsr, tsn, tmp, c, n, de;
  int k, len;

  if (!cycleReady) {
    cycleProcessing = true;

    gateData = port_M_gate->getinputdata ();
    retriggerData = port_M_retrigger->getinputdata ();
    attackData = port_M_attack->getinputdata ();
    decayData = port_M_decay->getinputdata ();
    sustainData = port_M_sustain->getinputdata ();
    releaseData = port_M_release->getinputdata ();

    switch(timeScale) {
      case 0: ts = 0.1;
              break;
      case 1: ts = 1.0;
              break;
      case 2: ts = 10.0; 
              break;
      default: tsr = 1.0;
    }
    tsr = 16.0 * ts / (double)synthdata->rate;
    tsn = ts * (double)synthdata->rate / 16.0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
//      fprintf(stderr, "gate:%d retrigger:%d noteActive:%d state: %d\n", gate[l1], retrigger[l1], noteActive[l1], state[l1]);
      len = synthdata->cyclesize;
      old_e[l1] = e[l1];
      l2 = -1;
      l2_out = 0;
      do {
        k = (len > 24) ? 16 : len;
        l2 += k;
        len -= k;
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
                    e[l1] = 1.0;
                  }
                  break;
          case 2: if (decayReleaseMode) {
                    n = tsn * (d0 + dGain * decayData[l1][l2]);
                    if (n < 1) n = 1;
                    c = 2.3 / n; 
                    e[l1] *= exp(-c);            
                  } else {
                    e[l1] -= ((tmp = d0 + dGain * decayData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  }
                  if (e[l1] <= s0 + sGain * sustainData[l1][l2] + 1e-20) {
                    state[l1] = 3;
                  } else {
                    break;
                  }
          case 3: e[l1] = s0 + sGain * sustainData[l1][l2];
                  break;
          case 4: if (decayReleaseMode) {
                    n = tsn * (r0 + rGain * releaseData[l1][l2]);
                    if (n < 1) n = 1;
                    c = 2.3 / n; 
                    e[l1] *= exp(-c);      
                  } else {
                    e[l1] -= ((tmp = r0 + rGain * releaseData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  }
                  if (e[l1] <= 1e-20) {
                    e[l1] = 0;
                    state[l1] = 0;
                    noteActive[l1] = false;
                  }
                  break;
          default: e[l1] = 0;
        }
        de = (e[l1] - old_e[l1]) / (double)k;
        while (k--) {
          old_e[l1] += de;
          data[0][l1][l2_out++] = old_e[l1];
        }   
      } while(len);
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vcenv::showConfigDialog() {
}
