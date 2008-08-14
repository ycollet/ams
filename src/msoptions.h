/* msoptions.h 
 * This file defines the struct for modular synth options
*/
#ifndef MSOPTIONS_H
#define MSOPTIONS_H

#include <QString>

struct ModularSynthOptions {
    QString synthName;
    QString pcmname;
    QString presetName;
    QString presetPath;
    int frsize;
    int fsamp;
    int ncapt;
    int nfrags;
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
