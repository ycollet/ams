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
#include "m_sh.h"
#include "port.h"

M_sh::M_sh(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(2, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_sh;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SH_WIDTH, MODULE_SH_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this, synthdata); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_M_trig = new Port("Trigger", PORT_IN, 1, this, synthdata); 
  port_M_trig->move(0, 55);
  port_M_trig->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_trig);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 35);
  port_out->outType = outType_audio;
  portList.append(port_out);
  port_gate = new Port("Gate", PORT_OUT, 1, this, synthdata);          
  port_gate->move(width() - port_gate->width(), 55);
  port_gate->outType = outType_audio;
  portList.append(port_gate);
  qs.sprintf("Sample & Hold ID %d", moduleID);
  configDialog->setCaption(qs);
  triggerLevel = 0.5;
  configDialog->addSlider(0, 10, triggerLevel, "Trigger Level", &triggerLevel);
  sample = 0;
  gate = false;
}

M_sh::~M_sh() {
}

void M_sh::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "Sample & Hold");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_sh::generateCycle() {

  int l1, l2;
  float log2;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l2 = 0; l2 < 2; l2++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[l2][l1], data[l2][l1], synthdata->cyclesize * sizeof(float));
      }
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
    if (port_M_trig->connectedPortList.count()) {
      in_M_trig = (Module *)port_M_trig->connectedPortList.at(0)->parentModule;
      if (!in_M_trig->cycleProcessing) {
        in_M_trig->generateCycle();
        trigData = in_M_trig->data[port_M_trig->connectedPortList.at(0)->index];
      } else {
        trigData = in_M_trig->lastdata[port_M_trig->connectedPortList.at(0)->index];
      }
    } else {
      in_M_trig = NULL;
      trigData = synthdata->zeroModuleData;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if ( !gate && (trigData[l1][l2] > triggerLevel)) {
          sample = inData[l1][l2]; 
          gate = true;
        } else {
          gate = trigData[l1][l2] > triggerLevel;
        }
        data[0][l1][l2] = sample;
        data[1][l1][l2] = (gate) ? 1 : 0;
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_sh::showConfigDialog() {
}
