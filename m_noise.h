#ifndef M_NOISE_H
#define M_NOISE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_NOISE_WIDTH                100
#define MODULE_NOISE_HEIGHT               100

class M_noise : public Module
{
  Q_OBJECT

  private:
    int count;
    float rate, level;
    float buf[3][MAXPOLY], r[MAXPOLY];
    Port *port_white, *port_pink, *port_random;

  public:
    M_noise(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_noise();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
