#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <qsignal.h>
#include <qthread.h>
#include "synthdata.h"
#include "capture.h"

Capture::Capture(SynthData *p_synthdata) {

  synthdata = p_synthdata;
  periodBuf = (unsigned char *)malloc(synthdata->periodsize * synthdata->framesize);
}

Capture::~Capture(){        

  free(periodBuf);
}

void Capture::run(){

  int l1, l2, pcmreturn;
  short s;
  int mlock_flag;
  float invgain;

  mlock_flag = 0;
  synthdata->upperInDataReady = false;
  invgain = 1.0 / CAPTURE_GAIN;
  struct sched_param schp;
  memset(&schp, 0, sizeof(schp));
  schp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0) {
    fprintf(stderr, "can't set sched_setscheduler - using normal priority\n");
  } else {
    printf("set SCHED_FIFO\n");
  }
  while(synthdata->doCapture) {
//    fprintf(stderr, "Capture running\n");
    while ((pcmreturn = snd_pcm_readi(synthdata->pcm_capture_handle, periodBuf, synthdata->periodsize)) < 0) {
      snd_pcm_prepare(synthdata->pcm_capture_handle);
//      fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Overrun >>>>>>>>>>>>>>>\n");
    }
    if (synthdata->upperInDataReady) {
      for (l2 = 0; l2 < synthdata->channels; l2++) {
        for (l1 = 0; l1 < synthdata->periodsize; l1++) {        // TODO Caution, this assumes periodsize == cyclesize !
          s = periodBuf[4*l1+2*l2] | (periodBuf[4*l1+2*l2 + 1] << 8);
          synthdata->indata[0][l2][l1] = invgain * (float)s; 
        }
      }
      synthdata->upperInDataReady = false;
    } else {
      for (l2 = 0; l2 < synthdata->channels; l2++) {
        for (l1 = 0; l1 < synthdata->periodsize; l1++) {        // TODO Caution, this assumes periodsize == cyclesize !
          s = periodBuf[4*l1+2*l2] | (periodBuf[4*l1+2*l2 + 1] << 8);
          synthdata->indata[1][l2][l1] = invgain * (float)s; 
        }
      }
      synthdata->upperInDataReady = true;
    }  
  } 
  if (mlock_flag) {
    munlockall();
  } 
}
