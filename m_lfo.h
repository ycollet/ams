#ifndef M_LFO_H
#define M_LFO_H

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

#define MODULE_LFO_WIDTH                 85
#define MODULE_LFO_HEIGHT               160

class M_lfo : public Module
{
  Q_OBJECT

  private:
    float freq, phi0;
    float phi[MAXPOLY];
    float wave_period;
    bool trigger[MAXPOLY];
    Module *in_M_trigger; 
    Port *port_M_trigger, *port_sine, *port_tri, *port_sawup, *port_sawdown, *port_rect;
    
    float **triggerData;  
                                
  public:
    M_lfo(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_lfo();
    virtual void noteOnEvent(int osc);

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
