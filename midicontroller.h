#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include <qobject.h>
#include <alsa/asoundlib.h>

class MidiControlCenter;
class MidiControllerList;
#include "midicontrolcenter.h"
#include "midibinding.h"

/**
  *@author Matthias Nagorni, Lukas Degener
  */

class MidiController : public QObject  {

  Q_OBJECT
  private:


  public:
    static MidiController * instance(MidiControlCenter *, int p_type, int p_ch, int p_param);
    static bool sendMidiEvent(snd_seq_event_t *ev, MidiControlCenter * controlCenter);
  private:
    int type;
    int ch;
    int param;
    /*! You can not directly instantiate this class. use the static member
     *  instance(MidiControlCenter *,int, int, int)
     */
    MidiControlCenter * mControlCenter;

    MidiController(int p_type, int p_ch, int p_param, MidiControlCenter *parent, const char * name = 0);
  public:
    ~MidiController();
    bool operator==(const MidiController & orig);
  public slots:
    void sendMidiValue(int value);
    void addBinding(MidiBinding * binding);
    void removeBinding(MidiBinding * binding);

  signals:
    void midiValueChanged(int);
    void bindingAdded(MidiController *, MidiBinding *);
    void bindingRemoved(MidiController *, MidiBinding *);

};

#endif
