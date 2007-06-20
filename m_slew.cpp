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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_slew.h"
#include "port.h"

M_slew::M_slew(QWidget* parent, const char *name) 
              : Module(1, parent, name) {

  QString qs;
  int l1;

  M_type = M_type_slew;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SLEW_WIDTH, MODULE_SLEW_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);
  timeUp = 0.5;
  timeDown = 0.5;
  configDialog->addSlider(0, 10, timeUp, "Time Up", &timeUp);
  configDialog->addSlider(0, 10, timeDown, "Time Down", &timeDown);
  qs.sprintf("Slew Limiter ID %d", moduleID);
  configDialog->setWindowTitle(qs);
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    lastData[l1] = 0;
  }
}

M_slew::~M_slew() {
}

void M_slew::generateCycle() {

  int l1, l2;
  float ds, slewUp, slewDown;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata ();

    if (timeUp > 0.0001) {
      slewUp = 1.0 / (timeUp * (float)synthdata->rate);
    } else {
      slewUp = 1.0 / (0.0001 * (float)synthdata->rate);
    }
    if (timeDown > 0.0001) {
      slewDown = -1.0 / (timeDown * (float)synthdata->rate);
    } else {
      slewDown = -1.0 / (0.0001 * (float)synthdata->rate);
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        ds = inData[l1][l2] - lastData[l1];
        if (ds > 0) {
          if (ds > slewUp) ds = slewUp;
        } else {
          if (ds < slewDown) ds = slewDown;
        }
        data[0][l1][l2] = lastData[l1] + ds;
        lastData[l1] = data[0][l1][l2];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_slew::showConfigDialog() {
}
