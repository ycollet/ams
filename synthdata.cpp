#include <stdio.h>
#include <stdlib.h>
#include <qobject.h>
#include <qstring.h>
#include <qthread.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_env.h"
#include "m_jackout.h"
#include "m_jackin.h"
#include "m_advenv.h"
#include "m_dynamicwaves.h"
#include "m_wavout.h"
#include "m_midiout.h"
//#include "m_scope.h"
#include "m_spectrum.h"

SynthData::SynthData(int p_poly, int p_periodsize) : port_sem(1) {

  int l1, l2;
  double dphi, phi, dy, dyd;
  int decaytime;

  decaytime = (int)((float)WAVE_PERIOD / 16.0);
  for (l1 = 0; l1 < MAX_SO; l1++) {
    ladspa_dsc_func_list[l1] = 0;
    ladspa_lib_name[l1] = "***";    
  }
  for (l1 = 0; l1 < MAXPOLY; l1++) {
    notes[l1] = 0;
    velocity[l1] = 0;
    notePressed[l1] = false;
    noteActive[l1] = false;
  }
  poly = p_poly;
  oscCount = 0;
  rate = DEFAULT_RATE;
  periods = DEFAULT_PERIODS;
  periodsize = p_periodsize;
  channels = DEFAULT_CHANNELS;
  samplesize = DEFAULT_SAMPLESIZE;
  cyclesize = periodsize; // TODO Allow cyclesize < periodsize;
  framesize = samplesize * channels;
  min = -32767;
  max = 32768;
  edge = 1.0;
  moduleCount = 0;
  moduleInCount = 0;
  moduleOutCount = 0;
  moduleID = 0;
  doSynthesis = false;
  doCapture = false;
  jackRunning = false;
  jackInCount = 0;
  jackOutCount = 0;
  withJack = false;
  exp_data = (float *)malloc(EXP_TABLE_LEN * sizeof(float));
  wave_sine = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_saw = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_saw2 = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_rect = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_tri = (float *)malloc(WAVE_PERIOD * sizeof(float));
  dphi = 2.0 * PI / WAVE_PERIOD;
  phi = 0;
  for (l1 = 0; l1 < WAVE_PERIOD; l1++) {
    wave_sine[l1] = sin(phi);
    phi += dphi;
  }
  for (l1 = 0; l1 < EXP_TABLE_LEN; l1++) {
    exp_data[l1] = exp(float(l1) / 1000.0 - 16.0);
  }
  dy = 2.0 / (float)(WAVE_PERIOD - decaytime);  
  dyd = 2.0 / decaytime;
  l2 = 0;
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++) {
    wave_saw[l2++] = (float)l1 * dy; 
  }
  for (l1 = 0; l1 < decaytime; l1++) {
    wave_saw[l2++] = 1.0 - (float)l1 * dyd;
  }
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++) {      
    wave_saw[l2++] = -1.0 + (float)l1 * dy;                                                              
  }
  l2 = WAVE_PERIOD - 1;
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++) {
    wave_saw2[l2--] = (float)l1 * dy; 
  }
  for (l1 = 0; l1 < decaytime; l1++) {
    wave_saw2[l2--] = 1.0 - (float)l1 * dyd;
  }
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++) {      
    wave_saw2[l2--] = -1.0 + (float)l1 * dy;                                                              
  }
  l2 = 0;
  dyd = 4.0 / decaytime;
  for (l1 = 0; l1 < decaytime>>2; l1++) {
    wave_rect[l2++] = (float)l1 * dyd;
  }
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++) {
    wave_rect[l2++] = 1.0; 
  }
  for (l1 = 0; l1 < decaytime>>1; l1++) {
    wave_rect[l2++] = 1.0 - (float)l1 * dyd;
  }
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++) {
    wave_rect[l2++] = -1.0; 
  }
  for (l1 = 0; l1 < decaytime>>2; l1++) {
    wave_rect[l2++] = -1.0 + (float)l1 * dyd;      
  }
  dy = 4.0 / (float)WAVE_PERIOD;  
  for (l1 = 0; l1 < (WAVE_PERIOD>>2); l1++) {
    wave_tri[l1] = (float)l1 * dy; 
  }
  for (l1 = (WAVE_PERIOD>>2); l1 < (WAVE_PERIOD >> 1) + (WAVE_PERIOD>>2); l1++) {
    wave_tri[l1] = 1.0 - (float)(l1 - (WAVE_PERIOD>>2)) * dy;                                                          
  }
  for (l1 = (WAVE_PERIOD >> 1) + (WAVE_PERIOD>>2); l1 < WAVE_PERIOD; l1++) {
    wave_tri[l1] = -1.0 + (float)(l1 - (WAVE_PERIOD >> 1) - (WAVE_PERIOD>>2)) * dy;                              
  }
  data = (float **)malloc(channels * sizeof(float *));
  for (l1 = 0; l1 < channels; l1++) {
    data[l1] = (float *)malloc(periodsize * sizeof(float));
    memset(data[l1], 0, periodsize * sizeof(float));
  }
  for (l2 = 0; l2 < 2; l2++) {
    indata[l2] = (float **)malloc(channels * sizeof(float *));
    for (l1 = 0; l1 < channels; l1++) {
      indata[l2][l1] = (float *)malloc(periodsize * sizeof(float));
      memset(indata[l2][l1], 0, periodsize * sizeof(float));
    }
  }
  zeroModuleData = (float **)malloc(poly * sizeof(float *)); 
  for (l1 = 0; l1 < poly; l1++) {                                    // TODO realloc, when poly changed
    zeroModuleData[l1] = (float *)malloc(periodsize * sizeof(float));
    memset(zeroModuleData[l1], 0, periodsize * sizeof(float));
  }
  jackName = "AlsaModularSynth";
}

SynthData::~SynthData() {
  
  int l1, l2;

  for (l1 = 0; l1 < channels; l1++) {
    free(data[l1]);
  }
  free(data);
  for (l2 = 0; l2 < 2; l2++) {
    for (l1 = 0; l1 < channels; l1++) {
      free(indata[l2][l1]);
    }
    free(indata[l2]);
  }
  free(exp_data);
  free(wave_sine);
  free(wave_saw);
  free(wave_saw2);
  free(wave_rect);
  free(wave_tri);
  for (l1 = 0; l1 < poly; l1++) {
    free(zeroModuleData[l1]);
  }
  free(zeroModuleData);
  delete(controlCenter);
  if (withJack) {
    jack_client_close(jack_handle);
  }
}

int SynthData::nextFreeOsc() {

  int l1, l2, osc;
  bool tmp_noteActive;
  float min_e;

  oscCount = 0;
  for (l2 = 0; l2 < poly; l2++) {
    tmp_noteActive = false;
    for (l1 = 0; l1 < listM_env.count(); l1++) {
      if (((M_env *)listM_env.at(l1))->noteActive[l2]) {   
        tmp_noteActive = true;
      }
    }  
    for (l1 = 0; l1 < listM_advenv.count(); l1++) {
      if (((M_advenv *)listM_advenv.at(l1))->noteActive[l2]) {   
        tmp_noteActive = true;
      }
    }  
    for (l1 = 0; l1 < listM_dynamicwaves.count(); l1++) {
      if (((M_dynamicwaves *)listM_dynamicwaves.at(l1))->noteActive[l2]) {   
        tmp_noteActive = true;
      }
    }  
    noteActive[l2] = tmp_noteActive || notePressed[l2];
    if (noteActive[l2]) oscCount++;
  }
  for (l2 = 0; l2 < poly; l2++) {
    if (!noteActive[l2]) {
      return(l2);
    }
  } 
  min_e = 1.0;
  osc = 0;
  for (l2 = 0; l2 < poly; l2++) {
    for (l1 = 0; l1 < listM_env.count(); l1++) {
      if (((M_env *)listM_env.at(l1))->noteActive[l2]) {
        if (((M_env *)listM_env.at(l1))->e[l2] < min_e) {
          min_e = ((M_env *)listM_env.at(l1))->e[l2];
          osc = l2;
        }
      }
    }
    for (l1 = 0; l1 < listM_advenv.count(); l1++) {
      if (((M_advenv *)listM_advenv.at(l1))->noteActive[l2]) {
        if (((M_advenv *)listM_advenv.at(l1))->e[l2] < min_e) {
          min_e = ((M_advenv *)listM_advenv.at(l1))->e[l2];
          osc = l2;
        }
      }
    }
    for (l1 = 0; l1 < listM_dynamicwaves.count(); l1++) {
      if (((M_dynamicwaves *)listM_dynamicwaves.at(l1))->noteActive[l2]) {
        if (((M_dynamicwaves *)listM_dynamicwaves.at(l1))->e[l2][0] < min_e) {
          min_e = ((M_dynamicwaves *)listM_dynamicwaves.at(l1))->e[l2][0];
          osc = l2;
        }
      }
    }
  }  
  return(osc);
}

int SynthData::setCycleSize(int p_cyclesize) {

  cyclesize = p_cyclesize;
}

int SynthData::setPeriodsize(int p_periodsize) {
 
  int l1;

  periodsize = p_periodsize;
  framesize = samplesize * channels;
  return(0);
}

int SynthData::setPeriods(int p_periods) {

  periods = p_periods;
  return(0);
}

int SynthData::setRate(int p_rate) {

  rate = p_rate;
  return(0);
}

int SynthData::setChannels(int p_channels) {

  int l1;

  for (l1 = 0; l1 < channels; l1++) {
    free(data[l1]);
  }
  free(data);
  channels = p_channels;
  framesize = samplesize * channels;
  data = (float **)malloc(channels * sizeof(float *));
  for (l1 = 0; l1 < channels; l1++) {
    data[l1] = (float *)malloc(periodsize * sizeof(float));
    memset(data[l1], 0, periodsize * sizeof(float));
  }
  return(0);
}

int SynthData::setSampleSize(int p_samplesize) {

  samplesize = p_samplesize;
  framesize = samplesize * channels;
  return(0);
}

int SynthData::incModuleCount() {

  moduleCount++;
  moduleID++;
  return(0);
}

int SynthData::decModuleCount() {

  moduleCount--;
  return(0);
}

int SynthData::getModuleCount() {

  return(moduleCount);
}

int SynthData::getModuleID() {

  return(moduleID);
}

int SynthData::getLadspaIDs(QString setName, QString pluginName, int *index, int *n) {
     
  int l1, l2, subID1, subID2;
  QString qs;

  setName = setName.stripWhiteSpace();
  pluginName = pluginName.stripWhiteSpace();
  l1 = 0;
  subID1 = -1;
  while (ladspa_dsc_func_list[l1]) {
    if (setName == ladspa_lib_name[l1].stripWhiteSpace()) {
      subID1 = l1;
      l2 = 0;
      subID2 = -1;
      while(ladspa_dsc_func_list[l1](l2) != NULL) {
        qs.sprintf("%s", ladspa_dsc_func_list[l1](l2)->Name);
        if (pluginName == qs.stripWhiteSpace()) {
          subID2 = l2;
          break;
        }
        l2++;
      }  
      break;
    } 
    l1++;
  }
  *index = subID1;
  *n = subID2;
  return((subID1 >= 0) && (subID2 >= 0));
}

float SynthData::exp_table(float x) {

  int index;

  index = (x + 16.0) * 1000.0;
  if (index >= EXP_TABLE_LEN) index = EXP_TABLE_LEN - 1;
  else if (index < 0) index = 0;
  return(exp_data[index]);
}     

int SynthData::initJack() {

  withJack = true;
  fprintf(stderr, "%s\n", jackName.latin1());
  if ((jack_handle = jack_client_new(jackName.latin1())) == 0) {
    fprintf(stderr, "jack server not running ?\n");
    exit(1);
  }
  jack_set_process_callback(jack_handle, playback_callback, (void *)this);
  setRate(jack_get_sample_rate(jack_handle));
  return(0);
}

int SynthData::activateJack() {

  jackRunning = true;
  if (jack_activate(jack_handle)) {
    fprintf(stderr, "cannot activate client");
  }
  return(0);
}

int SynthData::deactivateJack() {

  if (jackRunning) {
    jackRunning = false;
    if (jack_deactivate(jack_handle)) {
      fprintf(stderr, "cannot deactivate client");
    }
  }
  return(0);
}

int SynthData::playback_callback(jack_nframes_t nframes, void *arg) {

  SynthData *sd;

  sd = (SynthData *)arg;
  return(sd->jack_playback(nframes));
}

int SynthData::jack_playback(jack_nframes_t nframes) {

  int l1, l2;
  jack_default_audio_sample_t *buf[2];

  if (nframes > 16384) {
    fprintf(stderr, "nframes > 16384\n");
  } else {
    setCycleSize(nframes);
    for (l1 = 0; l1 < jackinModuleList.count(); l1++) {
      for (l2 = 0; l2 < 2; l2++) {
        buf[l2] = (jack_default_audio_sample_t *)jack_port_get_buffer(((M_jackin *)jackinModuleList.at(l1))->jack_in[l2], nframes);
        memcpy(((M_jackin *)jackinModuleList.at(l1))->jackdata[l2], buf[l2], sizeof(jack_default_audio_sample_t) * nframes);
      }
    }
    for (l1 = 0; l1 < jackoutModuleList.count(); l1++) {
      ((M_jackout *)jackoutModuleList.at(l1))->generateCycle();
      for (l2 = 0; l2 < 2; l2++) {
        buf[l2] = (jack_default_audio_sample_t *)jack_port_get_buffer(((M_jackout *)jackoutModuleList.at(l1))->port_out[l2], nframes);
        memcpy(buf[l2], ((M_jackout *)jackoutModuleList.at(l1))->jackdata[l2], sizeof(jack_default_audio_sample_t) * nframes);
      }
    }
    for (l1 = 0; l1 < wavoutModuleList.count(); l1++) {
      ((M_wavout *)wavoutModuleList.at(l1))->generateCycle();
    }
/*
    for (l1 = 0; l1 < scopeModuleList.count(); l1++) {
      ((M_scope *)scopeModuleList.at(l1))->generateCycle();
    }
    for (l1 = 0; l1 < spectrumModuleList.count(); l1++) {
      ((M_spectrum *)spectrumModuleList.at(l1))->generateCycle();
    }
*/
    for (l1 = 0; l1 < midioutModuleList.count(); l1++) {
      ((M_midiout *)midioutModuleList.at(l1))->generateCycle();
    }
    for (l1 = 0; l1 < moduleList.count(); l1++) {
      ((Module *)moduleList.at(l1))->cycleReady = false;
    }
  }
  return(0);
}
