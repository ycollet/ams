#ifndef M_DELAY_H
#define M_DELAY_H

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

#define MODULE_DELAY_WIDTH                 90
#define MODULE_DELAY_HEIGHT                75
#define MAX_DELAY_FRAMES                16384

class M_delay : public Module
{
  Q_OBJECT

  private:
    float delay;
    int read_ofs;
    Module *in_M_in; 
    Port *port_M_in, *port_out;
    
  public: 
    float **inData, **buf;       
                            
  public:
    M_delay(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_delay();

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
