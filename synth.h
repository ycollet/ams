#ifndef SYNTH_H
#define SYNTH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qobject.h>
#include <qthread.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"

#define GAIN 32767.0 // TODO This should depend on bits per sample

class Synth : public QThread
{
  private:
    SynthData *synthdata;
    unsigned char *periodBuf;
  
  public:
    Synth(SynthData *p_synthdata);
    ~Synth();
    virtual void run();                    
};
  
#endif
