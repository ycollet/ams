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
#include "m_delay.h"
#include "port.h"

M_delay::M_delay(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(1, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_delay;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DELAY_WIDTH, MODULE_DELAY_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this, synthdata); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Delay ID %d", moduleID);
  configDialog->setCaption(qs);
  delay = new FloatParameter(this, "Delay","",0.0,10.0,1.0);
  configDialog->addParameter(delay);
  buf = (float **)malloc(synthdata->poly * sizeof(float *));
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    buf[l1] = (float *)malloc(MAX_DELAY_FRAMES * sizeof(float));
    memset(buf[l1], 0, MAX_DELAY_FRAMES * sizeof(float));
  }
  read_ofs = 0;
}

M_delay::~M_delay() {

  int l1;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    free(buf[l1]);
  }
  free(buf);
}

void M_delay::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "Delay");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_delay::generateCycle() {

  int l1, l2, ofs, delay_frames;
  float log2;

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
    delay_frames = (int)((float)(MAX_DELAY_FRAMES - 3) * *delay / 10.0);
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        buf[l1][read_ofs] = inData[l1][l2];
        ofs = read_ofs - delay_frames;
        if (ofs < 0) ofs += MAX_DELAY_FRAMES;
        data[0][l1][l2] = buf[l1][ofs];
      }
      read_ofs++;
      if (read_ofs >= MAX_DELAY_FRAMES) read_ofs = 0;
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_delay::showConfigDialog() {
}
