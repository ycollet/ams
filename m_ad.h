#ifndef M_AD_H
#define M_AD_H

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
#include <qtimer.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_AD_WIDTH                140
#define MODULE_AD_HEIGHT                40

class M_ad : public Module
{
  Q_OBJECT

  private:
    Port *port_in, *port_out[MAX_ANALOGUE_DRIVER_OUT];
    float detune_amp, detune_rate, drift_amp, drift_rate, detune_mod, drift_mod;
    float detune_a[MAXPOLY], detune_c[MAXPOLY];
    float drift_a[MAX_ANALOGUE_DRIVER_OUT][MAXPOLY], drift_c[MAX_ANALOGUE_DRIVER_OUT][MAXPOLY];
    float bypass;
    int detuneCount, driftCount, voice[2];
    QTimer *timer;
        
  public: 
    float **inData;
    int outCount;
                            
  public:
    M_ad(int p_outCount, QWidget* parent=0, const char *name=0);
    ~M_ad();

  public slots:
    void generateCycle();
    void showConfigDialog();
    void updateVoices(int n);
    void timerProc();
    void autoTune();
};
  
#endif
