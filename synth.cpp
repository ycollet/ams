#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <qsignal.h>
#include <qthread.h>
#include "synthdata.h"
#include "synth.h"
#include "module.h"
#include "m_out.h"
#include "m_wavout.h"
#include "m_midiout.h"
//#include "m_scope.h"
//#include "m_spectrum.h"

Synth::Synth(SynthData *p_synthdata) {

  synthdata = p_synthdata;
  periodBuf = (unsigned char *)malloc(synthdata->periodsize * synthdata->framesize);
}

Synth::~Synth(){        

  free(periodBuf);
}

void Synth::run(){

  int l1, l2, pcmreturn;
  short s;
  int mlock_flag;
  float gain;
  struct timeval delayTime;

  mlock_flag = 0;
  struct sched_param schp;
  memset(&schp, 0, sizeof(schp));
  schp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0) {
    fprintf(stderr, "can't set sched_setscheduler - using normal priority\n");
  } else {
    printf("set SCHED_FIFO\n");
//    mlock_flag = 1;
//    mlockall(MCL_CURRENT | MCL_FUTURE);
  }
  while(synthdata->doSynthesis) {
    for (l1 = 0; l1 < synthdata->channels; l1++) {
      memset(synthdata->data[l1], 0, synthdata->cyclesize * sizeof(float)); 
    }
    synthdata->port_sem++;
    for (l1 = 0; l1 < synthdata->outModuleList.count(); l1++) {
      ((M_out *)synthdata->outModuleList.at(l1))->generateCycle();
    }
    for (l1 = 0; l1 < synthdata->wavoutModuleList.count(); l1++) {
      ((M_wavout *)synthdata->wavoutModuleList.at(l1))->generateCycle();
    }
/*
    for (l1 = 0; l1 < synthdata->scopeModuleList.count(); l1++) {
      ((M_scope *)synthdata->scopeModuleList.at(l1))->generateCycle();
    }
    for (l1 = 0; l1 < synthdata->spectrumModuleList.count(); l1++) {
      ((M_spectrum *)synthdata->spectrumModuleList.at(l1))->generateCycle();
    }
*/
    for (l1 = 0; l1 < synthdata->midioutModuleList.count(); l1++) {
      ((M_midiout *)synthdata->midioutModuleList.at(l1))->generateCycle();
    }
    for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
      ((Module *)synthdata->moduleList.at(l1))->cycleReady = false;
    }
    synthdata->port_sem--;
    if (synthdata->outModuleList.count() && synthdata->moduleOutCount) {
      gain = (synthdata->outModuleList.count() > 0) ? GAIN / (float)(synthdata->outModuleList.count() * synthdata->poly) : 0;
      for (l2 = 0; l2 < synthdata->channels; l2++) {
        for (l1 = 0; l1 < synthdata->periodsize; l1++) {        // TODO Caution, this assumes periodsize == cyclesize !
          s = (short)(gain * synthdata->data[l2][l1]);
          periodBuf[4*l1+2*l2] = (unsigned char)s;
          periodBuf[4*l1+2*l2+1] = s >> 8;
        }
      }
      while ((pcmreturn = snd_pcm_writei(synthdata->pcm_handle, periodBuf, synthdata->periodsize)) < 0) {
        snd_pcm_prepare(synthdata->pcm_handle);
//        fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
      }
    } else {
      delayTime.tv_sec = 0;
      delayTime.tv_usec = (int)(0.7 * 1000000.0 * (double)synthdata->periodsize / (double)synthdata->rate);
      select(1, NULL, NULL, NULL, &delayTime);
    }
  } 
  if (mlock_flag) {
    munlockall();
  } 
}
