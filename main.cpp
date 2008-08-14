#include <stdio.h>      
#include <stdlib.h>     
#include <getopt.h>  
#include <string.h>
#include <unistd.h>

#include <QApplication>
#include <QString>
#include <QObject>

#include "mainwindow.h"
#include "m_ladspa.h"
#include "msoptions.h"


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


QTextStream StdOut(stdout);
QTextStream StdErr(stderr);

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
            qWarning(QObject::tr("Failed to open file %1")
                    .arg(rcPath).toUtf8());
            exit(-1);
        }

        struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, 0};
        if (fcntl(fd, F_SETLK, &lock) == -1) {
            close(fd);
            StdOut << "occupied" << rcPath << endl;
        } else {
            lock.l_type = F_RDLCK;
            if (fcntl(fd, F_SETLK, &lock) == -1) {
                qWarning(QObject::tr("Ooops in %1 at %2")
                        .arg(__FUNCTION__).arg(__LINE__).toUtf8());
                exit(-1);
            }
            name = amsSynthName(name, index);
            return fd;
        }
    }
    qWarning(QObject::tr("%1 occupied.").arg(name).toUtf8());
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

  int getopt_return;
  int option_index;
  ModularSynthOptions msoptions;

  msoptions.synthName = "ams";
  msoptions.pcmname = DEFAULT_PCMNAME;
  msoptions.frsize = DEFAULT_PERIODSIZE;
  msoptions.fsamp = DEFAULT_RATE;
  msoptions.ncapt = DEFAULT_CAPT_PORTS;
  msoptions.nfrags = DEFAULT_PERIODS;
  msoptions.nplay = DEFAULT_PLAY_PORTS;
  msoptions.poly = 1;
  msoptions.edge = 1.0;
  msoptions.noGui = false;
  msoptions.presetName = "";
  msoptions.presetPath = "";
  msoptions.havePreset = false;
  msoptions.havePresetPath = false;
  msoptions.enableJack = false;

  while((getopt_return = getopt_long(argc, argv, "hnjb:p:f:e:c:l:d:r:i:o:N:",
				     options, &option_index)) >= 0) {
    switch(getopt_return) {
    case 'p': 
        msoptions.poly = atoi(optarg);
        break;
    case 'b': 
        msoptions.frsize = atoi(optarg);
        break;
    case 'f': 
        msoptions.nfrags = atoi(optarg);
        break;
    case 'e': 
        msoptions.edge = atof(optarg);
        break;
    case 'r': 
        msoptions.fsamp = atoi(optarg);
        break;
    case 'c': 
        msoptions.pcmname = optarg;
        break; 
    case 'l': 
        msoptions.presetName = optarg;
        msoptions.havePreset = true;
        break; 
    case 'd': 
        msoptions.presetPath = optarg;
        msoptions.havePresetPath = true;
        break; 
    case 'n':
        msoptions.noGui = true;
        break;
    case 'j':
        msoptions.enableJack = true;
        break;
    case 'i': 
        msoptions.ncapt = atoi(optarg);
        break;
    case 'o': 
        msoptions.nplay = atoi(optarg);
        break;
    case 'N':
      msoptions.synthName += optarg;
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

  msoptions.rcFd = makeSynthName(msoptions.synthName);
  MainWindow* top = new MainWindow(msoptions);
  Q_CHECK_PTR(top);
  top->resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

  M_ladspa::logo = new QPixmap(PIXMAPS_PATH"ladspa_logo_smaller_trans.png");

  return app.exec();
}
