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
#include "synthdata.h"
#include "synth.h"
#include "m_wavout.h"
#include "port.h"
#include "module.h"

M_wavout::M_wavout(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(0, parent, name, p_synthdata) {

  QString qs;
  QHBox *hbox1, *hbox2;

  M_type = M_type_wavout;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_WAVOUT_WIDTH, MODULE_WAVOUT_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  agc = 1;
  doRecord = 0;
  wavfile = NULL;
  port_in[0] = new Port("In 0", PORT_IN, 0, this, synthdata);
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  port_in[1] = new Port("In 1", PORT_IN, 1, this, synthdata);
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);
  qs.sprintf("WAV Out ID %d", moduleID);
  configDialog->setCaption(qs);
  //configDialog->initTabWidget();
  QString tabName="Actions";
  nameField=new QLineEdit("File:",configDialog->headerBox);
  timeField=new QLineEdit("Time:",configDialog->headerBox);
  nameField->setReadOnly(true);
  timeField->setReadOnly(true);
  ActionParameter *ap=new ActionParameter(this,"New file","");
  connect(ap,SIGNAL(triggered()),this,SLOT(openBrowser()));
  configDialog->addParameter(ap,tabName);
  ap=new ActionParameter(this,"Overwrite current file","");
  connect(ap,SIGNAL(triggered()),this,SLOT(createWav()));
  configDialog->addParameter(ap,tabName);
  recStart=new ActionParameter(this,"Record","",false);
  connect(ap,SIGNAL(triggered()),this,SLOT(recordClicked()));
  configDialog->addParameter(ap,tabName);
  recStop=new ActionParameter(this,"Stop","",false);
  connect(ap,SIGNAL(triggered()),this,SLOT(stopClicked()));
  configDialog->addParameter(ap,tabName);
  tabName="Gain controll";
  FloatParameter * pGain = new FloatParameter(this,"Gain","",0.0,1.0,&gain);
  FloatParameter * pGain1 = new FloatParameter(this,"Volume 1","",0.0,1.0,&mixer_gain[0]);
  FloatParameter * pGain2 = new FloatParameter(this,"Volume 2","",0.0,1.0,&mixer_gain[1]);
  EnumParameter * pAgc = new EnumParameter(this,"Automatic Gain Control","",(int *)&agc);
  pAgc->addItem(0,"Disabled");
  pAgc->addItem(1,"Enabled");

  configDialog->addParameter(pGain);
  configDialog->addParameter(pGain1);
  configDialog->addParameter(pGain2);
  configDialog->addParameter(pAgc);

  wavDataSize = 0;
  wavdata = (unsigned char *)malloc(synthdata->periodsize * 4);
  memset(wavdata, 0, synthdata->periodsize * 4);
  floatdata = (float *)malloc(2 * synthdata->periodsize * sizeof(float));
  memset(floatdata, 0, 2 * synthdata->periodsize * sizeof(float));
  timer = new QTimer(this);
  QObject::connect(timer, SIGNAL(timeout()),
                   this, SLOT(timerProc()));
}

M_wavout::~M_wavout() {

  if (wavfile) {
    fclose(wavfile); 
  }
  free(wavdata);
  free(floatdata);
}

void M_wavout::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  QString qs;
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
  p.setPen(QColor(255, 255, 255));
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, "WAV Out");
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(15, 32, qs);
}

int M_wavout::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

float M_wavout::getGain() {
  return(gain);
}

void M_wavout::generateCycle() {

  int l1, l2, l3, index;
  float max_ch, mixgain, wavgain;
  short s;

  memset(floatdata, 0, 2 * synthdata->cyclesize * sizeof(float));
  wavgain = GAIN / (float)synthdata->poly;
  for (l1 = 0; l1 < 2; l1++) {                       // TODO generalize to more than 2 channels
    if (port_in[l1]->connectedPortList.count()) {
      module_in[l1] = (Module *)port_in[l1]->connectedPortList.at(0)->parentModule;
      module_in[l1]->generateCycle();
      index = port_in[l1]->connectedPortList.at(0)->index;
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          floatdata[2 * l2 + l1] += mixgain * module_in[l1]->data[index][l3][l2]; 
        }
      }
      if (agc) {
        max_ch = 0;
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          if (max_ch < fabs(floatdata[2 * l2 + l1])) {
            max_ch = fabs(floatdata[2 * l2 + l1]);
          }    
        }
        if (max_ch > 0.9) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            floatdata[2 * l2 + l1] *= 0.9 / max_ch;
          }
        }
      }
    } else {
      module_in[l1] = NULL;
    }
  }  
  if (doRecord > 0) {
    for (l2 = 0; l2 < 2; l2++) {
      for (l1 = 0; l1 < synthdata->cyclesize; l1++) {   
        s = (short)(wavgain * floatdata[2 * l1 + l2]);
        wavdata[4*l1+2*l2] = (unsigned char)s;
        wavdata[4*l1+2*l2+1] = s >> 8;
      }   
    }
    fwrite(wavdata, 1, synthdata->cyclesize * 4, wavfile);
    wavDataSize += synthdata->cyclesize * 4;
  }
}

void M_wavout::showConfigDialog() {
}

void M_wavout::recordToggled(bool on) {

  int tmpint;

  if (!on) {
    tmpint = wavDataSize + 36;
    outbuf[3] = tmpint >> 24;  // ByteRate
    outbuf[2] = (tmpint >> 16) - ((tmpint >> 24) << 8);
    outbuf[1] = (tmpint >> 8) - ((tmpint >> 16) << 8);
    outbuf[0] = (unsigned char)tmpint;
    fseek(wavfile, 4, SEEK_SET);
    fwrite(outbuf, 1, 4, wavfile);
    tmpint = wavDataSize;
    outbuf[3] = tmpint >> 24;  // ByteRate
    outbuf[2] = (tmpint >> 16) - ((tmpint >> 24) << 8);
    outbuf[1] = (tmpint >> 8) - ((tmpint >> 16) << 8);
    outbuf[0] = (unsigned char)tmpint;
    fseek(wavfile, 40, SEEK_SET);
    fwrite(outbuf, 1, 4, wavfile);
    fseek(wavfile, 0, SEEK_END);
    fflush(wavfile);
  } else {
    timer->start(200, true);
  }
}

void M_wavout::recordClicked() {

  doRecord = true;
  recordToggled(true);
}


void M_wavout::stopClicked() {

  doRecord = false;
  recordToggled(false);
}

void M_wavout::createWav() {

  int tmpint;

  wavname = nameField->text();
  if ((wavfile = fopen(wavname, "w"))) {
    wavDataSize = 0;
    outbuf[0] = 0x52; outbuf[1] = 0x49; outbuf[2] = 0x46; outbuf[3] = 0x46; // "RIFF"
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x24; outbuf[1] = 0x00; outbuf[2] = 0xff; outbuf[3] = 0x00; // ChunkSize
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x57; outbuf[1] = 0x41; outbuf[2] = 0x56; outbuf[3] = 0x45; // "WAVE"
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x66; outbuf[1] = 0x6d; outbuf[2] = 0x74; outbuf[3] = 0x20; // "fmt "
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x10; outbuf[1] = 0x00; outbuf[2] = 0x00; outbuf[3] = 0x00; // Subchunk1Size
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x01; outbuf[1] = 0x00; // AudioFormat
    fwrite(outbuf, 1, 2, wavfile);
    outbuf[0] = 0x02; outbuf[1] = 0x00; // NumChannels
    fwrite(outbuf, 1, 2, wavfile);
    outbuf[3] = synthdata->rate >> 24;  // SampleRate
    outbuf[2] = (synthdata->rate >> 16) - ((synthdata->rate >> 24) << 8);
    outbuf[1] = (synthdata->rate >> 8) - ((synthdata->rate >> 16) << 8);
    outbuf[0] = (unsigned char)synthdata->rate;
    fwrite(outbuf, 1, 4, wavfile);
    tmpint = synthdata->rate * 2 * 2;
    outbuf[3] = tmpint >> 24;  // ByteRate
    outbuf[2] = (tmpint >> 16) - ((tmpint >> 24) << 8);
    outbuf[1] = (tmpint >> 8) - ((tmpint >> 16) << 8);
    outbuf[0] = (unsigned char)tmpint;
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x04; outbuf[1] = 0x00; // BlockAlign
    fwrite(outbuf, 1, 2, wavfile);
    outbuf[0] = 0x10; outbuf[1] = 0x00; // BitsPerSample
    fwrite(outbuf, 1, 2, wavfile);
    outbuf[0] = 0x64; outbuf[1] = 0x61; outbuf[2] = 0x74; outbuf[3] = 0x61; // "data"
    fwrite(outbuf, 1, 4, wavfile);
    outbuf[0] = 0x00; outbuf[1] = 0x00; outbuf[2] = 0xff; outbuf[3] = 0x00; // Subchunk2Size
    fwrite(outbuf, 1, 4, wavfile);
    recStart->setEnabled(true);
    recStop->setEnabled(true);
    timeField->setText("Time: 0:00:00        ");

  } else {
    recStart->setEnabled(false);
    recStop->setEnabled(false);
  }
}

void M_wavout::openBrowser() {

  char buf[2048];

  getcwd(buf, 2048);
  if ((wavname = QString(QFileDialog::getSaveFileName(QString(buf), "WAV files (*.wav)")))) {
    nameField->setText(wavname);
    createWav();
  }
}

void M_wavout::timerProc() {

  QString qs1, qs2, qs3;
  int seconds, minutes, displaySeconds;

  if (doRecord) {
    timer->start(200, true);
    seconds = (wavDataSize >> 2) / synthdata->rate;
    minutes = (seconds % 3600) / 60;
    displaySeconds = seconds % 60;
    qs1.sprintf("%d", seconds / 3600);
    if (minutes < 10) {
      qs2.sprintf("0%d", minutes);
    } else {
      qs2.sprintf("%d", minutes);
    }
    if (displaySeconds < 10) {
      qs3.sprintf("0%d", displaySeconds);
    } else {
      qs3.sprintf("%d", displaySeconds);
    }
    timeField->setText("Time: "+qs1+":"+qs2+":"+qs3+"  ");
  }
}
