#ifndef M_PCMIN_H
#define M_PCMIN_H

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
#include <qtimer.h>
#include <qdialog.h>
#include "module.h"
#include "port.h"

#define MODULE_PCMIN_WIDTH                 90
#define MODULE_PCMIN_HEIGHT                80

class M_pcmin : public Module
{
  Q_OBJECT

  private:
    float gain;
    float mixer_gain[2]; 
    Port *port_out[2];
    
  public: 
    float *pcmdata[2];
                            
  public:
    M_pcmin(QWidget* parent, const char *name, SynthData *p_synthdata, int port);
    ~M_pcmin();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
