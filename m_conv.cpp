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
#include "m_conv.h"
#include "port.h"

M_conv::M_conv(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;

  M_type = M_type_conv;
  convMode = 0;
  octave = 0;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_CONV_WIDTH, MODULE_CONV_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this, synthdata); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Converter ID %d", moduleID);
  configDialog->setCaption(qs);
  QStrList *convModeNames = new QStrList(true);
  convModeNames->append("V/Octave --> Hz");
  convModeNames->append("V/Octave --> 0..1, 1=rate/2");
  convModeNames->append("V/Octave --> 0..1, 1=20000 Hz");
  configDialog->addComboBox(0, "Conversion Mode ", &convMode, convModeNames->count(), convModeNames);
  configDialog->addIntSlider(-3, 3, octave, "Octave Offset", &octave);
}

M_conv::~M_conv() {
}

void M_conv::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "Converter");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_conv::generateCycle() {

  int l1, l2;
  float log2, offset;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      memcpy(lastdata[0][l1], data[0][l1], synthdata->cyclesize * sizeof(float));
    }
    if (port_M_in->connectedPortList.count()) {
      in_M_in = (Module *)port_M_in->connectedPortList.at(0)->parentModule;
      if (!in_M_in->cycleProcessing) {
        in_M_in->generateCycle();
        inData = in_M_in->data[port_M_in->connectedPortList.at(0)->index];
      } else {
        inData = in_M_in->lastdata[port_M_in->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in = NULL;
      inData = synthdata->zeroModuleData;
    }
    log2 = log(2.0);
    offset = octave + 5.0313842;
    if (convMode == 0) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = exp(log2 * (offset + inData[l1][l2]));
        }
      }
    } else if (convMode == 1) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = 2.0 * exp(log2 * (offset + inData[l1][l2])) / synthdata->rate;
        }
      }
    } else if (convMode == 2) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = exp(log2 * (offset + inData[l1][l2])) / 20000.0;
        }
      }
    } 
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_conv::showConfigDialog() {
}
