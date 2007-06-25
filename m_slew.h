#ifndef M_SLEW_H
#define M_SLEW_H

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

#define MODULE_SLEW_WIDTH                 90
#define MODULE_SLEW_HEIGHT                75

class M_slew : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_out;
    float timeUp, timeDown;
    float lastData[MAXPOLY];
    
  public: 
    float **inData;       
                            
  public:
    M_slew(QWidget* parent=0);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
