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
#include "m_vcdoubledecay.h"
#include "port.h"

M_vcdoubledecay::M_vcdoubledecay(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_vcdoubledecay;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCDOUBLEDECAY_WIDTH, MODULE_VCDOUBLEDECAY_HEIGHT);
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
  port_M_ratio = new Port("Ratio", PORT_IN, 5, this, synthdata); 
  port_M_ratio->move(0, 135);
  port_M_ratio->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_ratio);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 155);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("VC Double Decay ID %d", moduleID);
  a0 = 0.1;
  d0 = 0.1;
  s0 = 0.7;
  r0 = 0.1;
  aGain = 1.0;
  dGain = 1.0;
  sGain = 1.0;
  rGain = 1.0;
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    gate[l1] = false;
    retrigger[l1] = false;
    state[l1] = 0;
    noteActive[l1] = false;
    e[l1] = 0;
    e2[l1] = 0;
  }
  configDialog->addSlider(-8, 8, a0, "Attack Offset", &a0);
  configDialog->addSlider(-8, 8, d0, "Decay Offset", &d0);
  configDialog->addSlider(0, 1, s0, "Sustain Offset", &s0);
  configDialog->addSlider(0, 1, r0, "Ratio Offset", &r0);
  configDialog->addSlider(-8, 8, aGain, "Attack Gain", &aGain);
  configDialog->addSlider(-8, 8, dGain, "Decay Gain", &dGain);
  configDialog->addSlider(0, 1, sGain, "Sustain Gain", &sGain);
  configDialog->addSlider(0, 1, rGain, "Ratio Gain", &rGain);
  configDialog->setCaption(qs);
}

M_vcdoubledecay::~M_vcdoubledecay() {
}

void M_vcdoubledecay::generateCycle() {

  int l1, l2;
  double ts, tsr, tsn, tmp, c1, c2, n1, n, c, log2, s, astep;

  if (!cycleReady) {
    cycleProcessing = true;

    gateData = port_M_gate->getinputdata ();
    retriggerData = port_M_retrigger->getinputdata ();
    attackData = port_M_attack->getinputdata ();
    decayData = port_M_decay->getinputdata ();
    sustainData = port_M_sustain->getinputdata ();
    ratioData = port_M_ratio->getinputdata ();

    ts = 1.0;
    tsr = ts / (double)synthdata->rate;
    tsn = ts * (double)synthdata->rate;
    log2 = log(2.0);
    for (l1 = 0; l1 < synthdata->poly; l1++) {
//      fprintf(stderr, "gate:%d retrigger:%d noteActive:%d state: %d\n", gate[l1], retrigger[l1], noteActive[l1], state[l1]);
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!gate[l1] && gateData[l1][l2] > 0.5) {
          gate[l1] = true;
          noteActive[l1] = true;
          state[l1] = 1;
        }
        if (gate[l1] && gateData[l1][l2] < 0.5) {
          gate[l1] = false;
          state[l1] = 3;
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
        s = s0 + sGain * sustainData[l1][l2];
        switch (state[l1]) {
          case 0: e[l1] = 0;
                  e2[l1] = 0;
                  break;
          case 1: astep = ((tmp = synthdata->exp_table(log2 * (a0 + aGain * attackData[l1][l2]))) > 0.001) ? tsr / tmp : tsr / 0.001;
                  e[l1] += astep;
                  e2[l1] += astep;
                  if (e[l1] >= 1.0) {
                    state[l1] = 2;
                    e[l1] = 1.0;
                  }
                  if (e2[l1] >= 1.0) {
                    e2[l1] = 1.0;
                  }
                  break;
          case 2: n1 = tsn * (synthdata->exp_table(log2 * (d0 + dGain * decayData[l1][l2])));
                  if (n1 < 1) n1 = 1;
                  c1 = 2.3 / n1; 
                  c2 = c1 * (r0 + rGain * ratioData[l1][l2]);
                  if (c2 < 0) c2 = 0;
                  e[l1] *= exp(-c1);
                  e2[l1] *= exp(-c2);           
                  break;
          case 3: n = tsn * (synthdata->exp_table(log2 * (d0 + dGain * decayData[l1][l2])));
                  if (n < 1) n = 1;
                  c = 2.3 / n; 
                  e[l1] *= exp(-c);      
                  e2[l1] *= exp(-c);
                  if (e[l1] <= 0.000001) {
                    e[l1] = 0;
                    e2[l1] = 0;
                    state[l1] = 0;
                    noteActive[l1] = false;
                  }
                  break;
          default: e[l1] = 0;
                   e2[l1] = 0;
                   data[0][l1][l2] = e[l1];
        }
        data[0][l1][l2] = (1.0 - s) * e[l1] + s * e2[l1];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vcdoubledecay::showConfigDialog() {
}
