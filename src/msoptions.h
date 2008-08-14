/* msoptions.h 
 * This file defines the struct for modular synth options
*/
#ifndef MSOPTIONS_H
#define MSOPTIONS_H

#include <alsa/asoundlib.h>
#include <QString>

struct ModularSynthOptions {
    QString synthName;
    QString pcmname;
    QString presetName;
    QString presetPath;
    snd_pcm_uframes_t frsize;
    unsigned int fsamp;
    unsigned int nfrags;
    int ncapt;
    int nplay;
    int poly;
    int rcFd;
    bool noGui;
    bool havePreset;
    bool havePresetPath;
    bool enableJack;
    float edge;
};

#endif    // MSOPTIONS_H
