#include <stdio.h>      
#include <stdlib.h>     
#include <getopt.h>  
#include <string.h>
#include <unistd.h>

#include <QApplication>
#include <QDir>
#include <QString>
#include <QLibraryInfo>
#include <QLocale>
#include <QObject>
#include <QTranslator>

#include "mainwindow.h"
#include "msoptions.h"
#include "config.h"


#define AMSDIR ".alsamodular"

static struct option options[] =
        {{"periodsize", 1, 0, 'b'},
         {"frag", 1, 0, 'f'},   
         {"poly", 1, 0, 'p'},   
         {"rate", 1, 0, 'r'},   
         {"edge", 1, 0, 'e'},   
         {"help", 0, 0, 'h'},
         {"verbose", 0, 0, 'v'},
         {"version", 0, 0, 'V'},
         {"soundcard", 1, 0, 'c'},
#if defined (HAVE_CLALSADRV_API2) || defined (HAVE_LIBZITA_ALSA_PCMI)
         {"capture", 1, 0, 'C'},
         {"playback", 1, 0, 'P'},
#endif
         {"presetpath", 1, 0, 'd'},
         {"nogui", 0, 0, 'n'},
         {"jack", 0, 0, 'J'},
#ifdef JACK_SESSION
         {"jack_session_uuid", 1, 0, 'U' },
#endif
         {"alsa", 0, 0, 'A'},
         {"in", 1, 0, 'i'},
         {"out", 1, 0, 'o'},
         {"name", 1, 0, 'N'},
         {0, 0, 0, 0}};


QTextStream StdOut(stdout);
QTextStream StdErr(stderr);


QString amsRcPath(const QString &synthName)
{
  return QString("%1/" AMSDIR "/%2.cfg").arg(QDir::homePath())
      .arg(synthName);
}

QString amsSynthName(const QString &name, unsigned int index)
{
  return QString("%1_%2").arg(name).arg(index);
}

int makeSynthName(QString &name)
{
    int fd;

    QDir amshome = QDir(QDir::homePath());
    if (!amshome.exists(AMSDIR)) {
        if (!amshome.mkdir(AMSDIR)) {
            qWarning("%s", QObject::tr("Could not create ams home "
                        "directory.").toLatin1().constData());
            return -1;
        }
    }

    for (unsigned int index = 1; index <= 9; index++) {
        QString rcPath = amsRcPath(amsSynthName(name, index));
        //StdOut << "Resource file path: " << rcPath << endl;
        fd = open(rcPath.toLatin1().data(), O_CREAT|O_RDWR, 0666);
        if (fd == -1) {
            qWarning("%s", QObject::tr("Failed to open file '%1'")
                    .arg(rcPath).toLatin1().constData());
            return -1;
        }

        struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, 0};
        if (fcntl(fd, F_SETLK, &lock) == -1) {
            close(fd);
            StdOut << "File occupied: " << rcPath << endl;
        } else {
            lock.l_type = F_RDLCK;
            if (fcntl(fd, F_SETLK, &lock) == -1) {
                qWarning("%s", QObject::tr("Ooops in %1 at %2")
                        .arg(__FUNCTION__).arg(__LINE__).toLatin1().constData());
                return -1;
            }
            name = amsSynthName(name, index);
            return fd;
        }
    }
    qWarning("%s", QObject::tr("Client name '%1' occupied.").arg(name)
            .toLatin1().constData());
    return -1;
}


int main(int argc, char *argv[])  
{
  char aboutText[] = AMS_LONGNAME " " VERSION;
  /*                 "\nby Matthias Nagorni and Fons Adriaensen\n"
                     "(c)2002-2003 SuSE AG Nuremberg\n"
                     "(c)2003 Fons Adriaensen\n"
		     "Additional programming:\n"
		     "2007 Malte Steiner, Karsten Wiese\n"
		     "2008 Guido Scholz\n";
 */
  QApplication app(argc, argv);

  // translator for Qt library strings
  QTranslator qtTr;
  QLocale loc = QLocale::system();

  if (qtTr.load(QString("qt_") + loc.name(),
              QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
      app.installTranslator(&qtTr);
  else
      qWarning("No Qt translation for locale '%s' found.",
              loc.name().toLatin1().constData());
  
  // translator for ams strings
  QTranslator amsTr;

  if (amsTr.load(QString("ams_") + loc.name(), TRANSLATIONSDIR))
      app.installTranslator(&amsTr);
  else
      qWarning("No " AMS_LONGNAME " translation for locale '%s' found.",
              loc.name().toLatin1().constData());
  
  int getopt_return;
  int option_index;
  ModularSynthOptions msoptions;

  msoptions.synthName = "ams";
  msoptions.pcmname = DEFAULT_PCMNAME;
  msoptions.cname = "";
  msoptions.pname = "";
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
  msoptions.forceJack = false;
  msoptions.forceAlsa = false;
  msoptions.verbose = 0;
#ifdef JACK_SESSION
  msoptions.global_jack_session_uuid = "";
#endif

  while ((getopt_return = getopt_long(argc, argv,
                  "hnJjAab:p:f:e:c:d:r:i:o:vVN:"
#ifdef JACK_SESSION
                  "U:"
#endif
#if defined (HAVE_CLALSADRV_API2) || defined (HAVE_LIBZITA_ALSA_PCMI)
                  "C:P:"
#endif
                  ,
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
#if defined (HAVE_CLALSADRV_API2) || defined (HAVE_LIBZITA_ALSA_PCMI)
    case 'C': 
        msoptions.cname = optarg;
        break; 
    case 'P': 
        msoptions.pname = optarg;
        break; 
#endif
    case 'd': 
        msoptions.presetPath = optarg;
        msoptions.havePresetPath = true;
        break; 
    case 'n':
        msoptions.noGui = true;
        break;
    case 'J':
    case 'j':
        msoptions.forceJack = true;
        msoptions.forceAlsa = false;
        break;
    case 'A':
    case 'a':
        msoptions.forceJack = false;
        msoptions.forceAlsa = true;
        break;
    case 'i': 
        msoptions.ncapt = atoi(optarg);
        break;
    case 'o': 
        msoptions.nplay = atoi(optarg);
        break;
#ifdef JACK_SESSION
    case 'U': 
        msoptions.global_jack_session_uuid = QString(optarg);
        break;
#endif
    case 'v':
        msoptions.verbose++;
        break;
    case 'V':
        printf("%s\n", aboutText);
        exit(EXIT_SUCCESS);
        break;
    case 'N':
        msoptions.synthName += optarg;
        break;
    case 'h':
	printf("%s\n\n", aboutText);
	printf("Usage:\tams [OPTION]... [PRESETFILE]\n\nOptions:\n");
        printf("  -J, --jack                    Force JACK\n");
        printf("  -A, --alsa                    Force ALSA\n");
        printf("    -c, --soundcard <plug>        Soundcard [%s]\n", DEFAULT_PCMNAME);
#if defined (HAVE_CLALSADRV_API2) || defined (HAVE_LIBZITA_ALSA_PCMI)
        printf("    -C, --capture <device>        Capture device\n");
        printf("    -P, --playback <device>       Playback device\n");
#endif
        printf("    -b, --periodsize <frames>     Periodsize [%d]\n", DEFAULT_PERIODSIZE);
        printf("    -f, --frag <num>              Number of fragments [%d]\n", DEFAULT_PERIODS);
        printf("    -r, --rate <samples/s>        Samplerate [%d]\n", DEFAULT_RATE);
        printf("  -i, --in <num>                Number of ALSA/JACK input ports\n");
        printf("  -o, --out <num>               Number of ALSA/JACK output ports\n");
        printf("  -p, --poly <num>              Polyphony [1]\n");
        printf("  -e, --edge <0..10>            VCO Edge [1.0]\n");
        printf("  -d, --presetdir <dir>         Preset directory\n");
        printf("  -n, --nogui                   Start without GUI\n");
#ifdef JACK_SESSION
        printf("  -U, --jack_session_uuid       JACK session UUID\n");
#endif
        printf("  -v				verbose\n");
        printf("  -V, --version			Display program version information\n");
        printf("  -N, --name <name>             ALSA/JACK clientname, windowtitle\n\n");
        exit(EXIT_SUCCESS);
        break;
    }
  }

#if defined (HAVE_CLALSADRV_API2) || defined (HAVE_LIBZITA_ALSA_PCMI)
  if (msoptions.cname.isEmpty() && msoptions.pname.isEmpty()) {
      msoptions.cname = msoptions.pcmname;
      msoptions.pname = msoptions.pcmname;
  }
#else
  msoptions.cname = msoptions.pcmname;
  msoptions.pname = msoptions.pcmname;
#endif

  if (optind < argc){
      msoptions.presetName = argv[optind];
      msoptions.havePreset = true;
  }

  msoptions.rcFd = makeSynthName(msoptions.synthName);
  if (msoptions.rcFd == -1)
      exit(1);

  MainWindow* top = new MainWindow(msoptions);
  Q_CHECK_PTR(top);

  return app.exec();
}
