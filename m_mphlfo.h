#ifndef M_MPHLFO_H
#define M_MPHLFO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_MPHLFO_WIDTH                115
#define MODULE_MPHLFO_HEIGHT               360

class M_mphlfo : public Module
{
  Q_OBJECT

  private:
    Port *port_out[16];
    float freq, gain_saw, gain_tri;
    double tri, saw, d_tri, d_saw;
    double o[16];
    int state;
    int mode;
        
  public:
    M_mphlfo(QWidget* parent=0);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
