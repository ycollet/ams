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
#include "m_amp.h"
#include "port.h"

M_amp::M_amp(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_amp;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_AMP_WIDTH, MODULE_AMP_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this, synthdata); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);
  gain = 1;
  configDialog->addSlider(-10, 10, gain, "Gain", &gain);
  qs.sprintf("Amplifier ID %d", moduleID);
  configDialog->setCaption(qs);
}

M_amp::~M_amp() {
}

void M_amp::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = gain * inData[l1][l2];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_amp::showConfigDialog() {
}