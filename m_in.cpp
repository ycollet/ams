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
#include "m_in.h"
#include "port.h"
#include "module.h"

M_in::M_in(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(2, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_in;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_IN_WIDTH, MODULE_IN_HEIGHT);
  gain = 1.0;
  mixer_gain[0] = 1.0;
  mixer_gain[1] = 1.0;
  port_out[0] = new Port("Out 0", PORT_OUT, 0, this, synthdata);
  port_out[0]->move(width() - port_out[0]->width(), 35);
  port_out[0]->outType = outType_audio;
  portList.append(port_out[0]);
  port_out[1] = new Port("Out 1", PORT_OUT, 1, this, synthdata);
  port_out[1]->move(width() - port_out[1]->width(), 55);
  port_out[1]->outType = outType_audio;
  portList.append(port_out[1]);
  qs.sprintf("PCM In ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->addSlider(0, 2, gain, "Gain", &gain);
  configDialog->addSlider(0, 2, mixer_gain[0], "Volume 1", &mixer_gain[0]);
  configDialog->addSlider(0, 2, mixer_gain[1], "Volume 2", &mixer_gain[1]);
}

M_in::~M_in() {
}

void M_in::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "PCM In");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

int M_in::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

float M_in::getGain() {
  return(gain);
}

void M_in::generateCycle() {

  int l1, l2, l3;
  float mixgain[2];

  for (l1 = 0; l1 < 2; l1++) {                       // TODO generalize to more than 2 channels
    mixgain[l1] = mixer_gain[l1] * gain;
    if (synthdata->upperInDataReady) {
      for (l3 = 0; l3 < synthdata->poly; l3++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[l1][l3][l2] = mixgain[l1] * synthdata->indata[1][l1][l2];
        }
      }
    } else {
      for (l3 = 0; l3 < synthdata->poly; l3++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[l1][l3][l2] = mixgain[l1] * synthdata->indata[0][l1][l2];
        }
      }  
    }
  }  
}

void M_in::showConfigDialog() {
}
