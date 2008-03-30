#include <math.h>
#include <iostream>
#include <QTextStream>
#include "synthdata.h"
#include "m_pcmout.h"
#include "midicombobox.h"
#include "midicontrollable.h"


M_pcmout::M_pcmout(QWidget* parent, int port) 
  : Module(M_type_pcmout, 0, parent, "PCM Out")
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_PCMOUT_WIDTH, MODULE_PCMOUT_HEIGHT);
  gain = 0.5;
  ag = 1.0;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;

  polyroot = sqrt((double)synthdata->poly);
  agc = 0;
  qs.sprintf (" -> Out %2d", port);
  port_in[0] = new Port(qs, PORT_IN, 0, this);          
  port_in[0]->move(0, 35);
  portList.append(port_in[0]);
  qs.sprintf(" -> Out %2d", port + 1);
  port_in[1] = new Port(qs, PORT_IN, 1, this);          
  port_in[1]->move(0, 55);
  portList.append(port_in[1]);

  configDialog->addSlider("Gain", gain, 0, 1, false);
  configDialog->addSlider("Volume 1", mixer_gain[0], 0, 1, false);
  configDialog->addSlider("Volume 2", mixer_gain[1], 0, 1, false);
  QStringList agcNames;
  agcNames << "Disabled" << "Per Cycle" << "Keep" << "Reevaluate";
  configDialog->addComboBox("Automatic Gain Control", agc, agcNames);
  if (synthdata->withAlsa) {
    pcmdata[0] = new float[2 * synthdata->periodsize];
    pcmdata[1] = pcmdata[0] + synthdata->periodsize;
  }
}

void M_pcmout::mcAbleChanged(MidiControllableBase *mcAble)
{
  if (configDialog->midiComboBoxList.count() < 1 ||
      mcAble != &configDialog->midiComboBoxList.at(0)->mcAble)
    return;
 
  if (ag == ag_displayed)
    return;

  QString i3Name;
  if (ag == 1.0)
    i3Name = "Keep";
  else
    QTextStream(&i3Name) << "K. " << ag;

  for (typeof(mcAble->mcws.constBegin()) mcw = mcAble->mcws.constBegin();
       mcw != mcAble->mcws.constEnd();  mcw++) {
    MidiComboBox *b = dynamic_cast<MidiComboBox *>(*mcw);
    if (b) {
      b->comboBox->setItemText(2, i3Name);
      b->comboBox->update();
    }
  }

  ag_displayed = ag;
}

M_pcmout::~M_pcmout()
{
  if (synthdata->withAlsa)
    delete[] pcmdata[0];
}

void M_pcmout::generateCycle()
{
  int l1, l2, l3;
  float max = 0.0, **indata;
  char pipeMessage = 0;

  if (agc == 3) {
    agc = 2;
    ag = 1.0;
    pipeMessage = 1;
  }

  for (l1 = 0; l1 < 2; l1++) {
    indata = port_in [l1]->getinputdata ();
    if (indata == synthdata->zeroModuleData)
      continue;

    float mixgain = gain * mixer_gain[l1];
    if (agc == 2)
      mixgain *= ag;

    int poly = synthdata->poly;
    if (poly > 1 && indata[1] == indata[0])
      poly = 1;
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      pcmdata[l1][l2] = indata[0][l2];
      for (l3 = 1; l3 < poly; l3++)
	pcmdata[l1][l2] += indata[l3][l2];

      pcmdata[l1][l2] *= mixgain;
      if (agc && max < fabs(pcmdata[l1][l2]))
	  max = fabs(pcmdata[l1][l2]);
    }
  }
  if (max > 0.999f) {
    max = 0.999f / max;
    for (l1 = 0; l1 < 2; l1++)
      for (l2 = 0; l2 < synthdata->cyclesize; l2++)
	pcmdata[l1][l2] *= max;

    if (agc == 2) {
      ag *= max;
      pipeMessage = 1;
    }
  }
  if (pipeMessage) {
    synthdata->mcSet.put(&configDialog->midiComboBoxList.at(0)->mcAble);
    synthdata->pipeMessage |= pipeMessage;
  }
}
