#ifndef M_SH_H
#define M_SH_H

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

#define MODULE_SH_WIDTH                115
#define MODULE_SH_HEIGHT                80

class M_sh : public Module
{
  Q_OBJECT

  private:
    float triggerLevel, sample;
    bool gate;
    Module *in_M_in, *in_M_trig; 
    Port *port_M_in, *port_M_trig, *port_out, *port_gate;
    
  public: 
    float **inData, **trigData;       
                            
  public:
    M_sh(QWidget* parent=0, const char *name=0);
    ~M_sh();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
