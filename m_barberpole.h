#ifndef M_BARBERPOLE_H
#define M_BARBERPOLE_H

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

#define MODULE_BARBERPOLE_WIDTH                110
#define MODULE_BARBERPOLE_HEIGHT               360

class M_barberpole : public Module
{
  Q_OBJECT

  private:
    Port *port_out[16];
    float freq;
    double tri, saw, d_tri, d_saw;
    int state;
    int dir;
        
  public:
    M_barberpole(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_barberpole();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
