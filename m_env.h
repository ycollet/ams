#ifndef M_ENV_H
#define M_ENV_H

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

#define MODULE_ENV_WIDTH                100
#define MODULE_ENV_HEIGHT               120
#define ENVELOPE_RESPONSE               256

class M_env : public Module
{
  Q_OBJECT

  private:
    float delay, attack, hold, decay, sustain, release;
    float e_noteOff[MAXPOLY], de[MAXPOLY];
    Port *port_gate, *port_retrigger, *port_inverse_out, *port_gain_out;

  public: 
    float e[MAXPOLY];
    float timeScale;
    bool noteActive[MAXPOLY], gate[MAXPOLY], retrigger[MAXPOLY];
    int noteOnOfs[MAXPOLY];
    int noteOffOfs[MAXPOLY];
    float **gateData, **retriggerData;
                
  public:
    M_env(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_env();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
