#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>
#include <qregexp.h>
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
#include <qmessagebox.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_scquantizer.h"
#include "port.h"

M_scquantizer::M_scquantizer(QWidget* parent, const char *name, SynthData *p_synthdata, QString *p_sclname) 
              : Module(2, parent, name, p_synthdata) {

  QString qs;
  int l1;
  char buf[2048]; 
  glob_t globbuf;

  M_type = M_type_scquantizer;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SCQUANTIZER_WIDTH, MODULE_SCQUANTIZER_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this, synthdata); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_M_trigger = new Port("Trigger", PORT_IN, 1, this, synthdata); 
  port_M_trigger->move(0, 55);
  port_M_trigger->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_trigger);
  port_M_transpose = new Port("Transpose", PORT_IN, 2, this, synthdata); 
  port_M_transpose->move(0, 75);
  port_M_transpose->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_transpose);
  port_out = new Port("Out", PORT_OUT, 0, this, synthdata);          
  port_out->move(width() - port_out->width(), 95);
  port_out->outType = outType_audio;
  portList.append(port_out);
  port_trigger_out = new Port("Trigger Out", PORT_OUT, 1, this, synthdata);          
  port_trigger_out->move(width() - port_trigger_out->width(), 115);
  port_trigger_out->outType = outType_audio;
  portList.append(port_trigger_out);
  base = 0;
  lastbase = 12;
  configDialog->addIntSlider(-36, 36, base, "Note Offset", &base);
  qs.sprintf("Scala Quantizer ID %d", moduleID);
  configDialog->setCaption(qs);
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    qsig[l1] = 0;
    trigCount[l1] = 0;
    trigger[l1] = 0;
  }
  sclname = "No_Scale_loaded";
  configDialog->addLabel("   Scale: " + sclname);
  configDialog->addLabel("   ");
  configDialog->addPushButton("Load Scale");
  QObject::connect(configDialog->midiPushButtonList.at(0), SIGNAL(clicked()),
                   this, SLOT(openBrowser()));
  fileDialog = NULL;
  for (l1 = 0; l1 < 12; l1++) {
    scale_lut_isRatio[l1] = false;
    scale_lut[l1] = 100.0 + (float)l1 * 100.0;
  }
  scale_lut_isRatio[12] = true;
  scale_lut[12] = 2.0;
  scale_lut_length = 12;
  dirpath.sprintf("%s", getenv("SCALA_PATH"));
  if (dirpath.length() < 1) {
    fprintf(stderr, "\nYou did not set the environment variable SCALA_PATH.\n");
    fprintf(stderr, "Assuming SCALA_PATH=/usr/share/scala\n");
    dirpath = "/usr/share/scala";
  } else {
    fprintf(stderr, "SCALA_PATH: %s\n", dirpath.latin1());
  }
  if (p_sclname && !p_sclname->contains("No_Scale_loaded")) {
    loadScale(dirpath + "/" + *p_sclname);
  }
}

M_scquantizer::~M_scquantizer() {
}

void M_scquantizer::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "Scala Quantizer");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

void M_scquantizer::calcScale() {

  int l1, index;
  float log2, base_cv, base_freq;

  log2 = log(2.0);
  lastbase = base;
  base_cv = base / 12.0;
  base_freq = synthdata->exp_table(log2 * (4.0313842 + base_cv));
  fprintf(stderr, "base: %d, base_cv: %f, base_freq: %f\n", base, base_cv, base_freq);
  scale_notes[0] = base_cv;
  index = 1;
  while (index < 128) {
    for (l1 = 0; l1 < scale_lut_length; l1++) {
      if (scale_lut_isRatio[l1]) {
        scale_notes[index] = log(base_freq * scale_lut[l1])/log2 - 4.0313842;
      } else {
        scale_notes[index] = base_cv + scale_lut[l1] / 1200.0;
      }
      index++;
      if (index > 127) break;
    }
    base_cv = scale_notes[index - 1];
    base_freq = synthdata->exp_table(log2 * (4.0313842 + base_cv));
  }
}  

void M_scquantizer::generateCycle() {

  int l1, l2, l3, quant, transpose;
  float log2, lutquant;

  if (!cycleReady) {
    cycleProcessing = true;
    if (base != lastbase) {
      calcScale();
    }
    for (l2 = 0; l2 < 2; l2++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        memcpy(lastdata[l2][l1], data[l2][l1], synthdata->cyclesize * sizeof(float));
      }
    }
    if (port_M_in->connectedPortList.count()) {
      in_M_in = (Module *)port_M_in->connectedPortList.at(0)->parentModule;
      if (!in_M_in->cycleProcessing) {
        in_M_in->generateCycle();
        inData = in_M_in->data[port_M_in->connectedPortList.at(0)->index];
      } else {
        inData = in_M_in->lastdata[port_M_in->connectedPortList.at(0)->index];
      }
    } else {
      in_M_in = NULL;
      inData = synthdata->zeroModuleData;
    }
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
    if (port_M_transpose->connectedPortList.count()) {
      in_M_transpose = (Module *)port_M_transpose->connectedPortList.at(0)->parentModule;
      if (!in_M_transpose->cycleProcessing) {
        in_M_transpose->generateCycle();
        transposeData = in_M_transpose->data[port_M_transpose->connectedPortList.at(0)->index];
      } else {
        transposeData = in_M_transpose->lastdata[port_M_transpose->connectedPortList.at(0)->index];
      }
    } else {
      in_M_transpose = NULL;
      transposeData = synthdata->zeroModuleData;
    }
    if (!in_M_trigger) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          quant = 1;
          for (l2 = 0; l2 < 128; l2++) {
            if (scale_notes[quant] > 4.0 + inData[l1][l2]) {
              lutquant = scale_notes[quant-1];
              break;
            } else {
              quant++;
            }
          }
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (scale_notes[quant] > 4.0 + inData[l1][l2]) {
              lutquant = scale_notes[quant-1];
            }
            if (qsig[l1] != lutquant) {
              qsig[l1] = lutquant;
              data[1][l1][l2] = 1.0;
              trigCount[l1] = 512;  
            } else {
              if (trigCount[l1] > 0) {
                data[1][l1][l2] = 1;  
                trigCount[l1]--;
              } else {
                data[1][l1][l2] = 0;
              }
            }  
            transpose = (int)(transposeData[l1][l2] * 12.0);
            data[0][l1][l2] = (float)qsig[l1] - 4.0 + (float)(transpose + base) / 12.0;
          }
         } 
    } else {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
              trigger[l1] = true;
              quant = 1;
              for (l3 = 0; l3 < 128; l3++) {
                if (scale_notes[quant] > 4.0 + inData[l1][l2]) {
                  break;  
                } else {  
                  quant++;
                }
              }
              qsig[l1] = scale_notes[quant-1];
              data[1][l1][l2] = 1.0;
              trigCount[l1] = 512;
            } else {
              if (trigger[l1] && (triggerData[l1][l2] < 0.5)) {
                trigger[l1] = false;
              }
            }
            if (trigCount[l1] > 0) {
              data[1][l1][l2] = 1;
              trigCount[l1]--;
            } else {
              data[1][l1][l2] = 0;
            }
            transpose = (int)(transposeData[l1][l2] * 12.0);
            data[0][l1][l2] = (float)qsig[l1] - 4.0 + (float)(transpose + base) / 12.0;
          }   
        }
    }
  }
  cycleProcessing = false;
  cycleReady = true;
}

void M_scquantizer::showConfigDialog() {
}

void M_scquantizer::openBrowser() {

  if (!fileDialog) {
    fileDialog = new QFileDialog(dirpath, "Scala files (*.scl)");
    QObject::connect(fileDialog, SIGNAL(fileSelected(const QString &)), this, SLOT(loadScale(const QString &)));
  }
  fileDialog->show();
}

void M_scquantizer::loadScale(const QString &p_sclname) {

  QString qs, qs2, qs3;
  int index, n;

  sclname = p_sclname;  
  QFile *qfile = new QFile(sclname);
  if (!qfile->open(IO_ReadOnly)) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not load Scala file "+sclname);
    sclname = "No_Scale_loaded";
    return; 
  }
  configDialog->labelList.at(0)->setText("   Scale: " + sclname);
  QTextStream *stream  = new QTextStream(qfile);
  while (!stream->eof()) {
    qs = stream->readLine(); 
    if (!qs.contains("!")) 
      break;
  }
  configDialog->labelList.at(1)->setText("   " + qs);
  fprintf(stderr, "Scale: %s\n", qs.latin1());
  while (!stream->eof()) { 
    qs = stream->readLine();
    if (!qs.contains("!"))
      break;
  }
  index = 0;
  while (!stream->eof() && (index < 128)) {
    qs = stream->readLine();
    if (qs.contains("!")) {
      continue;
    }
    qs2 = qs.simplifyWhiteSpace();
    if (qs2.contains(".")) {
      if ((n = qs2.find(" ")) > 0) {
        qs = qs2.left(n); 
      } else {
        qs = qs2;
      }
      scale_lut_isRatio[index] = false;
      scale_lut[index] = qs.toFloat(); 
      index++;
    } else {  
      scale_lut_isRatio[index] = true;
      if (qs.contains("/")) {
        qs = qs2.left(qs2.find("/"));
        qs3 = qs2.mid(qs2.find("/") + 1);
        if ((n = qs3.find(" ")) > 0) {   
          qs2 = qs3.left(n); 
        } else {
          qs2 = qs3;
        }
        scale_lut[index] = qs.toFloat() / qs2.toFloat(); 
      } else {
        if ((n = qs2.find(" ")) > 0) {
          qs = qs2.left(n);
        } else {
          qs = qs2;
        }
        scale_lut[index] = qs.toFloat();
      }
      index++;
    }
  }  
  scale_lut_length = index;
  qfile->close();
  calcScale();
}
