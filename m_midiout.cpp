#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qtimer.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_midiout.h"
#include "module.h"
#include "port.h"


M_midiout::M_midiout(QWidget* parent, const char *name) 
              : Module(0, parent, name) {

  QString qs;
  int l1, l2;

  M_type = M_type_midiout;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_MIDIOUT_WIDTH, MODULE_MIDIOUT_HEIGHT);
  mixer_gain[0] = 1.0;
  mixer_gain[1] = 1.0;
  midiMode = 0;
  offset[0] = 0;
  offset[1] = 0;
  controller[0] = 24;
  controller[1] = 25;
  triggerLevel = 0.5;
  port_in[0] = new Port("In 0", PORT_IN, 0, this);          
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  port_in[1] = new Port("In 1", PORT_IN, 1, this);          
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);
  port_M_trigger = new Port("Trigger", PORT_IN, 2, this);
  port_M_trigger->move(0, 75);
  port_M_trigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_trigger);
  qs.sprintf("MIDI Out ID %d", moduleID);
  configDialog->setWindowTitle(qs);
  configDialog->initTabWidget();
  QVBoxLayout *gainTab = configDialog->addVBoxTab("Gain / Offset / Trigger Level");
  QVBoxLayout *midiTab = configDialog->addVBoxTab("MIDI Settings");
  QStringList channelNames;
  for (l1 = 0; l1 < 16; l1++) {
    qs.sprintf("%4d", l1);
    channelNames << qs;
  }
  channel = 0;
  configDialog->addComboBox(0, "MIDI Channel", &channel, channelNames.count(), &channelNames, midiTab);
  configDialog->addSlider(0, 10, mixer_gain[0], "Gain 0", &mixer_gain[0], false, gainTab);
  configDialog->addSlider(0, 10, mixer_gain[1], "Gain 1", &mixer_gain[1], false, gainTab);
  configDialog->addIntSlider(0, 127, offset[0], "Offset 0", &offset[0], gainTab);
  configDialog->addIntSlider(0, 127, offset[1], "Offset 1", &offset[1], gainTab);
  QStringList midiNames;
  midiNames <<
    "In 0/1: Controller" <<
    "In 0: Controller In 1: Pitchbend" <<
    "In 0/1: Note" <<
    "In 0: Note, In 1: Velocity";
  configDialog->addComboBox(midiMode, "MIDI Event Type", &midiMode, midiNames.count(), &midiNames, midiTab);
  configDialog->addIntSlider(0, 127, controller[0], "Controller 0", &controller[0], midiTab);
  configDialog->addIntSlider(0, 127, controller[1], "Controller 1", &controller[1], midiTab);
  configDialog->addSlider(0, 10, triggerLevel, "Trigger Level", &triggerLevel, false, gainTab);
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    trigger[l1] = false;
    for (l2 = 0; l2 < 2; l2++) {
      triggeredNote[l2][l1] = 0;
      lastmididata[l2][l1] = 0;
    }
  }
}

M_midiout::~M_midiout()
{
  synthdata->midioutModuleList.removeAll(this);
}

void M_midiout::generateCycle() {

  int l1, l2, l3, mididata, velocitydata;
  snd_seq_event_t ev;

  triggerData = port_M_trigger->getinputdata();
  for (l1 = 0; l1 < 2; l1++) inData [l1] = port_in [l1]->getinputdata();

  switch (midiMode) {
    case 0:
      if (triggerData == synthdata->zeroModuleData) {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
                mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                if (mididata < 0) mididata = 0;
                else if (mididata > 127) mididata = 127;
                if (mididata != lastmididata[l1][l3]) {
                  snd_seq_ev_clear(&ev);     
                  snd_seq_ev_set_subs(&ev);  
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_CONTROLLER;
                  ev.data.control.channel = channel;
                  ev.data.control.param = controller[l1];
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  lastmididata[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              } 
            }
          } 
        }
      } else {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              for (l1 = 0; l1 < 2; l1++) {
                if (mixer_gain[l1] > 0.01) {
                  snd_seq_ev_clear(&ev);     
                  snd_seq_ev_set_subs(&ev);  
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_CONTROLLER;
                  mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                  if (mididata < 0) mididata = 0;
                  else if (mididata > 127) mididata = 127;
                  ev.data.control.channel = channel;
                  ev.data.control.param = controller[l1];
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
              }
            }
          } 
        }
      }
      break;
    case 1:
      if (triggerData == synthdata->zeroModuleData ) {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
               if (l1) {
                 mididata = (int)(128.0 * offset[l1] + (int)(mixer_gain[l1] * inData[l1][l3][l2] * 16384.0) - 8192.0);
                 if (mididata < -8191) mididata = -8191;                                              
                 else if (mididata > 8191) mididata = 8191;
                } else {
                  mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                  if (mididata < 0) mididata = 0;
                  else if (mididata > 127) mididata = 127;
                }
                if (mididata != lastmididata[l1][l3]) {
                  snd_seq_ev_clear(&ev);     
                  snd_seq_ev_set_subs(&ev);  
                  snd_seq_ev_set_direct(&ev);
                  if (l1) {
                    ev.type = SND_SEQ_EVENT_PITCHBEND;
                    ev.data.control.param = 0;
                  } else {
                    ev.type = SND_SEQ_EVENT_CONTROLLER;
                    ev.data.control.param = controller[l1];
                  }
                  ev.data.control.channel = channel;
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  lastmididata[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              } 
            }
          } 
        }
      } else {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              for (l1 = 0; l1 < 2; l1++) {
                if (mixer_gain[l1] > 0.01) {
                  snd_seq_ev_clear(&ev);     
                  snd_seq_ev_set_subs(&ev);  
                  snd_seq_ev_set_direct(&ev);
                  if (l1) {
                    ev.type = SND_SEQ_EVENT_PITCHBEND; 
                    ev.data.control.param = 0;
                    mididata = (int)(128.0 * offset[l1] + (int)(mixer_gain[l1] * inData[l1][l3][l2] * 16384.0) - 8192.0);
                    if (mididata < -8191) mididata = -8191;
                    else if (mididata > 8191) mididata = 8191;
                  } else {
                    ev.type = SND_SEQ_EVENT_CONTROLLER; 
                    ev.data.control.param = controller[l1];
                    mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                    if (mididata < 0) mididata = 0;
                    else if (mididata > 127) mididata = 127;
                  }
                  ev.data.control.channel = channel;
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
              }
            }
          } 
        }
      }
      break;
    case 2: 
      if (triggerData != synthdata->zeroModuleData ) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              for (l1 = 0; l1 < 2; l1++) {
                if (mixer_gain[l1] > 0.01) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEON;
                  ev.data.control.channel = channel;
                  mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.0) - 1000;
                  if (mididata < 0) mididata = 0;
                  else if (mididata > 127) mididata = 127;
                  ev.data.note.note = mididata;
                  ev.data.note.velocity = 127; 
                  triggeredNote[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
                for (l1 = 0; l1 < 2; l1++) {
                  if (mixer_gain[l1] > 0.01) {
                    snd_seq_ev_clear(&ev);
                    snd_seq_ev_set_subs(&ev);
                    snd_seq_ev_set_direct(&ev);
                    ev.type = SND_SEQ_EVENT_NOTEOFF;
                    ev.data.control.channel = channel;   
                    ev.data.note.velocity = 0;
                    ev.data.note.note = triggeredNote[l1][l3];       
                    snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                    snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                  }
                }
              }
            } 
          }
        }
      } else {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
                mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.0) - 1000;
                if (mididata < 0) mididata = 0;
                else if (mididata > 127) mididata = 127;
                if (mididata != lastmididata[l1][l3]) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEOFF;
                  ev.data.control.channel = channel;
                  ev.data.note.note = triggeredNote[l1][l3];
                  ev.data.note.velocity = 0; 
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEON;
                  ev.data.control.channel = channel;
                  ev.data.note.note = mididata;
                  ev.data.note.velocity = 127; 
                  triggeredNote[l1][l3] = mididata;
                  lastmididata[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev); 
                }
              }  
            }
          } 
        }          
      }
      break;
    case 3: 
      if (triggerData != synthdata->zeroModuleData ) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              if (mixer_gain[0] > 0.01) {
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEON;
                ev.data.control.channel = channel;
                mididata = offset[0] + (int)(1000.0 + mixer_gain[0] * inData[0][l3][l2] * 12.0) - 1000;
                velocitydata = offset[1] + (int)(1000.0 + mixer_gain[1] * inData[1][l3][l2] * 12.8) - 1000;
                if (mididata < 0) mididata = 0;
                else if (mididata > 127) mididata = 127;
                if (velocitydata < 0) velocitydata = 0;
                else if (velocitydata > 127) velocitydata = 127;
                ev.data.note.note = mididata;
                ev.data.note.velocity = velocitydata; 
                triggeredNote[0][l3] = mididata;
                snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                snd_seq_event_output_direct(synthdata->seq_handle, &ev);
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
                if (mixer_gain[0] > 0.01) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEOFF;
                  ev.data.control.channel = channel;   
                  ev.data.note.velocity = 0;
                  ev.data.note.note = triggeredNote[0][l3];       
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            }
          }
        } 
      } else {
        if (mixer_gain[0] > 0.01) {
          for (l3 = 0; l3 < synthdata->poly; l3++) {
            for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
              mididata = offset[0] + (int)(1000.0 + mixer_gain[0] * inData[0][l3][l2] * 12.0) - 1000;
              if (mididata < 0) mididata = 0;
              else if (mididata > 127) mididata = 127;
              if (mididata != lastmididata[0][l3]) {
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEOFF;
                ev.data.control.channel = channel;
                ev.data.note.note = triggeredNote[0][l3];
                ev.data.note.velocity = 0;
                snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                snd_seq_ev_clear(&ev);   
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEON;
                ev.data.control.channel = channel;
                velocitydata = offset[1] + (int)(1000.0 + mixer_gain[1] * inData[1][l3][l2] * 12.8) - 1000;
                if (velocitydata < 0) velocitydata = 0; 
                else if (velocitydata > 127) velocitydata = 127;
                ev.data.note.note = mididata;
                ev.data.note.velocity = velocitydata;
                triggeredNote[0][l3] = mididata;
                lastmididata[0][l3] = mididata; 
                lastmididata[1][l3] = velocitydata;
                snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                snd_seq_event_output_direct(synthdata->seq_handle, &ev);
              }
            }
          }      
        }        
      }
      break;
  }
}

void M_midiout::showConfigDialog() {
}
