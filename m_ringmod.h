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

#define MODULE_RINGMOD_WIDTH                100
#define MODULE_RINGMOD_HEIGHT                95

class M_ringmod : public Module
{
  Q_OBJECT

  private:
    float gain;
    Module *in_M_vco1, *in_M_vco2; 
    Port *port_M_vco1, *port_M_vco2, *port_out;
    
  public: 
    float **vcoData1, **vcoData2;       
                            
  public:
    M_ringmod(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_ringmod();
    int setGain(float p_gain);
    int connect_vco1(Module *p_M_vco);
    int connect_vco2(Module *p_M_vco);
    float getGain();

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
