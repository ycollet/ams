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
#include "m_vcpanning.h"
#include "port.h"

M_vcpanning::M_vcpanning(QWidget* parent) 
  : Module(M_type_vcpanning, 2, parent, "VC Panning")
{
  QString qs;
  int l1, l2;
  float pos, q;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCPANNING_WIDTH, MODULE_VCPANNING_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_M_pan = new Port("Pan CV", PORT_IN, 1, this); 
  port_M_pan->move(0, 55);
  port_M_pan->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_pan);
  for (l1 = 0; l1 < 2; l1++) {
    qs.sprintf("Out %d", l1);
    port_out[l1] = new Port(qs, PORT_OUT, l1, this);          
    port_out[l1]->move(width() - port_out[l1]->width(), 75 + 20 * l1);
    port_out[l1]->outType = outType_audio;
    portList.append(port_out[l1]);
  }
  q = 2.0 / ((double)synthdata->poly - 1.0);
  for (l2 = 0; l2 < 2; l2++) {
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      pan[l2][l1] = 0;
      oldpan[l2][l1] = 0;
    }  
  }
  if (synthdata->poly & 1) {
    pos = 0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      if (l1 & 1) { 
        pos += q;            
      }
      panPos[l1] = (l1 & 1) ? -pos : pos; 
//      fprintf(stderr, "odd pan[%d] = %f\n", l1, panPos[l1]);
    }  
  } else {
    pos = q / 2.0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      panPos[l1] = (l1 & 1) ? -pos : pos;
      if (l1 & 1) { 
        pos += q;            
      }
//      fprintf(stderr, "even pan[%d] = %f\n", l1, panPos[l1]);
    }    
  }  
  panGain = 0.0;
  panOffset = 0.0;
  configDialog->addSlider("Pan Offset", panOffset, -1, 1);
  configDialog->addSlider("Pan Gain", panGain, 0, 2);
  QStringList panModeNames;
  panModeNames <<
    "VC control" <<
    "Fixed alternating panorama, full width" <<
    "Fixed alternating panorama, half width" <<
    "Fixed alternating panorama, quarter width" <<
    "Sort by pitch, Low <--> High" <<
    "Sort by pitch, High <--> Low" <<
    "Mono";
  configDialog->addComboBox("Panning Mode", panMode, panModeNames);
}

void M_vcpanning::generateCycle() {

  int l1, l2, k, len;
  double dpan[2], x, y, x_2, widthConst;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata ();
    panData = port_M_pan->getinputdata ();
    widthConst = 2.0 / 88.0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      len = synthdata->cyclesize;
      l2 = -1;
      do {
        k = (len > 24) ? 16 : len;
        l2 += k;
        len -= k;
        switch (panMode) {
          case 0: x = panOffset + panGain * panData[l1][l2];
                  x_2 = 0.5 * x;
                  if (x < -1) x = -1;
                  if (x > 1) x = 1;
                  y = 0.2125 * (1.0 - x * x);
                  pan[1][l1] = (0.5 + x_2) + y;
                  pan[0][l1] = (0.5 - x_2) + y;
                  break;
          case 1: x = panPos[l1];
                  x_2 = 0.5 * x;
                  y = 0.2125 * (1.0 - x * x);
                  pan[1][l1] = (0.5 + x_2) + y;    
                  pan[0][l1] = (0.5 - x_2) + y;  
                  break; 
          case 2: x = 0.5 * panPos[l1];
                  x_2 = 0.5 * x;
                  y = 0.2125 * (1.0 - x * x);
                  pan[1][l1] = (0.5 + x_2) + y;    
                  pan[0][l1] = (0.5 - x_2) + y;  
                  break; 
          case 3: x = 0.25 * panPos[l1];
                  x_2 = 0.5 * x;
                  y = 0.2125 * (1.0 - x * x);
                  pan[1][l1] = (0.5 + x_2) + y;    
                  pan[0][l1] = (0.5 - x_2) + y;  
                  break; 
          case 4: x = ((double)(synthdata->notes[l1] - 21) * widthConst - 1.0 ) * panGain + panOffset;
                  if (x < -1) x = -1;
                  if (x > 1) x = 1;
                  x_2 = 0.5 * x;
                  y = 0.2125 * (1.0 - x * x);
                  pan[1][l1] = (0.5 + x_2) + y;    
                  pan[0][l1] = (0.5 - x_2) + y;  
                  break; 
          case 5: x = ((double)(88 - (synthdata->notes[l1] - 21)) * widthConst - 1.0) * panGain + panOffset;
                  if (x < -1) x = -1;
                  if (x > 1) x = 1;
                  x_2 = 0.5 * x;
                  y = 0.2125 * (1.0 - x * x);
                  pan[1][l1] = (0.5 + x_2) + y;    
                  pan[0][l1] = (0.5 - x_2) + y;  
                  break; 
          case 6: pan[1][l1] = 1;
                  pan[0][l1] = 1;
                  break;
        }
        dpan[0] = (pan[0][l1] - oldpan[0][l1]) / (double) k;
        dpan[1] = (pan[1][l1] - oldpan[1][l1]) / (double) k;
        while(k--) {
          oldpan[0][l1] += dpan[0];
          oldpan[1][l1] += dpan[1];
          data[0][l1][l2-k] = oldpan[0][l1] * inData[l1][l2-k];
          data[1][l1][l2-k] = oldpan[1][l1] * inData[l1][l2-k];
        }                                   
      } while(len);  
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

