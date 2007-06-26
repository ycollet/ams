#ifndef M_VCO_H
#define M_VCO_H

#include "module.h"

#define MODULE_VCO_WIDTH                 85
#define MODULE_VCO_HEIGHT               220

enum auxWaveFormType { WAVE_SAW, WAVE_SAW2, WAVE_SAW3 };

class M_vco : public Module
{
    float freq, phi0, pw0;
    float edge;
    float wave_period;
    float phi[MAXPOLY];
   // float dphi, phi1, phi_const, pw, d, dd, dsaw, half_wave, third_wave; 
   // float freq_const, freq_tune, gain_linfm, log2, pw_low, pw_high,
    float wave_period_2,wave_period_3,Pi2Times,PKonst;
    float vcoExpFMGain, vcoLinFMGain, pwGain;
    auxWaveFormType waveForm;
    int harmonic, subharmonic, octave;
    Port *port_M_freq, *port_M_exp, *port_M_lin, *port_M_pw;
    Port *port_sine, *port_tri, *port_saw, *port_rect, *port_aux;
    
  public: 
    float **freqData;        
    float **pwData;  // Duty cycle modulation generated by M_lfo
    float **expFMData;        // Frequency modulation exp characteristic
    float **linFMData;        // Frequency modulation lin characteristic
                            
  public:
    M_vco(QWidget* parent=0);

    void generateCycle();
};
  
#endif
