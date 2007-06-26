#ifndef M_PCMOUT_H
#define M_PCMOUT_H

#include "module.h"


#define MODULE_PCMOUT_WIDTH                 90
#define MODULE_PCMOUT_HEIGHT                80


class M_pcmout : public Module
{
    float gain,polyroot;
    float mixer_gain[2]; 
    int agc;
    long cycleSize;	
    Module *module_in[2];
    Port *port_in[2];
    
  public: 
    float *pcmdata[2];
                            
  public:
    M_pcmout(QWidget* parent, int port);
    ~M_pcmout();

    void generateCycle();
};
  
#endif
