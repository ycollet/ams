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
#include "m_advenv.h"
#include "port.h"

M_advenv::M_advenv(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(2, parent, name, p_synthdata) {

  QString qs;
  int l1;

  M_type = M_type_advenv;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ADVENV_WIDTH, MODULE_ADVENV_HEIGHT);
  attack[0] = 0;
  attack[1] = 0.05;
  attack[2] = 0.5;
  attack[3] = 0.05;
  attack[4] = 1.0;
  attack[5] = 0.05;
  attack[6] = 0.9;
  attack[7] = 0.1;
  sustain = 0.7;
  release[0] = 0.05;
  release[1] = 0.5;
  release[2] = 0.05;
  release[3] = 0.2;
  release[4] = 0.05;
  timeScale = 1.0;
  port_gate = new Port("Gate", PORT_IN, 0, this, synthdata);
  port_gate->move(0, 35);
  port_gate->outTypeAcceptList.append(outType_audio);
  portList.append(port_gate);
  port_retrigger = new Port("Retrigger", PORT_IN, 1, this, synthdata);
  port_retrigger->move(0, 55);
  port_retrigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_retrigger);
  port_gain_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_gain_out->move(width() - port_gain_out->width(), 75);
  port_gain_out->outType = outType_audio;
  portList.append(port_gain_out);
  port_inverse_out = new Port("Inverse Out", PORT_OUT, 1, this, synthdata);          
  port_inverse_out->move(width() - port_inverse_out->width(), 95);
  port_inverse_out->outType = outType_audio;
  portList.append(port_inverse_out);
  qs.sprintf("Advanced ENV ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->addMultiEnvelope(1, &timeScale, &attack[0], &sustain, &release[0]);
  configDialog->initTabWidget();
  QVBox *sustainTab = new QVBox(configDialog->tabWidget);
  QVBox *attackTimeTab = new QVBox(configDialog->tabWidget);
  QVBox *attackLevelTab = new QVBox(configDialog->tabWidget);
  QVBox *releaseTimeTab = new QVBox(configDialog->tabWidget);
  QVBox *releaseLevelTab = new QVBox(configDialog->tabWidget);
  qs.sprintf("Time Scale");
  configDialog->addSlider(0.1, 10, timeScale, qs, &timeScale, false, sustainTab);
  qs.sprintf("Sustain");
  configDialog->addSlider(0, 1, sustain, qs, &sustain, false, sustainTab);
  qs.sprintf("Delay");
  configDialog->addSlider(0, 1, attack[0], qs, &attack[0], false, sustainTab);
  qs.sprintf("Attack Time 0");
  configDialog->addSlider(0, 1, attack[1], qs, &attack[1], false, attackTimeTab);
  qs.sprintf("Attack Level 0");
  configDialog->addSlider(0, 1, attack[2], qs, &attack[2], false, attackLevelTab);
  qs.sprintf("Attack Time 1");
  configDialog->addSlider(0, 1, attack[3], qs, &attack[3], false, attackTimeTab);
  qs.sprintf("Attack Level 1");
  configDialog->addSlider(0, 1, attack[4], qs, &attack[4], false, attackLevelTab);
  qs.sprintf("Attack Time 2");
  configDialog->addSlider(0, 1, attack[5], qs, &attack[5], false, attackTimeTab);
  qs.sprintf("Attack Level 2");
  configDialog->addSlider(0, 1, attack[6], qs, &attack[6], false, attackLevelTab);
  qs.sprintf("Attack Time 3");
  configDialog->addSlider(0, 1, attack[7], qs, &attack[7], false, attackTimeTab);
  configDialog->addTab(sustainTab, "Time Scale / Sustain / Delay");
  configDialog->addTab(attackTimeTab, "Attack Time");
  configDialog->addTab(attackLevelTab, "Attack Level");
  qs.sprintf("Release Time 0");
  configDialog->addSlider(0, 1, release[0], qs, &release[0], false, releaseTimeTab);
  qs.sprintf("Release Level 0");
  configDialog->addSlider(0, 1, release[1], qs, &release[1], false, releaseLevelTab);
  qs.sprintf("Release Time 1");
  configDialog->addSlider(0, 1, release[2], qs, &release[2], false, releaseTimeTab);
  qs.sprintf("Release Level 1");
  configDialog->addSlider(0, 1, release[3], qs, &release[3], false, releaseLevelTab);
  qs.sprintf("Release Time 2");
  configDialog->addSlider(0, 1, release[4], qs, &release[4], false, releaseTimeTab);
  configDialog->addTab(releaseTimeTab, "Release Time");
  configDialog->addTab(releaseLevelTab, "Release Level");
  for (l1 = 0; l1 < configDialog->midiSliderList.count(); l1++) {
    QObject::connect(configDialog->midiSliderList.at(l1), SIGNAL(valueChanged(int)), 
                     configDialog->multiEnvelopeList.at(0), SLOT(updateMultiEnvelope(int)));
  }
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    noteActive[l1] = false;
    gate[l1] = false;
    retrigger[l1] = false;
    noteOnOfs[l1] = 0;
    noteOffOfs[l1] = 0;
    e[l1] = 0;
    de[l1] = 0;
  }
}

M_advenv::~M_advenv() {

}

void M_advenv::generateCycle() {

  int l1, l2, status;
  float tscale, de_a[4], de_d[3];
  float t[8];

  if (!cycleReady) {
    cycleProcessing = true;

    gateData = port_gate->getinputdata();
    retriggerData = port_retrigger->getinputdata();

    tscale = timeScale * synthdata->rate;
    de_a[0] = (attack[1] > 0) ? attack[2] / (tscale * attack[1]) : 0;
    de_a[1] = (attack[3] > 0) ? (attack[4] - attack[2]) / (tscale * attack[3]) : 0;
    de_a[2] = (attack[5] > 0) ? (attack[6] - attack[4]) / (tscale * attack[5]) : 0;
    de_a[3] = (attack[7] > 0) ? (sustain - attack[6]) / (tscale * attack[7]) : 0;
    de_d[0] = (release[0] > 0) ? (release[1] - sustain) / (tscale * release[0]) : 0;
    de_d[1] = (release[2] > 0) ? (release[3] - release[1]) / (tscale * release[2]) : 0;
    de_d[2] = (release[4] > 0) ? - release[3] / (tscale * release[4]) : 0;
    t[0] = tscale * attack[0];
    t[1] = t[0] + tscale * attack[1];
    t[2] = t[1] + tscale * attack[3];
    t[3] = t[2] + tscale * attack[5];
    t[4] = t[3] + tscale * attack[7];
    t[5] = tscale * release[0];
    t[6] = t[5] + tscale * release[2];
    t[7] = t[6] + tscale * release[4];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!gate[l1] && (gateData[l1][l2] > 0.5)) {
          gate[l1] = true;
          noteActive[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ADVENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ADVENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (gate[l1] && (gateData[l1][l2] < 0.5)) {
          gate[l1] = false;
          noteOffOfs[l1] = 0;
          e_noteOff[l1] = e[l1];
          de_release[l1] = (release[0] > 0) ? (release[1] - e_noteOff[l1]) / (tscale * release[0]) : 0;
  //        fprintf(stderr, "de_release[%d]: %f, e_noteOff: %f\n", l1, de_release[l1], e_noteOff[l1]);
        }
        if (!retrigger[l1] && (retriggerData[l1][l2] > 0.5)) { 
          retrigger[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ADVENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ADVENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (retrigger[l1] && (retriggerData[l1][l2] < 0.5)) {
          retrigger[l1] = false;
        }
        if (gate[l1]) {
            status = 1;
            if (noteOnOfs[l1] < 0) status = 0;
            if (noteOnOfs[l1] >= long(t[0])) status = 2; 
            if (noteOnOfs[l1] >= long(t[1])) status = 3;
            if (noteOnOfs[l1] >= long(t[2])) status = 4;
            if (noteOnOfs[l1] >= long(t[3])) status = 5;
            if (noteOnOfs[l1] >= long(t[4])) status = 6;
            switch (status) {
              case 0: e[l1] -= de[l1];
                      break;
              case 1: e[l1] = 0;
                      break;
              case 2: e[l1] += de_a[0];
                      break;
              case 3: e[l1] += de_a[1];
                      break;
              case 4: e[l1] += de_a[2];
                      break;
              case 5: e[l1] += de_a[3];
                      break;
              case 6: e[l1] = sustain;
                      break;
              default: e[l1] = 0;
                       break;
            }
            if (e[l1] < 0) e[l1] = 0;
            data[0][l1][l2] = e[l1];
            data[1][l1][l2] = -e[l1];
            noteOnOfs[l1]++;
        } else {                          // Release      
          if (noteActive[l1]) {
            status = 1;
            if (noteOffOfs[l1] < 0) status = 0;
            if (noteOffOfs[l1] >= long(t[5])) status = 2;
            if (noteOffOfs[l1] >= long(t[6])) status = 3;
            if (noteOffOfs[l1] >= long(t[7])) status = 4;
            switch (status) {
              case 0: e[l1] = 0;
                      break;
              case 1: e[l1] += de_release[l1];
                      break;
              case 2: e[l1] += de_d[1];
                      break;
              case 3: e[l1] += de_d[2];
                      break;
              case 4: e[l1] = 0;
                      break;
              default: e[l1] = 0;
                     break;
            }
            if (e[l1] < 0) {
//              fprintf(stderr, "status: %d e[%d] < 0: %f\n", status, l1, e[l1]);
              e[l1] = 0;
            }
            noteOffOfs[l1]++;
            if (noteOffOfs[l1] >= int(t[7])) {
//              fprintf(stderr, "noteOffOfs[%d] = %d >= t[7] = %f; e[%d] = %f\n", l1, noteOffOfs[l1], t[7], l1, e[l1]);
              noteActive[l1] = false;
            }
          }
          data[0][l1][l2] = e[l1];
          data[1][l1][l2] = -e[l1];
        }
      }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_advenv::showConfigDialog() {
}
