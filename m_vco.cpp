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
#include "m_vco.h"
#include "port.h"

M_vco::M_vco(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(5, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_vco;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCO_WIDTH, MODULE_VCO_HEIGHT);
  wave_period = (float)WAVE_PERIOD;
  freq = 0;
  vcoExpFMGain = 0;
  vcoLinFMGain = 0;
  pwGain = 0;
  phi0 = 0;
  harmonic = 1;
  subharmonic = 1;
  octave = 3;
  edge = 0.95;
  for (l1 = 0; l1 < MAXPOLY; ++l1) {
    phi[l1] = 0;
  }
  pw0 = 0.5;
  waveForm = WAVE_SAW;
  port_M_freq = new Port("Freq", PORT_IN, 1, this, synthdata);
  port_M_freq->move(0, 35);    
  port_M_freq->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_freq); 
  port_M_exp = new Port("Exp. FM", PORT_IN, 2, this, synthdata);
  port_M_exp->move(0, 55);    
  port_M_exp->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_exp); 
  port_M_lin = new Port("Lin. FM", PORT_IN, 3, this, synthdata);
  port_M_lin->move(0, 75);
  port_M_lin->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_lin);
  port_M_pw = new Port("PW", PORT_IN, 4, this, synthdata);
  port_M_pw->move(0, 95);
  port_M_pw->outTypeAcceptList.append(outType_audio); 
  portList.append(port_M_pw);     
  port_sine = new Port("Sine", PORT_OUT, 0, this, synthdata);          
  port_sine->move(width() - port_sine->width(), 115);
  port_sine->outType = outType_audio;
  portList.append(port_sine);
  port_tri = new Port("Triangle", PORT_OUT, 1, this, synthdata);          
  port_tri->move(width() - port_tri->width(), 135);
  port_tri->outType = outType_audio;
  portList.append(port_tri);
  port_saw = new Port("Saw", PORT_OUT, 2, this, synthdata);          
  port_saw->move(width() - port_saw->width(), 155);
  port_saw->outType = outType_audio;
  portList.append(port_saw);
  port_rect = new Port("Rectangle", PORT_OUT, 3, this, synthdata);          
  port_rect->move(width() - port_rect->width(), 175);
  port_rect->outType = outType_audio;
  portList.append(port_rect);
  port_aux = new Port("Aux", PORT_OUT, 4, this, synthdata);          
  port_aux->move(width() - port_aux->width(), 195);
  port_aux->outType = outType_audio;
  portList.append(port_aux);
  qs.sprintf("VCO ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->initTabWidget();
  QVBox *freqTab = new QVBox(configDialog->tabWidget);
  configDialog->addIntSlider(0, 6, octave, "Octave", &octave, freqTab);
  configDialog->addSlider(0, 1, freq, "Tune", &freq, false, freqTab);
  configDialog->addIntSlider(1, 16, harmonic, "Harmonic", &harmonic, freqTab);
  configDialog->addIntSlider(1, 16, subharmonic, "Subharmonic", &subharmonic, freqTab);
  configDialog->addTab(freqTab, "Frequency");
  QVBox *pulseTab = new QVBox(configDialog->tabWidget);
  configDialog->addSlider(0.1, 0.9, pw0, "PW", &pw0, false, pulseTab);
  configDialog->addSlider(0, 1, pwGain, "PW Gain", &pwGain, false, pulseTab);
  configDialog->addSlider(0, 6.283, phi0, "Phi0", &phi0, false, pulseTab);
  configDialog->addTab(pulseTab, "Pulse Width / Phase");
  QVBox *modulationTab = new QVBox(configDialog->tabWidget);
  QStrList *waveFormNames = new QStrList(true);
  waveFormNames->append("Saw");
  waveFormNames->append("Saw 1");
  waveFormNames->append("Saw 2");
  configDialog->addComboBox(0, "Aux Wave Form", (int *)&waveForm, waveFormNames->count(), waveFormNames, modulationTab);
  configDialog->addSlider(0, 10, vcoExpFMGain, "Exp. FM Gain", &vcoExpFMGain, false, modulationTab);
  configDialog->addSlider(0, 10, vcoLinFMGain, "Lin. FM Gain", &vcoLinFMGain, false, modulationTab);
  configDialog->addTab(modulationTab, "Modulation / Aux Waveform");
}

M_vco::~M_vco() {
}

void M_vco::noteOnEvent(int osc) {
}

void M_vco::generateCycle() {

  int l1, l2;
  float dphi, phi1, phi_const, pw, d, dd, dsaw, half_wave, third_wave, edge; 
  float freq_const, freq_tune, gain_linfm, log2, pw_low, pw_high, wave_period_2;

  if (!cycleReady) {
    cycleProcessing = true; 
    log2 = log(2.0);
    edge = 0.01 + 1.8 * synthdata->edge;
    wave_period_2 = wave_period / 2.0;

    freqData = port_M_freq->getinputdata();
    expFMData = port_M_exp->getinputdata();
    linFMData = port_M_lin->getinputdata();
    pwData = port_M_pw->getinputdata();

      freq_const = wave_period / (float)synthdata->rate * (float)harmonic / (float)subharmonic;
      freq_tune = 4.0313842 + octave + freq;
      gain_linfm = 1000.0 * vcoLinFMGain;
      phi_const = phi0 * wave_period / (2.0 * M_PI);
      pw_low = 0.1 * wave_period;
      pw_high = 0.9 * wave_period;

      if (phi0 > 0) {
        for (l1 = 0; l1 < synthdata->poly; ++l1) {
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
            dphi = freq_const * (synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcoExpFMGain * expFMData[l1][l2])) 
                                 + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) dphi = wave_period_2;
            phi1 = phi[l1] + phi_const;
            if (phi1 < 0) phi1 += wave_period;
            else if (phi1 > wave_period) phi1 -= wave_period;
            data[0][l1][l2] = synthdata->wave_sine[(int)phi1];
            data[1][l1][l2] = synthdata->wave_tri[(int)phi1];
            switch (waveForm) {
              case WAVE_SAW: 
                data[4][l1][l2] = synthdata->wave_saw2[(int)phi1];
                break;
              case WAVE_SAW2:
                half_wave = wave_period * 0.5;// / 2.0;              
                data[4][l1][l2] = (phi1 < half_wave) 
                                ? synthdata->wave_saw2[(int)(2.0 * phi1)]
                                : 0;
                break;
              case WAVE_SAW3: 
                third_wave = wave_period / 3.0;
                data[4][l1][l2] = (phi1 < third_wave)
                                ? synthdata->wave_saw2[(int)(3.0 * phi1)]
                                : 0;
                break;
            }
            pw = (pw0 + pwGain * pwData[l1][l2]) * wave_period;
            if (pw < pw_low) pw = pw_low;
            else if (pw > pw_high) pw = pw_high;
            d = edge * dphi;
            dd = 1.0 / d;
            dsaw = 2.0 / (wave_period - 2.0 * d);
            if (phi1 <= d) {
              data[3][l1][l2] = phi1 * dd;
              data[2][l1][l2] = phi1 * dd;
            } else {
              if (phi1 <= pw - d) {
                data[3][l1][l2] = 1.0;
                data[2][l1][l2] = 1.0 - (phi1 - d) * dsaw;
              } else { 
                if (phi1 <= pw + d) {
                  data[3][l1][l2] = (pw - phi1) * dd;
                  data[2][l1][l2] = 1.0 - (phi1 - d) * dsaw;
                } else {
                  if (phi1 <= wave_period - d) {
                    data[3][l1][l2] = -1.0;
                    data[2][l1][l2] = 1.0 - (phi1 - d) * dsaw;
                  } else {
                    data[3][l1][l2] = (phi1 - wave_period) * dd;
                    data[2][l1][l2] = (phi1 - wave_period) * dd;
                  } 
                }
              }
            }
            phi[l1] += dphi;
            while (phi[l1] < 0) phi[l1] += wave_period;
            while (phi[l1] > wave_period) phi[l1] -= wave_period;
          }
        }
      } else {
        for (l1 = 0; l1 < synthdata->poly; ++l1) {
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
            dphi = freq_const * (synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcoExpFMGain * expFMData[l1][l2])) 
                                 + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) dphi = wave_period_2;
            data[0][l1][l2] = synthdata->wave_sine[(int)phi[l1]];
            data[1][l1][l2] = synthdata->wave_tri[(int)phi[l1]];
            switch (waveForm) {
              case WAVE_SAW: 
                data[4][l1][l2] = synthdata->wave_saw2[(int)phi[l1]];
                break;
              case WAVE_SAW2:
                half_wave = wave_period * 0.5; // / 2.0;
                data[4][l1][l2] = (phi[l1] < half_wave)
                                ? synthdata->wave_saw2[(int)(2.0 * phi[l1])]
                                : 0;
                break;
              case WAVE_SAW3:
                third_wave = wave_period / 3.0;
                data[4][l1][l2] = (phi[l1] < third_wave)
                                ? synthdata->wave_saw2[(int)(3.0 * phi[l1])]
                                : 0;
                break;
            }
            pw = (pw0 + pwGain * pwData[l1][l2]) * wave_period;
            if (pw < pw_low) pw = pw_low;  
            else if (pw > pw_high) pw = pw_high;
            d = edge * dphi;
            dd = 1.0 / d;
            dsaw = 2.0 / (wave_period - 2.0 * d);
            if (phi[l1] <= d) {
              data[3][l1][l2] = phi[l1] * dd;
              data[2][l1][l2] = phi[l1] * dd;
            } else {
              if (phi[l1] <= pw - d) {
                data[3][l1][l2] = 1.0;
                data[2][l1][l2] = 1.0 - (phi[l1] - d) * dsaw;
              } else { 
                if (phi[l1] <= pw + d) {
                  data[3][l1][l2] = (pw - phi[l1]) * dd;
                  data[2][l1][l2] = 1.0 - (phi[l1] - d) * dsaw;
                } else {
                  if (phi[l1] <= wave_period - d) {
                    data[3][l1][l2] = -1.0;
                    data[2][l1][l2] = 1.0 - (phi[l1] - d) * dsaw;
                  } else {
                    data[3][l1][l2] = (phi[l1] - wave_period) * dd;
                    data[2][l1][l2] = (phi[l1] - wave_period) * dd;
                  } 
                }
              }
            }
            phi[l1] += dphi;
            while (phi[l1] < 0) phi[l1] += wave_period;
            while (phi[l1] > wave_period) phi[l1] -= wave_period;
          }
        }
      }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vco::showConfigDialog() {
}
