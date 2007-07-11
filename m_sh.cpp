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
#include "m_sh.h"
#include "port.h"

M_sh::M_sh(QWidget* parent)
  : Module(M_type_sh, 2, parent, "Sample & Hold")
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SH_WIDTH, MODULE_SH_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_M_trig = new Port("Trigger", PORT_IN, 1, this); 
  port_M_trig->move(0, 55);
  port_M_trig->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_trig);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 35);
  port_out->outType = outType_audio;
  portList.append(port_out);
  port_gate = new Port("Gate", PORT_OUT, 1, this);          
  port_gate->move(width() - port_gate->width(), 55);
  port_gate->outType = outType_audio;
  portList.append(port_gate);
  triggerLevel = 0.5;
  configDialog->addSlider("Trigger Level", triggerLevel, 0, 10);
  sample = 0;
  gate = false;
}

void M_sh::generateCycle() {

  int l1, l2;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata ();
    trigData = port_M_trig->getinputdata ();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if ( !gate && (trigData[l1][l2] > triggerLevel)) {
          sample = inData[l1][l2]; 
          gate = true;
        } else {
          gate = trigData[l1][l2] > triggerLevel;
        }
        data[0][l1][l2] = sample;
        data[1][l1][l2] = (gate) ? 1 : 0;
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

