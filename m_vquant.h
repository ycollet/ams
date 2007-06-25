#ifndef M_VQUANT_H
#define M_VQUANT_H

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

#define MODULE_VQUANT_WIDTH                100
#define MODULE_VQUANT_HEIGHT                80

class M_vquant : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_quant;
    
  public: 
    float **inData;       
    float gain;
                                
  public:
    M_vquant(QWidget* parent=0);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
