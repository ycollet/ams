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
#include "m_out.h"
#include "port.h"
#include "module.h"

M_out::M_out(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(0, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_out;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_OUT_WIDTH, MODULE_OUT_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  agc = 1;
  port_in[0] = new Port("In 0", PORT_IN, 0, this, synthdata);
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  port_in[1] = new Port("In 1", PORT_IN, 1, this, synthdata);
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);
  qs.sprintf("PCM Out ID %d", moduleID);
  configDialog->setCaption(qs);
  FloatParameter * pGain = new FloatParameter(this,"Gain","",0.0,1.0,&gain);
  FloatParameter * pGain1 = new FloatParameter(this,"Volume 1","",0.0,1.0,&mixer_gain[0]);
  FloatParameter * pGain2 = new FloatParameter(this,"Volume 2","",0.0,1.0,&mixer_gain[1]);
  EnumParameter * pAgc = new EnumParameter(this,"Automatic Gain Control","",(int *)&agc);
  pAgc->addItem(0,"Disabled");
  pAgc->addItem(1,"Enabled");

  configDialog->addParameter(pGain);
  configDialog->addParameter(pGain1);
  configDialog->addParameter(pGain2);
  configDialog->addParameter(pAgc);

}

M_out::~M_out() {
}

void M_out::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "PCM Out");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

int M_out::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

float M_out::getGain() {
  return(gain);
}

void M_out::generateCycle() {

  int l1, l2, l3, index;
  float max_ch, mixgain;

  for (l1 = 0; l1 < 2; l1++) {                       // TODO generalize to more than 2 channels
    if (port_in[l1]->connectedPortList.count()) {
      module_in[l1] = (Module *)port_in[l1]->connectedPortList.at(0)->parentModule;
      module_in[l1]->generateCycle();
      index = port_in[l1]->connectedPortList.at(0)->index;
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          synthdata->data[l1][l2] += mixgain * module_in[l1]->data[index][l3][l2]; 
        }
      }
      if (agc) {
        max_ch = 0;
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          if (max_ch < fabs(synthdata->data[l1][l2])) {
            max_ch = fabs(synthdata->data[l1][l2]);
          }    
        }
        if (max_ch > 0.9) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            synthdata->data[l1][l2] *= 0.9 / max_ch;
          }
        }
      }
    } else {
      module_in[l1] = NULL;
    }
  }  
}

void M_out::showConfigDialog() {
}
