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

M_ladspa::M_ladspa(QWidget* parent, const char *name, SynthData *p_synthdata, int p_ladspaDesFuncIndex, int p_n, bool poly, bool extCtrlPorts) 
              : Module(MAX_OUTPORTS, parent, name, p_synthdata) {

  QString qs;
  int l1, l2, itmp, port_ofs;
  int audio_in_index, audio_out_index, ctrl_in_index, ctrl_out_index, control_port_count;
  float control_min, control_max;
  
  
  M_type = M_type_ladspa;
  ladspaDesFuncIndex = p_ladspaDesFuncIndex;
  n = p_n;
  isPoly = poly;
  hasExtCtrlPorts = extCtrlPorts;
//  fprintf(stderr, "new LADSPA module, Poly: %d\n", (int)isPoly);
  rate_factor = 1.0;
  ladspa_dsc = synthdata->ladspa_dsc_func_list[ladspaDesFuncIndex](n);
  ladspa_audio_in_count = 0;
  ladspa_audio_out_count = 0;
  ladspa_ctrl_in_count = 0;
  ladspa_ctrl_out_count = 0;
  ctrl_in_index = 0;
  ctrl_out_index = 0;
  for (l1 = 0; l1 < ladspa_dsc->PortCount; l1++) {
    if (LADSPA_IS_PORT_AUDIO(ladspa_dsc->PortDescriptors[l1])) {
      if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
        ladspa_audio_in_count++;
      }
      if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
        ladspa_audio_out_count++;
      }
    }
    if (LADSPA_IS_PORT_CONTROL(ladspa_dsc->PortDescriptors[l1])) {
      if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
        ladspa_ctrl_in_count++;
      }
      if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
        ladspa_ctrl_out_count++;
      }
    }
  }
  if (ladspa_audio_in_count > MAX_AUDIOPORTS) ladspa_audio_in_count = MAX_AUDIOPORTS;
  if (ladspa_audio_out_count > MAX_AUDIOPORTS) ladspa_audio_out_count = MAX_AUDIOPORTS;
  if (ladspa_ctrl_in_count > MAX_CONTROLPORTS) ladspa_ctrl_in_count = MAX_CONTROLPORTS;
  if (ladspa_ctrl_out_count > MAX_CONTROLPORTS) ladspa_ctrl_out_count = MAX_CONTROLPORTS;
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
    }
    if (ladspa_dsc->activate) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
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
  configDialog->addLabel(QString("Name: ")+QString(ladspa_dsc->Name));
  configDialog->addLabel(QString("Author: ")+QString(ladspa_dsc->Maker));
  configDialog->addLabel(QString("Copyright: ")+QString(ladspa_dsc->Copyright));
  port_ofs = 35;
  audio_in_index = 0;
  audio_out_index = 0;
  control_port_count = 0;
  for (l1 = 0; l1 < ladspa_dsc->PortCount; l1++) {
    if (LADSPA_IS_PORT_AUDIO(ladspa_dsc->PortDescriptors[l1])) {
      if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
        Port *audio_in_port = new Port(ladspa_dsc->PortNames[l1], PORT_IN, in_port_list.count() + in_ctrl_port_list.count(), this, synthdata, 90);
        audio_in_port->move(0, port_ofs + 20 * (in_port_list.count() + in_ctrl_port_list.count()));
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
        Port *audio_out_port = new Port(ladspa_dsc->PortNames[l1], PORT_OUT, out_port_list.count() + out_ctrl_port_list.count(), this, synthdata, 90);
        audio_out_port->move(MODULE_LADSPA_WIDTH - audio_out_port->width(), 
                             port_ofs + 20 * (out_port_list.count() + out_ctrl_port_list.count()));
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
        audio_index[audio_out_index] = out_port_list.count() + out_ctrl_port_list.count() - 1;
//        fprintf(stderr, "audio_index[audio_out_index] = audio_index[%d] = %d\n", audio_out_index, audio_index[audio_out_index]);
        audio_out_index++;
      }
    }
    if (LADSPA_IS_PORT_CONTROL(ladspa_dsc->PortDescriptors[l1])) {
      controlPortRate[control_port_count] = false;
      if (hasExtCtrlPorts) {
        if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
          Port *ctrl_in_port = new Port(ladspa_dsc->PortNames[l1], PORT_IN, in_port_list.count() + in_ctrl_port_list.count(), this, synthdata, 160, 1);
          ctrl_in_port->move(0, port_ofs + 20 * (in_port_list.count() + in_ctrl_port_list.count()));
          ctrl_in_port->outTypeAcceptList.append(outType_audio);
          in_ctrl_port_list.append(ctrl_in_port);
          portList.append(ctrl_in_port);
//          fprintf(stderr, "input: %s\n", ladspa_dsc->PortNames[l1]);
        }
        if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
          Port *ctrl_out_port = new Port(ladspa_dsc->PortNames[l1], PORT_OUT, out_port_list.count() + out_ctrl_port_list.count(), this, synthdata, 110, 1);
          ctrl_out_port->move(MODULE_LADSPA_WIDTH - ctrl_out_port->width(),
                              port_ofs + 20 * (out_port_list.count() + out_ctrl_port_list.count()));
          ctrl_out_port->outType = outType_audio;
          out_ctrl_port_list.append(ctrl_out_port);
          portList.append(ctrl_out_port);
          if (LADSPA_IS_HINT_SAMPLE_RATE(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
             controlPortRate[ctrl_out_index] = true;
          }
          if (isPoly) {
            for (l2 = 0; l2 < synthdata->poly; l2++) {
              ladspa_dsc->connect_port(ladspa_handle[l2], l1, &control_out[ctrl_out_index]);
            }
          } else {
            ladspa_dsc->connect_port(ladspa_handle[0], l1, &control_out[ctrl_out_index]);
          }  
//          fprintf(stderr, "output: %s\n", ladspa_dsc->PortNames[l1]);
          ctrl_index[ctrl_out_index] = out_port_list.count() + out_ctrl_port_list.count() - 1;
          ctrl_out_index++;
        }
      }
      if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
        if (LADSPA_IS_HINT_TOGGLED(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
          configDialog->addCheckBox(0, ladspa_dsc->PortNames[l1], &control_gui[ctrl_in_index]);
          control_data[ctrl_in_index] = 0;
          if (isPoly) {
            for (l2 = 0; l2 < synthdata->poly; l2++) {
              ladspa_dsc->connect_port(ladspa_handle[l2], l1, &control_data[ctrl_in_index]);
            }
          } else {
            ladspa_dsc->connect_port(ladspa_handle[0], l1, &control_data[ctrl_in_index]);
          }
          ctrl_in_index++;
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
            controlPortRate[ctrl_in_index] = true;
          } else {
            rate_factor = 1.0;
          }
          control_gui[ctrl_in_index] = control_min * rate_factor;
          control_data[ctrl_in_index] = control_min * rate_factor;
          control_data_min[ctrl_in_index] = control_min * rate_factor;
          control_data_max[ctrl_in_index] = control_max * rate_factor;
          if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
            control_gui[ctrl_in_index] = control_max * rate_factor;
            control_data[ctrl_in_index] = control_max * rate_factor;
          }
          if (LADSPA_IS_HINT_LOGARITHMIC(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
            if (LADSPA_IS_HINT_DEFAULT_LOW(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
              if (control_min <=0 ) control_min = 0.01;
              if (control_max <=0 ) control_max = 0.01;
              control_gui[ctrl_in_index] = exp(log(control_min) * 0.75 + log(control_max) * 0.25) * rate_factor;
              control_data[ctrl_in_index] = exp(log(control_min) * 0.75 + log(control_max) * 0.25) * rate_factor;
            }
            if (LADSPA_IS_HINT_DEFAULT_MIDDLE(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
              if (control_min <=0 ) control_min = 0.01;
              if (control_max <=0 ) control_max = 0.01;
              control_gui[ctrl_in_index] = exp(log(control_min) * 0.5 + log(control_max) * 0.5) * rate_factor;
              control_data[ctrl_in_index] = exp(log(control_min) * 0.5 + log(control_max) * 0.5) * rate_factor;
            }
            if (LADSPA_IS_HINT_DEFAULT_HIGH(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
              if (control_min <=0 ) control_min = 0.01;
              if (control_max <=0 ) control_max = 0.01;
              control_gui[ctrl_in_index] = exp(log(control_min) * 0.25 + log(control_max) * 0.75) * rate_factor;
              control_data[ctrl_in_index] = exp(log(control_min) * 0.25 + log(control_max) * 0.75) * rate_factor;
            }
          } else {
            if (LADSPA_IS_HINT_DEFAULT_LOW(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
              control_gui[ctrl_in_index] = (control_min * 0.75 + control_max * 0.25) * rate_factor;
              control_data[ctrl_in_index] = (control_min * 0.75 + control_max * 0.25) * rate_factor;
            }
            if (LADSPA_IS_HINT_DEFAULT_MIDDLE(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
              control_gui[ctrl_in_index] = (control_min * 0.5 + control_max * 0.5) * rate_factor;
              control_data[ctrl_in_index] = (control_min * 0.5 + control_max * 0.5) * rate_factor;
            }
            if (LADSPA_IS_HINT_DEFAULT_HIGH(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
              control_gui[ctrl_in_index] = (control_min * 0.25 + control_max * 0.75) * rate_factor;
              control_data[ctrl_in_index] = (control_min * 0.25 + control_max * 0.75) * rate_factor;
            }
          }
          if (LADSPA_IS_HINT_DEFAULT_0(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
            control_gui[ctrl_in_index] = 0;
            control_data[ctrl_in_index] = 0;
          }
          if (LADSPA_IS_HINT_DEFAULT_1(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
            control_gui[ctrl_in_index] = rate_factor;
            control_data[ctrl_in_index] = rate_factor;
          }
          if (LADSPA_IS_HINT_DEFAULT_100(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
            control_gui[ctrl_in_index] = 100.0 * rate_factor;
            control_data[ctrl_in_index] = 100.0 * rate_factor;
          }
          if (LADSPA_IS_HINT_DEFAULT_440(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
            control_gui[ctrl_in_index] = 440.0 * rate_factor;
            control_data[ctrl_in_index] = 440.0 * rate_factor;
          }
          if (LADSPA_IS_HINT_LOGARITHMIC(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) { 
            configDialog->addSlider(control_min * rate_factor, control_max * rate_factor, control_min * rate_factor, 
                                    ladspa_dsc->PortNames[l1], &control_gui[ctrl_in_index], true);
          } else {
            if (LADSPA_IS_HINT_INTEGER(ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
//              fprintf(stderr, "LADSPA_IS_HINT_INTEGER %s\n", ladspa_dsc->PortNames[l1]);
              configDialog->addFloatIntSlider(control_min * rate_factor, control_max * rate_factor, control_min * rate_factor,
                                              ladspa_dsc->PortNames[l1], &control_gui[ctrl_in_index]);
            } else {
              configDialog->addSlider(control_min * rate_factor, control_max * rate_factor, control_min * rate_factor,
                                      ladspa_dsc->PortNames[l1], &control_gui[ctrl_in_index], false);
            }
          }
          if (isPoly) {
            for (l2 = 0; l2 < synthdata->poly; l2++) {
              ladspa_dsc->connect_port(ladspa_handle[l2], l1, &control_data[ctrl_in_index]);
            }
          } else {
            ladspa_dsc->connect_port(ladspa_handle[0], l1, &control_data[ctrl_in_index]);
          }  
          ctrl_in_index++;
        }
      }
    }
  }
  itmp = ((out_port_list.count() + out_ctrl_port_list.count()) > (in_port_list.count() + in_ctrl_port_list.count())) 
       ? (out_port_list.count() + out_ctrl_port_list.count()) : (in_port_list.count() + in_ctrl_port_list.count());
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LADSPA_WIDTH, 
              MODULE_LADSPA_HEIGHT + 20 * itmp);
  pluginName.sprintf("%s", ladspa_dsc->Label);
  if (isPoly) {
    qs.sprintf("%s ID %d Poly", ladspa_dsc->Label, moduleID);
  } else {
    qs.sprintf("%s ID %d", ladspa_dsc->Label, moduleID);
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
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
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
  int l1;
  QString qs;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  qs.sprintf("LADSPA %s", synthdata->ladspa_dsc_func_list[ladspaDesFuncIndex](n)->Label);
  p.drawText(10, 20, qs);
  p.setFont(QFont("Helvetica", 8));
  qs.sprintf("ID %d", moduleID);
  p.drawText(10, 32, qs);
}

void M_ladspa::generateCycle() {

  int l1, l2, l3;
  float ctrlVal;

  if (!cycleReady) {
    cycleProcessing = true;

    for (l3 = 0; l3 < in_port_list.count(); l3++) inData [l3] = in_port_list.at(l3)->getinputdata();
    if (hasExtCtrlPorts)
    {
        for (l3 = 0; l3 < in_ctrl_port_list.count(); l3++) inData_ctrl [l3] = in_ctrl_port_list.at(l3)->getinputdata();
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
    if (hasExtCtrlPorts) {
      for (l3 = 0; l3 < in_ctrl_port_list.count(); l3++) {
        if (controlPortRate[l3]) {
          ctrlVal = synthdata->rate * inData_ctrl[l3][0][0] + control_gui[l3];
        } else {
          ctrlVal = inData_ctrl[l3][0][0] + control_gui[l3];
        }
        if (ctrlVal < control_data_min[l3]) ctrlVal = control_data_min[l3];
        if (ctrlVal > control_data_max[l3]) ctrlVal = control_data_max[l3];
        control_data[l3] = ctrlVal;
      } 
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l3 = 0; l3 < out_ctrl_port_list.count(); l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {    
            data[ctrl_index[l3]][l1][l2] = control_out[l3];              
          }    
        }
      }
    } else {
      for (l3 = 0; l3 < ladspa_ctrl_in_count; l3++) {
        control_data[l3] = control_gui[l3];
      }
    }
    if (isPoly) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        ladspa_dsc->run(ladspa_handle[l1], synthdata->cyclesize);
        for (l3 = 0; l3 < out_port_list.count(); l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[audio_index[l3]][l1][l2] = ladspaDataOut[l3][l1][l2];
          }
        }
      }
    } else {
      ladspa_dsc->run(ladspa_handle[0], synthdata->cyclesize);
      for (l3 = 0; l3 < out_port_list.count(); l3++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[audio_index[l3]][l1][l2] = ladspaDataOut[l3][0][l2];
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
