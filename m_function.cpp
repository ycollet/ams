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
  QHBox *hbox;
  int l1, l2;

  M_type = M_type_function;
  functionCount = p_functionCount;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_FUNCTION_WIDTH, 
              MODULE_FUNCTION_HEIGHT + 20 + 20 * functionCount);
  port_in = new Port("In", PORT_IN, 0, this, synthdata);
  port_in->move(0, 40);
  port_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_in);
  for (l1 = 0; l1 < functionCount; l1++) {
    for (l2 = 0; l2 < MAXPOLY; l2++) {
      i[l2][l1] = 1;
      y[l1][l2] = 0;
      old_y[l1][l2] = 0;
    }
    points[l1] = new QPointArray(MAX_POINTS);
    for (l2 = 0; l2 < MAX_POINTS; l2++) {
      points[l1]->setPoint(l2, l2 * FUNCTION_WIDTH / (MAX_POINTS - 1), 
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
  configDialog->addFunction(functionCount, &mode, &editIndex, points, MAX_POINTS, synthdata);
  zoomIndex = 0;
  zoom = 1.0;
  mode = 0;
  editIndex = 0;
  QStrList *zoomNames = new QStrList(true);
  zoomNames->append("   1  ");
  zoomNames->append("   2  ");
  zoomNames->append("   4  ");
  zoomNames->append("   8  ");
  QStrList *modeNames = new QStrList(true);
  modeNames->append("Move Point");
  modeNames->append("Shift X");
  modeNames->append("Shift Y");
  modeNames->append("Scale X");
  modeNames->append("Scale Y");
  modeNames->append("Reset");
  modeNames->append("Linear");
  QStrList *editNames = new QStrList(true);
  editNames->append("All");
  for (l1 = 0; l1 < functionCount; l1++) {
    qs.sprintf("%d", l1);
    editNames->append(qs);
  }
  hbox = configDialog->addHBox();
  configDialog->addLabel("                       Mouse X: _____ Y: _____", hbox);
  hbox = configDialog->addHBox();
  configDialog->addComboBox(mode, "Mode", &mode, modeNames->count(), modeNames, hbox);
  configDialog->addComboBox(0, "Edit Function", &editIndex, editNames->count(), editNames, hbox);
  configDialog->addComboBox(0, "Zoom", &zoomIndex, zoomNames->count(), zoomNames, hbox);
  QObject::connect(configDialog->midiComboBoxList.at(2)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateZoom(int)));
  QObject::connect(configDialog->midiComboBoxList.at(1)->comboBox, SIGNAL(highlighted(int)),
                   configDialog->functionList.at(0), SLOT(highlightFunction(int)));
  QObject::connect(configDialog->functionList.at(0), SIGNAL(mousePos(int, int)),
                   this, SLOT(updateMouseLabels(int, int)));
  qs.sprintf("Function ID %d", moduleID);
  configDialog->setCaption(qs);
}

M_function::~M_function() {
}

void M_function::generateCycle() {

  int l1, l2, l3, k, len, l2_out;
  int pointCount;
  Function *cf;  
  float xg, dy;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_in->getinputdata();
    cf = configDialog->functionList.at(0);
    pointCount = configDialog->functionList.at(0)->pointCount;
    for (l3 = 0; l3 < functionCount; l3++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        len = synthdata->cyclesize;
        l2 = -1;
        l2_out = 0;
        do {
          k = (len > 24) ? 16 : len;
          l2 += k;
          len -= k;    
          xg = inData[l1][l2];
          while (xg < cf->f[0][l3][i[l1][l3]]) i[l1][l3]--;
          while (xg >= cf->f[0][l3][i[l1][l3]+1]) i[l1][l3]++;
          if (i[l1][l3] < 1) {
            y[l3][l1] = cf->f[1][l3][1];
          } else if (i[l1][l3] >= pointCount) {
            y[l3][l1] = cf->f[1][l3][pointCount];
          } else {
            y[l3][l1] = cf->f[1][l3][i[l1][l3]] + (xg - cf->f[0][l3][i[l1][l3]]) 
                                                * (cf->f[1][l3][i[l1][l3]+1] - cf->f[1][l3][i[l1][l3]]) 
                                                / (cf->f[0][l3][i[l1][l3]+1] - cf->f[0][l3][i[l1][l3]]);
          } 
          dy = (y[l3][l1] - old_y[l3][l1]) / (double)k;
          while (k--) {
            old_y[l3][l1] += dy;
            data[l3][l1][l2_out++] = old_y[l3][l1];
          }                                                
        } while (len); 
      }  
    }  
    cycleProcessing = false;
    cycleReady = true;
  }
}

void M_function::showConfigDialog() {
}

void M_function::updateZoom(int p_zoomIndex) {

  zoom = pow(2.0, (double)zoomIndex); // zoomIndex is already set in MidiComboBox event handler
  configDialog->functionList.at(0)->setZoom(zoom);
}

void M_function::updateMouseLabels(int x, int y) {

  QString qs;

  qs.sprintf("                       Mouse X: %6.3f  Y: %6.3f", (float)(x-FUNCTION_CENTER_X)/(float)FUNCTION_SCALE,
                                         (float)(FUNCTION_CENTER_Y-y)/(float)FUNCTION_SCALE);
  configDialog->labelList.at(0)->setText(qs);
}
