#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
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
#include "m_noise.h"
#include "port.h"

M_noise::M_noise(QWidget* parent, const char *name) 
              : Module(3, parent, name) {

  QString qs;
  int l2;
  long t;

  M_type = M_type_noise;
  rate = 5;
  level = 0.5;
  count = 0;
  
  randmax = 2.0 / (double)RAND_MAX;
  
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_NOISE_WIDTH, MODULE_NOISE_HEIGHT);
  port_white = new Port("White", PORT_OUT, 0, this);          
  port_white->move(width() - port_white->width(), 35);
  port_white->outType = outType_audio;
  portList.append(port_white);
  port_pink = new Port("Pink", PORT_OUT, 1, this);          
  port_pink->move(width() - port_pink->width(), 55);
  port_pink->outType = outType_audio;
  portList.append(port_pink);
  port_random = new Port("Random", PORT_OUT, 2, this);          
  port_random->move(width() - port_random->width(), 75);
  port_random->outType = outType_audio;
  portList.append(port_random);
  qs.sprintf("Noise ID %d", moduleID);
  configDialog->setWindowTitle(qs);
  configDialog->addSlider(0, 10, rate, "Random Rate", &rate);
  configDialog->addSlider(0, 1, level, "Random Level", &level);
  r = 0;
  for (l2 = 0; l2 < 3; ++l2) {
    buf[l2] = 0;
  }
  t = time(NULL) % 1000000;
  srand(abs(t - 10000 * (t % 100)));
}

void M_noise::generateCycle() {

  unsigned int l1, l2, random_rate;
  double white_noise;

  if (!cycleReady) {
    cycleProcessing = true;
    random_rate = (int)(5000.0 * (double)rate + 100.0);
    for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
      count++; 
      if (count > random_rate) {
        count = 0;
        r = level * (double)rand() * randmax - 1.0;
      }
      white_noise = (double)rand() * randmax - 1.0;
      buf[0] = 0.99765 * buf[0] + white_noise * 0.099046;
      buf[1] = 0.963 * buf[1] + white_noise * 0.2965164;
      buf[2] = 0.57 * buf[2] + white_noise * 1.0526913;
      data[1][0][l2] = buf[0] + buf[1] + buf[2] + white_noise * 0.1848;
      for (l1 = 0; l1 < synthdata->poly; ++l1) {
        data[0][l1][l2] = white_noise;
        data[1][l1][l2] = data[1][0][l2];
        data[2][l1][l2] = r;
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_noise::showConfigDialog() {
}
