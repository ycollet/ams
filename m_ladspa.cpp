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
#include <ladspa.h>
#include "synthdata.h"
#include "m_ladspa.h"
#include "port.h"

M_ladspa::M_ladspa(QWidget* parent, const char *name, SynthData *p_synthdata, int p_ladspaDesFuncIndex, int p_n, bool poly) 
              : Module(MAX_AUDIOPORTS, parent, name, p_synthdata) {

  QString qs;
  int l1, l2, itmp, port_ofs;
  int audio_in_index, audio_out_index, control_port_count;
  float control_min, control_max;
  
  
  M_type = M_type_ladspa;
  ladspaDesFuncIndex = p_ladspaDesFuncIndex;
  n = p_n;
  isPoly = poly;
//  fprintf(stderr, "new LADSPA module, Poly: %d\n", (int)isPoly);
  rate_factor = 1.0;
  ladspa_dsc = synthdata->ladspa_dsc_func_list[ladspaDesFuncIndex](n);
  ladspa_audio_in_count = 0;
  ladspa_audio_out_count = 0;
  for (l1 = 0; l1 < ladspa_dsc->PortCount; l1++) {
    if (LADSPA_IS_PORT_AUDIO(ladspa_dsc->PortDescriptors[l1])) {
      if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
        ladspa_audio_in_count++;
      }
      if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
        ladspa_audio_out_count++;
      }
    }
  }
  if (ladspa_audio_in_count > MAX_AUDIOPORTS) ladspa_audio_in_count = MAX_AUDIOPORTS;
  if (ladspa_audio_out_count > MAX_AUDIOPORTS) ladspa_audio_out_count = MAX_AUDIOPORTS;
  if (isPoly) {
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < ladspa_audio_in_count; l2++) {
        ladspaDataIn[l2][l1] = (LADSPA_Data *)malloc(synthdata->periodsize * sizeof(LADSPA_Data));
        memset(ladspaDataIn[l2][l1], 0, synthdata->periodsize * sizeof(LADSPA_Data));
      }
      for (l2 = 0; l2 < ladspa_audio_out_count; l2++) {
        ladspaDataOut[l2][l1] = (LADSPA_Data *)malloc(synthdata->periodsize * sizeof(LADSPA_Data));
        memset(ladspaDataOut[l2][l1], 0, synthdata->periodsize * sizeof(LADSPA_Data));
      }
      ladspa_handle[l1] = ladspa_dsc->instantiate(ladspa_dsc, synthdata->rate);
      if (ladspa_dsc->activate) {
        ladspa_dsc->activate(ladspa_handle[l1]);
      }
    }
  } else {
    for (l2 = 0; l2 < ladspa_audio_in_count; l2++) {
      ladspaDataIn[l2][0] = (LADSPA_Data *)malloc(synthdata->periodsize * sizeof(LADSPA_Data));
      memset(ladspaDataIn[l2][0], 0, synthdata->periodsize * sizeof(LADSPA_Data));
    }
    for (l2 = 0; l2 < ladspa_audio_out_count; l2++) {
      ladspaDataOut[l2][0] = (LADSPA_Data *)malloc(synthdata->periodsize * sizeof(LADSPA_Data));
      memset(ladspaDataOut[l2][0], 0, synthdata->periodsize * sizeof(LADSPA_Data));
      ladspa_handle[0] = ladspa_dsc->instantiate(ladspa_dsc, synthdata->rate);
      if (ladspa_dsc->activate) {
        ladspa_dsc->activate(ladspa_handle[0]);
      }
    }
  }
  configDialog->addLabel(QString("Label: ")+QString(ladspa_dsc->Label));
  configDialog->addLabel(QString("Author: ")+QString(ladspa_dsc->Maker));
  configDialog->addLabel(QString("Copyright: ")+QString(ladspa_dsc->Copyright));
  port_ofs = 35;
  audio_in_index = 0;
  audio_out_index = 0;
  control_port_count = 0;
  for (l1 = 0; l1 < ladspa_dsc->PortCount; l1++) {
    if (LADSPA_IS_PORT_AUDIO(ladspa_dsc->PortDescriptors[l1])) {
      if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
        Port *audio_in_port = new Port(ladspa_dsc->PortNames[l1], PORT_IN, in_port_list.count(), this, synthdata);
        audio_in_port->move(0, port_ofs + 20 * in_port_list.count());
        audio_in_port->outTypeAcceptList.append(outType_audio);
        in_port_list.append(audio_in_port);
        portList.append(audio_in_port);
        if (isPoly) {
          for (l2 = 0; l2 < synthdata->poly; l2++) {
            ladspa_dsc->connect_port(ladspa_handle[l2], l1, ladspaDataIn[audio_in_index][l2]);
           }
        } else {
          ladspa_dsc->connect_port(ladspa_handle[0], l1, ladspaDataIn[audio_in_index][0]);
        }
        audio_in_index++;
      }
      if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
        Port *audio_out_port = new Port(ladspa_dsc->PortNames[l1], PORT_OUT, out_port_list.count(), this, synthdata);
        audio_out_port->move(MODULE_LADSPA_WIDTH - audio_out_port->width(), 
                             port_ofs + 20 * out_port_list.count());
        audio_out_port->outType = outType_audio;
        out_port_list.append(audio_out_port);
        portList.append(audio_out_port);
        if (isPoly) {
          for (l2 = 0; l2 < synthdata->poly; l2++) {
            ladspa_dsc->connect_port(ladspa_handle[l2], l1, ladspaDataOut[audio_out_index][l2]);
          }
        } else {
          ladspa_dsc->connect_port(ladspa_handle[0], l1, ladspaDataOut[audio_out_index][0]);
        }
        audio_out_index++;
      }
    }
    if (LADSPA_IS_PORT_CONTROL(ladspa_dsc->PortDescriptors[l1])) {
      if (LADSPA_IS_HINT_TOGGLED(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
          configDialog->addCheckBox(0, ladspa_dsc->PortNames[l1], &control_data[control_port_count]);
        if (isPoly) {
          for (l2 = 0; l2 < synthdata->poly; l2++) {
            ladspa_dsc->connect_port(ladspa_handle[l2], l1, &control_data[control_port_count]);
          }
        } else {
          ladspa_dsc->connect_port(ladspa_handle[0], l1, &control_data[control_port_count]);
        }
        control_port_count++;
      } else {
        control_min = 0;
        control_max = 1;
        if (LADSPA_IS_HINT_BOUNDED_BELOW(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
          control_min = ladspa_dsc->PortRangeHints[l1].LowerBound;
        }
        if (LADSPA_IS_HINT_BOUNDED_ABOVE(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
          control_max = ladspa_dsc->PortRangeHints[l1].UpperBound;
        }
        if (LADSPA_IS_HINT_SAMPLE_RATE(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) { // TODO implement this
          rate_factor = synthdata->rate;
        } else {
          rate_factor = 1.0;
        }
        if (LADSPA_IS_HINT_LOGARITHMIC(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) { // TODO implement this
        }
        control_data[control_port_count] = control_min * rate_factor;
        if (LADSPA_IS_HINT_LOGARITHMIC(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) { 
          configDialog->addSlider(control_min * rate_factor, control_max * rate_factor, control_min * rate_factor, 
                                  ladspa_dsc->PortNames[l1], &control_data[control_port_count], true);
        } else {
          configDialog->addSlider(control_min * rate_factor, control_max * rate_factor, control_min * rate_factor,
                                  ladspa_dsc->PortNames[l1], &control_data[control_port_count], false);
        }
        if (isPoly) {
          for (l2 = 0; l2 < synthdata->poly; l2++) {
            ladspa_dsc->connect_port(ladspa_handle[l2], l1, &control_data[control_port_count]);
          }
        } else {
          ladspa_dsc->connect_port(ladspa_handle[0], l1, &control_data[control_port_count]);
        }  
        control_port_count++;
      }
    }
  }
  itmp = (out_port_list.count() > in_port_list.count()) ? out_port_list.count() : in_port_list.count();
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LADSPA_WIDTH, 
              MODULE_LADSPA_HEIGHT + 20 * itmp);
  pluginName.sprintf("%s", ladspa_dsc->Name);
  if (isPoly) {
    qs.sprintf("%s ID %d Poly", ladspa_dsc->Name, moduleID);
  } else {
    qs.sprintf("%s ID %d", ladspa_dsc->Name, moduleID);
  }
  configDialog->setCaption(qs);
}

M_ladspa::~M_ladspa() {

  int l1, l2;

  if (isPoly) {
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < ladspa_audio_in_count; l2++) {     
        free(ladspaDataIn[l2][l1]); 
      }
      for (l2 = 0; l2 < ladspa_audio_out_count; l2++) {
        free(ladspaDataOut[l2][l1]);
      }
      if (ladspa_dsc->deactivate) {
        ladspa_dsc->deactivate(ladspa_handle[l1]);
      }
      ladspa_dsc->cleanup(ladspa_handle[l1]);
    }
  } else {
    for (l2 = 0; l2 < ladspa_audio_in_count; l2++) {
      free(ladspaDataIn[l2][0]);
    }
    for (l2 = 0; l2 < ladspa_audio_out_count; l2++) {
      free(ladspaDataOut[l2][0]);
    }
    if (ladspa_dsc->deactivate) {
      ladspa_dsc->deactivate(ladspa_handle[0]);
    }
    ladspa_dsc->cleanup(ladspa_handle[0]);
  }
}

void M_ladspa::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  int l1, qsi;
  QString qs;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(17, 20, "LADSPA");
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("%s ID %d", synthdata->ladspa_dsc_func_list[ladspaDesFuncIndex](n)->Name, moduleID);
  qsi = qs.find(" ");
  qsi = qs.find(" ", qsi+1);
  if (qsi > 22) qsi = 22;
  p.drawText(17, 32, qs.left(qsi));
}

void M_ladspa::generateCycle() {

  int l1, l2, l3;

  if (!cycleReady) {
    cycleProcessing = true;
    for (l3 = 0; l3 < out_port_list.count(); l3++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[l3][l1], data[l3][l1], synthdata->cyclesize * sizeof(float));
      }
    }
    for (l3 = 0; l3 < in_port_list.count(); l3++) {
      if (in_port_list.at(l3)->connectedPortList.count()) {
        in_M[l3] = (Module *)in_port_list.at(l3)->connectedPortList.at(0)->parentModule;
        if (!in_M[l3]->cycleProcessing) {
          in_M[l3]->generateCycle();
          inData[l3] = in_M[l3]->data[in_port_list.at(l3)->connectedPortList.at(0)->index]; 
        } else {
          inData[l3] = in_M[l3]->lastdata[in_port_list.at(l3)->connectedPortList.at(0)->index];
        }
      } else {
        in_M[l3] = NULL;
        inData[l3] = synthdata->zeroModuleData;
      }
    }
    if (isPoly) {
      for (l3 = 0; l3 < in_port_list.count(); l3++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            ladspaDataIn[l3][l1][l2] = inData[l3][l1][l2];
          }
        }
      }
    } else {
      for (l3 = 0; l3 < in_port_list.count(); l3++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          ladspaDataIn[l3][0][l2] = inData[l3][0][l2];
        }
        for (l1 = 1; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            ladspaDataIn[l3][0][l2] += inData[l3][l1][l2];
          }
        }
      }
    }
    if (isPoly) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        ladspa_dsc->run(ladspa_handle[l1], synthdata->cyclesize);
        for (l3 = 0; l3 < out_port_list.count(); l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[l3][l1][l2] = ladspaDataOut[l3][l1][l2];
          }
        }
      }
    } else {
      ladspa_dsc->run(ladspa_handle[0], synthdata->cyclesize);
      for (l3 = 0; l3 < out_port_list.count(); l3++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[l3][l1][l2] = ladspaDataOut[l3][0][l2];
          }
        }
      }
    }
  }  
  cycleProcessing = false;
  cycleReady = true;
}

void M_ladspa::showConfigDialog() {
}
