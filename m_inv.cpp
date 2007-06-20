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
#include "m_inv.h"
#include "port.h"

M_inv::M_inv(QWidget* parent, const char *name) 
              : Module(1, parent, name) {

  QString qs;

  M_type = M_type_inv;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_INV_WIDTH, MODULE_INV_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Inverter ID %d", moduleID);
  configDialog->setWindowTitle(qs);
}

M_inv::~M_inv() {
}

void M_inv::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = -inData[l1][l2];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_inv::showConfigDialog() {
}
