#include <stdio.h>      
#include <stdlib.h>     
#include <getopt.h>  
#include <string.h>
#include <unistd.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qmenubar.h>   
#include <qpopupmenu.h>
#include <qstring.h>
#include <qvbox.h>
#include <qscrollview.h>
#include "modularsynth.h"


static struct option options[] =
        {{"periodsize", 1, 0, 'b'},
         {"frag", 1, 0, 'f'},   
         {"poly", 1, 0, 'p'},   
         {"rate", 1, 0, 'r'},   
         {"edge", 1, 0, 'e'},   
         {"help", 0, 0, 'h'},
         {"soundcard", 1, 0, 'c'},
         {"preset", 1, 0, 'l'},
         {"presetpath", 1, 0, 'd'},
         {"nogui", 0, 0, 'n'},
         {"jack", 0, 0, 'j'},
         {"in", 1, 0, 'i'},
         {"out", 1, 0, 'o'},
         {0, 0, 0, 0}};

int main(int argc, char *argv[])  
{
  QString aboutText ("AlsaModularSynth " AMS_VERSION 
                     "\nby Matthias Nagorni and Fons Adriaensen\n"
                     "(c)2002-2003 SuSE AG Nuremberg\n"
                     "(c)2003 Fons Adriaensen\n\n");
  QApplication *qApp = new QApplication(argc, argv);
  QMainWindow *top = new QMainWindow();
  top->setCaption("AlsaModularSynth");
  int getopt_return;
  int option_index; 
  int poly = 1;
  int fsamp = DEFAULT_RATE;
  int ncapt = DEFAULT_CAPT_PORTS;
  int nplay = DEFAULT_PLAY_PORTS;
  int frsize = DEFAULT_PERIODSIZE;
  int nfrags = DEFAULT_PERIODS;
  char pcmname [256];
  strcpy (pcmname, DEFAULT_PCMNAME);
  QString presetName, presetPath;
  bool havePreset = false;
  bool havePresetPath = false;
  bool noGui = false;
  bool enableJack = false;
  char buf [2048];
  float edge = 1.0;

  while((getopt_return = getopt_long(argc, argv, "hnjb:p:f:e:c:l:d:r:i:o:", options, &option_index)) >= 0)
  {
    switch(getopt_return)
    {
    case 'p': 
        poly = atoi(optarg);
        break;
    case 'b': 
        frsize = atoi(optarg);
        break;
    case 'f': 
        nfrags = atoi(optarg);
        break;
    case 'e': 
        edge = atof(optarg);
        break;
    case 'r': 
        fsamp = atoi(optarg);
        break;
    case 'c': 
        strcpy (pcmname, optarg);
        break; 
    case 'l': 
        presetName.sprintf("%s", optarg);
        havePreset = true;
        break; 
    case 'd': 
        presetPath.sprintf("%s", optarg);
        havePresetPath = true;
        break; 
    case 'n':
        noGui = true;
        break;
    case 'j':
        enableJack = true;
        break;
    case 'i': 
        ncapt = atoi(optarg);
        break;
    case 'o': 
        nplay = atoi(optarg);
        break;
    case 'h':
        printf("\n%s", aboutText.latin1());
        printf("--jack                       Enable JACK I/O\n");
        printf("--in <num>                   Number of JACK input ports\n");
        printf("--out <num>                  Number of JACK output ports\n");
        printf("--poly <num>                 Polyphony [1]\n");
        printf("--periodsize <frames>        Periodsize [%d]\n", DEFAULT_PERIODSIZE);
        printf("--frag <num>                 Number of fragments [%d]\n", DEFAULT_PERIODS);
        printf("--rate <samples/s>           Samplerate [%d]\n", DEFAULT_RATE);
        printf("--edge <0..10>               VCO Edge [1.0]\n");
        printf("--soundcard <plug>           Soundcard [hw:0,0]\n");
        printf("--preset <file>              Preset file\n");
        printf("--presetpath <path>          Preset path\n");
        printf("--nogui                      Start without GUI\n\n");
        exit(EXIT_SUCCESS);
        break;
    }
  }

  ModularSynth *modularSynth = new ModularSynth (top, pcmname, fsamp, frsize, nfrags, ncapt, nplay, poly, edge);
  modularSynth->resizeContents(3000, 4000);

  QPopupMenu *filePopup = new QPopupMenu(top);
  QPopupMenu *synthesisPopup = new QPopupMenu(top);
  QPopupMenu *modulePopup = new QPopupMenu(top);
  QPopupMenu *newModulePopup = new QPopupMenu(top);
  QPopupMenu *midiMenu = new QPopupMenu(top);
  QPopupMenu *aboutMenu = new QPopupMenu(top);
  top->menuBar()->insertItem("&File", filePopup);
  top->menuBar()->insertSeparator();
  top->menuBar()->insertItem("&Synthesis", synthesisPopup);
  top->menuBar()->insertSeparator();
  top->menuBar()->insertItem("&Module", modulePopup);
  top->menuBar()->insertSeparator();
  top->menuBar()->insertItem("&View", midiMenu);
  top->menuBar()->insertSeparator();
  top->menuBar()->insertItem("&About", aboutMenu);
  filePopup->insertItem("&New", modularSynth, SLOT(clearConfig()));
  filePopup->insertSeparator();
  filePopup->insertItem("&Load Patch", modularSynth, SLOT(load()));
  filePopup->insertItem("&Save Patch", modularSynth, SLOT(save()));
  filePopup->insertSeparator();
  filePopup->insertItem("&Load Colors", modularSynth, SLOT(loadColors()));
  filePopup->insertItem("&Save Colors", modularSynth, SLOT(saveColors()));
  filePopup->insertSeparator();
  filePopup->insertItem("&Quit", qApp, SLOT(quit()));
  synthesisPopup->insertItem("Start", modularSynth, SLOT(startSynth()));
  synthesisPopup->insertItem("Stop", modularSynth, SLOT(stopSynth()));
  synthesisPopup->insertItem("All Voices Off", modularSynth, SLOT(allVoicesOff()));

  newModulePopup->insertItem("Advanced ENV", modularSynth, SLOT(newM_advenv()));
  newModulePopup->insertItem("Advanced MCV", modularSynth, SLOT(newM_advmcv()));
  newModulePopup->insertItem("Comment", modularSynth, SLOT(new_textEdit()));
  newModulePopup->insertItem("Converter", modularSynth, SLOT(newM_conv()));
  newModulePopup->insertItem("CVS", modularSynth, SLOT(newM_cvs()));
  newModulePopup->insertItem("Delay", modularSynth, SLOT(newM_delay()));
  newModulePopup->insertItem("Dynamic Waves (4 Oscillators)", modularSynth, SLOT(newM_dynamicwaves_4()));
  newModulePopup->insertItem("Dynamic Waves (6 Oscillators)", modularSynth, SLOT(newM_dynamicwaves_6()));
  newModulePopup->insertItem("Dynamic Waves (8 Oscillators)", modularSynth, SLOT(newM_dynamicwaves_8()));
  newModulePopup->insertItem("ENV", modularSynth, SLOT(newM_env()));
  newModulePopup->insertItem("Function 1 --> 1", modularSynth, SLOT(newM_function_1()));
  newModulePopup->insertItem("Function 1 --> 2", modularSynth, SLOT(newM_function_2()));
  newModulePopup->insertItem("Function 1 --> 4", modularSynth, SLOT(newM_function_4()));
  newModulePopup->insertItem("INV", modularSynth, SLOT(newM_inv()));
  newModulePopup->insertItem("LFO", modularSynth, SLOT(newM_lfo()));
  newModulePopup->insertItem("MCV", modularSynth, SLOT(newM_mcv()));
  newModulePopup->insertItem("MIDI Out", modularSynth, SLOT(newM_midiout()));
  newModulePopup->insertItem("Mixer 2 -> 1", modularSynth, SLOT(newM_mix_2()));
  newModulePopup->insertItem("Mixer 4 -> 1", modularSynth, SLOT(newM_mix_4()));
  newModulePopup->insertItem("Mixer 8 -> 1", modularSynth, SLOT(newM_mix_8()));
  newModulePopup->insertItem("Noise / Random", modularSynth, SLOT(newM_noise()));
  newModulePopup->insertItem("PCM Out", modularSynth, SLOT(newM_pcmout()));
  newModulePopup->insertItem("PCM In", modularSynth, SLOT(newM_pcmin()));  
  newModulePopup->insertItem("Quantizer", modularSynth, SLOT(newM_quantizer()));
  newModulePopup->insertItem("Ring Modulator", modularSynth, SLOT(newM_ringmod()));
  newModulePopup->insertItem("Sample && Hold", modularSynth, SLOT(newM_sh()));
  newModulePopup->insertItem("Scala MCV", modularSynth, SLOT(newM_scmcv()));  
  newModulePopup->insertItem("Scala Quantizer", modularSynth, SLOT(newM_scquantizer()));
  newModulePopup->insertItem("Scope View", modularSynth, SLOT(newM_scope()));
  newModulePopup->insertItem("SEQ  8", modularSynth, SLOT(newM_seq_8()));
  newModulePopup->insertItem("SEQ 12", modularSynth, SLOT(newM_seq_12()));
  newModulePopup->insertItem("SEQ 16", modularSynth, SLOT(newM_seq_16()));
  newModulePopup->insertItem("SEQ 24", modularSynth, SLOT(newM_seq_24()));
  newModulePopup->insertItem("SEQ 32", modularSynth, SLOT(newM_seq_32()));
  newModulePopup->insertItem("Slew Limiter", modularSynth, SLOT(newM_slew()));
  newModulePopup->insertItem("Spectrum View", modularSynth, SLOT(newM_spectrum()));
  newModulePopup->insertItem("Stereo Mixer 2", modularSynth, SLOT(newM_stereomix_2()));
  newModulePopup->insertItem("Stereo Mixer 4", modularSynth, SLOT(newM_stereomix_4())); 
  newModulePopup->insertItem("Stereo Mixer 8", modularSynth, SLOT(newM_stereomix_8())); 
  newModulePopup->insertItem("VC Envelope", modularSynth, SLOT(newM_vcenv()));
  newModulePopup->insertItem("VC Envelope II", modularSynth, SLOT(newM_vcenv2()));
  newModulePopup->insertItem("VC Organ (4 Oscillators)", modularSynth, SLOT(newM_vcorgan_4()));
  newModulePopup->insertItem("VC Organ (6 Oscillators)", modularSynth, SLOT(newM_vcorgan_6()));
  newModulePopup->insertItem("VC Organ (8 Oscillators)", modularSynth, SLOT(newM_vcorgan_8()));
  newModulePopup->insertItem("VC Panning", modularSynth, SLOT(newM_vcpanning()));
  newModulePopup->insertItem("VC Switch", modularSynth, SLOT(newM_vcswitch()));
  newModulePopup->insertItem("VCA lin.", modularSynth, SLOT(newM_vca_lin()));
  newModulePopup->insertItem("VCA exp.", modularSynth, SLOT(newM_vca_exp()));
  newModulePopup->insertItem("VCF", modularSynth, SLOT(newM_vcf()));
  newModulePopup->insertItem("VCO", modularSynth, SLOT(newM_vco()));
  newModulePopup->insertItem("WAV Out", modularSynth, SLOT(newM_wavout()));

  modulePopup->insertItem("&New", newModulePopup);
  modulePopup->insertItem("&Show Ladspa Browser", modularSynth, SLOT(displayLadspaPlugins()));
  midiMenu->insertItem("Control Center", modularSynth, SLOT(displayMidiController()));
  midiMenu->insertItem("Parameter View", modularSynth, SLOT(displayParameterView()));
  aboutMenu->insertItem("About AlsaModularSynth", modularSynth, SLOT(displayAbout()));
  top->setGeometry(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
  top->setCentralWidget(modularSynth);
  if (noGui) top->hide();
  else       top->show();
  qApp->setMainWidget(top);
  QObject::connect(qApp, SIGNAL(aboutToQuit()), modularSynth, SLOT(cleanUpSynth()));

  if (havePresetPath)
  {
    fprintf(stderr, "Preset path now %s\n", presetPath.latin1()); 
    modularSynth->setPresetPath(presetPath);
  }
  getcwd(buf, 2048);
  modularSynth->setSavePath(QString(buf));
  modularSynth->go (enableJack);
  if (havePreset)
  {
    fprintf(stderr, "Loading preset %s\n", presetName.latin1()); 
    modularSynth->load(&presetName);
  }

  return qApp->exec();
}
