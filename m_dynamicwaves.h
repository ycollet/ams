#ifndef M_DYNAMICWAVES_H
#define M_DYNAMICWAVES_H

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

#define MODULE_DYNAMICWAVES_WIDTH                115
#define MODULE_DYNAMICWAVES_HEIGHT               140
#define MODULE_DYNAMICWAVES_MAX_OSC                8
#define DYNAMICWAVES_EXP_TABLE_LEN             32768
#define DYNAMICWAVES_ENVELOPE_RESPONSE           256

enum dynamicWaveFormType { DYNAMICWAVE_SINE, DYNAMICWAVE_SAW, DYNAMICWAVE_TRI, DYNAMICWAVE_RECT, DYNAMICWAVE_SAW2 };

class M_dynamicwaves : public Module
{
  Q_OBJECT

  private:
    float attack[8][MODULE_DYNAMICWAVES_MAX_OSC], sustain[MODULE_DYNAMICWAVES_MAX_OSC];
    float release[5][MODULE_DYNAMICWAVES_MAX_OSC];
    float timeScale[MODULE_DYNAMICWAVES_MAX_OSC];
    float e_noteOff[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC];    
    float de[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC];
    float de_release[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC];
    float tune, osc_tune[MODULE_DYNAMICWAVES_MAX_OSC], gain[MODULE_DYNAMICWAVES_MAX_OSC];
    int octave, osc_octave[MODULE_DYNAMICWAVES_MAX_OSC];
    int harmonic[MODULE_DYNAMICWAVES_MAX_OSC], subharmonic[MODULE_DYNAMICWAVES_MAX_OSC];
    dynamicWaveFormType waveForm[MODULE_DYNAMICWAVES_MAX_OSC];
    float expFMGain, linFMGain;
    float phi0[MODULE_DYNAMICWAVES_MAX_OSC], phi[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC];
    float wave_period;
    bool allEnvTerminated;
    Module *in_M_freq, *in_M_exp, *in_M_lin, *in_M_gate; 
    Port *port_M_freq, *port_M_exp, *port_M_lin, *port_gate;
    Port *port_out;
                    
  public: 
    int oscCount;
    float e[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC]; 
    bool noteActive[MAXPOLY], gate[MAXPOLY];
    bool oscActive[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC];
    long noteOnOfs[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC]; 
    long noteOffOfs[MAXPOLY][MODULE_DYNAMICWAVES_MAX_OSC];
    float **gateData;
    float **freqData;        
    float **expFMData;        // Frequency modulation exp characteristic
    float **linFMData;        // Frequency modulation lin characteristic
                            
  public:
    M_dynamicwaves(int p_oscCount, QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_dynamicwaves();
    virtual void noteOnEvent(int osc);

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
