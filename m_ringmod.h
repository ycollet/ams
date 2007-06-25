#ifndef M_RINGMOD_H
#define M_RINGMOD_H

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

#define MODULE_RINGMOD_WIDTH                100
#define MODULE_RINGMOD_HEIGHT                95

class M_ringmod : public Module
{
  Q_OBJECT

  private:
    float gain;
    Port *port_M_vco1, *port_M_vco2, *port_out;
    
  public: 
    float **vcoData1, **vcoData2;       
                            
  public:
    M_ringmod(QWidget* parent=0);

    int setGain(float p_gain);
    float getGain();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
