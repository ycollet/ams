#include <stdio.h>
#include <stdlib.h>
#include <qobject.h>
#include <qstring.h>
#include <math.h>
#include "synthdata.h"
#include "m_env.h"
#include "m_vcenv.h"
#include "m_vcenv2.h"
#include "m_pcmout.h"
#include "m_pcmin.h"
#include "m_advenv.h"
#include "m_dynamicwaves.h"
#include "m_wavout.h"
#include "m_midiout.h"
#include "m_scope.h"
#include "m_spectrum.h"


SynthData::SynthData (int p_poly, float p_edge) : port_sem(1)
{
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
    noteCounter[l1] = 1000000;
    sustainNote[l1] = false;
    for (l2 = 0; l2 < MAXDRIFTARR; l2++) { 
      drift_a[l1][l2] = 2.0 * (double)random() / (double)RAND_MAX - 1.0;
      drift_c[l1][l2] = 0;
      drift_r[l1][l2] = 0;
    }  
    detune_a[l1] = 2.0 * (double)random() / (double)RAND_MAX - 1.0;
    detune_c[l1] = 0;
    detune_r[l1] = 0;
  }

  drift_rate = 0;
  drift_amp = 0;
  detune_rate = 0;
  detune_amp = 0;
  poly = p_poly;
  edge = p_edge;
  rate = 0;
  periods = 0;
  periodsize = 0;
  cyclesize = 0;
  moduleCount = 0;
  moduleID = 0;
  doSynthesis = false;
  sustainFlag = false;
  midiChannel = -1;
  midiControllerMode = 0;
  loadPath = "";
  savePath = "";
  rcPath = "";

  exp_data = (float *)malloc(EXP_TABLE_LEN * sizeof(float));
  wave_sine = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_saw = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_saw2 = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_rect = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_tri = (float *)malloc(WAVE_PERIOD * sizeof(float));

  dphi = 2.0 * M_PI / WAVE_PERIOD;
  phi = 0;
  for (l1 = 0; l1 < WAVE_PERIOD; l1++) {
    wave_sine[l1] = sin(phi);
    phi += dphi;
  }
  for (l1 = 0; l1 < EXP_TABLE_LEN; l1++) {
    exp_data[l1] = exp(l1 / 1000.0 - 16.0);
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

  jackName = "AMS";
  play_ports = 0;
  capt_ports = 0;
  withJack = false;
  withAlsa = false;
  for (int i = 0; i < MAX_PLAY_PORTS / 2; i++) play_mods [i] = 0;
  for (int i = 0; i < MAX_CAPT_PORTS / 2; i++) capt_mods [i] = 0;
  colorBackground = QColor(COLOR_MAINWIN_BG);
  colorModuleBackground = QColor(COLOR_MODULE_BG);
  colorModuleBorder = QColor(195, 195, 195);
  colorModuleFont = QColor(255, 255, 255);
  colorPortFont1 = QColor(255, 255, 255);
  colorPortFont2 = QColor(255, 240, 140);
  colorCable = QColor(180, 180, 180);
  colorJack = QColor(250, 200, 50);
}

SynthData::~SynthData()
{
    if (withJack) closeJack ();
    if (withAlsa) closeAlsa ();
    free (exp_data);
    free (wave_sine);
    free (wave_saw);
    free (wave_saw2);
    free (wave_rect);
    free (wave_tri);
    for (int i = 0; i < poly; i++) free (zeroModuleData [i]);
    free (zeroModuleData);
    delete (midiWidget);
    delete (guiWidget);
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
     
  int l1, l2, subID1, subID2Name, subID2Label;
  QString qsn, qsl;

  setName = setName.stripWhiteSpace();
  pluginName = pluginName.stripWhiteSpace();
  l1 = 0;
  subID1 = -1;
  while (ladspa_dsc_func_list[l1]) {
    if (setName == ladspa_lib_name[l1].stripWhiteSpace()) {
      subID1 = l1;
      l2 = 0;
      subID2Label = -1;
      subID2Name = -1;
      while(ladspa_dsc_func_list[l1](l2) != NULL) {
        qsl.sprintf("%s", ladspa_dsc_func_list[l1](l2)->Label);
        qsn.sprintf("%s", ladspa_dsc_func_list[l1](l2)->Name);
        if (pluginName == qsl.stripWhiteSpace()) {
          subID2Label = l2;
          break;
        } 
        if (pluginName == qsn.stripWhiteSpace()) {
          subID2Name = l2;                            // No break to give the priority to "Label"
        }
        l2++;
      }  
      break;
    } 
    l1++;
  }
  *index = subID1;
  *n = (subID2Label < 0) ? subID2Name : subID2Label; // Use "Name" only if no match for "Label"
  return( (subID1 >= 0) && ( (subID2Name >= 0) || (subID2Label >= 0) ) );
}

float SynthData::exp_table(float x) {

  int index;

  index = (x + 16.0) * 1000.0;
  if (index >= EXP_TABLE_LEN) index = EXP_TABLE_LEN - 1;
  else if (index < 0) index = 0;
  return(exp_data[index]);
}     

void SynthData::create_zero_data (void)
{
    zeroModuleData = (float **) malloc (poly * sizeof(float *)); 
    for (int i = 0; i < poly; i++)
    {
        zeroModuleData [i] = (float *) malloc (periodsize * sizeof(float));
        memset (zeroModuleData [i], 0, periodsize * sizeof(float));
    }
}



int SynthData::find_play_mod (void *M)
{
    for (int i = 0; i < play_ports / 2; i++) if (play_mods [i] == M) return i;
    return -1;
}

int SynthData::find_capt_mod (void *M)
{
    for (int i = 0; i < capt_ports / 2; i++) if (capt_mods [i] == M) return i;
    return -1;
}



int SynthData::initAlsa (const char *name, int fsamp, int frsize, int nfrags, int ncapt, int nplay)
{
    pthread_attr_t     attr;
    struct sched_param parm;

    withAlsa = true;
    ncapt &= ~1;
    nplay &= ~1;

    alsa_handle = new Alsa_driver (name, fsamp, frsize, nfrags, nplay > 0, ncapt > 0, false);
    if (alsa_handle->stat () < 0)
    {
        fprintf (stderr, "Can't connect to ALSA\n");
        exit (1);
    } 
    capt_ports = alsa_handle->ncapt ();
    play_ports = alsa_handle->nplay ();
    if (capt_ports > ncapt) capt_ports = ncapt;
    if (play_ports > nplay) play_ports = nplay;
    if (capt_ports > MAX_CAPT_PORTS) capt_ports = MAX_CAPT_PORTS;
    if (play_ports > MAX_PLAY_PORTS) play_ports = MAX_PLAY_PORTS;

    fprintf (stderr, "ALSA device %s opened with %d inputs and %d outputs\n", name, capt_ports, play_ports); 

    rate = fsamp;
    periodsize = frsize;
    cyclesize  = frsize;
    create_zero_data ();

    parm.sched_priority = sched_get_priority_max (SCHED_FIFO);
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    pthread_attr_setschedparam (&attr, &parm);
    pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
    if (pthread_create (&alsa_thread, &attr, alsa_static_thr_main, this))
    {
        fprintf (stderr, "Can't create ALSA thread with RT priority\n");
        pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
        if (pthread_create (&alsa_thread, &attr, alsa_static_thr_main, this))
        {
            fprintf (stderr, "Can't create ALSA thread\n");
            exit (1);
	}
    }

    return 0;
}

int SynthData::closeAlsa ()
{
    fprintf (stderr, "Closing ALSA...\n");
    withAlsa = false;
    sleep (1);
    delete alsa_handle;
    return 0;
}

void *SynthData::alsa_static_thr_main (void *arg)
{
    return ((SynthData *) arg)->alsa_thr_main ();
}

void *SynthData::alsa_thr_main (void)
{
    int           i;
    unsigned long k;
    M_pcmin     *C;
    M_pcmout    *P;

    alsa_handle->pcm_start ();

    while (withAlsa)
    {
	k = alsa_handle->pcm_wait ();  

        while (k >= cyclesize)
       	{
            if (capt_ports)
	    {
                alsa_handle->read_init ();
                for (i = 0; i < capt_ports; i += 2)
                {
                    C = doSynthesis ? (M_pcmin *)(capt_mods [i / 2]) : 0;
                    if (C)
                    {
                        alsa_handle->read_chan (i,     C->pcmdata [0]);
                        alsa_handle->read_chan (i + 1, C->pcmdata [1]);
		    }
		}
                alsa_handle->read_done ();
	    }

            if (play_ports)
	    {
                alsa_handle->write_init ();
                for (i = 0; i < play_ports; i += 2)
                {
                    P = doSynthesis ? (M_pcmout *)(play_mods [i / 2]) : 0;
                    if (P)
                    {
                        P->generateCycle ();
                        alsa_handle->write_chan (i,     P->pcmdata [0]);
                        alsa_handle->write_chan (i + 1, P->pcmdata [1]);
		    }
	            else
                    {
                        alsa_handle->clear_chan (i);
                        alsa_handle->clear_chan (i + 1);
		    }
		}
                alsa_handle->write_done ();
	    }       

            if (doSynthesis) call_modules ();

            k -= cyclesize;
	}
    }
 
    alsa_handle->pcm_stop ();

    return 0;
}



int SynthData::initJack (int ncapt, int nplay)
{
    QString qs;

    withJack = true;

    play_ports = nplay & ~1;
    capt_ports = ncapt & ~1;
    if (capt_ports > MAX_CAPT_PORTS) capt_ports = MAX_CAPT_PORTS;
    if (play_ports > MAX_PLAY_PORTS) play_ports = MAX_PLAY_PORTS;

    if ((jack_handle = jack_client_new (jackName.latin1())) == 0)
    {
        fprintf (stderr, "Can't connect to JACK\n");
        exit (1);
    }

    jack_set_process_callback (jack_handle, jack_static_callback, (void *)this);
 
    rate = jack_get_sample_rate (jack_handle);
    periodsize = MAXIMUM_PERIODSIZE;
    create_zero_data ();

    for (int i = 0; i < play_ports; i++)
    {
        qs.sprintf("ams_out_%d", i);
        jack_out [i] = jack_port_register (jack_handle, qs.latin1(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }
    for (int i = 0; i < capt_ports; i++)
    {
        qs.sprintf("ams_in_%d", i);
        jack_in [i] = jack_port_register (jack_handle, qs.latin1(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    }

    if (jack_activate (jack_handle))
    {
        fprintf(stderr, "Can't activate JACK");
        exit (1);
    }

    fprintf (stderr, "Connected to JACK with %d inputs and %d outputs\n", capt_ports, play_ports); 

    return 0;
}

int SynthData::closeJack ()
{
    fprintf (stderr, "Closing JACK...\n");
    jack_deactivate (jack_handle);
    for (int i = 0; i < play_ports; i++) jack_port_unregister(jack_handle, jack_out[i]);
    for (int i = 0; i < capt_ports; i++) jack_port_unregister(jack_handle, jack_in[i]);
    jack_client_close (jack_handle);
    return 0;
}


int SynthData::jack_static_callback (jack_nframes_t nframes, void *arg)
{
    return ((SynthData *) arg)->jack_callback (nframes);
}


int SynthData::jack_callback (jack_nframes_t nframes)
{
    int         i, j;
    M_pcmin    *C;
    M_pcmout   *P;
    jack_default_audio_sample_t *p;

    if (nframes > MAXIMUM_PERIODSIZE)
    { 
        fprintf(stderr, "nframes exceeds allowed value %d\n", MAXIMUM_PERIODSIZE);
        return 0;
    }

    cyclesize = nframes;

    for (i = 0; i < capt_ports; i += 2)
    {
        C = doSynthesis ? (M_pcmin *)(capt_mods [i / 2]) : 0;
        for (j = 0; j < 2; j++)
        {
            p = (jack_default_audio_sample_t *)(jack_port_get_buffer (jack_in [i + j], nframes));
            if (C) memcpy (C->pcmdata [j], p, sizeof(jack_default_audio_sample_t) * nframes);
	}
    }

    for (i = 0; i < play_ports; i += 2)
    {
        P = doSynthesis ? (M_pcmout *)(play_mods [i / 2]) : 0;    
        if (P) P->generateCycle ();
        for (j = 0; j < 2; j++)
        {
            p = (jack_default_audio_sample_t *)(jack_port_get_buffer (jack_out [i + j], nframes));
            if (P) memcpy (p, P->pcmdata [j], sizeof(jack_default_audio_sample_t) * nframes);
            else   memset (p, 0, sizeof(jack_default_audio_sample_t) * nframes);
	}
    }

    if (doSynthesis) call_modules ();
 
    return 0;
}



void SynthData::call_modules (void)
{
    int i;
     
    for (i = 0; i < wavoutModuleList.count();   i++) ((M_wavout *)wavoutModuleList.at(i))->generateCycle();
    for (i = 0; i < scopeModuleList.count();    i++) ((M_scope *)scopeModuleList.at(i))->generateCycle();
    for (i = 0; i < spectrumModuleList.count(); i++) ((M_spectrum *)spectrumModuleList.at(i))->generateCycle(); 
    for (i = 0; i < midioutModuleList.count();  i++) ((M_midiout *)midioutModuleList.at(i))->generateCycle();
    for (i = 0; i < moduleList.count();         i++) ((Module *)moduleList.at(i))->cycleReady = false;
    for (i = 0; i < poly; i++) {
      noteCounter[i]++;
      if (noteCounter[i] > 1000000000) noteCounter[i] = 1000000000;
    }
}

