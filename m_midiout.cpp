#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "synth.h"
#include "m_midiout.h"
#include "port.h"
#include "module.h"

M_midiout::M_midiout(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(0, parent, name, p_synthdata) {

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
  port_in[0] = new Port("In 0", PORT_IN, 0, this, synthdata);
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  port_in[1] = new Port("In 1", PORT_IN, 1, this, synthdata);
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);
  port_M_trigger = new Port("Trigger", PORT_IN, 2, this, synthdata);
  port_M_trigger->move(0, 75);
  port_M_trigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_trigger);
  qs.sprintf("MIDI Out ID %d", moduleID);
  configDialog->setCaption(qs);
/*
  configDialog->initTabWidget();
  QVBox *gainTab = new QVBox(configDialog->tabWidget);
  QVBox *midiTab = new QVBox(configDialog->tabWidget);
  QStrList *channelNames = new QStrList(true);
  for (l1 = 0; l1 < 16; l1++) {
    qs.sprintf("%4d", l1);
    channelNames->append(qs);
  }
*/
  qs = "Midi Settings";
  channel = 0;
  EnumParameter * ep = new EnumParameter( this,"MIDI Channel", "", &channel);
  for (l1 = 1; l1 < 17; l1++) {
    qs.sprintf("%4d", l1);
    ep->addItem(l1,qs);
  }
  ep->addItem(0,"Any");
  configDialog->addParameter(ep,qs);

  ep->selectItem(0);

/*
  QStrList *midiNames = new QStrList(true);
  midiNames->append("In 0/1: Controller");
  midiNames->append("In 0: Controller In 1: Pitchbend");
  midiNames->append("In 0/1: Note");
  midiNames->append("In 0: Note, In 1: Velocity");
  configDialog->addComboBox(midiMode, "MIDI Event Type", &midiMode, midiNames->count(), midiNames, midiTab);
*/
  midiMode=0;
  ep = new EnumParameter( this,"MIDI Event Type", "", &midiMode);
  ep->addItem(0,"In 0/1: Controller");
  ep->addItem(1,"In 0: Controller In 1: Pitchbend");
  ep->addItem(2,"In 0/1: Note");
  ep->addItem(3,"In 0: Note, In 1: Velocity");
  configDialog->addParameter(ep,qs);
  ep->selectItem(0);

  IntParameter * ip = new IntParameter(this,"Controller 0","", 0,127,&controller[0]);
  configDialog->addParameter(ip,qs);
  ip = new IntParameter(this,"Controller 0","", 0,127,&controller[1]);
  configDialog->addParameter(ip,qs);

  qs="Gain / Offset / Trigger Level";
  FloatParameter *fp = new FloatParameter(this,"Gain 0","", 0.0,10.0,&mixer_gain[0]);
  configDialog->addParameter(fp,qs);
  fp = new FloatParameter(this,"Gain 1","", 0.0,10.0,&mixer_gain[1]);
  configDialog->addParameter(fp,qs);
  ip = new IntParameter(this,"Offset 0","", 0,127,&offset[0]);
  configDialog->addParameter(ip,qs);
  ip = new IntParameter(this,"Offset 1","", 0,127,&offset[1]);
  configDialog->addParameter(ip,qs);
  fp = new FloatParameter(this,"Trigger Level","", 0.0,10.0,&triggerLevel);
  configDialog->addParameter(fp,qs);


  for (l1 = 0; l1 < synthdata->poly; l1++) {
    trigger[l1] = false;
    for (l2 = 0; l2 < 2; l2++) {
      triggeredNote[l2][l1] = 0;
      lastmididata[l2][l1] = 0;
    }
  }
}

M_midiout::~M_midiout() {

}

void M_midiout::paintEvent(QPaintEvent *ev) {

  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "MIDI Out");
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_midiout::generateCycle() {

  int l1, l2, l3, mididata, velocitydata;
  snd_seq_event_t ev;

  if (port_M_trigger->connectedPortList.count()) {
    in_M_trigger = (Module *)port_M_trigger->connectedPortList.at(0)->parentModule;
    if (!in_M_trigger->cycleProcessing) {   
      in_M_trigger->generateCycle();
      triggerData = in_M_trigger->data[port_M_trigger->connectedPortList.at(0)->index];
    } else {
      triggerData = in_M_trigger->lastdata[port_M_trigger->connectedPortList.at(0)->index];
    }
  } else {
    in_M_trigger = NULL;
    triggerData = synthdata->zeroModuleData;
  }
  for (l1 = 0; l1 < 2; l1++) {                      
    if (port_in[l1]->connectedPortList.count()) {
      in_M_in[l1] = (Module *)port_in[l1]->connectedPortList.at(0)->parentModule;
      in_M_in[l1]->generateCycle();
      inData[l1] = in_M_in[l1]->data[port_in[l1]->connectedPortList.at(0)->index];
    } else {
      in_M_in[l1] = NULL;
      inData[l1] = synthdata->zeroModuleData;
    }
  }
  switch (midiMode) {
    case 0:
      if (!in_M_trigger) {
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
      if (!in_M_trigger) {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
               if (l1) {
                 mididata = 128.0 * offset[l1] + (int)(mixer_gain[l1] * inData[l1][l3][l2] * 16384.0) - 8192.0;
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
                    mididata = 128.0 * offset[l1] + (int)(mixer_gain[l1] * inData[l1][l3][l2] * 16384.0) - 8192.0;
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
      if (in_M_trigger) {
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
      if (in_M_trigger) {
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
