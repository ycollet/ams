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
#include "m_vquant.h"
#include "port.h"

M_vquant::M_vquant(QWidget* parent) 
  : Module(M_type_vquant, 1, parent, "Quantizer 2")
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VQUANT_WIDTH, MODULE_VQUANT_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_quant = new Port("Out", PORT_OUT, 0, this);          
  port_quant->move(width() - port_quant->width(), 55);
  port_quant->outType = outType_audio;
  portList.append(port_quant);
  gain = 1.0;
  configDialog->addSlider("Gain", gain, 0, 10);
}

void M_vquant::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = (int)(gain * inData[l1][l2]);
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}
