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
#include "m_delay.h"
#include "port.h"

M_delay::M_delay(QWidget* parent, const char *name) 
              : Module(1, parent, name) {

  QString qs;
  int l1;

  M_type = M_type_delay;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DELAY_WIDTH, MODULE_DELAY_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);
  qs.sprintf("Delay ID %d", moduleID);
  configDialog->setWindowTitle(qs);
  configDialog->addSlider(0, 10, delay, "Delay", &delay);
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

void M_delay::generateCycle() {

  int l1, l2, ofs, delay_frames;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata();

    delay_frames = (int)((float)(MAX_DELAY_FRAMES - 3) * delay / 10.0);
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
