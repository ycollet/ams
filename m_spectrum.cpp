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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_spectrum.h"
#include "module.h"
#include "port.h"


M_spectrum::M_spectrum(QWidget* parent, const char *name, SynthData *p_synthdata) 
              : Module(0, parent, name, p_synthdata)
{
  QString qs;
  QHBox *hbox1, *labelBox;
  QVBox *vbox1, *vbox2;
 
  M_type = M_type_spectrum;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SPECTRUM_WIDTH, MODULE_SPECTRUM_HEIGHT);
  gain = 0;
  mixer_gain[0] = 1.0;
  mixer_gain[1] = 1.0;
  agc = 1;
  port_in[0] = new Port("In 0", PORT_IN, 0, this, synthdata);          
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  port_in[1] = new Port("In 1", PORT_IN, 1, this, synthdata);          
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);
  qs.sprintf("Spectrum ID %d", moduleID);
  configDialog->setCaption(qs);
  configDialog->initTabWidget();
  zoom = 1;
  viewMode = 0;
  fftFrames = 4;
  normMode = 0;
  f_min = 0;
  f_max = (double)synthdata->rate / 2.0;
  freqZoom = 0;  
  refreshMode = 0;
  fftMode = 1;
  window = 2;
  QVBox *spectrumTab = new QVBox(configDialog->tabWidget);
  configDialog->addSpectrumScreen(spectrumTab);
  labelBox = configDialog->addHBox();
  minLabel = new QLabel(labelBox);
  QWidget *dummy = new QWidget(labelBox);
  maxLabel = new QLabel(labelBox);
  labelBox->setStretchFactor(minLabel, 1);
  labelBox->setStretchFactor(dummy, 20);
  labelBox->setStretchFactor(maxLabel, 1);
  qs.sprintf("%d Hz", (int)rint(f_min));
  minLabel->setText(qs);
  qs.sprintf("%d Hz", (int)rint(f_max));
  maxLabel->setText(qs);
  configDialog->addTab(spectrumTab, "Spectrum");
  QVBox *paramTab = new QVBox(configDialog->tabWidget);
  QVBox *zoomTab = new QVBox(configDialog->tabWidget);
  configDialog->addSlider(-20, 20, gain, "Gain (dB)", &gain, false, zoomTab);
  QObject::connect(configDialog->midiSliderList.at(0), SIGNAL(valueChanged(int)),
                   this, SLOT(updateZoom(int)));
  configDialog->addSlider(0, f_max, f_min, "f_min", &f_min, false, zoomTab);
  QObject::connect(configDialog->midiSliderList.at(1), SIGNAL(valueChanged(int)),
                   this, SLOT(update_f_min(int)));
  configDialog->addSlider(0, f_max, f_max, "f_max", &f_max, false, zoomTab);
  QObject::connect(configDialog->midiSliderList.at(2), SIGNAL(valueChanged(int)),
                   this, SLOT(update_f_max(int)));
  configDialog->addCheckBox(freqZoom, "Frequency Zoom", &freqZoom, zoomTab);
  QObject::connect(configDialog->midiCheckBoxList.at(0)->checkBox, SIGNAL(toggled(bool)),
                   this, SLOT(freqZoomToggled(bool)));
  
  hbox1 = configDialog->addHBox(paramTab);
  vbox1 = configDialog->addVBox(hbox1);
  vbox2 = configDialog->addVBox(hbox1);
  QStrList *viewModeNames = new QStrList(true);
  viewModeNames->append("Normal Spectrum");
  viewModeNames->append("Spectrum over Time");
  configDialog->addComboBox(viewMode, "Display Mode", &viewMode, viewModeNames->count(), viewModeNames, vbox1);
  QObject::connect(configDialog->midiComboBoxList.at(0)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateViewMode(int)));
  QStrList *refreshModeNames = new QStrList(true);
  refreshModeNames->append("Continuous");
  refreshModeNames->append("Single");
  refreshModeNames->append("Mouse");
  configDialog->addComboBox(refreshMode, "Refresh Mode", &refreshMode, refreshModeNames->count(), refreshModeNames, vbox2);
  QObject::connect(configDialog->midiComboBoxList.at(1)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateRefreshMode(int)));
  QStrList *normModeNames = new QStrList(true);
  normModeNames->append("Each Line");
  normModeNames->append("Global");
  normModeNames->append("Fixed");
  configDialog->addComboBox(normMode, "Normalization Mode", &normMode, normModeNames->count(), normModeNames, vbox1);
  QObject::connect(configDialog->midiComboBoxList.at(2)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateNormMode(int)));
  QStrList *fftModeNames = new QStrList(true);
  fftModeNames->append("Power Spectrum");
  fftModeNames->append("Abs");
  fftModeNames->append("dB");
  configDialog->addComboBox(fftMode, "Spectrum Mode", &fftMode, fftModeNames->count(), fftModeNames, vbox2);
  QObject::connect(configDialog->midiComboBoxList.at(3)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateFFTMode(int)));
  QStrList *windowNames = new QStrList(true);
  windowNames->append("Hamming");
  windowNames->append("Bartlett");
  windowNames->append("Hanning");
  windowNames->append("Welch");
  configDialog->addComboBox(window, "Window Function", &window, windowNames->count(), windowNames, vbox2);
  QObject::connect(configDialog->midiComboBoxList.at(4)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateWindow(int)));
  configDialog->addPushButton("Trigger", paramTab);
  QObject::connect(configDialog->midiPushButtonList.at(0), SIGNAL(clicked()),
                   configDialog->spectrumScreenList.at(0), SLOT(singleShot()));
  QObject::connect(configDialog->spectrumScreenList.at(0),
                   SIGNAL(runSpectrum()), this, SLOT(startSpectrum()));
  QStrList *fftFramesNames = new QStrList(true);
  fftFramesNames->append("  128");
  fftFramesNames->append("  256");
  fftFramesNames->append("  512");
  fftFramesNames->append(" 1024");
  fftFramesNames->append(" 2048");
  fftFramesNames->append(" 4096");
  fftFramesNames->append(" 8192");
  fftFramesNames->append("16384");
  fftFramesNames->append("32768");
  configDialog->addComboBox(fftFrames, "Window Size", &fftFrames, fftFramesNames->count(), fftFramesNames, vbox1);
  QObject::connect(configDialog->midiComboBoxList.at(5)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateFFTFrames(int)));
  configDialog->addTab(zoomTab, "Zoom");
  configDialog->addTab(paramTab, "Mode / Window");
  floatdata = (float *)malloc(2 * synthdata->periodsize * sizeof(float));
  memset(floatdata, 0, 2 * synthdata->periodsize * sizeof(float));
  configDialog->spectrumScreenList.at(0)->writeofs = 0;
  timer = new QTimer(this);   
  QObject::connect(timer, SIGNAL(timeout()),
                   this, SLOT(timerProc()));
  startSpectrum();
}

M_spectrum::~M_spectrum()
{
  free(floatdata);
}

int M_spectrum::setGain(float p_gain)
{
  gain = p_gain;
  return(0);
}

float M_spectrum::getGain()
{
  return(gain);
}

void M_spectrum::generateCycle()
{
    int l1, l2, l3, ofs;
    float  mixgain, wavgain, lin_gain;
    float *spectrumdata, **indata;

    wavgain = 1.0 / (float)synthdata->poly;
    lin_gain = pow(10, gain/20.0);
    memset(floatdata, 0, 2 * synthdata->cyclesize * sizeof(float));
    for (l1 = 0; l1 < 2; l1++)
    {
        indata = port_in[l1]->getinputdata ();
        mixgain = lin_gain * mixer_gain[l1];
        for (l2 = 0; l2 < synthdata->cyclesize; l2++)
        {
            for (l3 = 0; l3 < synthdata->poly; l3++)
            {
                floatdata[2 * l2 + l1] += mixgain * indata[l3][l2]; 
            }
        }
    }

    spectrumdata = configDialog->spectrumScreenList.at(0)->spectrumdata;
    ofs = configDialog->spectrumScreenList.at(0)->writeofs;
    for (l1 = 0; l1 < synthdata->cyclesize; l1++)
    {   
        spectrumdata[2 * ofs] = wavgain * floatdata[2 * l1];
        spectrumdata[2 * ofs + 1] = wavgain * floatdata[2 * l1 + 1];
        if (++ofs >= SPECTRUM_BUFSIZE >> 1) ofs -= SPECTRUM_BUFSIZE >> 1;
    }
    configDialog->spectrumScreenList.at(0)->writeofs = ofs;
}

void M_spectrum::showConfigDialog()
{
}

void M_spectrum::timerProc()
{          
 
  if (configDialog->spectrumScreenList.at(0)->getTriggerMode() == SPECTRUM_TRIGGERMODE_CONTINUOUS) {
    startSpectrum();
  }
  configDialog->spectrumScreenList.at(0)->refreshSpectrum();
}

void M_spectrum::updateFFTFrames(int val)
{
  configDialog->spectrumScreenList.at(0)->setFFTFrames((int)rint(exp(log(2.0) * (7.0 + (float)fftFrames))));
}

void M_spectrum::updateViewMode(int val)
{
  configDialog->spectrumScreenList.at(0)->setViewMode((viewModeType)viewMode);
}

void M_spectrum::updateZoom(int val) {

  configDialog->spectrumScreenList.at(0)->setZoom(zoom);
}

void M_spectrum::update_f_min(int val)
{
  QString qs;

  configDialog->spectrumScreenList.at(0)->set_f_min(f_min);
  qs.sprintf("%d Hz", (int)rint(f_min));   
  minLabel->setText(qs);
}

void M_spectrum::update_f_max(int val)
{
  QString qs;

  configDialog->spectrumScreenList.at(0)->set_f_max(f_max);
  qs.sprintf("%d Hz", (int)rint(f_max));
  maxLabel->setText(qs);
}

void M_spectrum::updateNormMode(int val)
{
  configDialog->spectrumScreenList.at(0)->setNormMode((normModeType)normMode);
}

void M_spectrum::updateWindow(int val)
{
  configDialog->spectrumScreenList.at(0)->setWindow((windowType)window);
}

void M_spectrum::updateFFTMode(int val)
{
  configDialog->spectrumScreenList.at(0)->setFFTMode((fftModeType)fftMode);
}

void M_spectrum::updateRefreshMode(int val)
{
  if (refreshMode == 0)
  {
    configDialog->spectrumScreenList.at(0)->setTriggerMode(SPECTRUM_TRIGGERMODE_CONTINUOUS); 
    startSpectrum();
  }
  else
  {
    configDialog->spectrumScreenList.at(0)->setTriggerMode(SPECTRUM_TRIGGERMODE_SINGLE);
  } 
  if (refreshMode == 2)
  {
    configDialog->spectrumScreenList.at(0)->setEnableMouse(true);
  }
  else
  {
    configDialog->spectrumScreenList.at(0)->setEnableMouse(false);
  }
}

void M_spectrum::freqZoomToggled(bool on)
{
  configDialog->spectrumScreenList.at(0)->toggleFreqZoom(on);
}

void M_spectrum::startSpectrum()
{
  timer->start(int((float)configDialog->spectrumScreenList.at(0)->getFFTFrames() / (float)synthdata->rate * 1000.0), true);
}
