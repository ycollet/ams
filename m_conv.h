#ifndef M_CONV_H
#define M_CONV_H

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

#define MODULE_CONV_WIDTH                100
#define MODULE_CONV_HEIGHT                75

class M_conv : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_out;
    int convMode, octave;
    
  public: 
    float **inData;       
                            
  public:
    M_conv(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_conv();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
