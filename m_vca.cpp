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
#include "m_vca.h"
#include "port.h"

M_vca::M_vca(bool p_expMode, QWidget* parent) 
  : Module(M_type_vca, 1, parent, p_expMode ? "Exp. VCA" : "Lin. VCA")
{
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCA_WIDTH, MODULE_VCA_HEIGHT);
  gain1 = 0;
  gain2 = 0;
  in1 = 1.0;
  in2 = 1.0;
  out = 1.0;
  expMode = p_expMode;
  port_M_gain1 = new Port("Gain 0", PORT_IN, 0, this); 
  port_M_gain1->move(0, 35);
  portList.append(port_M_gain1);
  port_M_gain2 = new Port("Gain 1", PORT_IN, 1, this); 
  port_M_gain2->move(0, 55);
  portList.append(port_M_gain2);
  port_M_in1 = new Port("In 0", PORT_IN, 2, this); 
  port_M_in1->move(0, 75);
  portList.append(port_M_in1);
  port_M_in2 = new Port("In 1", PORT_IN, 3, this); 
  port_M_in2->move(0, 95);
  portList.append(port_M_in2);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 115);
  port_out->outType = outType_audio;
  portList.append(port_out);

  configDialog->addSlider("Gain", gain1, 0, 1);
  configDialog->addSlider("Gain 1", gain2, 0, 1);
  configDialog->addSlider("In 0", in1, 0, 2);
  configDialog->addSlider("In 1", in2, 0, 2);
  configDialog->addSlider("Output level", out, 0, 2);
}

void M_vca::generateCycle() {

  int l1, l2;
  float  v, g;

    float **gainData1, **gainData2, **inData1, **inData2;       

    gainData1 = port_M_gain1->getinputdata ();
    gainData2 = port_M_gain2->getinputdata ();
    inData1 = port_M_in1->getinputdata ();
    inData2 = port_M_in2->getinputdata ();

    if (expMode) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	  v = gain1 + gainData1[l1][l2] + gain2 * gainData2[l1][l2];                      
          g = (v > 0) ? synthdata->exp_table ((v - 1.0) * 9.21) : 0;  // This gives a range of 80 dB
          data[0][l1][l2] = g * out * (in1 * inData1[l1][l2] + in2 * inData2[l1][l2]);
        }
      }  
    } else {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = (gain1 + gainData1[l1][l2] + gain2 * gainData2[l1][l2]) 
                          * out * (in1 * inData1[l1][l2] + in2 * inData2[l1][l2]);
        }
      }   
    }
}

