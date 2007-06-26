#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "m_vcorgan.h"
#include "port.h"

M_vcorgan::M_vcorgan(int p_oscCount, QWidget* parent) 
  : Module(M_type_vcorgan, 1, parent, "VC Organ")
{
  QString qs;
  int l1, l2;
  QVBoxLayout *oscTab[MODULE_VCORGAN_MAX_OSC];

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCORGAN_WIDTH, MODULE_VCORGAN_HEIGHT);
  wave_period = (float)WAVE_PERIOD;
  tune = 0;
  octave = 3;
  expFMGain = 0;
  linFMGain = 0;
  oscCount = p_oscCount;
  for (l1 = 0; l1 < oscCount; l1++) {
    gain[l1] = 1;
    osc_tune[l1] = 0;
    harmonic[l1] = 1 + l1;
    subharmonic[l1] = 1;
    osc_octave[l1] = 0;
    waveForm[l1] = ORGAN_SINE;
    phi0[l1] = 0;
  }
  for (l1 = 0; l1 < MAXPOLY; l1++) {
    for (l2 = 0; l2 < oscCount; l2++) {
      phi[l1][l2] = 0;
    }
  }
  port_M_freq = new Port("Freq", PORT_IN, 0, this);
  port_M_freq->move(0, 35);    
  port_M_freq->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_freq); 
  port_M_exp = new Port("Exp. FM", PORT_IN, 1, this);
  port_M_exp->move(0, 55);    
  port_M_exp->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_exp); 
  port_M_lin = new Port("Lin. FM", PORT_IN, 2, this);
  port_M_lin->move(0, 75);
  port_M_lin->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_lin);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 95);
  port_out->outType = outType_audio;
  portList.append(port_out);

  configDialog->initTabWidget();
  QStringList waveFormNames;
  waveFormNames <<
    "Sine" <<
    "Saw" <<
    "Tri" <<
    "Rect" <<
    "Saw 2";
  QVBoxLayout *generalTab = configDialog->addVBoxTab("Tune / Modulation");
  configDialog->addIntSlider(0, 6, octave, "Octave", &octave, generalTab);
  configDialog->addSlider(0, 1, tune, "Tune", &tune, false, generalTab);
  configDialog->addSlider(0, 10, expFMGain, "Exp. FM Gain", &expFMGain, false, generalTab);
  configDialog->addSlider(0, 10, linFMGain, "Lin. FM Gain", &linFMGain, false, generalTab);

  if (oscCount < 8) {
    QVBoxLayout *mixTab = configDialog->addVBoxTab("Mixer");
    for (l1 = 0; l1 < oscCount; l1++) {
      qs.sprintf("Volume %d", l1);
      configDialog->addSlider(0, 1, gain[l1], qs, &gain[l1], false, mixTab);
    }
  } else {
    QVBoxLayout *mixTab = configDialog->addVBoxTab("Mixer 0-3");
    QVBoxLayout *mixTab2 = configDialog->addVBoxTab("Mixer 4-7");
    for (l1 = 0; l1 < 4; l1++) {
      qs.sprintf("Volume %d", l1);
      configDialog->addSlider(0, 1, gain[l1], qs, &gain[l1], false, mixTab);
      qs.sprintf("Volume %d", l1 + 4);
      configDialog->addSlider(0, 1, gain[l1+4], qs, &gain[l1+4], false, mixTab2);
    }
  }
  for (l1 = 0; l1 < oscCount; l1++) {
    qs.sprintf("Osc %d", l1);
    oscTab[l1] = configDialog->addVBoxTab(qs);
    qs.sprintf("Wave Form %d", l1);
    configDialog->addComboBox(0, qs, (int *)&waveForm[l1], waveFormNames.count(), &waveFormNames, oscTab[l1]);
    qs.sprintf("Octave %d", l1);
    configDialog->addIntSlider(0, 3, osc_octave[l1], qs, &osc_octave[l1], oscTab[l1]);
    qs.sprintf("Tune %d", l1);
    configDialog->addSlider(0, 1, osc_tune[l1], qs, &osc_tune[l1], false, oscTab[l1]);
    qs.sprintf("Harmonic %d", l1);
    configDialog->addIntSlider(1, 16, harmonic[l1], qs, &harmonic[l1], oscTab[l1]);
    qs.sprintf("Subharmonic %d", l1);
    configDialog->addIntSlider(1, 16, subharmonic[l1], qs, &subharmonic[l1], oscTab[l1]);
    qs.sprintf("Phi0 %d", l1);
    configDialog->addSlider(0, 6.283, phi0[l1], qs, &phi0[l1], false, oscTab[l1]);
  }
}

void M_vcorgan::generateCycle() {

  int l1, l2, l3;
  float dphi, phi1; 
  float freq_const[MODULE_VCORGAN_MAX_OSC], freq_tune[MODULE_VCORGAN_MAX_OSC];
  float gain_linfm, wave_period_2, current_gain;
  float gain_const[MODULE_VCORGAN_MAX_OSC], phi_const[MODULE_VCORGAN_MAX_OSC];

  if (!cycleReady) {
    cycleProcessing = true; 
    wave_period_2 = wave_period / 2.0;

    freqData = port_M_freq->getinputdata();
    expFMData = port_M_exp->getinputdata();
    linFMData = port_M_lin->getinputdata();

    gain_linfm = 1000.0 * linFMGain;
    for (l3 = 0; l3 < oscCount; l3++) {
      gain_const[l3] = gain[l3] / (float)oscCount;
      freq_tune[l3] = 4.0313842 + octave + tune + osc_octave[l3] + osc_tune[l3];
      freq_const[l3] = wave_period / (float)synthdata->rate * (float)harmonic[l3] / (float)subharmonic[l3];
      phi_const[l3] = phi0[l3] * wave_period / (2.0 * M_PI);
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {  
      memset(data[0][l1], 0, synthdata->cyclesize * sizeof(float));
      for (l3 = 0; l3 < oscCount; l3++) {
        if (phi0[l3] == 0) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            dphi = freq_const[l3] * (synthdata->exp_table_ln2(freq_tune[l3] + freqData[l1][l2] + expFMGain * expFMData[l1][l2]) 
                                                         + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) {
              dphi = wave_period_2;
              current_gain = 0;
            } else {
              current_gain = gain_const[l3];
            }
            switch (waveForm[l3]) {
              case ORGAN_SINE: 
                data[0][l1][l2] += current_gain * synthdata->wave_sine[(int)phi[l1][l3]];
                break;
              case ORGAN_SAW: 
                data[0][l1][l2] += current_gain * synthdata->wave_saw[(int)phi[l1][l3]];
                break;
              case ORGAN_TRI: 
                data[0][l1][l2] += current_gain * synthdata->wave_tri[(int)phi[l1][l3]];
                break;
              case ORGAN_RECT: 
                data[0][l1][l2] += current_gain * synthdata->wave_rect[(int)phi[l1][l3]];
                break;
              case ORGAN_SAW2: 
                data[0][l1][l2] += current_gain * synthdata->wave_saw2[(int)phi[l1][l3]];
                break;
            }
            phi[l1][l3] += dphi;
            while (phi[l1][l3] < 0) phi[l1][l3] += wave_period;
            while (phi[l1][l3] >= wave_period) phi[l1][l3] -= wave_period;
          }
        } else {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            dphi = freq_const[l3] * (synthdata->exp_table_ln2(freq_tune[l3] + freqData[l1][l2] + expFMGain * expFMData[l1][l2])
                                                         + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) {
              dphi = wave_period_2;
              current_gain = 0;
            } else {
              current_gain = gain_const[l3];
            }
            phi1 = phi[l1][l3] + phi_const[l3];
            if (phi1 < 0) phi1 += wave_period;
            else if (phi1 >= wave_period) phi1 -= wave_period;
            switch (waveForm[l3]) {
              case ORGAN_SINE: 
                data[0][l1][l2] += current_gain * synthdata->wave_sine[(int)phi1];
                break;
              case ORGAN_SAW: 
                data[0][l1][l2] += current_gain * synthdata->wave_saw[(int)phi1];
                break;
              case ORGAN_TRI: 
                data[0][l1][l2] += current_gain * synthdata->wave_tri[(int)phi1];
                break;
              case ORGAN_RECT: 
                data[0][l1][l2] += current_gain * synthdata->wave_rect[(int)phi1];
                break;
              case ORGAN_SAW2: 
                data[0][l1][l2] += current_gain * synthdata->wave_saw2[(int)phi1];
                break;
            }
            phi[l1][l3] += dphi;
            while (phi[l1][l3] < 0) phi[l1][l3] += wave_period;
            while (phi[l1][l3] >= wave_period) phi[l1][l3] -= wave_period;
          }
        }
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

