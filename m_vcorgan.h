#ifndef M_VCORGAN_H
#define M_VCORGAN_H

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

#define MODULE_VCORGAN_WIDTH                 85
#define MODULE_VCORGAN_HEIGHT               120
#define MODULE_VCORGAN_MAX_OSC                8
#define VCORGAN_EXP_TABLE_LEN             32768

enum waveFormType { ORGAN_SINE, ORGAN_SAW, ORGAN_TRI, ORGAN_RECT, ORGAN_SAW2 };

class M_vcorgan : public Module
{
  Q_OBJECT

  private:
    float tune, osc_tune[MODULE_VCORGAN_MAX_OSC], gain[MODULE_VCORGAN_MAX_OSC];
    int octave, osc_octave[MODULE_VCORGAN_MAX_OSC];
    int harmonic[MODULE_VCORGAN_MAX_OSC], subharmonic[MODULE_VCORGAN_MAX_OSC];
    waveFormType waveForm[MODULE_VCORGAN_MAX_OSC];
    float expFMGain, linFMGain;
    float phi0[MODULE_VCORGAN_MAX_OSC], phi[MAXPOLY][MODULE_VCORGAN_MAX_OSC];
    float wave_period;
    Port *port_M_freq, *port_M_exp, *port_M_lin;
    Port *port_out;
                    
  public: 
    int oscCount;
    float **freqData;        
    float **expFMData;        // Frequency modulation exp characteristic
    float **linFMData;        // Frequency modulation lin characteristic
                            
  public:
    M_vcorgan(int p_oscCount, QWidget* parent=0);

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
