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
#include "m_vcenv2.h"
#include "port.h"

M_vcenv2::M_vcenv2(QWidget* parent) 
  : Module(M_type_vcenv2, 1, parent, "VC Envelope II")
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCENV2_WIDTH, MODULE_VCENV2_HEIGHT);
  port_M_gate = new Port("Gate", PORT_IN, 0, this); 
  port_M_gate->move(0, 35);
  port_M_gate->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_gate);
  port_M_retrigger = new Port("Retrigger", PORT_IN, 1, this); 
  port_M_retrigger->move(0, 55);
  port_M_retrigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_retrigger);
  port_M_attack = new Port("Attack", PORT_IN, 2, this); 
  port_M_attack->move(0, 75);
  port_M_attack->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_attack);
  port_M_decay = new Port("Decay", PORT_IN, 3, this); 
  port_M_decay->move(0, 95);
  port_M_decay->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_decay);
  port_M_sustain = new Port("Sustain", PORT_IN, 4, this); 
  port_M_sustain->move(0, 115);
  port_M_sustain->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_sustain);
  port_M_release = new Port("Release", PORT_IN, 5, this); 
  port_M_release->move(0, 135);
  port_M_release->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_release);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 155);
  port_out->outType = outType_audio;
  portList.append(port_out);

  a0 = 0.0;
  d0 = 0.0;
  s0 = 0.7;
  r0 = 0.0;
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
  }
  configDialog->addSlider(-8, 8, a0, "Attack Offset", &a0);
  configDialog->addSlider(-8, 8, d0, "Decay Offset", &d0);
  configDialog->addSlider(0, 1, s0, "Sustain Offset", &s0);
  configDialog->addSlider(-8, 8, r0, "Release Offset", &r0);
  configDialog->addSlider(-8, 8, aGain, "Attack Gain", &aGain);
  configDialog->addSlider(-8, 8, dGain, "Decay Gain", &dGain);
  configDialog->addSlider(0, 1, sGain, "Sustain Gain", &sGain);
  configDialog->addSlider(-8, 8, rGain, "Release Gain", &rGain);
}

M_vcenv2::~M_vcenv2()
{
  synthdata->listM_vcenv2.removeAll(this);
}

void M_vcenv2::generateCycle() {

  int l1, l2;
  double ts, tsr, tsn, tmp, c, n;

  if (!cycleReady) {
    cycleProcessing = true;

    gateData = port_M_gate->getinputdata ();
    retriggerData = port_M_retrigger->getinputdata ();
    attackData = port_M_attack->getinputdata ();
    decayData = port_M_decay->getinputdata ();
    sustainData = port_M_sustain->getinputdata ();
    releaseData = port_M_release->getinputdata ();

    ts = 1.0;
    tsr = ts / (double)synthdata->rate;
    tsn = ts * (double)synthdata->rate;
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
          case 1: e[l1] += ((tmp = synthdata->exp_table_ln2(a0 + aGain * attackData[l1][l2])) > 0.001) ? tsr / tmp : tsr / 0.001;
                  if (e[l1] >= 1.0) {
                    state[l1] = 2;
                    e[l1] = 1.0;
                  }
                  break;
          case 2: n = tsn * synthdata->exp_table_ln2(d0 + dGain * decayData[l1][l2]);
                  if (n < 1) n = 1;
                  c = 2.3 / n; 
                  e[l1] *= exp(-c);            
                  if (e[l1] <= s0 + sGain * sustainData[l1][l2] + 1e-20) {
                    state[l1] = 3;
                  } else {
                    break;
                  }
          case 3: e[l1] = s0 + sGain * sustainData[l1][l2];
                  break;
          case 4: n = tsn * synthdata->exp_table_ln2(r0 + rGain * releaseData[l1][l2]);
                  if (n < 1) n = 1;
                  c = 2.3 / n; 
                  e[l1] *= exp(-c);      
                  if (e[l1] <= 1e-20) {
                    e[l1] = 0;
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

