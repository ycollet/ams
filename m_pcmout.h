#ifndef M_PCMOUT_H
#define M_PCMOUT_H

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
#include <qtimer.h>
#include <qdialog.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"


#define MODULE_PCMOUT_WIDTH                 90
#define MODULE_PCMOUT_HEIGHT                80


class M_pcmout : public Module
{
  Q_OBJECT

  private:
    float gain,polyroot;
    float mixer_gain[2]; 
    int agc;
    long cycleSize;	
    Module *module_in[2];
    Port *port_in[2];
    
  public: 
    float *pcmdata[2];
                            
  public:
    M_pcmout(QWidget* parent, const char *name, int port);
    ~M_pcmout();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
