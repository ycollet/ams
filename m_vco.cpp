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
  for (l1 = 0; l1 < MAXPOLY; l1++) {
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
  //configDialog->initTabWidget();
  //QVBox *freqTab = new QVBox(configDialog->tabWidget);
  qs="Frequency";
  IntParameter *ip = new IntParameter(this,"Octave","",0,6,&octave);
  configDialog->addParameter(ip,qs);
  FloatParameter *fp = new FloatParameter(this,"Tune","",0.0,1.0,&freq);
  configDialog->addParameter(fp,qs);
  ip = new IntParameter(this,"Harmonic","",1,16,&harmonic);
  configDialog->addParameter(ip,qs);
  ip = new IntParameter(this,"Subharmonic","",1,16,&subharmonic);
  configDialog->addParameter(ip,qs);

  qs="Pulse Width / Phase";
  fp=new FloatParameter(this,"PW","", 0.1,0.9,&pw0);
  configDialog->addParameter(fp,qs);
  fp=new FloatParameter(this,"PW Gain","",0.0,1.0,&pwGain);
  configDialog->addParameter(fp,qs);
  fp = new FloatParameter(this,"Phi0","",0.0,6.283,&phi0);
  configDialog->addParameter(fp,qs);
  qs="Modulation / Aux Waveform";
  EnumParameter *ep = new EnumParameter(this, "Aux Wave Form","",(int *)&waveForm);
  ep->addItem(WAVE_SAW,"Saw");
  ep->addItem(WAVE_SAW2,"Saw 1");
  ep->addItem(WAVE_SAW3,"Saw 2");
  configDialog->addParameter(ep,qs);
  fp = new FloatParameter(this,"Exp. FM Gain","",0.0,10.0,&vcoExpFMGain);
  configDialog->addParameter(fp,qs);
  fp = new FloatParameter(this,"Lin. FM Gain","",0.0,10.0,&vcoLinFMGain);
  configDialog->addParameter(fp,qs);
 
}

M_vco::~M_vco() {
}

void M_vco::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(12, 20, "VCO");
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
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
    for (l2 = 0; l2 < 5; l2++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[l2][l1], data[l2][l1], synthdata->cyclesize * sizeof(float));
      }
    }
    if (port_M_freq->connectedPortList.count()) {
      in_M_freq = (Module *)port_M_freq->connectedPortList.at(0)->parentModule;
      if (!in_M_freq->cycleProcessing) {
        in_M_freq->generateCycle();
        freqData = in_M_freq->data[port_M_freq->connectedPortList.at(0)->index];
      } else {
        freqData = in_M_freq->lastdata[port_M_freq->connectedPortList.at(0)->index];
      }
    } else {
      in_M_freq = NULL;
      freqData = synthdata->zeroModuleData;
    }
    if (port_M_exp->connectedPortList.count()) {
      in_M_exp = (Module *)port_M_exp->connectedPortList.at(0)->parentModule;
    if (!in_M_exp->cycleProcessing) {
        in_M_exp->generateCycle();
        expFMData = in_M_exp->data[port_M_exp->connectedPortList.at(0)->index];
    } else {
        expFMData = in_M_exp->lastdata[port_M_exp->connectedPortList.at(0)->index];
    }
    } else {
      in_M_exp = NULL;
      expFMData = synthdata->zeroModuleData;
    }
    if (port_M_lin->connectedPortList.count()) {
      in_M_lin = (Module *)port_M_lin->connectedPortList.at(0)->parentModule;
      if (!in_M_lin->cycleProcessing) {
        in_M_lin->generateCycle();
        linFMData = in_M_lin->data[port_M_lin->connectedPortList.at(0)->index];
      } else {
        linFMData = in_M_lin->lastdata[port_M_lin->connectedPortList.at(0)->index];
      }
    } else {
      in_M_lin = NULL;
      linFMData = synthdata->zeroModuleData;
    }
    if (port_M_pw->connectedPortList.count()) {
      in_M_pw = (Module *)port_M_pw->connectedPortList.at(0)->parentModule;
      if (!in_M_pw->cycleProcessing) {
        in_M_pw->generateCycle();
        pwData = in_M_pw->data[port_M_pw->connectedPortList.at(0)->index];
      } else {
        pwData = in_M_pw->lastdata[port_M_pw->connectedPortList.at(0)->index];
      }
    } else {
      in_M_pw = NULL;
      pwData = synthdata->zeroModuleData;
    }
      freq_const = wave_period / (float)synthdata->rate * (float)harmonic / (float)subharmonic;
      freq_tune = 4.0313842 + octave + freq;
      gain_linfm = 1000.0 * vcoLinFMGain;
      phi_const = phi0 * wave_period / (2.0 * PI);
      pw_low = 0.1 * wave_period;
      pw_high = 0.9 * wave_period;
      if (phi0 > 0) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
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
                half_wave = wave_period / 2.0;              
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
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
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
                half_wave = wave_period / 2.0;
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
