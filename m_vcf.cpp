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
#include "m_vcf.h"
#include "port.h"

M_vcf::M_vcf(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_vcf;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCF_WIDTH, MODULE_VCF_HEIGHT);
  gain = 1.0;
  freq = 5;
  resonance = 0.8;
  dBgain = 3.0;
  initBuf(0);
  port_M_in = new Port("In", PORT_IN, 0, this, synthdata); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_M_freq = new Port("Freq", PORT_IN, 1, this, synthdata); 
  port_M_freq->move(0, 55);
  port_M_freq->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_freq);
  port_M_exp = new Port("Exp. FM", PORT_IN, 2, this, synthdata);
  port_M_exp->move(0, 75);    
  port_M_exp->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_exp);
  port_M_lin = new Port("Lin. FM", PORT_IN, 3, this, synthdata);
  port_M_lin->move(0, 95);
  port_M_lin->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_lin);
  port_M_resonance = new Port("Resonance", PORT_IN, 4, this, synthdata); 
  port_M_resonance->move(0, 115);
  port_M_resonance->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_resonance);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 130);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("VCF ID %d", moduleID);
  configDialog->setCaption(qs);
  
/*
  QStrList *vcfTypeNames = new QStrList(true);
  vcfTypeNames->append("Resonant Lowpass");
  vcfTypeNames->append("Lowpass");
  vcfTypeNames->append("Highpass");
  vcfTypeNames->append("Bandpass I");
  vcfTypeNames->append("Bandpass II");
  vcfTypeNames->append("Notch");
  vcfTypeNames->append("Moog Lowpass I");
  vcfTypeNames->append("Moog Lowpass II");
  configDialog->addComboBox(6, "VCF Type", (int *)&vcfType, vcfTypeNames->count(), vcfTypeNames);
  QObject::connect(configDialog->midiComboBoxList.at(0)->comboBox, SIGNAL(highlighted(int)), this, SLOT(initBuf(int)));
*/
  EnumParameter *ep = new EnumParameter(this,"VCF Type","",(int *)&vcfType);
  ep->addItem(VCF_LR,"Resonant Lowpass");
  ep->addItem(VCF_LPF,"Lowpass");
  ep->addItem(VCF_HPF,"Highpass");
  ep->addItem(VCF_BPF_I,"Bandpass I");
  ep->addItem(VCF_BPF_II,"Bandpass II");
  ep->addItem(VCF_NF,"Notch");
  ep->addItem(VCF_MOOG1,"Moog Lowpass I");
  ep->addItem(VCF_MOOG2,"Moog Lowpass II");
  configDialog->addParameter(ep);

  FloatParameter *fp = new FloatParameter(this,"Input Gain","",0.0,10.0,&gain);
  configDialog->addParameter(fp);
  
  fp = new FloatParameter(this,"Frequency","",0.0,10.0,&freq);
  configDialog->addParameter(fp);
  fp = new FloatParameter(this,"Exp. FM Gain","",0.0,10.0,&vcfExpFMGain);
  configDialog->addParameter(fp);
  fp = new FloatParameter(this,"Lin. FM Gain","",0.0,10.0,&vcfLinFMGain);
  configDialog->addParameter(fp);
  fp = new FloatParameter(this,"Resonance","",0.01,1.0,&resonance);
  configDialog->addParameter(fp);
  fp = new FloatParameter(this,"Resonance Gain","",0.0,1.0,&resonanceGain);
  configDialog->addParameter(fp);

}

M_vcf::~M_vcf() {
}

void M_vcf::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "VCF");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_vcf::initBuf(int index) {

  int l1, l2;

  for (l1 = 0; l1 < MAXPOLY; l1++) {
    for (l2 = 0; l2 < 5; l2++) {
      buf[l2][l1] = 0;   
      in[l2][l1] = 0;
    }
  }
}

void M_vcf::generateCycle() {

  int l1, l2;
  double t1, t2, fa, fb, q0, f, q, p, iv_sin, iv_cos, iv_alpha, a0, a1, a2, b0, b1, b2;
  float freq_const, freq_tune, gain_linfm, pi2_rate, inv2_rate, qr, moog_f;
  const float log2 = log(2.0);

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }
    if (port_M_in->connectedPortList.count()) {
      in_M_in = (Module *)port_M_in->connectedPortList.at(0)->parentModule;
      if (!in_M_in->cycleProcessing) {
        in_M_in->generateCycle();
        inData = in_M_in->data[port_M_in->connectedPortList.at(0)->index];
      } else {
        inData = in_M_in->lastdata[port_M_in->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in = NULL;
      inData = synthdata->zeroModuleData;
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
    if (port_M_resonance->connectedPortList.count()) {
      in_M_resonance = (Module *)port_M_resonance->connectedPortList.at(0)->parentModule;
      if (!in_M_resonance->cycleProcessing) {
        in_M_resonance->generateCycle();
        resonanceData = in_M_resonance->data[port_M_resonance->connectedPortList.at(0)->index];
      } else {
        resonanceData = in_M_resonance->lastdata[port_M_resonance->connectedPortList.at(0)->index];
      }
    } else {
      in_M_resonance = NULL;
      resonanceData = synthdata->zeroModuleData;
    }
    pi2_rate = 2.0 * M_PI / synthdata->rate;
    switch (vcfType) {
      case VCF_LR:
        q0 = resonance;
        freq_tune = 5.0313842 + freq;
        freq_const = 2.85 / 20000.0;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = freq_const * (synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                               + gain_linfm * linFMData[l1][l2]);
            if (f < 0) f = 0;
            else if (f > 0.99) f = 0.99;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            fa = 1.0 - f;
            fb = q * (1.0 + (1.0 / fa));
            buf[0][l1] = fa * buf[0][l1] + f * (gain * inData[l1][l2] + fb * (buf[0][l1] - buf[1][l1]) + 0.00001 * ((float)rand() / (float)RAND_MAX - 1.0));
            buf[1][l1] = fa * buf[1][l1] + f * buf[0][l1];
            data[0][l1][l2] = buf[1][l1];
          }
        }
        break;
      case VCF_LPF:
        q0 = resonance;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                    + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ) f = MAX_FREQ;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            iv_sin = sin(pi2_rate * f);
            iv_cos = cos(pi2_rate * f);
            iv_alpha = iv_sin/(64.0 * q);
            b0 = (1.0 - iv_cos) * 0.5; 
            b1 = 1.0 - iv_cos;
            b2 = b0;
            a0 = 1.0 + iv_alpha;
            a1 = -2.0 * iv_cos; 
            a2 = 1.0 - iv_alpha;
            data[0][l1][l2] = 1.0 / a0 * (b0 * gain * inData[l1][l2] + b1 * buf[0][l1] + b2 * buf[1][l1]
                                       - a1 * buf[2][l1] - a2 * buf[3][l1]);
            buf[1][l1] = buf[0][l1];                          
            buf[0][l1] = gain * inData[l1][l2];
            buf[3][l1] = buf[2][l1];
            buf[2][l1] = data[0][l1][l2];
          }
        } 
        break;
      case VCF_HPF:
        q0 = resonance;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                    + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ) f = MAX_FREQ;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            iv_sin = sin(pi2_rate * f);
            iv_cos = cos(pi2_rate * f);
            iv_alpha = iv_sin/(64.0 * q);
            b0 = (1.0 + iv_cos) * 0.5;
            b1 = - 1.0 - iv_cos;
            b2 = b0;
            a0 = 1.0 + iv_alpha;
            a1 = -2.0 * iv_cos; 
            a2 = 1.0 - iv_alpha;
            data[0][l1][l2] = 1.0 / a0 * (gain * b0 * inData[l1][l2] + b1 * buf[0][l1] + b2 * buf[1][l1]
                                       - a1 * buf[2][l1] - a2 * buf[3][l1]);
            buf[1][l1] = buf[0][l1];                          
            buf[0][l1] = gain * inData[l1][l2];
            buf[3][l1] = buf[2][l1];
            buf[2][l1] = data[0][l1][l2];
          }
        } 
        break;
      case VCF_BPF_I:
        q0 = resonance;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                    + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ) f = MAX_FREQ;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            iv_sin = sin(pi2_rate * f);
            iv_cos = cos(pi2_rate * f);
            iv_alpha = iv_sin/(64.0 * q);
            b0 = q * iv_alpha;
            b1 = 0;
            b2 = -q * iv_alpha;
            a0 = 1.0 + iv_alpha;
            a1 = -2.0 * iv_cos;
            a2 = 1.0 - iv_alpha;
            data[0][l1][l2] = 1.0 / a0 * (gain * b0 * inData[l1][l2] + b1 * buf[0][l1] + b2 * buf[1][l1]
                                       - a1 * buf[2][l1] - a2 * buf[3][l1]);
            buf[1][l1] = buf[0][l1];                          
            buf[0][l1] = gain * inData[l1][l2];
            buf[3][l1] = buf[2][l1];
            buf[2][l1] = data[0][l1][l2];
          }
        } 
        break;
      case VCF_BPF_II:
        q0 = resonance;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                    + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ) f = MAX_FREQ;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            iv_sin = sin(pi2_rate * f);
            iv_cos = cos(pi2_rate * f);
            iv_alpha = iv_sin/(64.0 * q);
            b0 = iv_alpha; 
            b1 = 0;
            b2 = -iv_alpha;
            a0 = 1.0 + iv_alpha;
            a1 = -2.0 * iv_cos;
            a2 = 1.0 - iv_alpha;
            data[0][l1][l2] = 1.0 / a0 * (gain * b0 * inData[l1][l2] + b1 * buf[0][l1] + b2 * buf[1][l1]
                                       - a1 * buf[2][l1] - a2 * buf[3][l1]);
            buf[1][l1] = buf[0][l1];                          
            buf[0][l1] = gain * inData[l1][l2];
            buf[3][l1] = buf[2][l1];
            buf[2][l1] = data[0][l1][l2];
          }
        } 
        break;
      case VCF_NF:
        q0 = resonance;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                    + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ) f = MAX_FREQ;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            iv_sin = sin(pi2_rate * f);
            iv_cos = cos(pi2_rate * f);
            iv_alpha = iv_sin/(64.0 * q);
            b0 = 1;
            b1 = -2.0 * iv_cos;
            b2 = 1;
            a0 = 1.0 + iv_alpha;
            a1 = -2.0 * iv_cos;
            a2 = 1.0 - iv_alpha;
            data[0][l1][l2] = 1.0 / a0 * (gain * b0 * inData[l1][l2] + b1 * buf[0][l1] + b2 * buf[1][l1]
                                       - a1 * buf[2][l1] - a2 * buf[3][l1]);
            buf[1][l1] = buf[0][l1];                          
            buf[0][l1] = gain * inData[l1][l2];
            buf[3][l1] = buf[2][l1];
            buf[2][l1] = data[0][l1][l2];
          }
        } 
        break;
      case VCF_MOOG1:                   // Timo Tossavainen version
        q0 = resonance;
        inv2_rate = 2.0 / (double)synthdata->rate;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                     + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ2) f = MAX_FREQ2;
            q = q0 + resonanceGain * resonanceData[l1][l2];
            if (q < 0.01) q = 0.01;
            else if (q > 1) q = 1;
            fa = inv2_rate * f;
            moog_f = fa * 1.16;
            fb = 4.1 * q * (1.0 - 0.15 * moog_f * moog_f);
            in[0][l1] = gain * inData[l1][l2] + 0.000001 * ((float)rand() / (float)RAND_MAX - 1.0);
            in[0][l1] -= fb * buf[4][l1];
            in[0][l1] *=0.35013 * (moog_f * moog_f * moog_f * moog_f);
            buf[1][l1] = in[0][l1] + 0.3 * in[1][l1] + (1.0 - moog_f) * buf[1][l1];
            in[1][l1] = in[0][l1];
            buf[2][l1] = buf[1][l1] + 0.3 * in[2][l1] + (1.0 - moog_f) * buf[2][l1];
            in[2][l1] = buf[1][l1]; 
            buf[3][l1] = buf[2][l1] + 0.3 * in[3][l1] + (1.0 - moog_f) * buf[3][l1];
            in[3][l1] = buf[2][l1];
            buf[4][l1] = buf[3][l1] + 0.3 * in[4][l1] + (1.0 - moog_f) * buf[4][l1];
            in[4][l1] = buf[3][l1];
            data[0][l1][l2] = buf[4][l1];
          }
        }  
        break;
      case VCF_MOOG2:                       // Paul Kellet version
        q0 = resonance;
        inv2_rate = 2.0 / (double)synthdata->rate;
        freq_tune = 5.0313842 + freq;
        gain_linfm = 1000.0 * vcfLinFMGain;
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            f = synthdata->exp_table(log2 * (freq_tune + freqData[l1][l2] + vcfExpFMGain * expFMData[l1][l2]))
                     + gain_linfm * linFMData[l1][l2];
            if (f < MIN_FREQ) f = MIN_FREQ;
            else if (f > MAX_FREQ2) f = MAX_FREQ2;
            qr = q0 + resonanceGain * resonanceData[l1][l2];
            if (qr < 0.01) qr = 0.01;
            else if (qr > 1) qr = 1;
            fb = inv2_rate * f;
            q = 1.0 - fb;                                              
            p = fb + 0.8 * fb * q;
            fa = p + p - 1.0;
            q = qr * (1.0 + 0.5 * q * (1.0 - q + 5.6 * q * q));
            in[0][l1] = gain * inData[l1][l2] + 0.000001 * ((float)rand() / (float)RAND_MAX - 1.0);
            in[0][l1] -= q * buf[4][l1];
            if (in[0][l1] < -1.0) in[0][l1] = -1.0;
            if (in[0][l1] > 1.0) in[0][l1] = 1.0;        
            t1 = buf[1][l1];
            buf[1][l1] = (in[0][l1] + buf[0][l1]) * p - buf[1][l1] * fa;
            t2 = buf[2][l1];
            buf[2][l1] = (buf[1][l1] + t1) * p - buf[2][l1] * fa;
            t1 = buf[3][l1];
            buf[3][l1] = (buf[2][l1] + t2) * p - buf[3][l1] * fa;
            buf[4][l1] = (buf[3][l1] + t1) * p - buf[4][l1] * fa;
            buf[4][l1] -= buf[4][l1] * buf[4][l1] * buf[4][l1] * 0.166667;
            buf[0][l1] = in[0][l1];
            data[0][l1][l2] = buf[4][l1];
          }
        }
        break;
    } 
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vcf::showConfigDialog() {
}
