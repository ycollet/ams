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
#include "m_vcorgan.h"
#include "port.h"

M_vcorgan::M_vcorgan(int p_oscCount, QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;
  int l1, l2;
  QVBox *oscTab[MODULE_VCORGAN_MAX_OSC];

  M_type = M_type_vcorgan;
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
  port_M_freq = new Port("Freq", PORT_IN, 0, this, synthdata);
  port_M_freq->move(0, 35);
  port_M_freq->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_freq); 
  port_M_exp = new Port("Exp. FM", PORT_IN, 1, this, synthdata);
  port_M_exp->move(0, 55);    
  port_M_exp->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_exp); 
  port_M_lin = new Port("Lin. FM", PORT_IN, 2, this, synthdata);
  port_M_lin->move(0, 75);
  port_M_lin->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_lin);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 95);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("VC Organ ID %d", moduleID);
  configDialog->setCaption(qs);

  
  QString tabName = "Tune / Modulation";
  IntParameter *ip = new IntParameter(this,"Octave","", 0,6,&octave);
  configDialog->addParameter(ip,tabName);
  FloatParameter *fp = new FloatParameter(this,"Tune","",0.0,1.0,&tune);
  configDialog->addParameter(fp,tabName);
  fp = new FloatParameter(this,"Exp. FM Gain","",0.0,10.0,&expFMGain);
  configDialog->addParameter(fp,tabName);
  fp = new FloatParameter(this,"Lin. FM Gain","",0.0,10.0,&linFMGain);
  configDialog->addParameter(fp,tabName);


  if (oscCount < 8) {
    tabName="Mixer";
    for (l1 = 0; l1 < oscCount; l1++) {
      qs.sprintf("Volume %d", l1);
      fp=new FloatParameter(this,qs,"",0.0,1.0,&gain[l1]);
      configDialog->addParameter(fp,tabName);

    }

  } else {
    
    for (l1 = 0; l1 < 4; l1++) {
      qs.sprintf("Volume %d", l1);
      fp=new FloatParameter(this,qs,"",0.0,1.0,&gain[l1]);
      configDialog->addParameter(fp,"Mixer 0-3");

      qs.sprintf("Volume %d", l1 + 4);
      fp=new FloatParameter(this,qs,"",0.0,1.0,&gain[l1+4]);
      configDialog->addParameter(fp,"Mixer 4-7");
    }

  }
  for (l1 = 0; l1 < oscCount; l1++) {
    tabName.sprintf("Osc %d", l1);
    qs.sprintf("Wave Form %d", l1);
    EnumParameter *ep = new EnumParameter(this,qs,"",(int *)&waveForm[l1]);
    ep->addItem(ORGAN_SINE,"Sine");
    ep->addItem(ORGAN_SAW,"Saw");
    ep->addItem(ORGAN_TRI,"Tri");
    ep->addItem(ORGAN_RECT,"Rect");
    ep->addItem(ORGAN_SAW2,"Saw2");
    configDialog->addParameter(ep,tabName);
    qs.sprintf("Octave %d", l1);
    ip=new IntParameter(this,qs,"",0,3,&osc_octave[l1]);
    configDialog->addParameter(ip,tabName);
    qs.sprintf("Tune %d", l1);
    fp = new FloatParameter(this,qs,"",0.0,1.0,&osc_tune[l1]);
    configDialog->addParameter(fp,tabName);

    qs.sprintf("Harmonic %d", l1);
    ip=new IntParameter(this,qs,"",1,16,&harmonic[l1]);
    configDialog->addParameter(ip,tabName);

    qs.sprintf("Subharmonic %d", l1);
    ip=new IntParameter(this,qs,"",1,16,&subharmonic[l1]);
    configDialog->addParameter(ip,tabName);

    qs.sprintf("Phi0 %d", l1);
    fp = new FloatParameter(this,qs,"",0.0,6.283,&phi0[l1]);
    configDialog->addParameter(fp,tabName);
  }
}

M_vcorgan::~M_vcorgan() {
}

void M_vcorgan::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(12, 20, "VC Organ");
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_vcorgan::noteOnEvent(int osc) {
}

void M_vcorgan::generateCycle() {

  int l1, l2, l3;
  float dphi, phi1; 
  float freq_const[MODULE_VCORGAN_MAX_OSC], freq_tune[MODULE_VCORGAN_MAX_OSC];
  float gain_linfm, log2, wave_period_2, current_gain;
  float gain_const[MODULE_VCORGAN_MAX_OSC], phi_const[MODULE_VCORGAN_MAX_OSC];

  if (!cycleReady) {
    cycleProcessing = true; 
    log2 = log(2.0);
    wave_period_2 = wave_period / 2.0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
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
    gain_linfm = 1000.0 * linFMGain;
    for (l3 = 0; l3 < oscCount; l3++) {
      gain_const[l3] = gain[l3] / (float)oscCount;
      freq_tune[l3] = 4.0313842 + octave + tune + osc_octave[l3] + osc_tune[l3];
      freq_const[l3] = wave_period / (float)synthdata->rate * (float)harmonic[l3] / (float)subharmonic[l3];
      phi_const[l3] = phi0[l3] * wave_period / (2.0 * PI);
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {  
      memset(data[0][l1], 0, synthdata->cyclesize * sizeof(float));
      for (l3 = 0; l3 < oscCount; l3++) {
        if (phi0[l3] == 0) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            dphi = freq_const[l3] * (synthdata->exp_table(log2 * (freq_tune[l3] + freqData[l1][l2] + expFMGain * expFMData[l1][l2])) 
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
            while (phi[l1][l3] > wave_period) phi[l1][l3] -= wave_period;
          }
        } else {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            dphi = freq_const[l3] * (synthdata->exp_table(log2 * (freq_tune[l3] + freqData[l1][l2] + expFMGain * expFMData[l1][l2])) 
                                                         + gain_linfm * linFMData[l1][l2]);
            if (dphi > wave_period_2) {
              dphi = wave_period_2;
              current_gain = 0;
            } else {
              current_gain = gain_const[l3];
            }
            phi1 = phi[l1][l3] + phi_const[l3];
            if (phi1 < 0) phi1 += wave_period;
            else if (phi1 > wave_period) phi1 -= wave_period;
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
            while (phi[l1][l3] > wave_period) phi[l1][l3] -= wave_period;
          }
        }
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vcorgan::showConfigDialog() {
}

