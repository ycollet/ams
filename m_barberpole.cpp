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
#include <qstrlist.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_barberpole.h"
#include "port.h"

M_barberpole::M_barberpole(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(16, parent, name, p_synthdata) {

  QString qs;
  int l1, l2;

  M_type = M_type_barberpole;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_BARBERPOLE_WIDTH, 
              MODULE_BARBERPOLE_HEIGHT);
  for (l1 = 0; l1 < 16; l1++) {
    if (l1 < 8) {
      qs.sprintf("Saw Out %4d", l1 * 45);
    } else {
      qs.sprintf("Tri Out %4d", (l1-8) * 45);
    }  
    port_out[l1] = new Port(qs, PORT_OUT, l1, this, synthdata);
    port_out[l1]->move(MODULE_BARBERPOLE_WIDTH - port_out[l1]->width(), 
                       35 + 20 * l1);
    port_out[l1]->outType = outType_audio;
    portList.append(port_out[l1]);
  }
  freq = 0.1;
  tri = 0;
  saw = 0; 
  state = 0;
  d_tri = 4.0 * freq / (double)synthdata->rate;
  d_saw = 0.5 * d_tri;
  configDialog->addSlider(0.01, 10, freq, "Frequency", &freq, true);
}

M_barberpole::~M_barberpole() {
}

void M_barberpole::generateCycle() {

  int l1, l2, l3;
  double tri45, tri90, tri135, saw45, saw90, saw135, saw180, saw225, saw270, saw315;
  
  if (!cycleReady) {
    cycleProcessing = true;
    
    d_saw = 0;
    d_tri = ((state > 1) && (state < 6)) ? -4.0 * freq / (double)synthdata->rate 
                                         :  4.0 * freq / (double)synthdata->rate;
    d_saw = 0.5 * fabs(d_tri);
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      tri += d_tri;
      saw += d_saw;
      switch(state) {
        case 0:
          if (tri > 0.5) {
            state++;
          }  
          tri45 = 0.5 + tri;
          tri90 = 1.0 - tri;
          tri135 = 0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw; 
          saw135 = 0.75 + saw; 
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 1:
          if (tri > 1.0) {
            state++;
            tri = 1.0;
            d_tri = -4.0 * freq / (double)synthdata->rate;
            d_saw = -0.5 * d_tri;
          }  
          tri45 = 1.5 - tri;
          tri90 = 1.0 - tri;
          tri135 = 0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = -1.25 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 2:
          if (tri < 0.5) {
            state++;
          }  
          tri45 = tri - 0.5;
          tri90 = tri - 1.0;
          tri135 = -1.5 + tri;
          saw45 = 0.25 + saw;
          saw90 = -1.5 + saw;
          saw135 = -1.25 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 3:
          tri45 = tri - 0.5;
          tri90 = tri - 1.0;
          tri135 = -0.5 - tri;
          saw45 = -1.75 + saw;
          saw90 = -1.5 + saw;
          saw135 = -1.25 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          if (tri < 0.0) {
            state++;
            saw = -1.0;
          }  
          break;
        case 4:
          if (tri < -0.5) {
            state++;
          }
          tri45 = tri - 0.5;
          tri90 = -1.0 - tri;
          tri135 = -0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = 1.25 + saw;
          saw270 = 1.5 + saw;
          saw315 = 1.75 + saw;
          break;
        case 5:
          if (tri < -1.0) {
            tri = -1.0;
            d_tri = 4.0 * freq / (double)synthdata->rate;
            d_saw = 0.5 * d_tri;
            state++;
          }
          tri45 = - 1.5 - tri;
          tri90 = -1.0 - tri;
          tri135 = -0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = 1.25 + saw;
          saw270 = 1.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 6:
          if (tri > -0.5) {
            state++;
          }
          tri45 = 0.5 + tri;
          tri90 = 1.0 + tri;
          tri135 = 1.5 + tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = 1.25 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 7:
          if (tri > 0.0) {
            state = 0;
          }  
          tri45 = 0.5 + tri;
          tri90 = 1.0 + tri;
          tri135 = 0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
      }
          
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        data[0][l1][l2] = 1.0 + saw45;
        data[1][l1][l2] = 1.0 + saw;
        data[2][l1][l2] = 1.0 + saw315;
        data[3][l1][l2] = 1.0 + saw270;
        data[4][l1][l2] = 1.0 + saw225;
        data[5][l1][l2] = 1.0 + saw180;
        data[6][l1][l2] = 1.0 + saw135;
        data[7][l1][l2] = 1.0 + saw90;
        data[8][l1][l2] = 1.0 + tri135;
        data[9][l1][l2] = 1.0 + tri90;
        data[10][l1][l2] = 1.0 + tri45;
        data[11][l1][l2] = 1.0 + tri;
        data[12][l1][l2] = 1.0 - tri135;
        data[13][l1][l2] = 1.0 - tri90;
        data[14][l1][l2] = 1.0 - tri45;
        data[15][l1][l2] = 1.0 - tri;
      }
    }
    cycleProcessing = false;
    cycleReady = true;
  }
}

void M_barberpole::showConfigDialog() {
}
