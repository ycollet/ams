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
    double si[MAXPOLY], old_si[MAXPOLY];
    double sa[MAXPOLY], old_sa[MAXPOLY];
    double t[MAXPOLY], old_t[MAXPOLY];
    double r[MAXPOLY], old_r[MAXPOLY];
    int state[MAXPOLY];
    double dt[MAXPOLY], dsi[MAXPOLY], ddsi[MAXPOLY], ddsi0[MAXPOLY], wave_period;
    bool trigger[MAXPOLY];
    Port *port_M_trigger, *port_sine, *port_tri, *port_sawup, *port_sawdown, *port_rect;
    float **triggerData;  
                                
  public:
    M_lfo(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_lfo();
    virtual void noteOnEvent(int osc);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
