#ifndef M_VCSWITCH_H
#define M_VCSWITCH_H

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

#define MODULE_VCSWITCH_WIDTH                115
#define MODULE_VCSWITCH_HEIGHT               100

class M_vcswitch : public Module
{
  Q_OBJECT

  private:
    float switchLevel;
    Port *port_M_in[2], *port_M_cv, *port_out[2], *port_mix;
    
  public: 
    float **inData[2], **cvData;       
                            
  public:
    M_vcswitch(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_vcswitch();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
