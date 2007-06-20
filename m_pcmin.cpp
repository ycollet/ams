#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qtimer.h>
#include "synthdata.h"
#include "m_pcmin.h"
#include "module.h"
#include "port.h"


M_pcmin::M_pcmin(QWidget* parent, const char *name, int port) 
              : Module(2, parent, name)
 {
  QString qs;

  M_type = M_type_pcmin;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_PCMIN_WIDTH, MODULE_PCMIN_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  qs.sprintf ("In %2d -> ", port);
  port_out[0] = new Port(qs, PORT_OUT, 0, this);          
  port_out[0]->move(width() - port_out[0]->width(), 35);
  port_out[0]->outType = outType_audio;
  portList.append(port_out[0]);
  qs.sprintf ("In %2d -> ", port + 1);
  port_out[1] = new Port(qs, PORT_OUT, 1, this);          
  port_out[1]->move(width() - port_out[1]->width(), 55);
  port_out[1]->outType = outType_audio;
  portList.append(port_out[1]);
  qs.sprintf("Alsa / Jack In  ID %d", moduleID);
  configDialog->setWindowTitle(qs);
  configDialog->addSlider(0, 1, gain, "Gain", &gain, false);
  configDialog->addSlider(0, 1, mixer_gain[0], "Volume 1", &mixer_gain[0], false);
  configDialog->addSlider(0, 1, mixer_gain[1], "Volume 2", &mixer_gain[1], false);
  pcmdata[0] = new float [synthdata->periodsize];
  pcmdata[1] = new float [synthdata->periodsize];
}

M_pcmin::~M_pcmin()
{
  delete[] pcmdata [0];
  delete[] pcmdata [1];
}

void M_pcmin::generateCycle() {

  int l1, l2, l3;
  float mixgain;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < 2; l1++) {
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          data[l1][l3][l2] = mixgain * pcmdata[l1][l2];
        }
      }
    }  
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_pcmin::showConfigDialog() {
}
