#include <stdio.h>      
#include <stdlib.h>     
#include <getopt.h>  
#include <string.h>
#include <unistd.h>
#include <QApplication>
#include <QMenuBar>
#include <QScrollArea>
#include <qstring.h>
#include "modularsynth.h"
#include "mainwindow.h"


class ScrollArea: public QScrollArea {
  void resizeEvent(QResizeEvent *ev)
  {
    QScrollArea::resizeEvent(ev);
    ((ModularSynth*)widget())->resize();
  }
};


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
         {"name", 1, 0, 'N'},
         {0, 0, 0, 0}};

QTextStream StdErr(stderr);
QTextStream StdOut(stdout);

QString amsHOME()
{
  return QString(getenv("HOME")) + "/.alsamodular/";
}
QString amsRcPath(const QString &synthName)
{
  QString string;
  QTextStream rcPath(&string);
  rcPath << amsHOME() << synthName;
  rcPath << ".cfg";
  return string;
}
QString amsSynthName(const QString &name, int index)
{
  QString string;
  QTextStream synthName(&string);
  synthName << name;
  if (index)
    synthName << "_" << index;
  return string; 
}
int makeSynthName(QString &name)
{
  int fd;
  mkdir(amsHOME().toLatin1().data(), 0777);

  for (int index = 0; index < 9; index++) {
    QString rcPath = amsRcPath(amsSynthName(name, index));
      StdOut << rcPath << endl;
    fd = open(rcPath.toLatin1().data(), O_CREAT|O_RDWR, 0666);
    if (fd == -1) {
      StdErr << "Failed to open file " << rcPath << endl;
      exit(-1);
    }

    struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, 0};
    if (fcntl(fd, F_SETLK, &lock) == -1) {
      close(fd);
      StdOut << "occupied" << rcPath << endl;
    } else {
      lock.l_type = F_RDLCK;
      if (fcntl(fd, F_SETLK, &lock) == -1) {
	StdErr << "Ooops in " << __FUNCTION__ << " at " << __LINE__ << endl;
	exit(-1);
      }
      name = amsSynthName(name, index);
      return fd;
    }
  }
  StdErr << name << " occupied." << endl;
  exit(-1);
  return -1;
}


int main(int argc, char *argv[])  
{
  char aboutText[] = AMS_LONGNAME " " AMS_VERSION 
                     "\nby Matthias Nagorni and Fons Adriaensen\n"
                     "(c)2002-2003 SuSE AG Nuremberg\n"
                     "(c)2003 Fons Adriaensen\n"
		     "additional programming:\n"
		     "2007 Malte Steiner\n"
		     "2007 Karsten Wiese\n";
  QApplication app(argc, argv);
  MainWindow top;

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
  QString presetName, presetPath, synthName("ams");
  bool havePreset = false;
  bool havePresetPath = false;
  bool noGui = false;
  bool enableJack = false;
  //  char buf [2048];
  float edge = 1.0;

  while((getopt_return = getopt_long(argc, argv, "hnjb:p:f:e:c:l:d:r:i:o:N:",
				     options, &option_index)) >= 0) {
    switch(getopt_return) {
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
    case 'N':
      synthName += optarg;
      break;
    case 'h':
        printf("\n%s", aboutText);
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
        printf("--nogui                      Start without GUI\n");
        printf("--name                       ALSASEQ/JACK clientname, windowtitle [ams_ALSASEQ-ID]\n\n");
        exit(EXIT_SUCCESS);
        break;
    }
  }
  StdOut << "ScrollArea *scrollArea = new ScrollArea();" << endl;
  ScrollArea *scrollArea = new ScrollArea();
  StdOut << scrollArea << ":" << scrollArea->maximumViewportSize().width() << ":" <<endl;

  int rcFd = makeSynthName(synthName);
  ModularSynth *modularSynth =
    new ModularSynth(&top, synthName, rcFd, pcmname, fsamp, frsize, nfrags,
		     ncapt, nplay, poly, edge);
  scrollArea->setWidget(modularSynth);

  //  modularSynth->setBaseSize(3000, 4000);

  

  QMenu *filePopup = top.menuBar()->addMenu("&File");
  //  top->menuBar()->insertSeparator();
  QMenu *synthesisPopup = top.menuBar()->addMenu("&Synthesis");
  //  top->menuBar()->insertSeparator();
  QMenu *modulePopup = top.menuBar()->addMenu("&Module");
  QMenu *newModulePopup = modulePopup->addMenu("&New");
  modularSynth->contextMenu = newModulePopup;
  //  top->menuBar()->insertSeparator();
  QMenu *midiMenu = top.menuBar()->addMenu("&View");
  //  top->menuBar()->insertSeparator();
  QMenu *aboutMenu = top.menuBar()->addMenu("&About");
  filePopup->addAction("&New", modularSynth, SLOT(clearConfig()));
  filePopup->addSeparator();
  filePopup->addAction("&Load Patch", modularSynth, SLOT(load()));
  filePopup->addAction("&Save Patch", modularSynth, SLOT(save()));
  filePopup->addSeparator();
  filePopup->addAction("&Load Colors", modularSynth, SLOT(loadColors()));
  filePopup->addAction("&Save Colors", modularSynth, SLOT(saveColors()));
  filePopup->addSeparator();
  filePopup->addAction("&Quit", qApp, SLOT(quit()));
  synthesisPopup->addAction("Start", modularSynth, SLOT(startSynth()));
  synthesisPopup->addAction("Stop", modularSynth, SLOT(stopSynth()));
  synthesisPopup->addAction("All Voices Off", modularSynth, SLOT(allVoicesOff()));

  newModulePopup->addAction("Amplifier", modularSynth, SLOT(newM_amp()));
  newModulePopup->addAction("Analog Driver (2 Out)", modularSynth, SLOT(newM_ad_2()));
  newModulePopup->addAction("Analog Driver (4 Out)", modularSynth, SLOT(newM_ad_4()));
  newModulePopup->addAction("Analog Driver (6 Out)", modularSynth, SLOT(newM_ad_6()));
  newModulePopup->addAction("Advanced ENV", modularSynth, SLOT(newM_advenv()));
  newModulePopup->addAction("Advanced MCV", modularSynth, SLOT(newM_advmcv()));
  newModulePopup->addAction("Comment", modularSynth, SLOT(new_textEdit()));
  newModulePopup->addAction("Converter", modularSynth, SLOT(newM_conv()));
  newModulePopup->addAction("CVS", modularSynth, SLOT(newM_cvs()));
  newModulePopup->addAction("Delay", modularSynth, SLOT(newM_delay()));
  newModulePopup->addAction("Dynamic Waves (4 Oscillators)", modularSynth, SLOT(newM_dynamicwaves_4()));
  newModulePopup->addAction("Dynamic Waves (6 Oscillators)", modularSynth, SLOT(newM_dynamicwaves_6()));
  newModulePopup->addAction("Dynamic Waves (8 Oscillators)", modularSynth, SLOT(newM_dynamicwaves_8()));
  newModulePopup->addAction("ENV", modularSynth, SLOT(newM_env()));
  newModulePopup->addAction("Function 1 --> 1", modularSynth, SLOT(newM_function_1()));
  newModulePopup->addAction("Function 1 --> 2", modularSynth, SLOT(newM_function_2()));
  newModulePopup->addAction("Function 1 --> 4", modularSynth, SLOT(newM_function_4()));
  newModulePopup->addAction("INV", modularSynth, SLOT(newM_inv()));
  newModulePopup->addAction("LFO", modularSynth, SLOT(newM_lfo()));
  newModulePopup->addAction("MCV", modularSynth, SLOT(newM_mcv()));
  newModulePopup->addAction("MIDI Out", modularSynth, SLOT(newM_midiout()));
  newModulePopup->addAction("Mixer 2 -> 1", modularSynth, SLOT(newM_mix_2()));
  newModulePopup->addAction("Mixer 4 -> 1", modularSynth, SLOT(newM_mix_4()));
  newModulePopup->addAction("Mixer 8 -> 1", modularSynth, SLOT(newM_mix_8()));
  newModulePopup->addAction("Multiphase LFO", modularSynth, SLOT(newM_mphlfo()));
  newModulePopup->addAction("Noise / Random", modularSynth, SLOT(newM_noise()));
  newModulePopup->addAction("Noise / Random 2", modularSynth, SLOT(newM_noise2()));
  newModulePopup->addAction("PCM Out", modularSynth, SLOT(newM_pcmout()));
  newModulePopup->addAction("PCM In", modularSynth, SLOT(newM_pcmin()));  
  newModulePopup->addAction("Quantizer", modularSynth, SLOT(newM_quantizer()));
  newModulePopup->addAction("Quantizer 2", modularSynth, SLOT(newM_vquant()));
  newModulePopup->addAction("Ring Modulator", modularSynth, SLOT(newM_ringmod()));
  newModulePopup->addAction("Sample && Hold", modularSynth, SLOT(newM_sh()));
  newModulePopup->addAction("Scala MCV", modularSynth, SLOT(newM_scmcv()));  
  newModulePopup->addAction("Scala Quantizer", modularSynth, SLOT(newM_scquantizer()));
  newModulePopup->addAction("Scope View", modularSynth, SLOT(newM_scope()));
  newModulePopup->addAction("SEQ  8", modularSynth, SLOT(newM_seq_8()));
  newModulePopup->addAction("SEQ 12", modularSynth, SLOT(newM_seq_12()));
  newModulePopup->addAction("SEQ 16", modularSynth, SLOT(newM_seq_16()));
  newModulePopup->addAction("SEQ 24", modularSynth, SLOT(newM_seq_24()));
  newModulePopup->addAction("SEQ 32", modularSynth, SLOT(newM_seq_32()));
  newModulePopup->addAction("Slew Limiter", modularSynth, SLOT(newM_slew()));
  newModulePopup->addAction("Spectrum View", modularSynth, SLOT(newM_spectrum()));
  newModulePopup->addAction("Stereo Mixer 2", modularSynth, SLOT(newM_stereomix_2()));
  newModulePopup->addAction("Stereo Mixer 4", modularSynth, SLOT(newM_stereomix_4())); 
  newModulePopup->addAction("Stereo Mixer 8", modularSynth, SLOT(newM_stereomix_8())); 
  newModulePopup->addAction("VC Double Decay", modularSynth, SLOT(newM_vcdoubledecay()));
  newModulePopup->addAction("VC Envelope", modularSynth, SLOT(newM_vcenv()));
  newModulePopup->addAction("VC Envelope II", modularSynth, SLOT(newM_vcenv2()));
  newModulePopup->addAction("VC Organ (4 Oscillators)", modularSynth, SLOT(newM_vcorgan_4()));
  newModulePopup->addAction("VC Organ (6 Oscillators)", modularSynth, SLOT(newM_vcorgan_6()));
  newModulePopup->addAction("VC Organ (8 Oscillators)", modularSynth, SLOT(newM_vcorgan_8()));
  newModulePopup->addAction("VC Panning", modularSynth, SLOT(newM_vcpanning()));
  newModulePopup->addAction("VC Switch", modularSynth, SLOT(newM_vcswitch()));
  newModulePopup->addAction("VCA lin.", modularSynth, SLOT(newM_vca_lin()));
  newModulePopup->addAction("VCA exp.", modularSynth, SLOT(newM_vca_exp()));
  newModulePopup->addAction("VCF", modularSynth, SLOT(newM_vcf()));
  newModulePopup->addAction("VCO", modularSynth, SLOT(newM_vco()));
  newModulePopup->addAction("VCO2", modularSynth, SLOT(newM_vco2()));
  newModulePopup->addAction("WAV Out", modularSynth, SLOT(newM_wavout()));

  modulePopup->addAction("&Show Ladspa Browser", modularSynth, SLOT(displayLadspaPlugins()));
  midiMenu->addAction("Control Center", modularSynth, SLOT(displayMidiController()));
  midiMenu->addAction("Parameter View", modularSynth, SLOT(displayParameterView()));
  midiMenu->addAction("Preferences", modularSynth, SLOT(displayPreferences()));
  aboutMenu->addAction("About AlsaModularSynth", modularSynth, SLOT(displayAbout()));
  top.setGeometry(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
  top.setCentralWidget(scrollArea);
  if (noGui)
    top.hide();
  else
    top.show();

  QObject::connect(qApp, SIGNAL(aboutToQuit()), modularSynth, SLOT(cleanUpSynth()));

  if (havePresetPath) {
    StdErr << "Preset path now " << presetPath << endl; 
    modularSynth->setPresetPath(presetPath);
  }
//  getcwd(buf, 2048);
//  modularSynth->setSavePath(QString(buf));
  modularSynth->go (enableJack);
  if (havePreset) {
    StdErr << "Loading preset " << presetName << endl; 
    modularSynth->load(presetName);
  }

  return qApp->exec();
}
