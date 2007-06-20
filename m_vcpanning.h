#ifndef M_VCPANNING_H
#define M_VCPANNING_H

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

#define MODULE_VCPANNING_WIDTH                110
#define MODULE_VCPANNING_HEIGHT               120

class M_vcpanning : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_M_pan, *port_out[2];
    
  public: 
    float **inData, **panData;        
    float panGain, panOffset;
    float panPos[MAXPOLY], pan[2][MAXPOLY], oldpan[2][MAXPOLY];
    int panMode;
                                    
  public:
    M_vcpanning(QWidget* parent=0, const char *name=0);
    ~M_vcpanning();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
