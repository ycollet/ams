#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
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
#include "m_lfo.h"
#include "port.h"

M_lfo::M_lfo(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(6, parent, name, p_synthdata) {

  QString qs;
  int l1;
  long tm;

  M_type = M_type_lfo;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LFO_WIDTH, MODULE_LFO_HEIGHT);
  freq = 5;
  phi0 = 0;
  wave_period = (double)synthdata->rate / (16.0 * freq);
  for (l1 = 0; l1 < MAXPOLY; l1++) {
    trigger[l1] = false;
    si[l1] = 0;
    old_si[l1] = 0;
    sa[l1] = -1;
    old_sa[l1] = 0;
    t[l1] = 0;
    old_t[l1] = 0;
    r[l1] = 0;
    old_r[l1] = 0;
    old_sh[l1] = 0;
    sh[l1] = 0;
    state[l1] = 0;
    dt[l1] = 4.0 / wave_period;
  }
  port_M_trigger = new Port("Reset", PORT_IN, 0, this, synthdata);          
  port_M_trigger->move(0, 35);
  port_M_trigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_trigger);     
  port_sine = new Port("Sine", PORT_OUT, 0, this, synthdata);          
  port_sine->move(width() - port_sine->width(), 55);
  port_sine->outType = outType_audio;
  portList.append(port_sine);
  port_tri = new Port("Triangle", PORT_OUT, 1, this, synthdata);          
  port_tri->move(width() - port_tri->width(), 75);
  port_tri->outType = outType_audio;
  portList.append(port_tri);
  port_sawup = new Port("Saw Up", PORT_OUT, 2, this, synthdata);          
  port_sawup->move(width() - port_sawup->width(), 95);
  port_sawup->outType = outType_audio;
  portList.append(port_sawup);
  port_sawdown = new Port("Saw Down", PORT_OUT, 3, this, synthdata);          
  port_sawdown->move(width() - port_sawdown->width(), 115);
  port_sawdown->outType = outType_audio;
  portList.append(port_sawdown);
  port_rect = new Port("Rectangle", PORT_OUT, 4, this, synthdata);          
  port_rect->move(width() - port_rect->width(), 135);
  port_rect->outType = outType_audio;
  portList.append(port_rect);
  port_sh = new Port("S & H", PORT_OUT, 5, this, synthdata);          
  port_sh->move(width() - port_sh->width(), 155);
  port_sh->outType = outType_audio;
  portList.append(port_sh);
  qs.sprintf("LFO ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->addSlider(0, 100, freq, "Frequency", &freq, true);
  configDialog->addSlider(0, 6.283, phi0, "Phi0", &phi0);
  tm = time(NULL) % 1000000;
  srand(abs(tm - 10000 * (tm % 100)));
}

M_lfo::~M_lfo() {
}


void M_lfo::noteOnEvent(int osc) {

}

void M_lfo::generateCycle() {

  int l1, l2, k, len, phi0i, l2_out;
  double ldsi, ldsa, ldt, ldr, ldsh, dt0, dsa;

  if (!cycleReady)
  {
    cycleProcessing = true; 
    triggerData = port_M_trigger->getinputdata();
    
    wave_period = (double)synthdata->rate / (16.0 * freq); 
    dsa = 2.0 / wave_period;
    dt0 = 4.0 / wave_period;
    phi0i = phi0 / 6.283 * wave_period;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      len = synthdata->cyclesize;
      l2 = -1;
      l2_out = 0;
      do {
        k = (len > 24) ? 16 : len;
        l2 += k;
        if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
          trigger[l1] = true;
          t[l1] = 0;
          state[l1] = 0;
          dt[l1] = dt0;
          r[l1] = -1;
          si[l1] = 0;
        }
        if (trigger[l1] && (triggerData[l1][l2] < 0.5)) {
          trigger[l1] = false;
        }
        if (t[l1] >= 1.0) {
          state[l1] = 1;
          dt[l1] = -dt0;
        } else if (t[l1] <= -1.0) {
          state[l1] = 3;
          dt[l1] = dt0;
        } else if ((state[l1] == 1) && (t[l1] < 0)) {
          state[l1] = 2;
          r[l1] = 1;
          sh[l1] = 2.0 * (double)rand() / (double)RAND_MAX - 1.0;
        } else if ((state[l1] == 3) && (t[l1] > 0)) {
          state[l1] = 0;
          r[l1] = -1;
          sh[l1] = 2.0 * (double)rand() / (double)RAND_MAX - 1.0;
          sa[l1] = -1;
        }
        si[l1] = (state[l1] < 2) ? t[l1] * (2.0 - t[l1]) : t[l1] * (2.0 + t[l1]);
        sa[l1] += dsa;
        t[l1] += dt[l1];
        len -= k;
        ldsi = (si[l1] - old_si[l1]) / (double)k;
        ldsa = (sa[l1] - old_sa[l1]) / (double)k;
        ldsh = (sh[l1] - old_sh[l1]) / (double)k;
        ldt = (t[l1] - old_t[l1]) / (double)k;
        ldr = (r[l1] - old_r[l1]) / (double)k;
        while (k--) {
          old_si[l1] += ldsi;
          old_sa[l1] += ldsa;
          old_sh[l1] += ldsh;
          old_t[l1] += ldt;
          old_r[l1] += ldr;
          data[0][l1][l2_out] = old_si[l1];
          data[1][l1][l2_out] = old_t[l1];
          data[2][l1][l2_out] = old_sa[l1];
          data[3][l1][l2_out] = -old_sa[l1];
          data[4][l1][l2_out] = old_r[l1];
          data[5][l1][l2_out] = old_sh[l1];
          l2_out++;
        }
      } while(len);  
    }  
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_lfo::showConfigDialog() {
}
