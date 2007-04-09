#ifndef M_VCF_H
#define M_VCF_H

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

#define MODULE_VCF_WIDTH                 90
#define MODULE_VCF_HEIGHT               155
#define MIN_FREQ                         20
#define MAX_FREQ                      20000
#define MAX_FREQ2                     22000

enum vcfTypeEnum { VCF_LR, VCF_LPF, VCF_HPF, VCF_BPF_I, VCF_BPF_II, VCF_NF, VCF_MOOG1, VCF_MOOG2 };

class M_vcf : public Module
{
  Q_OBJECT

  private:
    float gain, freq, resonance, dBgain;
    float vcfExpFMGain, vcfLinFMGain, resonanceGain;
    float freq_const;
    float fInvertRandMax ;
    float pi2_rate,inv2_rate;
    float freq_tune, gain_linfm, qr, moog_f, revMoog, moog2times;
    double b_noise;
     double in[5][MAXPOLY], buf[5][MAXPOLY];
   

    vcfTypeEnum vcfType;
    Port *port_M_in, *port_M_resonance, *port_M_freq, *port_M_exp, *port_M_lin, *port_out;
  public: 
    float **inData, **resonanceData, **freqData, **linFMData, **expFMData;       
                            
  public:
    M_vcf(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_vcf();
                                      
  public slots:
    void generateCycle();
    void showConfigDialog();
    void initBuf(int index);
};
  
#endif
