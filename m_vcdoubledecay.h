#ifndef M_VCDOUBLEDECAY_H
#define M_VCDOUBLEDECAY_H

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

#define MODULE_VCDOUBLEDECAY_WIDTH                140
#define MODULE_VCDOUBLEDECAY_HEIGHT               175

class M_vcdoubledecay : public Module
{
  Q_OBJECT

  private:
    Port *port_M_gate, *port_M_retrigger, *port_M_attack, *port_M_decay, *port_M_sustain, *port_M_ratio, *port_out;
    
  public: 
    float **gateData, **retriggerData, **attackData, **decayData, **sustainData, **ratioData;        
    float a0, d0, s0, r0, aGain, dGain, sGain, rGain;
    float e[MAXPOLY], e2[MAXPOLY];
    int state[MAXPOLY];
    bool noteActive[MAXPOLY], gate[MAXPOLY], retrigger[MAXPOLY];
                                    
  public:
    M_vcdoubledecay(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_vcdoubledecay();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
