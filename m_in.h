#ifndef M_IN_H
#define M_IN_H

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

#define MODULE_IN_WIDTH                 90
#define MODULE_IN_HEIGHT                80

class M_in : public Module
{
  Q_OBJECT

  private:
    float gain;
    float mixer_gain[2]; 
    Port *port_out[2];
    
  public:
    M_in(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_in();
    int setGain(float p_gain);
    float getGain();

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
