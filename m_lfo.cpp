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
#include "m_lfo.h"
#include "port.h"

M_lfo::M_lfo(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(5, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_lfo;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LFO_WIDTH, MODULE_LFO_HEIGHT);
  freq = 5;
  phi0 = 0;
  wave_period = (float)WAVE_PERIOD;
  for (l1 = 0; l1 < MAXPOLY; l1++) {
    trigger[l1] = false;
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
  qs.sprintf("LFO ID %d", moduleID);
  configDialog->setCaption(qs);
  FloatParameter *fp = new FloatParameter(this,"Frequency","", 0.0,100.0,&freq);
  configDialog->addParameter(fp);
  fp = new FloatParameter(this,"Phi0","",0.0,6.283,&phi0);
  configDialog->addParameter(fp);
  
}

M_lfo::~M_lfo() {
}

void M_lfo::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(12, 20, "LFO");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_lfo::noteOnEvent(int osc) {

}

void M_lfo::generateCycle() {

  int l1, l2;
  float dphi, phi1, phi_const; 

  if (!cycleReady) {
    cycleProcessing = true; 
    for (l2 = 0; l2 < 5; l2++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
      }
    }
    if (port_M_trigger->connectedPortList.count()) {
      in_M_trigger = (Module *)port_M_trigger->connectedPortList.at(0)->parentModule;
      if (!in_M_trigger->cycleProcessing) {
        in_M_trigger->generateCycle();
        triggerData = in_M_trigger->data[0];
      } else {
        triggerData = in_M_trigger->lastdata[0];
      }
    } else { 
      in_M_trigger = NULL;
      triggerData = synthdata->zeroModuleData;
    }
      dphi = freq * wave_period / (float)synthdata->rate;
      phi_const = phi0 * wave_period / (2.0 * PI);
      if (phi0 > 0) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            phi1 = phi[l1] + phi_const;
            if (phi1 < 0) phi1 += wave_period;
            if (phi1 > wave_period) phi1 -= wave_period;
            data[0][l1][l2] = synthdata->wave_sine[(int)phi1];
            data[1][l1][l2] = synthdata->wave_tri[(int)phi1];
            data[2][l1][l2] = synthdata->wave_saw[(int)phi1];
            data[3][l1][l2] = synthdata->wave_saw2[(int)phi1];
            data[4][l1][l2] = synthdata->wave_rect[(int)phi1];
            phi[l1] += dphi;
            while (phi[l1] < 0) phi[l1] += wave_period;
            while (phi[l1] > wave_period) phi[l1] -= wave_period;
            if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
              trigger[l1] = true;
              phi[l1] = 0;
            }
            if (trigger[l1] && (triggerData[l1][l2] < 0.5)) {
              trigger[l1] = false;
            }
          }
        }
      } else {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[0][l1][l2] = synthdata->wave_sine[(int)phi[l1]];
            data[1][l1][l2] = synthdata->wave_tri[(int)phi[l1]]; 
            data[2][l1][l2] = synthdata->wave_saw[(int)phi[l1]]; 
            data[3][l1][l2] = synthdata->wave_saw2[(int)phi[l1]];
            data[4][l1][l2] = synthdata->wave_rect[(int)phi[l1]];
            phi[l1] += dphi;
            while (phi[l1] < 0) phi[l1] += wave_period;
            while (phi[l1] > wave_period) phi[l1] -= wave_period;
            if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
              trigger[l1] = true;
              phi[l1] = 0;    
            }
            if (trigger[l1] && (triggerData[l1][l2] < 0.5)) {
              trigger[l1] = false;
            }
          }
        }
      }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_lfo::showConfigDialog() {
}
