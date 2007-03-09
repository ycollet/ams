#ifndef M_NOISE2_H
#define M_NOISE2_H

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

#define MODULE_NOISE2_WIDTH                90
#define MODULE_NOISE2_HEIGHT               75

enum Noises {WHITE,RAND,PINK};
class M_noise2 : public Module
{
  Q_OBJECT

  private:
    Noises NoiseType;
    int count;
    float rate, level;
    float buf[3], r;
    Port *port_white, *port_pink, *port_random;
    float randmax; 
  public:
    M_noise2(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_noise2();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
