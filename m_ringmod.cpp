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
#include "m_ringmod.h"
#include "port.h"

M_ringmod::M_ringmod(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_ringmod;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_RINGMOD_WIDTH, MODULE_RINGMOD_HEIGHT);
  gain = 0.5;
  port_M_vco1 = new Port("In 0", PORT_IN, 0, this, synthdata); 
  port_M_vco1->move(0, 35);
  port_M_vco1->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_vco1);
  port_M_vco2 = new Port("In 1", PORT_IN, 1, this, synthdata);
  port_M_vco2->move(0, 55);
  port_M_vco2->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_vco2);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 70);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Ring Modulator ID %d", moduleID);
  configDialog->setCaption(qs);
  FloatParameter *fp = new FloatParameter(this,"Gain","",0.0,5.0,&gain);
  configDialog->addParameter(fp);
}

M_ringmod::~M_ringmod() {
}

void M_ringmod::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "Ring Mod");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

int M_ringmod::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

int M_ringmod::connect_vco1(Module *p_M_vco) {
  in_M_vco1 = p_M_vco;
  return(0);
}

int M_ringmod::connect_vco2(Module *p_M_vco) {
  in_M_vco2 = p_M_vco;
  return(0);
}

float M_ringmod::getGain() {
  return(gain);
}

void M_ringmod::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }
    if (port_M_vco1->connectedPortList.count()) {
      in_M_vco1 = (Module *)port_M_vco1->connectedPortList.at(0)->parentModule;
      if (!in_M_vco1->cycleProcessing) {
        in_M_vco1->generateCycle();
        vcoData1 = in_M_vco1->data[port_M_vco1->connectedPortList.at(0)->index];
      } else {
        vcoData1 = in_M_vco1->lastdata[port_M_vco1->connectedPortList.at(0)->index];
      }
    } else {
      in_M_vco1 = NULL;
      vcoData1 = synthdata->zeroModuleData;
    }
    if (port_M_vco2->connectedPortList.count()) {
      in_M_vco2 = (Module *)port_M_vco2->connectedPortList.at(0)->parentModule;
      if (!in_M_vco2->cycleProcessing) {
        in_M_vco2->generateCycle();
        vcoData2 = in_M_vco2->data[port_M_vco2->connectedPortList.at(0)->index];
      } else {
        vcoData2 = in_M_vco2->lastdata[port_M_vco2->connectedPortList.at(0)->index];
      }
    } else {
      in_M_vco2 = NULL;
      vcoData2 = synthdata->zeroModuleData;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = gain * vcoData1[l1][l2] * vcoData2[l1][l2];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_ringmod::showConfigDialog() {
}
