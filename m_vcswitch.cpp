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
#include "m_vcswitch.h"
#include "port.h"

M_vcswitch::M_vcswitch(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(3, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_vcswitch;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCSWITCH_WIDTH, MODULE_VCSWITCH_HEIGHT);
  port_M_cv = new Port("CV", PORT_IN, 0, this, synthdata); 
  port_M_cv->move(0, 35);
  port_M_cv->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_cv);
  port_M_in[0] = new Port("In 0", PORT_IN, 1, this, synthdata); 
  port_M_in[0]->move(0, 55);
  port_M_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in[0]);
  port_M_in[1] = new Port("In 1", PORT_IN, 2, this, synthdata); 
  port_M_in[1]->move(0, 75);
  port_M_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in[1]);
  port_out[0] = new Port("Out 0", PORT_OUT, 0, this, synthdata);          
  port_out[0]->move(width() - port_out[0]->width(), 35);
  port_out[0]->outType = outType_audio;
  portList.append(port_out[0]);
  port_out[1] = new Port("Out 1", PORT_OUT, 1, this, synthdata);          
  port_out[1]->move(width() - port_out[1]->width(), 55);
  port_out[1]->outType = outType_audio;
  portList.append(port_out[1]);
  port_mix = new Port("Mix", PORT_OUT, 2, this, synthdata);          
  port_mix->move(width() - port_mix->width(), 75);
  port_mix->outType = outType_audio;
  portList.append(port_mix);
  qs.sprintf("VC Switch ID %d", moduleID);
  configDialog->setCaption(qs);
  switchLevel = 0.5;
  FloatParameter *fp = new FloatParameter(this,"Switch Level","",0.0,10.0, &switchLevel);
  configDialog->addParameter(fp);
}

M_vcswitch::~M_vcswitch() {
}

void M_vcswitch::paintEvent(QPaintEvent *ev) {

  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "VC Switch");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_vcswitch::generateCycle() {

  int l1, l2;
  float mix1, mix2;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l2 = 0; l2 < 3; l2++) {        
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[l2][l1], data[l2][l1], synthdata->cyclesize * sizeof(float));
      }
    }
    if (port_M_in[0]->connectedPortList.count()) {
      in_M_in[0] = (Module *)port_M_in[0]->connectedPortList.at(0)->parentModule;
      if (!in_M_in[0]->cycleProcessing) {
        in_M_in[0]->generateCycle();
        inData[0] = in_M_in[0]->data[port_M_in[0]->connectedPortList.at(0)->index];
      } else {
        inData[0] = in_M_in[0]->lastdata[port_M_in[0]->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in[0] = NULL;
      inData[0] = synthdata->zeroModuleData;
    }
    if (port_M_in[1]->connectedPortList.count()) {
      in_M_in[1] = (Module *)port_M_in[1]->connectedPortList.at(0)->parentModule;
      if (!in_M_in[1]->cycleProcessing) {
        in_M_in[1]->generateCycle();
        inData[1] = in_M_in[1]->data[port_M_in[1]->connectedPortList.at(0)->index];
      } else {
        inData[1] = in_M_in[1]->lastdata[port_M_in[1]->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in[1] = NULL;
      inData[1] = synthdata->zeroModuleData;
    }
    if (port_M_cv->connectedPortList.count()) {
      in_M_cv = (Module *)port_M_cv->connectedPortList.at(0)->parentModule;
      if (!in_M_cv->cycleProcessing) {
        in_M_cv->generateCycle();
        cvData = in_M_cv->data[port_M_cv->connectedPortList.at(0)->index];
      } else {
        cvData = in_M_cv->lastdata[port_M_cv->connectedPortList.at(0)->index];
      }
    } else {
      in_M_cv = NULL;
      cvData = synthdata->zeroModuleData;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (cvData[l1][l2] > switchLevel) {
          data[0][l1][l2] = inData[1][l1][l2]; 
          data[1][l1][l2] = inData[0][l1][l2];
        } else {
          data[0][l1][l2] = inData[0][l1][l2];
          data[1][l1][l2] = inData[1][l1][l2];
        }
        mix1 = cvData[l1][l2];
        mix2 = 2.0 * switchLevel - mix1;
        if (mix2 < 0) {
          mix2 = 0;
          mix1 = 2.0 * switchLevel;
        }
        data[2][l1][l2] = (mix1 * inData[0][l1][l2] + mix2 * inData[1][l1][l2]) / (mix1 + mix2);
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_vcswitch::showConfigDialog() {
}
