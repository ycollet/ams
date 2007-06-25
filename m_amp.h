#ifndef M_AMP_H
#define M_AMP_H

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

#define MODULE_AMP_WIDTH                 90
#define MODULE_AMP_HEIGHT                80

class M_amp : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_out;
    float gain;
    
  public: 
    float **inData;       
                            
  public:
    M_amp(QWidget* parent=0);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
