#ifndef __ALSA_DRIVER_H
#define __ALSA_DRIVER_H

#define ALSA_PCM_OLD_HW_PARAMS_API 
#define ALSA_PCM_OLD_SW_PARAMS_API 

#include <alsa/asoundlib.h>



class Alsa_driver
{
public:

    Alsa_driver (const char     *name,
                 unsigned long   fsamp,
                 unsigned long   frsize,
                 unsigned long   nfrags,
                 bool            play,
                 bool            capt,
                 bool            ctrl);
    ~Alsa_driver (void);  

    void printinfo (void);
    int pcm_start (void);
    int pcm_stop (void);
    snd_pcm_sframes_t pcm_wait (void);
    int pcm_idle (snd_pcm_uframes_t nframes);
    int write_init (void);
    int write_done (void);
    int read_init (void);
    int read_done (void);
    void clear_chan (int chan) { _clear_func (_play_ptr [chan], _play_step, _frsize); }
    void write_chan (int chan, const float *src) { _write_func (src, _play_ptr [chan], _play_step, _frsize); }
    void read_chan  (int chan, float *dst) { _read_func (_capt_ptr [chan], dst, _capt_step, _frsize); }

    int stat  (void) { return _stat; }
    int nplay (void) { return _play_nchan; }
    int ncapt (void) { return _capt_nchan; }

private:

    typedef void (*clear_function)(char *, int, int);
    typedef void (*write_function)(const float *, char *, int, int);
    typedef void (*read_function) (const char *, float *, int, int);

    enum { MAXPFD = 8, MAXPLAY = 32, MAXCAPT = 32 };

    int set_hwpar (snd_pcm_t *handle,  snd_pcm_hw_params_t *hwpar, int *nchan);
    int set_swpar (snd_pcm_t *handle,  snd_pcm_sw_params_t *swpar);
    int recover (void);

    unsigned long          _fsamp;
    unsigned long          _frsize;
    unsigned long          _nfrags;
    snd_pcm_format_t       _format;
    snd_pcm_access_t       _access;
    bool                   _synced;
    snd_pcm_t             *_play_handle;
    snd_pcm_t             *_capt_handle;
    snd_ctl_t             *_ctrl_handle;
    snd_pcm_hw_params_t   *_play_hwpar;
    snd_pcm_sw_params_t   *_play_swpar;
    snd_pcm_hw_params_t   *_capt_hwpar;
    snd_pcm_sw_params_t   *_capt_swpar;
    int                    _play_nchan;
    int                    _capt_nchan;
    int                    _play_npfd;
    int                    _capt_npfd;
    bool                   _xrun;
    struct pollfd          _pfd [MAXPFD];
    int                    _play_step;
    int                    _capt_step;
    char                  *_play_ptr [MAXPLAY];
    char                  *_capt_ptr [MAXCAPT];
    snd_pcm_uframes_t      _offs;
    snd_pcm_uframes_t      _nfrm;
    int                    _stat;
    int                    _pcnt;
    clear_function         _clear_func;
    write_function         _write_func;
    read_function          _read_func;

    static void clear_32le (char *dst, int step, int nfrm);
    static void clear_16le (char *dst, int step, int nfrm);
    static void write_32le (const float *src, char *dst, int step, int nfrm);
    static void write_16le (const float *src, char *dst, int step, int nfrm);
    static void read_32le (const char *src, float *dst, int step, int nfrm);
    static void read_16le (const char *src, float *dst, int step, int nfrm);
};



#endif

