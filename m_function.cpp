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
#include "m_function.h"
#include "port.h"

M_function::M_function(int p_functionCount, SynthData *p_synthdata, QWidget* parent, const char *name) 
              : Module(p_functionCount, parent, name, p_synthdata) {

  QString qs;
  int l1, l2;

  M_type = M_type_function;
  functionCount = p_functionCount;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_FUNCTION_WIDTH, 
              MODULE_FUNCTION_HEIGHT + 20 + 20 * functionCount);
  gainIn = 1.0;
  gainOut = 1.0;
  port_in = new Port("In", PORT_IN, 0, this, synthdata);
  port_in->move(0, 40);
  port_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_in);
  for (l1 = 0; l1 < functionCount; l1++) {
    i[l1] = 1;
    points[l1] = new QPointArray(MAX_POINTS);
    for (l2 = 0; l2 < MODULE_FUNCTION_DEFAULT_POINTCOUNT; l2++) {
      points[l1]->setPoint(l2, l2 * FUNCTION_WIDTH / (MODULE_FUNCTION_DEFAULT_POINTCOUNT - 1), 
                               (FUNCTION_HEIGHT >> 1) + (l1 - (functionCount >> 1)) * 1000);
    }
    qs.sprintf("Out %d", l1);
    Port *audio_out_port = new Port(qs, PORT_OUT, out_port_list.count(), this, synthdata);
    audio_out_port->move(MODULE_FUNCTION_WIDTH - audio_out_port->width(), 55 + 20 * l1);
    audio_out_port->outType = outType_audio;
    out_port_list.append(audio_out_port);
    portList.append(audio_out_port);
  }
  qs.sprintf("Function %d -> 1 ID %d", functionCount, moduleID);
  configDialog->addFunction(functionCount, points, MODULE_FUNCTION_DEFAULT_POINTCOUNT, synthdata);
  configDialog->addSlider(-5, 5, gainIn, "Input Gain (linear)", &gainIn);
  configDialog->addSlider(-5, 5, gainOut, "Output Gain (linear)", &gainOut);
  configDialog->setCaption(qs);
}

M_function::~M_function() {
}

void M_function::generateCycle() {

  int l1, l2, l3;
  int pointCount;
  float *x;
  float xg, y, y1, y2;
  QPointArray **points;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_in->getinputdata();
    points = configDialog->functionList.at(0)->points;
    pointCount = configDialog->functionList.at(0)->pointCount;
    for (l3 = 0; l3 < functionCount; l3++) {    
      x = configDialog->functionList.at(0)->fx[l3];
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          xg = gainIn * inData[l1][l2];
          while (xg < x[i[l3]]) i[l3]--;
          while (xg >= x[i[l3]+1]) i[l3]++;
          if ((i[l3] >= 1) && (i[l3] < pointCount)) {
            y1 = (double)(points[l3]->point(i[l3]-1).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE; // note the different ranges of x and points arrays
            y2 = (double)(points[l3]->point(i[l3]).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE;
            y = y1 + (xg - x[i[l3]]) * (y2 - y1) / (x[i[l3]+1] - x[i[l3]]);
          } else if (i[l3] < 1) {
            y = (double)(points[l3]->point(0).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE;
          } else if (i[l3] >= pointCount) {
            y = (double)(points[l3]->point(pointCount - 1).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE;
          } 
          data[l3][l1][l2] = gainOut * y;     
        }
      }
    }  
    cycleProcessing = false;
    cycleReady = true;
  }
}

void M_function::showConfigDialog() {
}
