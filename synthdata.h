#ifndef SYNTHDATA_H
#define SYNTHDATA_H

#include <stdlib.h>
#include <qobject.h>
#include <qthread.h>
#include <qstring.h>
#include <qlist.h>
#include <ladspa.h>
#include <jack/jack.h>
#include "main.h"
#include "alsa_driver.h"


 
class SynthData : public QObject
{
  private:

    int  play_ports;
    int  capt_ports;
    void *play_mods [MAX_PLAY_PORTS / 2];
    void *capt_mods [MAX_CAPT_PORTS / 2];

    bool withAlsa;
    Alsa_driver *alsa_handle;
    pthread_t    alsa_thread;
 
    bool withJack;
    jack_client_t *jack_handle;
    jack_port_t *jack_in  [MAX_CAPT_PORTS];
    jack_port_t *jack_out [MAX_PLAY_PORTS];

  public:

    float *wave_sine;
    float *wave_saw;
    float *wave_saw2;
    float *wave_rect;
    float *wave_tri;
    float *exp_data;
    float **zeroModuleData;
    float edge;

    int notes[MAXPOLY];               // Midi notes
    int velocity[MAXPOLY];            // Velocity
    int channel[MAXPOLY];
    long noteCounter[MAXPOLY];

    int poly;
    int rate;
    int cyclesize;
    int periodsize;
    int periods;
    int moduleCount;
    int moduleID;

    QList<QObject> outModuleList;
    QList<QObject> wavoutModuleList;
    QList<QObject> midioutModuleList;
    QList<QObject> scopeModuleList;
    QList<QObject> spectrumModuleList;
    QList<QObject> moduleList;
    QList<QObject> listM_env;
    QList<QObject> listM_vcenv;
    QList<QObject> listM_advenv;
    QList<QObject> listM_dynamicwaves;
    QList<QObject> listM_mcv;
    QList<QObject> listM_advmcv;
    QList<QObject> listM_scmcv;

    snd_seq_t *seq_handle;
    bool doSynthesis;
    QSemaphore port_sem;          
    LADSPA_Descriptor_Function ladspa_dsc_func_list[MAX_SO];
    QString ladspa_lib_name[MAX_SO];
    QObject *midiWidget, *guiWidget;
    QString jackName;
    int midi_out_port[2];
    int midiChannel;

  private:

    void create_zero_data (void);
    static void *alsa_static_thr_main (void *arg);
    static int   jack_static_callback (jack_nframes_t nframes, void *arg);
    void  *alsa_thr_main (void);
    int    jack_callback (jack_nframes_t nframes);
    void   call_modules (void);

  public:

    SynthData(int p_poly, float p_edge);
    ~SynthData();

    float exp_table(float x);
    int incModuleCount();
    int decModuleCount();
    int getModuleCount();
    int getModuleID();
    int getLadspaIDs(QString setName, QString pluginName, int *index, int *n);

    int find_capt_mod (void *);
    int find_play_mod (void *);
    void set_capt_mod (unsigned int k, void *M) { if (k < MAX_CAPT_PORTS / 2) capt_mods [k] = M; }
    void set_play_mod (unsigned int k, void *M) { if (k < MAX_PLAY_PORTS / 2) play_mods [k] = M; }

    int initAlsa (const char *name, int fsamp, int frsize, int nfrags, int ncapt, int nplay);
    int closeAlsa();

    int initJack (int ncapt, int nplay);
    int closeJack();

};
  
#endif
      
