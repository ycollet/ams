#include <sys/time.h>
#include <assert.h>
#include "alsa_driver.h"


// Public members ----------------------------------------------------------------------


Alsa_driver::~Alsa_driver (void)
{
   snd_pcm_sw_params_free (_capt_swpar);
   snd_pcm_hw_params_free (_capt_hwpar);
   snd_pcm_sw_params_free (_play_swpar);
   snd_pcm_hw_params_free (_play_hwpar);

   if (_play_handle) snd_pcm_close (_play_handle);
   if (_capt_handle) snd_pcm_close (_capt_handle);
   if (_ctrl_handle) snd_ctl_close (_ctrl_handle); 
}


Alsa_driver::Alsa_driver (const char     *name,
                          unsigned long   fsamp,
                          unsigned long   frsize,
                          unsigned long   nfrags,
                          bool            play,
                          bool            capt,
                          bool            ctrl) :
    _play_handle (0),
    _capt_handle (0),
    _ctrl_handle (0),
    _play_hwpar (0),
    _play_swpar (0),
    _capt_hwpar (0),
    _capt_swpar (0),
    _fsamp (fsamp),
    _frsize (frsize),
    _nfrags (nfrags),
    _stat (-1)
{
    int err, dir;
    snd_ctl_card_info_t *card;

    if (play)
    {
	if (snd_pcm_open (&_play_handle, name, SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
            _play_handle = 0;
	    fprintf (stderr, "Alsa_driver: Cannot open PCM device %s for playback.\n", name);
	}
    }
    
    if (capt)
    {
	if (snd_pcm_open (&_capt_handle, name, SND_PCM_STREAM_CAPTURE, 0) < 0)
	{
            _capt_handle = 0;
	    fprintf (stderr, "Alsa_driver: Cannot open PCM device %s for capture.\n", name);
	}
    }

    if (! _play_handle && ! _capt_handle) return;

    if (ctrl)
    {
        snd_ctl_card_info_alloca (&card);

        if ((err = snd_ctl_open (&_ctrl_handle, name, 0)) < 0)
        {
	    fprintf  (stderr, "Alse_driver: ctl_open(): %s\n", snd_strerror (err));
    	    return;
        }
	
        if ((err = snd_ctl_card_info (_ctrl_handle, card)) < 0)
        {
   	     fprintf  (stderr, "Alsa_driver: ctl_card_info(): %s\n", snd_strerror (err));
	     return;
        }
    }

    // check capabilities here

    if (_play_handle)
    {
	if (snd_pcm_hw_params_malloc (&_play_hwpar) < 0)
        {
 	    fprintf (stderr, "Alsa_driver: can't allocate playback hw params\n");
            return;
	}
	if (snd_pcm_sw_params_malloc (&_play_swpar) < 0)
        {
 	    fprintf (stderr, "Alsa_driver: can't allocate playback sw params\n");
            return;
	}
	if (set_hwpar (_play_handle, _play_hwpar, &_play_nchan) < 0)
	{
	    fprintf (stderr, "Alsa_driver: can't set playback hardware parameters.\n");
            return;
	}
        if (_play_nchan > MAXPLAY)
	{
	    fprintf (stderr, "Alsa_driver: device has more than %d playback channels\n", MAXPLAY);
            return;
	}
	if (set_swpar (_play_handle, _play_swpar) < 0)
	{
	    fprintf (stderr, "Alsa_driver: can't set playback software parameters.\n");
            return;
	}
    }
	
    if (_capt_handle)
    {
	if (snd_pcm_hw_params_malloc (&_capt_hwpar) < 0)
        {
 	    fprintf (stderr, "Alsa_driver: can't allocate capture hw params\n");
            return;
	}
	if (snd_pcm_sw_params_malloc (&_capt_swpar) < 0)
        {
 	    fprintf (stderr, "Alsa_driver: can't allocate capture sw params\n");
            return;
	}
	if (set_hwpar (_capt_handle, _capt_hwpar, &_capt_nchan) < 0)
	{
	    fprintf (stderr, "Alsa_driver: can't set capture hardware parameters.\n");
            return;
	}
        if (_capt_nchan > MAXCAPT)
	{
	    fprintf (stderr, "Alsa_driver: device has more than %d capture channels\n", MAXCAPT);
            return;
	}
	if (set_swpar (_capt_handle, _capt_swpar) < 0)
	{
	    fprintf (stderr, "Alsa_driver: can't set capture software parameters.\n");
            return;
	}
    }

    if (_play_handle)
    {
	if (snd_pcm_hw_params_get_period_size (_play_hwpar, &dir) != _frsize)
	{
	    fprintf (stderr, "Alsa_driver: can't get requested fragment size for playback.\n"); 
            return;
	}

        _fsamp  = snd_pcm_hw_params_get_rate (_play_hwpar, &dir);
        _nfrags = snd_pcm_hw_params_get_periods (_play_hwpar, &dir);
 	_format = (snd_pcm_format_t) snd_pcm_hw_params_get_format (_play_hwpar);
        _access = (snd_pcm_access_t) snd_pcm_hw_params_get_access (_play_hwpar);
    }

    if (_capt_handle)
    {
	if (snd_pcm_hw_params_get_period_size (_capt_hwpar, &dir) != _frsize)
	{
	    fprintf (stderr, "Alsa_driver: can't get requested fragment size for capture.\n"); 
            return;
	}

        if (_play_handle)
	{
	    if (snd_pcm_hw_params_get_periods (_capt_hwpar, &dir) != _nfrags)
	    {
  	        fprintf (stderr, "Alsa_driver: fragment sizes for playback and capture are not the same.\n"); 
                return;
	    }
	    if (snd_pcm_hw_params_get_format (_capt_hwpar) != _format)
	    {
  	        fprintf (stderr, "Alsa_driver: sample formats for playback and capture are not the same.\n"); 
                return;
	    }
	    if (snd_pcm_hw_params_get_access (_capt_hwpar) != _access)
	    {
  	        fprintf (stderr, "Alsa_driver: access methods for playback and capture are not the same.\n"); 
                return;
	    }
	}
        else
	{
            _nfrags = snd_pcm_hw_params_get_periods (_capt_hwpar, &dir);
 	    _format = (snd_pcm_format_t) snd_pcm_hw_params_get_format (_capt_hwpar);
            _access = (snd_pcm_access_t) snd_pcm_hw_params_get_access (_capt_hwpar);
        }
    }

    switch (_format)
    {
    case SND_PCM_FORMAT_S32_LE:
        _clear_func = clear_32le;     
        _write_func = write_32le;     
        _read_func  = read_32le;     
        break;

    case SND_PCM_FORMAT_S16_LE:
        _clear_func = clear_16le;     
        _write_func = write_16le;     
        _read_func  = read_16le;     
	break;

    default:
        fprintf (stderr, "Alsa_driver: can't handle sample format.\n"); 
        return;
    }
 
    _synced = (_capt_handle && _play_handle) ? (snd_pcm_link (_capt_handle, _play_handle) == 0) : false;
    _play_npfd = _play_handle ? snd_pcm_poll_descriptors_count (_play_handle) : 0;
    _capt_npfd = _capt_handle ? snd_pcm_poll_descriptors_count (_capt_handle) : 0;
    if (_play_npfd + _capt_npfd > MAXPFD)
    {
        fprintf (stderr, "Alsa_driver: interface requires more than %d pollfd\n", MAXPFD);
   	return;
    }      

    _stat = 0;
}	


int Alsa_driver::write_init (void)
{
    int  err, i;
    const snd_pcm_channel_area_t *a;

    _nfrm = _frsize;
    if ((err = snd_pcm_mmap_begin (_play_handle, &a, &_offs, &_nfrm)) < 0)
    {
        fprintf (stderr, "Alsa_driver: snd_pcm_mmap_begin(play): %s.\n", snd_strerror (err)); 
        return -1;
    }
    assert (_nfrm == _frsize);

    _play_step = (a->step) >> 3;
    for (i = 0; i < _play_nchan; i++, a++)
    {
        _play_ptr [i] = (char *) a->addr + ((a->first + a->step * _offs) >> 3);
    } 
   
    return 0;
}


int Alsa_driver::write_done (void)
{
   return snd_pcm_mmap_commit (_play_handle, _offs, _frsize);
}


int Alsa_driver::read_init (void)
{
    int  err, i;
    const snd_pcm_channel_area_t *a;

    _nfrm = _frsize;
    if ((err = snd_pcm_mmap_begin (_capt_handle, &a, &_offs, &_nfrm)) < 0)
    {
        fprintf (stderr, "Alsa_driver: snd_pcm_mmap_begin(capt): %s.\n", snd_strerror (err)); 
        return -1;
    }
    assert (_nfrm == _frsize);

   _capt_step = (a->step) >> 3;
    for (i = 0; i < _capt_nchan; i++, a++)
    {
        _capt_ptr [i] = (char *) a->addr + ((a->first + a->step * _offs) >> 3);
    } 
   
    return 0;
}


int Alsa_driver::read_done (void)
{
   return snd_pcm_mmap_commit (_capt_handle, _offs, _frsize);
}


int Alsa_driver::pcm_start (void)
{
    int   err, i, j;

    if (_play_handle && ((err = snd_pcm_prepare (_play_handle)) < 0))
    {
	fprintf (stderr, "Alsa_driver: pcm_prepare(play): %s\n", snd_strerror (err));
        return -1;
    }

    if (_capt_handle && !_synced && ((err = snd_pcm_prepare (_capt_handle)) < 0))
    {
	fprintf (stderr, "Alsa_driver: pcm_prepare(capt): %s\n", snd_strerror (err));
        return -1;
    }

    if (_play_handle)
    {
	_nfrm = snd_pcm_avail_update (_play_handle);
	if (_nfrm != _frsize * _nfrags)
        {
	    fprintf  (stderr, "Alsa_driver: full buffer not available at start.\n");
            return -1;
	}

        for (i = 0; i < _nfrags; i++)
	{     
            if (write_init ()) return -1;
            for (j = 0; j < _play_nchan; j++) clear_chan (j);
	    write_done ();
	}
		
	if ((err = snd_pcm_start (_play_handle)) < 0)
        {
	    fprintf (stderr, "Alsa_driver: pcm_start(play): %s.\n", snd_strerror (err));
   	    return -1;
	}
    }

    if (_capt_handle && !_synced && ((err = snd_pcm_start (_capt_handle)) < 0))
    {
	fprintf (stderr, "Alsa_driver: pcm_start(capt): %s.\n", snd_strerror (err));
   	return -1;
    }

    return 0;
}


int Alsa_driver::pcm_stop (void)
{
    int err;

    if (_play_handle && ((err = snd_pcm_drop (_play_handle)) < 0))
    {
	fprintf (stderr, "Alsa_driver: pcm_drop(play): %s\n", snd_strerror (err));
        return -1;
    }

    if (_capt_handle && ((err = snd_pcm_drop (_capt_handle)) < 0))
    {
	fprintf (stderr, "Alsa_driver: pcm_drop(capt): %s\n", snd_strerror (err));
        return -1;
    }

    return 0;
}


snd_pcm_sframes_t Alsa_driver::pcm_wait (void)
{
    snd_pcm_sframes_t avail = 0;
    snd_pcm_sframes_t capt_avail = 0;
    snd_pcm_sframes_t play_avail = 0;
    bool              need_capt;
    bool              need_play;
    int               i, n;
    int               p_timed_out;
    int               c_timed_out;

    _pcnt = 0;
    _stat = 0;
    _xrun = false;

    need_capt = _capt_handle ? true : false;
    need_play = _play_handle ? true : false;

    while (need_play || need_capt)
    {
        _pcnt++;
	n = 0;
        if (need_play)
        {
	    snd_pcm_poll_descriptors (_play_handle, &_pfd[0], _play_npfd);
	    n += _play_npfd;
	}
	if (need_capt)
        {
	    snd_pcm_poll_descriptors (_capt_handle, &_pfd[n], _capt_npfd);
            n += _capt_npfd;
	}

	for (i = 0; i < n; i++) _pfd[i].events |= POLLERR;

	if (poll (_pfd, n, 1000000) < 0)
        {
            if (errno == EINTR)
            {
                _stat = 1;
		return 0;
            }
            fprintf (stderr, "Alsa_driver: poll(): %s\n.", strerror (errno));
            _stat = 2;
	    return 0;
	}
 
        i = 0;
	p_timed_out = 0;
 	c_timed_out = 0;

	if (need_play)
        {
	    for (; i < _play_npfd; i++)
            {
 	        if (_pfd[i].revents & POLLERR) { _xrun = true; _stat |= 4; }
   	        if (_pfd[i].revents == 0) p_timed_out++;
     	    }
  	    if (p_timed_out == 0) need_play = false;
	}
	if (need_capt)
        {
  	    for (; i < n; i++)
            {
	        if (_pfd[i].revents & POLLERR) { _xrun = true; _stat |= 8; }
 		if (_pfd[i].revents == 0) c_timed_out++;
            }
            if (c_timed_out == 0) need_capt = false;
        }
		
	if (   (p_timed_out && (p_timed_out == _play_npfd))
            && (c_timed_out && (c_timed_out == _capt_npfd)))
        {
  	    fprintf (stderr, "Alsa_driver: poll timed out\n.");
	    _stat |= 16;
	    return 0;
        }		
    }

    if (_capt_handle)
    {
        if ((capt_avail = snd_pcm_avail_update (_capt_handle)) < 0)
        {
            _xrun = true;
            _stat |= 32;
        }
    }
    else capt_avail = 999999;

    if (_play_handle)
    {
        if ((play_avail = snd_pcm_avail_update (_play_handle)) < 0)
        {
            _xrun = true;
            _stat |= 64;
        }
    }
    else play_avail = 999999;

    if (_xrun)
    {
        recover ();
        return 0;
    }

    avail = capt_avail < play_avail ? capt_avail : play_avail;
    return avail;
}


int Alsa_driver::pcm_idle (snd_pcm_uframes_t nframes)
{
    snd_pcm_uframes_t n;
    snd_pcm_uframes_t p ;
    snd_pcm_uframes_t k ;
    const snd_pcm_channel_area_t *a;

    if (_capt_handle)
    {
        for (n = nframes, p = 0; n >= _frsize; n -= k)
        {
            k = n;
	    if (snd_pcm_mmap_begin (_capt_handle, &a, &p, &k)) return -1;
  	    if (snd_pcm_mmap_commit (_capt_handle, p, k) < 0) return -1;
	}
    }

    if (_play_handle)
    {
        for (n = nframes, p = 0; n >= _frsize; n -= k)
        {
            k = n;
	    if (snd_pcm_mmap_begin (_play_handle, &a, &p, &k)) return -1;
//    	    for (i = 0; i < _play_nchan; i++) silence_channel (i, k);
  	    if (snd_pcm_mmap_commit (_play_handle, p, k) < 0) return -1;
	}
    }

    return 0;
}


void Alsa_driver::printinfo (void)
{
    fprintf (stderr, "playack chan : %2d\n", _play_nchan);
    fprintf (stderr, "capture chan : %2d\n", _capt_nchan);
    fprintf (stderr, "%s\n", _synced ? "synced" : "not synced");
    fprintf (stderr, "fsamp  : %ld\n", _fsamp);
    fprintf (stderr, "frsize : %ld\n", _frsize);
    fprintf (stderr, "nfrags : %ld\n", _nfrags);
    fprintf (stderr, "format : ");
    switch (_format)
    {
    case SND_PCM_FORMAT_S32_LE:
        fprintf (stderr, "S32_LE\n");
        break;
    case SND_PCM_FORMAT_S16_LE:
        fprintf (stderr, "S16_LE\n");
        break;
    default:
        fprintf (stderr, "???\n");
    }        
       
}


// Private members ---------------------------------------------------------------------


int Alsa_driver::set_hwpar (snd_pcm_t *handle,  snd_pcm_hw_params_t *hwpar, int *nchan)
{
    int n, err;

    if ((err = snd_pcm_hw_params_any (handle, hwpar)) < 0)
    {
	fprintf (stderr, "Alsa_driver: no hw configurations available: %s.\n", snd_strerror (err));
	return -1;
    }

    if ((err = snd_pcm_hw_params_set_periods_integer (handle, hwpar)) < 0)
    {
        fprintf (stderr, "Alsa_driver: can't set period size to integral value.\n");
	return -1;
    }

    if (   ((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) < 0)
        && ((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0))
    {
        fprintf (stderr, "Alsa_driver: the interface doesn't support mmap-based access.\n");
        return -1;
    }

    if (   ((err = snd_pcm_hw_params_set_format (handle, hwpar, SND_PCM_FORMAT_S32)) < 0)
	&& ((err = snd_pcm_hw_params_set_format (handle, hwpar, SND_PCM_FORMAT_S16)) < 0))
    {
        fprintf (stderr, "Alsa_driver: the interface doesn't support 16 or 32 bit access.\n.");
	return -1;
    }

    if ((err = snd_pcm_hw_params_set_rate_near (handle, hwpar, _fsamp, 0)) < 0)
    {
        fprintf (stderr, "Alsa_driver: can't set sample rate to %u.\n", _fsamp);
	return -1;
    }

    n = snd_pcm_hw_params_get_channels_max (hwpar);

    if (n > 1024)
    { 
        fprintf (stderr, "You appear to be using the ALSA software \"plug\" layer, probably\n"
	    	         "a result of using the \"default\" ALSA device. This is less\n"
	   	         "efficient than it could be. Consider using a ~/.asoundrc file\n"
	  	         "to define a hardware audio device rather than using the plug layer.\n");
        n = 2;     
    }				
    *nchan = n;

    if ((err = snd_pcm_hw_params_set_channels (handle, hwpar, n)) < 0)
    {
        fprintf (stderr, "Alsa_driver: can't set channel count to %d.\n", n);
	return -1;
    }
	
    if ((err = snd_pcm_hw_params_set_period_size (handle, hwpar, _frsize, 0)) < 0)
    {
	fprintf (stderr, "Alsa_driver: can't set period size to %u.\n", _frsize);
	return -1;
    }

    if ((err = snd_pcm_hw_params_set_periods (handle, hwpar, _nfrags, 0)) < 0)
    {
	fprintf (stderr, "Alsa_driver: can't set number of periods to %u.\n", _nfrags);
 	return -1;
    }
	
    if ((err = snd_pcm_hw_params_set_buffer_size (handle, hwpar, _frsize * _nfrags)) < 0)
    {
	fprintf (stderr, "Alsa_driver: can't set buffer length to %u.\n", _frsize * _nfrags);
	return -1;
    }

    if ((err = snd_pcm_hw_params (handle, hwpar)) < 0)
    {
	fprintf (stderr, "Alsa_driver: can't set hardware parameters.\n");
	return -1;
    }

    return 0;
}


int Alsa_driver::set_swpar (snd_pcm_t *handle, snd_pcm_sw_params_t *swpar) 
{
    int err;

    snd_pcm_sw_params_current (handle, swpar);

    if ((err = snd_pcm_sw_params_set_start_threshold (handle, swpar, 0)) < 0)
    {
	fprintf (stderr, "Alsa_driver: can't set start mode.\n");
	return -1;
    }

    if ((err = snd_pcm_sw_params_set_stop_threshold (handle, swpar, _frsize * _nfrags)) < 0)
    {
        fprintf (stderr, "Alsa_driver: can't set stop mode.\n");
        return -1;
    }

    if ((err = snd_pcm_sw_params_set_silence_threshold (handle, swpar, 0)) < 0)
    {
        fprintf (stderr, "Alsa_driver: can't set silence threshold.\n");
        return -1;
    }

    if ((err = snd_pcm_sw_params_set_silence_size (handle, swpar, 0)) < 0)
    {
       fprintf (stderr, "Alsa_driver: can't set silence size.\n");
       return -1;
    }

    if ((err = snd_pcm_sw_params_set_avail_min (handle, swpar, _frsize)) < 0)
    {
       fprintf (stderr, "Alsa_driver: can't set avail min to %u.\n", _frsize);
       return -1;
    }

    if ((err = snd_pcm_sw_params (handle, swpar)) < 0)
    {
	fprintf (stderr, "Alsa_driver: can't set software parameters.\n");
 	return -1;
    }

    return 0;
}


int Alsa_driver::recover (void)
{
    int                err; 
    snd_pcm_status_t  *stat;

    snd_pcm_status_alloca (&stat);

    if ((err = snd_pcm_status (_play_handle ? _play_handle : _capt_handle, stat)) < 0)
    {
	fprintf (stderr, "Alsa_driver: pcm_status(): %s\n",  snd_strerror (err));
    }
    else if (snd_pcm_status_get_state (stat) == SND_PCM_STATE_XRUN)
    {
        struct timeval tnow, trig;

	gettimeofday (&tnow, 0);
	snd_pcm_status_get_trigger_tstamp (stat, &trig);
//	fprintf (stderr, "Alsa_driver: stat = %02x, xrun of at least %8.3lf ms\n", _stat,
//		 1e3 * tnow.tv_sec - 1e3 * trig.tv_sec + 1e-3 * tnow.tv_usec - 1e-3 * trig.tv_usec);
    }

    if (pcm_stop () || pcm_start ()) return -1;

    return 0;
}	


// Static members ----------------------------------------------------------------------


void Alsa_driver::clear_16le (char *dst, int step, int nfrm)
{
    while (nfrm--)
    {
        *((short int *) dst) = 0;
        dst += step;
    }  
}

void Alsa_driver::write_16le (const float *src, char *dst, int step, int nfrm)
{
    float     s;
    short int d;

    while (nfrm--)
    {
        s = *src++;
        if      (s >  1) d = 0x7fff;
        else if (s < -1) d = 0x8001;
        else d = (short int)(0x7fff * s); 
        *((short int *) dst) = d;
        dst += step;
    }  
}

void Alsa_driver::read_16le (const char *src, float *dst, int step, int nfrm )
{
    float     d;
    short int s;

    while (nfrm--)
    {
        s = *((short int *) src);
        d = (float) s / 0x7fff;  
        *dst++ = d;  
        src += step;
    }  
}

void Alsa_driver::clear_32le (char *dst, int step, int nfrm)
{
    while (nfrm--)
    {
        *((int *) dst) = 0;
        dst += step;
    }  
}

void Alsa_driver::write_32le (const float *src, char *dst, int step, int nfrm)
{
    float   s;
    int     d;

    while (nfrm--)
    {
        s = *src++;
        if      (s >  1) d = 0x007fffff;
        else if (s < -1) d = 0x00800001;  
        else d = (int)(0x007fffff * s); 
	*((int *) dst) = d << 8;
        dst += step;
    }  
}

void Alsa_driver::read_32le (const char *src, float *dst, int step, int nfrm)
{
    float   d;
    int     s;

    while (nfrm--)
    {
        s = *((int *) src);
        d = (float) s / 0x7fffff00;  
        *dst++ = d;  
        src += step;
    }  
}


// Unit test ---------------------------------------------------------------------------


#ifdef _UNIT_TEST_


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include "p_thread.h"


char          name [64];
unsigned long fsamp;
unsigned long frsize;
unsigned long nfrags;


class Testthr : public P_thread
{
public:
 
    virtual void thr_main (void);

private:

    float *buf0;
    float *buf1;

};


void Testthr::thr_main (void)
{
    Alsa_driver *D;
    unsigned long k;

    buf0 = new float [frsize];
    buf1 = new float [frsize];

    D = new Alsa_driver (name, fsamp, frsize, nfrags, true, true, true);
    if (D->stat ()) 
    {
        delete D;
        return;
    }
    D->printinfo ();

    D->pcm_start ();
    while (1)
    {
	k = D->pcm_wait ();  
//        D->pcm_idle (k);

        while (k >= frsize)
       	{
            D->read_init ();
            D->read_chan (0, buf0);
            D->read_chan (1, buf1);              
            D->read_done ();

            D->write_init ();
            D->write_chan (0, buf0);
            D->write_chan (1, buf1);              
            D->write_done ();

            k -= frsize;
	}
    }
    D->pcm_stop ();

    delete D;
}


int main (int ac, char *av [])
{
    Testthr T;

    if (ac < 4) return 1;

    mlockall(MCL_CURRENT | MCL_FUTURE);

    strcpy (name, av [1]);
    fsamp  = atoi (av [2]);
    frsize = atoi (av [3]);
    nfrags = atoi (av [4]);

    T.thr_start (SCHED_FIFO, 0);
    pthread_exit (0);

    return 0;
}


#endif
