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
#include "m_vco2.h"
#include "port.h"

M_vco2::M_vco2(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_vco2;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCO2_WIDTH, MODULE_VCO2_HEIGHT);
  wave_period = (float)WAVE_PERIOD;
  wave_period_2 = wave_period * 0.5f;
  wave_period_3 = wave_period / 3.0;
  Pi2Times= (2.0f * M_PI);
  PKonst = wave_period / Pi2Times;

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
  waveForm = SINUS;
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
  port_sine = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_sine->move(width() - port_sine->width(), 115);
  port_sine->outType = outType_audio;
  portList.append(port_sine);
  /*
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
  */
  qs.sprintf("VCO2 ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->initTabWidget();
  QVBox *freqTab = new QVBox(configDialog->tabWidget);
  QStrList *waveFormNames = new QStrList(true);
  waveFormNames->append("Sinus");
  waveFormNames->append("Triangle");
  waveFormNames->append("Sawtooth");
  waveFormNames->append("Rectangle");
  waveFormNames->append("Aux Saw");
  waveFormNames->append("Aux Saw 1");
  waveFormNames->append("Aux Saw 2");
  configDialog->addComboBox(0, "Wave Form", (int *)&waveForm, waveFormNames->count(), waveFormNames, freqTab);
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
  configDialog->addSlider(0, 10, vcoExpFMGain, "Exp. FM Gain", &vcoExpFMGain, false, modulationTab);
  configDialog->addSlider(0, 10, vcoLinFMGain, "Lin. FM Gain", &vcoLinFMGain, false, modulationTab);
  configDialog->addTab(modulationTab, "Modulation / Aux Waveform");
}

M_vco2::~M_vco2() {
}

void M_vco2::noteOnEvent(int osc) {
}

void M_vco2::generateCycle() {

  unsigned int l1, l2,phint;
  float dphi, phi1, phi_const, pw, d, dd, dsaw, half_wave, third_wave; 
  float freq_const, freq_tune, gain_linfm,  pw_low, pw_high;
 

  if (!cycleReady) {
    cycleProcessing = true; 
    edge = 0.01f + 1.8f * synthdata->edge;

    freqData = port_M_freq->getinputdata();
    expFMData = port_M_exp->getinputdata();
    linFMData = port_M_lin->getinputdata();
    pwData = port_M_pw->getinputdata();

      freq_const = wave_period / (float)synthdata->rate * (float)harmonic / (float)subharmonic;
      freq_tune = 4.0313842f + octave + freq;
      gain_linfm = 1000.0f * vcoLinFMGain;
      phi_const = phi0 * PKonst;
      pw_low = 0.1f * wave_period;
      pw_high = 0.9f * wave_period;

      if (phi0 > 0.0f) {
        for (l1 = 0; l1 < synthdata->poly; ++l1) {
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {

            dphi = freq_const * (synthdata->exp_table_ln2(freq_tune + freqData[l1][l2] + vcoExpFMGain * expFMData[l1][l2])
                                 + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) dphi = wave_period_2;
            phi1 = phi[l1] + phi_const;
            if (phi1 < 0.0f) phi1 += wave_period;
            else if (phi1 > wave_period) phi1 -= wave_period;
            phint=(int)phi1;

            switch (waveForm) {
		case SINUS:
		{
	    		data[0][l1][l2] = synthdata->wave_sine[phint];
		}
		break;
		case TRIANGLE:
		{
            		data[0][l1][l2] = synthdata->wave_tri[phint];
		}
		break;
              case AWAVE_SAW: 
                data[0][l1][l2] = synthdata->wave_saw2[phint];
                break;
              case AWAVE_SAW2:
                half_wave = wave_period_2;// * 0.5f;// / 2.0;              
                data[0][l1][l2] = (phi1 < half_wave) 
                                ? synthdata->wave_saw2[(int)(2.0f * phi1)]
                                : 0.0f;
                break;
              case AWAVE_SAW3: 
                third_wave = wave_period_3;// * 0.3333333333334f;// / 3.0f;
                data[0][l1][l2] = (phi1 < third_wave)
                                ? synthdata->wave_saw2[(int)(3.0f * phi1)]
                                : 0.0f;
                break;
            
	    case SAWTOOTH:
	    {
            pw = (pw0 + pwGain * pwData[l1][l2]) * wave_period;
            if (pw < pw_low) pw = pw_low;
            else if (pw > pw_high) pw = pw_high;
            d = edge * dphi;
            dd = 1.0f / d;
            dsaw = 2.0f / (wave_period - 2.0f * d);
            if (phi1 <= d) {
              data[0][l1][l2] = phi1 * dd;
            } else {
              if (phi1 <= pw - d) {
                data[0][l1][l2] = 1.0f - (phi1 - d) * dsaw;
              } else { 
                if (phi1 <= pw + d) {
                  data[0][l1][l2] = 1.0f - (phi1 - d) * dsaw;
                } else {
                  if (phi1 <= wave_period - d) {
                    data[0][l1][l2] = 1.0f - (phi1 - d) * dsaw;
                  } else {
                    data[0][l1][l2] = (phi1 - wave_period) * dd;
                  } 
                }
              }
            }
	    }
	    break;
	    case RECTANGLE:
	    {
            pw = (pw0 + pwGain * pwData[l1][l2]) * wave_period;
            if (pw < pw_low) pw = pw_low;
            else if (pw > pw_high) pw = pw_high;
            d = edge * dphi;
            dd = 1.0f / d;
            if (phi1 <= d) {
              data[0][l1][l2] = phi1 * dd;
            } else {
              if (phi1 <= pw - d) {
                data[0][l1][l2] = 1.0f;
              } else { 
                if (phi1 <= pw + d) {
                  data[0][l1][l2] = (pw - phi1) * dd;
                } else {
                  if (phi1 <= wave_period - d) {
                    data[0][l1][l2] = -1.0f;
                  } else {
                    data[0][l1][l2] = (phi1 - wave_period) * dd;
                  } 
                }
              }
            }
	    }
	    break;
	    } // end of case
            phi[l1] += dphi;
            while (phi[l1] < 0.0f) phi[l1] += wave_period;
            while (phi[l1] > wave_period) phi[l1] -= wave_period;
          }
        }
      } else {
        for (l1 = 0; l1 < synthdata->poly; ++l1) {
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
            dphi = freq_const * (synthdata->exp_table_ln2(freq_tune + freqData[l1][l2] + vcoExpFMGain * expFMData[l1][l2]) + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) dphi = wave_period_2;
	    phint=(int)phi[l1];
            switch (waveForm) {
	    case SINUS:
	    {
            	data[0][l1][l2] = synthdata->wave_sine[phint];
	    }
	    break;
	    case TRIANGLE:
	    {
            	data[0][l1][l2] = synthdata->wave_tri[phint];
            }
	    break;
	    case AWAVE_SAW: 
                data[0][l1][l2] = synthdata->wave_saw2[phint];
                break;
              case AWAVE_SAW2:
                half_wave = wave_period_2;// * 0.5f; // / 2.0;
                data[0][l1][l2] = (phi[l1] < half_wave)
                                ? synthdata->wave_saw2[(int)(2.0f * phi[l1])]
                                : 0.0f;
                break;
              case AWAVE_SAW3:
                third_wave = wave_period_3;// * 0.3333333333334f;// / 3.0f;
                data[0][l1][l2] = (phi[l1] < third_wave)
                                ? synthdata->wave_saw2[(int)(3.0f * phi[l1])]
                                : 0.0f;
                break;
            
	    case SAWTOOTH:
	    {
            pw = (pw0 + pwGain * pwData[l1][l2]) * wave_period;
            if (pw < pw_low) pw = pw_low;  
            else if (pw > pw_high) pw = pw_high;
            d = edge * dphi;
            dd = 1.0f / d;
            dsaw = 2.0f / (wave_period - 2.0f * d);
            if (phi[l1] <= d) {
              data[0][l1][l2] = phi[l1] * dd;
            } else {
              if (phi[l1] <= pw - d) {
                data[0][l1][l2] = 1.0f - (phi[l1] - d) * dsaw;
              } else { 
                if (phi[l1] <= pw + d) {
                  data[0][l1][l2] = 1.0f - (phi[l1] - d) * dsaw;
                } else {
                  if (phi[l1] <= wave_period - d) {
                    data[0][l1][l2] = 1.0f - (phi[l1] - d) * dsaw;
                  } else {
                    data[0][l1][l2] = (phi[l1] - wave_period) * dd;
                  } 
                }
              }
            }
	    }
	    break;
	     case RECTANGLE:
	    {
            pw = (pw0 + pwGain * pwData[l1][l2]) * wave_period;
            if (pw < pw_low) pw = pw_low;  
            else if (pw > pw_high) pw = pw_high;
            d = edge * dphi;
            dd = 1.0f / d;
            if (phi[l1] <= d) {
              data[0][l1][l2] = phi[l1] * dd;
            } else {
              if (phi[l1] <= pw - d) {
                data[0][l1][l2] = 1.0f;
              } else { 
                if (phi[l1] <= pw + d) {
                  data[0][l1][l2] = (pw - phi[l1]) * dd;
                } else {
                  if (phi[l1] <= wave_period - d) {
                    data[0][l1][l2] = -1.0f;
                  } else {
                    data[0][l1][l2] = (phi[l1] - wave_period) * dd;
                  } 
                }
              }
            }
	    }
	    break;
	}// end of case
            phi[l1] += dphi;
            while (phi[l1] < 0.0f) phi[l1] += wave_period;
            while (phi[l1] > wave_period) phi[l1] -= wave_period;
          }
        }
      }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vco2::showConfigDialog() {
}
