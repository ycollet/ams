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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_stereomix.h"
#include "port.h"

M_stereomix::M_stereomix(int p_in_channels, QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(2, parent, name, p_synthdata) {

  QString qs;
  QHBox *hbox;
  int l1;

  M_type = M_type_stereomix;
  in_channels = p_in_channels;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_STEREOMIX_WIDTH, 
              MODULE_STEREOMIX_HEIGHT + 40 + 20 * in_channels);
  gain = 1.0;
  configDialog->addSlider(0, 10, gain, "Master Volume", &gain, true);
  ignore_check = false;
  solo_index = -1;
  for (l1 = 0; l1 < in_channels; l1++) {
    qs.sprintf("In %d", l1);
    Port *audio_in_port = new Port(qs, PORT_IN, in_port_list.count(), this, synthdata);
    audio_in_port->move(0, 40 + 20 * in_port_list.count());
    audio_in_port->outTypeAcceptList.append(outType_audio);
    in_port_list.append(audio_in_port);
    portList.append(audio_in_port);
    hbox = configDialog->addHBox();
    mute[l1] = 0.0;
    qs.sprintf("Mute %d", l1);
    configDialog->addCheckBox(mute[l1], qs, &mute[l1], hbox);
    QObject::connect(configDialog->midiCheckBoxList.at(2 * l1)->checkBox, SIGNAL(toggled(bool)),
                     this, SLOT(muteToggled(bool)));
    solo[l1] = 0.0;    
    qs.sprintf("Solo %d", l1);
    configDialog->addCheckBox(solo[l1], qs, &solo[l1], hbox);
    QObject::connect(configDialog->midiCheckBoxList.at(2 * l1 + 1)->checkBox, SIGNAL(toggled(bool)),
                     this, SLOT(soloToggled(bool)));
    mixer_gain[l1] = 1.0;    
    qs.sprintf("Volume %d", l1);
    configDialog->addSlider(0, 2, mixer_gain[l1], qs, &mixer_gain[l1], true, hbox);
    pan[l1] = 0.0;    
    qs.sprintf("Pan %d", l1);
    configDialog->addSlider(-1, 1, pan[l1], qs, &pan[l1], false, hbox);
  }
  for (l1 = 0; l1 < 2; l1++) {
    qs.sprintf("Out %d", l1);
    port_out[l1] = new Port(qs, PORT_OUT, l1, this, synthdata);
    port_out[l1]->move(MODULE_STEREOMIX_WIDTH - port_out[l1]->width(),
                       35 + 20 * (in_channels + l1));
    port_out[l1]->outType = outType_audio;
    portList.append(port_out[l1]);
  }
  qs.sprintf("Stereo Mixer %d ID %d", in_channels, moduleID);
  configDialog->setCaption(qs);
}

M_stereomix::~M_stereomix() {
}

void M_stereomix::generateCycle() {

  int l1, l2, l3;
  float mixgain[2];

  if (!cycleReady) {
    cycleProcessing = true;

    for (l3 = 0; l3 < in_port_list.count(); l3++)
    {
	if (   ((solo_index < 0) || (solo_index == l3))
            && ! configDialog->midiCheckBoxList.at(2 * l3)->getMidiValue())
	{
            inData [l3] = in_port_list.at(l3)->getinputdata();
	}
        else 
	{
            inData[l3] = synthdata->zeroModuleData;
	}
    }         

    mixgain[0] = gain * (1.0 - pan[0]) * mixer_gain[0];
    mixgain[1] = gain * (1.0 + pan[0]) * mixer_gain[0];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = mixgain[0] * inData[0][l1][l2];     
        data[1][l1][l2] = mixgain[1] * inData[0][l1][l2];     
      }
    }  
    for (l3 = 1; l3 < in_port_list.count(); l3++) {
      mixgain[0] = gain * (1.0 - pan[l3]) * mixer_gain[l3];
      mixgain[1] = gain * (1.0 + pan[l3]) * mixer_gain[l3];
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] += mixgain[0] * inData[l3][l1][l2];
          data[1][l1][l2] += mixgain[1] * inData[l3][l1][l2];
        }
      }
    }
    cycleProcessing = false;
    cycleReady = true;
  }
}

void M_stereomix::showConfigDialog() {
}

void M_stereomix::soloToggled(bool) {

  int l1;
  QCheckBox *checkbox;

  if (ignore_check) return;
  checkbox = (QCheckBox *)sender();
  ignore_check = true;
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count() >> 1; l1++) {
    if (configDialog->midiCheckBoxList.at(2 * l1 + 1)->checkBox != checkbox) {
      configDialog->midiCheckBoxList.at(2 * l1 + 1)->updateCheck(false);
    } else {
      if (checkbox->isChecked()) {
        solo_index = l1;
      } else { 
        solo_index = -1;
      }
      configDialog->midiCheckBoxList.at(2 * l1)->updateCheck(false);
    }
  }
  ignore_check = false;
}

void M_stereomix::muteToggled(bool) {

  int l1;
  QCheckBox *checkbox;

  if (ignore_check) return;
  checkbox = (QCheckBox *)sender();
  ignore_check = true;
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count() >> 1; l1++) {
    if ((configDialog->midiCheckBoxList.at(2 * l1)->checkBox == checkbox)
        && configDialog->midiCheckBoxList.at(2 * l1)->checkBox->isChecked() 
        && configDialog->midiCheckBoxList.at(2 * l1 + 1)->checkBox->isChecked()) {
      configDialog->midiCheckBoxList.at(2 * l1 + 1)->updateCheck(false);
      solo_index = -1;
    } 
  }
  ignore_check = false;
}
