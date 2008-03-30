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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midicontrollable.h"
#include "midicombobox.h"
#include "m_ad.h"
#include "midipushbutton.h"
#include "port.h"

M_ad::M_ad(int outCount, QWidget* parent) 
  : Module(M_type_ad, outCount, parent,
	   QString("Analog Driver ") + QString::number(outCount) + " Out")
{
  QString qs;
  int l1, l2;
  QHBoxLayout *tuneBox, *detuneBox[2], *driftBox[2];

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_AD_WIDTH, 
              MODULE_AD_HEIGHT + 20 + 20 * outCount);
  qs.sprintf("CV In");
  port_in = new Port(qs, PORT_IN, 0, this);
  port_in->move(0, 40);
  portList.append(port_in);  for (l1 = 0; l1 < outCount; l1++) {
    qs.sprintf("CV Out %d", l1);
    port_out[l1] = new Port(qs, PORT_OUT, l1, this);
    port_out[l1]->move(MODULE_AD_WIDTH - port_out[l1]->width(), 
                       55 + 20 * l1);
    port_out[l1]->outType = outType_audio;
    portList.append(port_out[l1]);
    for (l2 = 0; l2 < MAXPOLY; l2++) {
      drift_a[l1][l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
      drift_c[l1][l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
    }
  }
  for (l2 = 0; l2 < MAXPOLY; l2++) {
    detune_a[l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
    detune_c[l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
  }
  detune_amp = 0.005;
  detune_mod = 0.01;
  detune_rate = 1;
  drift_amp = 0.005;
  drift_mod = 0.01;
  drift_rate = 3;
  detuneCount = 0;
  driftCount = 0;
  bypass = 0;
  configDialog->initTabWidget();
  QVBoxLayout *paramTab = configDialog->addVBoxTab("Parameter");

  configDialog->addSlider("Detune Amplitude", detune_amp, 0, 0.084, true, paramTab);
  configDialog->addSlider("Detune Modulation", detune_mod, 0.01, 1, true, paramTab);
  configDialog->addSlider("Detune Rate", detune_rate, 0.01, 10, true, paramTab);
  configDialog->addSlider("Drift Amplitude", drift_amp, 0, 0.084, true, paramTab);
  configDialog->addSlider("Drift Modulation", drift_mod, 0.01, 1, true, paramTab);
  configDialog->addSlider("Drift Rate", drift_rate, 0.01, 10, true, paramTab);  

  QVBoxLayout *displayTab = configDialog->addVBoxTab("Display");
  for (l1 = 0 ; l1 < 2; l1++) {
    voice[l1] = (synthdata->poly > 1) ? l1 : 0;
    QStringList voiceNames;
    for (l2 = 0; l2 < synthdata->poly; l2++) {
      qs.sprintf("%d", l2);
      voiceNames << qs;
    }
    configDialog->addComboBox("Voice", voice[l1], voiceNames, displayTab);
    QObject::connect(configDialog->midiComboBoxList.at(l1)->comboBox, 
                     SIGNAL(highlighted(int)), this, SLOT(updateVoices(int)));
    qs.sprintf("Detune %d", l1);
    configDialog->addLabel(qs, displayTab);                                    
    detuneBox[l1] = configDialog->addHBox(displayTab);                          
    configDialog->addLabel(" 0.000 ", detuneBox[l1]);
    qs.sprintf("Drift %d", l1);
    configDialog->addLabel(qs, displayTab);
    driftBox[l1] = configDialog->addHBox(displayTab);
    for (l2 = 0; l2 < outCount; l2++) {
      configDialog->addLabel(" 0.000 ", driftBox[l1]);
    }     
  }  

  tuneBox = configDialog->addHBox();  
  MidiControllableDoOnce * do0 = configDialog->addPushButton("Autotune", tuneBox);
  QObject::connect(do0, SIGNAL(triggered()), this, SLOT(autoTune()));
  configDialog->addCheckBox("Bypass", bypass, tuneBox);                       

  timer = new QTimer(this);   
  QObject::connect(timer, SIGNAL(timeout()),
                   this, SLOT(timerProc()));
  timer->start(1000);
}

void M_ad::generateCycle() {

  int l1, l2, l3, l4, l5;
  float dta, dra, rdt, rdr;
  double qdt, qdr;
  

    inData = port_in->getinputdata();
 
    dta = detune_amp;
    dra = drift_amp; 
    rdt = detune_mod / (float)synthdata->rate;
    rdr = drift_mod / (float)synthdata->rate;
    qdt = (double)outPortCount * (double)synthdata->poly * (double)synthdata->rate / (detune_rate + 1e-3);
    qdr = (double)outPortCount * (double)synthdata->poly * (double)synthdata->rate / (detune_rate + 1e-3);
    if (bypass) {
      for (l3 = 0; l3 < outPortCount; l3++) {   
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[l3][l1][l2] = inData[l1][l2];     
          }
        }  
      }
    } else {
      for (l3 = 0; l3 < outPortCount; l3++) {   
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[l3][l1][l2] = inData[l1][l2] 
                             + dta * detune_a[l1]
                             + dra * drift_a[l3][l1];     
            detune_a[l1] += rdt * detune_c[l1];
            if (detune_a[l1] > 1.0) { 
              detune_a[l1] = 1.0;
              detune_c[l1] = -1.0;
            } else if (detune_a[l1] < -1.0) { 
              detune_a[l1] = -1.0;
              detune_c[l1] = 1.0;
            }
            if (drift_a[l3][l1] > 1.0) { 
              drift_a[l3][l1] = 1.0;
              drift_c[l3][l1] = -1.0;
            } else if (drift_a[l3][l1] < -1.0) { 
              drift_a[l3][l1] = -1.0;
              drift_c[l3][l1] = 1.0;
            }
            drift_a[l3][l1] += rdr * drift_c[l3][l1];
            detuneCount++;
            driftCount++;
            if (detuneCount > qdt) {
              detuneCount = 0;
              for (l4 = 0; l4 < synthdata->poly; l4++) {
                detune_c[l4] = 2.0 * (double)random() / (double)RAND_MAX - 1.0;
              }  
//              fprintf(stderr, "%5.3f\n", detune_c[0]);
            }
            if (driftCount > qdr) {
              driftCount = 0;
              for (l5 = 0; l5 < outPortCount; l5++) {
                for (l4 = 0; l4 < synthdata->poly; l4++) { 
                  drift_c[l5][l4] = 2.0 * (double)random() / (double)RAND_MAX - 1.0;
                }
              }    
            }
          }
        }
      }
    }
}

void M_ad::updateVoices(int) {
  
  QString qs;
  
  qs.sprintf("Detune %d", voice[0]);
  configDialog->labelList.at(0)->setText(qs);
  qs.sprintf("Drift %d", voice[0]);
  configDialog->labelList.at(2)->setText(qs);
  qs.sprintf("Detune %d", voice[1]);
  configDialog->labelList.at(3 + outPortCount)->setText(qs);
  qs.sprintf("Drift %d", voice[1]);
  configDialog->labelList.at(5 + outPortCount)->setText(qs);
}

void M_ad::timerProc() {

  QString qs;
  int l1;
  
  qs.sprintf(" %2.3f", detune_a[voice[0]]);
  configDialog->labelList.at(1)->setText(qs);
  for (l1 = 0; l1 < outPortCount; l1++) { 
    qs.sprintf(" %2.3f", drift_a[l1][voice[0]]);
    configDialog->labelList.at(3 + l1)->setText(qs);
  }
  qs.sprintf(" %2.3f", detune_a[voice[1]]);
  configDialog->labelList.at(4 + outPortCount)->setText(qs);
  for (l1 = 0; l1 < outPortCount; l1++) { 
    qs.sprintf(" %2.3f", drift_a[l1][voice[1]]);
    configDialog->labelList.at(6 + outPortCount + l1)->setText(qs);
  }
}

void M_ad::autoTune() {

  int l1, l2;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    detune_a[l1] = 0;
    for (l2 = 0; l2 < outPortCount; l2++) {
      drift_a[l2][l1] = 0;
    }                  
  }    
}                        
              
