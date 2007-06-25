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
#include "m_ringmod.h"
#include "port.h"

M_ringmod::M_ringmod(QWidget* parent) 
  : Module(M_type_ringmod, 1, parent, "Ring Modulator")
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_RINGMOD_WIDTH, MODULE_RINGMOD_HEIGHT);
  gain = 0.5;
  port_M_vco1 = new Port("In 0", PORT_IN, 0, this); 
  port_M_vco1->move(0, 35);
  port_M_vco1->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_vco1);
  port_M_vco2 = new Port("In 1", PORT_IN, 1, this); 
  port_M_vco2->move(0, 55);
  port_M_vco2->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_vco2);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 70);
  port_out->outType = outType_audio;
  portList.append(port_out);

  configDialog->addSlider(0, 5, gain, "Gain", &gain);
}

int M_ringmod::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

float M_ringmod::getGain() {
  return(gain);
}


void M_ringmod::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;

    vcoData1 = port_M_vco1->getinputdata ();
    vcoData2 = port_M_vco2->getinputdata ();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = gain * vcoData1[l1][l2] * vcoData2[l1][l2];
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_ringmod::showConfigDialog() {
}
