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
#include <qtimer.h>
#include <qfiledialog.h>
#include <jack/jack.h>
#include "synthdata.h"
#include "synth.h"
#include "m_jackin.h"
#include "port.h"
#include "module.h"

M_jackin::M_jackin(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(2, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_jackin;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_JACKIN_WIDTH, MODULE_JACKIN_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  port_out[0] = new Port("Out 0", PORT_OUT, 0, this, synthdata);          
  port_out[0]->move(width() - port_out[0]->width(), 35);
  port_out[0]->outType = outType_audio;
  portList.append(port_out[0]);
  port_out[1] = new Port("Out 1", PORT_OUT, 1, this, synthdata);          
  port_out[1]->move(width() - port_out[1]->width(), 55);
  port_out[1]->outType = outType_audio;
  portList.append(port_out[1]);
  qs.sprintf("JACK In ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->addSlider(0, 1, gain, "Gain", &gain, false);
  configDialog->addSlider(0, 1, mixer_gain[0], "Volume 1", &mixer_gain[0], false);
  configDialog->addSlider(0, 1, mixer_gain[1], "Volume 2", &mixer_gain[1], false);
  QStrList *agcNames = new QStrList(true);
  if (synthdata->jack_in_ports < 2 * (1 + synthdata->jackInCount)) {
    synthdata->addJackPorts(2, 0);
  }
  for (l1 = 0; l1 < 2; l1++) {
    qs.sprintf("In_ID%d_%d", moduleID, l1);
    jackdata[l1] = (jack_default_audio_sample_t *)malloc(synthdata->periodsize * sizeof(jack_default_audio_sample_t));
    jack_in[l1] = synthdata->jack_in[2 * synthdata->jackInCount + l1];
  }
}

M_jackin::~M_jackin() {

  int l1;

  for (l1 = 0; l1 < 2; l1++) {
    free(jackdata[l1]);
  }
}

void M_jackin::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "JACK In");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_jackin::generateCycle() {

  int l1, l2, l3, index;
  float max_ch, mixgain;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < 2; l1++) {                       // TODO generalize to more than 2 channels
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          data[l1][l3][l2] = jackdata[l1][l2];
        }
      }
    }  
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_jackin::showConfigDialog() {
}
