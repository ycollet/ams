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
#include "m_vca.h"
#include "port.h"

M_vca::M_vca(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_vca;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCA_WIDTH, MODULE_VCA_HEIGHT);
  gain1 = 0;
  gain2 = 0;
  in1 = 1.0;
  in2 = 1.0;
  out = 1.0;
  port_M_gain1 = new Port("Gain 0", PORT_IN, 0, this, synthdata); 
  port_M_gain1->move(0, 35);
  port_M_gain1->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_gain1);
  port_M_gain2 = new Port("Gain 1", PORT_IN, 1, this, synthdata); 
  port_M_gain2->move(0, 55);
  port_M_gain2->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_gain2);
  port_M_in1 = new Port("In 0", PORT_IN, 2, this, synthdata); 
  port_M_in1->move(0, 75);
  port_M_in1->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in1);
  port_M_in2 = new Port("In 1", PORT_IN, 3, this, synthdata); 
  port_M_in2->move(0, 95);
  port_M_in2->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in2);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 115);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("VCA ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->addSlider(0, 10, gain1, "Gain", &gain1);
  configDialog->addSlider(0, 10, gain2, "Gain 1", &gain2);
  configDialog->addSlider(0, 2, in1, "In 0", &in1);
  configDialog->addSlider(0, 2, in2, "In 1", &in2);
  configDialog->addSlider(0, 2, out, "Output level", &out);
}

M_vca::~M_vca() {
}

void M_vca::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(12, 20, "VCA");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_vca::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }
    if (port_M_gain1->connectedPortList.count()) {
      in_M_gain1 = (Module *)port_M_gain1->connectedPortList.at(0)->parentModule;
      if (!in_M_gain1->cycleProcessing) {
        in_M_gain1->generateCycle();
        gainData1 = in_M_gain1->data[port_M_gain1->connectedPortList.at(0)->index];
      } else {
        gainData1 = in_M_gain1->lastdata[port_M_gain1->connectedPortList.at(0)->index];
      }
    } else {
      in_M_gain1 = NULL;
      gainData1 = synthdata->zeroModuleData;
    }
    if (port_M_in1->connectedPortList.count()) {
      in_M_in1 = (Module *)port_M_in1->connectedPortList.at(0)->parentModule;
      if (!in_M_in1->cycleProcessing) {
        in_M_in1->generateCycle();
        inData1 = in_M_in1->data[port_M_in1->connectedPortList.at(0)->index];
      } else {
        inData1 = in_M_in1->lastdata[port_M_in1->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in1 = NULL;
      inData1 = synthdata->zeroModuleData;
    }
    if (port_M_gain2->connectedPortList.count()) {
      in_M_gain2 = (Module *)port_M_gain2->connectedPortList.at(0)->parentModule;
      if (!in_M_gain2->cycleProcessing) {
        in_M_gain2->generateCycle();
        gainData2 = in_M_gain2->data[port_M_gain2->connectedPortList.at(0)->index];
      } else {
        gainData2 = in_M_gain2->lastdata[port_M_gain2->connectedPortList.at(0)->index];
      }
    } else {
      in_M_gain2 = NULL;
      gainData2 = synthdata->zeroModuleData;
    }
    if (port_M_in2->connectedPortList.count()) {
      in_M_in2 = (Module *)port_M_in2->connectedPortList.at(0)->parentModule;
      if (!in_M_in2->cycleProcessing) {
        in_M_in2->generateCycle();
        inData2 = in_M_in2->data[port_M_in2->connectedPortList.at(0)->index];
      } else {
        inData2 = in_M_in2->lastdata[port_M_in2->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in2 = NULL;
      inData2 = synthdata->zeroModuleData;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = (gain1 + gainData1[l1][l2] + gain2 * gainData2[l1][l2]) 
                        * out * (in1 * inData1[l1][l2] + in2 * inData2[l1][l2]);
      }
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }  
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vca::showConfigDialog() {
}
