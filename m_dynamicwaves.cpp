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
#include "m_dynamicwaves.h"
#include "multi_envelope.h"
#include "port.h"

M_dynamicwaves::M_dynamicwaves(int p_oscCount, QWidget* parent, const char *name, SynthData *p_synthdata)
              : Module(1, parent, name, p_synthdata) {

  QString qs,label;
  int l1, l2;
  QVBox *oscTab[MODULE_DYNAMICWAVES_MAX_OSC];
  QVBox *envAttackTab[MODULE_DYNAMICWAVES_MAX_OSC];
  QVBox *envReleaseTab[MODULE_DYNAMICWAVES_MAX_OSC];

  M_type = M_type_dynamicwaves;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DYNAMICWAVES_WIDTH, MODULE_DYNAMICWAVES_HEIGHT);
  wave_period = (float)WAVE_PERIOD;
  tune = 0;
  octave = 3;
  expFMGain = 0;
  linFMGain = 0;
  oscCount = p_oscCount;
  allEnvTerminated = true;
  for (l1 = 0; l1 < oscCount; l1++) {
    gain[l1] = 1;
    timeScale[l1] = 1;
    osc_tune[l1] = 0;
    harmonic[l1] = 1 + l1;
    subharmonic[l1] = 1;
    osc_octave[l1] = 0;
    waveForm[l1] = DYNAMICWAVE_SINE;
    phi0[l1] = 0;
    attack[0][l1] = 0;
    attack[1][l1] = 0.01;
    attack[2][l1] = 0.5;
    attack[3][l1] = 0.01;
    attack[4][l1] = 1.0;
    attack[5][l1] = 0.1;
    attack[6][l1] = 0.9;
    attack[7][l1] = 0.1;
    sustain[l1] = 0.8;
    release[0][l1] = 0.01;
    release[1][l1] = 0.7;
    release[2][l1] = 0.01;
    release[3][l1] = 0.5;
    release[4][l1] = 0.01;
  }
  for (l1 = 0; l1 < MAXPOLY; l1++) {
    gate[l1] = false;
    noteActive[l1] = false;
    for (l2 = 0; l2 < oscCount; l2++) {
      phi[l1][l2] = 0;
      oscActive[l1][l2] = false;
      noteOnOfs[l1][l2] = 0;
      noteOffOfs[l1][l2] = 0;
      e[l1][l2] = 0;
      de[l1][l2] = 0;
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
  port_gate = new Port("Gate", PORT_IN, 3, this, synthdata);
  port_gate->move(0, 95);
  port_gate->outTypeAcceptList.append(outType_audio);
  portList.append(port_gate);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);
  port_out->move(width() - port_out->width(), 115);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Dynamic Waves ID %d", moduleID);
  configDialog->setCaption(qs);


  MultiEnvelope *menv = new MultiEnvelope(oscCount, &timeScale[0], &attack[0][0], &sustain[0], &release[0][0],configDialog->headerBox);

  qs.sprintf("Tune/Modulation");
  Parameter *p;
  p = new IntParameter(this, "Octave","",0,6,&octave);
  configDialog->addParameter(p,qs);
  p = new FloatParameter(this, "Tune","",0.0,1.0,&tune);
  configDialog->addParameter(p,qs);
  p = new FloatParameter(this, "Exp. FM Gain","",0.0,1.0,&expFMGain);
  configDialog->addParameter(p,qs);
  p = new FloatParameter(this, "Lin. FM Gain","",0.0,1.0,&linFMGain);
  configDialog->addParameter(p,qs);

  qs.sprintf("Mixer");
  for (l1 = 0; l1 < oscCount; l1++) {
    label.sprintf("Volume %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&gain[l1]);
    configDialog->addParameter(p,qs);
  }
  qs.sprintf("Env. Time Scale");
  for (l1 = 0; l1 < oscCount; l1++) {
    label.sprintf("Time Scale %d", l1);
    p = new FloatParameter(this,label,"",0.1,10.0,&timeScale[l1]);
    configDialog->addParameter(p,qs);
  }
  EnumParameter *ep;
  for (l1 = 0; l1 < oscCount; l1++) {
    qs.sprintf("Osc %d", l1);

    label.sprintf("Wave Form %d", l1);
    ep=new EnumParameter(this,label,"",(int *)&waveForm[l1]);
    ep->addItem(DYNAMICWAVE_SINE,"Sine");
    ep->addItem(DYNAMICWAVE_SAW,"Saw");
    ep->addItem(DYNAMICWAVE_TRI,"Triangel");
    ep->addItem(DYNAMICWAVE_RECT,"Square");
    ep->addItem(DYNAMICWAVE_SAW2,"Saw 2");
    configDialog->addParameter(ep,qs);

    label.sprintf("Octave %d", l1);
    p=new IntParameter(this,label,"",0,3,&osc_octave[l1]);
    configDialog->addParameter(p,qs);
    label.sprintf("Tune %d", l1);
    p=new FloatParameter(this,label,"",0.0,1.0,&osc_tune[l1]);
    configDialog->addParameter(p,qs);

    label.sprintf("Harmonic %d", l1);
    p=new IntParameter(this,label,"",1,16,&harmonic[l1]);
    configDialog->addParameter(p,qs);

    label.sprintf("Subharmonic %d", l1);
    p=new IntParameter(this,label,"",1,16,&subharmonic[l1]);
    configDialog->addParameter(p,qs);

    label.sprintf("Phi0 %d", l1);
    p=new FloatParameter(this,label,"",0.0,6.283,phi0[l1]);
    configDialog->addParameter(p,qs);

  }
  for (l1 = 0; l1 < oscCount; l1++) {
    qs.sprintf("Attack %d", l1);

    label.sprintf("Attack Time 0 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[1][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Attack Level 0 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[2][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Attack Time 1 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[3][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Attack Level 1 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[4][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Attack Time 2 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[5][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Attack Level 2 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[6][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Attack Time 3 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[7][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));


    qs.sprintf("Sustain / Release %d", l1);


    label.sprintf("Delay %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&attack[0][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Sustain %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&sustain[l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Release Time 0 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&release[0][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Release Level 0 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&release[1][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Release Time 1 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&release[2][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Release Level 1 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&release[3][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));

    label.sprintf("Release Time 2 %d", l1);
    p = new FloatParameter(this,label,"",0.0,1.0,&release[4][l1]);
    configDialog->addParameter(p,qs);
    connect(p,SIGNAL(valueChanged(float)),menv,SLOT(updateMultiEnvelope(float)));
  }

}

M_dynamicwaves::~M_dynamicwaves() {
}

void M_dynamicwaves::paintEvent(QPaintEvent *ev) {

  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(12, 20, "Dynamic Waves");
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_dynamicwaves::noteOnEvent(int osc) {
}

void M_dynamicwaves::generateCycle() {

  int l1, l2, l3, l4, status;
  float dphi, phi1; 
  float freq_const[MODULE_DYNAMICWAVES_MAX_OSC], freq_tune[MODULE_DYNAMICWAVES_MAX_OSC];
  float gain_linfm, log2, wave_period_2, current_gain;
  float gain_const[MODULE_DYNAMICWAVES_MAX_OSC], phi_const[MODULE_DYNAMICWAVES_MAX_OSC];
  float t[8][MODULE_DYNAMICWAVES_MAX_OSC], tscale[MODULE_DYNAMICWAVES_MAX_OSC];
  float de_a[4][MODULE_DYNAMICWAVES_MAX_OSC];
  float de_d[3][MODULE_DYNAMICWAVES_MAX_OSC];

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
    if (port_gate->connectedPortList.count()) {
      in_M_gate = (Module *)port_gate->connectedPortList.at(0)->parentModule;
      if (!in_M_gate->cycleProcessing) {
        in_M_gate->generateCycle();
        gateData = in_M_gate->data[port_gate->connectedPortList.at(0)->index];
      } else {
        gateData = in_M_gate->lastdata[port_gate->connectedPortList.at(0)->index];
      }
    } else {
      in_M_gate = NULL; 
      gateData = synthdata->zeroModuleData;
    }
    gain_linfm = 1000.0 * linFMGain;
    for (l3 = 0; l3 < oscCount; l3++) {
      gain_const[l3] = gain[l3] / (float)oscCount;
      freq_tune[l3] = 4.0313842 + octave + tune + osc_octave[l3] + osc_tune[l3];
      freq_const[l3] = wave_period / (float)synthdata->rate * (float)harmonic[l3] / (float)subharmonic[l3];
      phi_const[l3] = phi0[l3] * wave_period / (2.0 * PI);
      tscale[l3] = timeScale[l3] * synthdata->rate;
      de_a[0][l3] = (attack[1][l3] > 0) ? attack[2][l3] / (tscale[l3] * attack[1][l3]) : 0;
      de_a[1][l3] = (attack[3][l3] > 0) ? (attack[4][l3] - attack[2][l3]) / (tscale[l3] * attack[3][l3]) : 0;
      de_a[2][l3] = (attack[5][l3] > 0) ? (attack[6][l3] - attack[4][l3]) / (tscale[l3] * attack[5][l3]) : 0;
      de_a[3][l3] = (attack[7][l3] > 0) ? (sustain[l3] - attack[6][l3]) / (tscale[l3] * attack[7][l3]) : 0;
      de_d[0][l3] = (release[0][l3] > 0) ? (release[1][l3] - sustain[l3]) / (tscale[l3] * release[0][l3]) : 0;
      de_d[1][l3] = (release[2][l3] > 0) ? (release[3][l3] - release[1][l3]) / (tscale[l3] * release[2][l3]) : 0;
      de_d[2][l3] = (release[4][l3] > 0) ? - release[3][l3] / (tscale[l3] * release[4][l3]) : 0;
      t[0][l3] = tscale[l3] * attack[0][l3];
      t[1][l3] = t[0][l3] + tscale[l3] * attack[1][l3];
      t[2][l3] = t[1][l3] + tscale[l3] * attack[3][l3];
      t[3][l3] = t[2][l3] + tscale[l3] * attack[5][l3];
      t[4][l3] = t[3][l3] + tscale[l3] * attack[7][l3];
      t[5][l3] = tscale[l3] * release[0][l3];
      t[6][l3] = t[5][l3] + tscale[l3] * release[2][l3];
      t[7][l3] = t[6][l3] + tscale[l3] * release[4][l3];
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {  
      memset(data[0][l1], 0, synthdata->cyclesize * sizeof(float));
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        noteActive[l1] = !allEnvTerminated;
        allEnvTerminated = true;
        for (l3 = 0; l3 < oscCount; l3++) {
          if (l3 == 0) {
            if (!gate[l1] && (gateData[l1][l2] > 0.5)) {
              gate[l1] = true;
              noteActive[l1] = true;
              for (l4 = 0; l4 < oscCount; l4++) {
                oscActive[l1][l4] = true;
                if (e[l1][l4] > 0) {  
                  noteOnOfs[l1][l4] = -DYNAMICWAVES_ENVELOPE_RESPONSE; 
                  de[l1][l4] = e[l1][l4] / (float)DYNAMICWAVES_ENVELOPE_RESPONSE;
                } else {
                  noteOnOfs[l1][l4] = 0;
                }
              }
            }
            if (gate[l1] && (gateData[l1][l2] < 0.5)) {
              gate[l1] = false;
              for (l4 = 0; l4 < oscCount; l4++) {
                noteOffOfs[l1][l4] = 0;
                e_noteOff[l1][l4] = e[l1][l4];
                de_release[l1][l4] = (release[0][l4] > 0) ? (release[1][l4] - e_noteOff[l1][l4]) / (tscale[l4] * release[0][l4]) : 0;
              }
            }
          }
          if (gate[l1]) {
            status = 1;
            if (noteOnOfs[l1][l3] < 0) status = 0;
            if (noteOnOfs[l1][l3] >= long(t[0][l3])) status = 2;
            if (noteOnOfs[l1][l3] >= long(t[1][l3])) status = 3;
            if (noteOnOfs[l1][l3] >= long(t[2][l3])) status = 4;
            if (noteOnOfs[l1][l3] >= long(t[3][l3])) status = 5;
            if (noteOnOfs[l1][l3] >= long(t[4][l3])) status = 6;
            switch (status) {
              case 0: e[l1][l3] -= de[l1][l3];
                      break;
              case 1: e[l1][l3] = 0;
                      break;
              case 2: e[l1][l3] += de_a[0][l3];
                      break;
              case 3: e[l1][l3] += de_a[1][l3];
                      break;
              case 4: e[l1][l3] += de_a[2][l3];
                      break;
              case 5: e[l1][l3] += de_a[3][l3];
                      break;
              case 6: e[l1][l3] = sustain[l3];
                      break;
              default: e[l1][l3] = 0;
                       break;
            }
            if (e[l1][l3] < 0) e[l1][l3] = 0;
            noteOnOfs[l1][l3]++;
//fprintf(stderr, "Attack status: %d, e[%d][%d]: %f\n", status, l1, l3, e[l1][l3]);
          } else {
            if (oscActive[l1][l3] > 0) {
              status = 1;
              if (noteOffOfs[l1][l3] < 0) status = 0;
              if (noteOffOfs[l1][l3] >= long(t[5][l3])) status = 2;
              if (noteOffOfs[l1][l3] >= long(t[6][l3])) status = 3;
              if (noteOffOfs[l1][l3] >= long(t[7][l3])) status = 4;
              switch (status) {
                case 0: e[l1][l3] = 0;
                        break;
                case 1: e[l1][l3] += de_release[l1][l3];
                        break;
                case 2: e[l1][l3] += de_d[1][l3];
                        break;
                case 3: e[l1][l3] += de_d[2][l3];
                        break;
                case 4: e[l1][l3] = 0;
                        break;
                default: e[l1][l3] = 0;
                         break;
              }
              if (e[l1][l3] < 0) e[l1][l3] = 0;
            } 
            noteOffOfs[l1][l3]++;
            if (noteOffOfs[l1][l3] >= int(t[7][l3])) {
              oscActive[l1][l3] = false;
            }
//            if (l3 == 0) {
//              fprintf(stderr, "Release status: %d, e[%d][%d]: %f\n", status, l1, l3, e[l1][l3]);
//            }
          }
          if (oscActive[l1][l3]) {
            allEnvTerminated = false;
          }
          dphi = freq_const[l3] * (synthdata->exp_table(log2 * (freq_tune[l3] + freqData[l1][l2] + expFMGain * expFMData[l1][l2])) 
                                                       + gain_linfm * linFMData[l1][l2]);
          if (dphi > wave_period_2) {
            dphi = wave_period_2;
            current_gain = 0;
          } else {
            current_gain = gain_const[l3] * e[l1][l3];
          }
//fprintf(stderr, "current_gain: %f\n", current_gain);
          phi1 = phi[l1][l3] + phi_const[l3];
//fprintf(stderr, "phi1: %f\n", phi1);
          if (phi1 < 0) phi1 += wave_period;
          else if (phi1 > wave_period) phi1 -= wave_period;
          switch (waveForm[l3]) {
            case DYNAMICWAVE_SINE:
              data[0][l1][l2] += current_gain * synthdata->wave_sine[(int)phi1];
              break;
            case DYNAMICWAVE_SAW:
              data[0][l1][l2] += current_gain * synthdata->wave_saw[(int)phi1];
              break;
            case DYNAMICWAVE_TRI: 
              data[0][l1][l2] += current_gain * synthdata->wave_tri[(int)phi1];
              break;
            case DYNAMICWAVE_RECT: 
              data[0][l1][l2] += current_gain * synthdata->wave_rect[(int)phi1];
              break;
            case DYNAMICWAVE_SAW2: 
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
  cycleProcessing = false;
  cycleReady = true;
}

void M_dynamicwaves::showConfigDialog() {
}
