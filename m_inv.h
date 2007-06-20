#ifndef M_INV_H
#define M_INV_H

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

#define MODULE_INV_WIDTH                 90
#define MODULE_INV_HEIGHT                80

class M_inv : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_out;
    
  public: 
    float **inData;       
                            
  public:
    M_inv(QWidget* parent=0, const char *name=0);
    ~M_inv();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
