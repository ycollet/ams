#ifndef SYNTHDATA_H
#define SYNTHDATA_H

#include <stdio.h>
#include <stdlib.h>
#include <qobject.h>
#include <qthread.h>
#include <qstring.h>
#include <alsa/asoundlib.h>
#include <jack/jack.h>
#include <ladspa.h>
#include <qlist.h>

#define DEFAULT_RATE            44100
#define DEFAULT_PERIODSIZE       1024
#define DEFAULT_PERIODS             2
#define DEFAULT_CHANNELS            2
#define DEFAULT_SAMPLESIZE          2
#define MAXPOLY                   128
#define MAX_SO                    256
#define PI 3.1415926535897932384626433832795029
#define WAVE_PERIOD             65536
#define M_MCV_MAX_FREQ        20000.0
#define EXP_TABLE_LEN           32768
 
class SynthData : public QObject
{
  private:
    static int playback_callback(jack_nframes_t nframes, void *arg);
    int jack_playback(jack_nframes_t nframes);
  public:
    float *wave_sine;
    float *wave_saw;
    float *wave_saw2;
    float *wave_rect;
    float *wave_tri;
    float *exp_data;
    int notes[MAXPOLY];               // Midi notes
    int velocity[MAXPOLY];            // Velocity
    int channel[MAXPOLY];
    bool notePressed[MAXPOLY];
    bool noteActive[MAXPOLY];
    float **data; // Global output ringbuffer 
    float **indata[2]; // Global input ringbuffer
    bool upperInDataReady;
    int cyclesize;     // Size of synth cycle (cycleSize <= periodsize)
    int poly;
    int oscCount;
    float **zeroModuleData;
    float edge;
    int periodsize;
    int periods;
    int rate;
    int channels;
    int framesize;
    int samplesize;    // Size of one sample per channel (16 Bit --> 2)
    int min, max;      // Min, max value in last completed cycle
    int moduleCount, moduleID, moduleInCount, moduleOutCount;
    int jackInCount, jackOutCount;
    QList<QObject> outModuleList;
    QList<QObject> wavoutModuleList;
    QList<QObject> midioutModuleList;
    QList<QObject> jackoutModuleList;
    QList<QObject> jackinModuleList;
    QList<QObject> scopeModuleList;
    QList<QObject> spectrumModuleList;
    QList<QObject> moduleList;
    QList<QObject> listM_env;
    QList<QObject> listM_advenv;
    QList<QObject> listM_dynamicwaves;
    QList<QObject> listM_mcv;
    snd_pcm_t *pcm_handle;
    snd_pcm_t *pcm_capture_handle;
    snd_seq_t *seq_handle;
    bool doSynthesis, doCapture, withJack, jackRunning;
    QSemaphore port_sem;          
    LADSPA_Descriptor_Function ladspa_dsc_func_list[MAX_SO];
    QString ladspa_lib_name[MAX_SO];
    QObject *midiWidget;
    jack_client_t *jack_handle;
    QString jackName;
    int midi_out_port[2];
      
  public:
    SynthData(int p_poly, int p_periodsize);
    ~SynthData();
    float exp_table(float x);
    int nextFreeOsc();
    int incModuleCount();
    int decModuleCount();
    int getModuleCount();
    int getModuleID();
    int initJack();
    int activateJack();
    int deactivateJack(); 
    int setCycleSize(int p_cyclesize);
    int setPeriodsize(int p_periodsize);
    int setPeriods(int p_periods);
    int setRate(int p_rate);
    int setChannels(int p_channels);
    int setSampleSize(int p_samplesize);
    int getLadspaIDs(QString setName, QString pluginName, int *index, int *n);
};
  
#endif
      