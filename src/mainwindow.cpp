#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QScrollArea>

#include "mainwindow.h"
#include "synthdata.h"

#include "../pixmaps/ams_32.xpm"

/*some constants*/
#define APPNAME  "AlsaModularSynth"
#define PATCHEXT ".ams"


class ScrollArea: public QScrollArea {
  void resizeEvent(QResizeEvent *ev)
  {
    QScrollArea::resizeEvent(ev);
    widget()->adjustSize();
  }
};


int MainWindow::pipeFd[2];

MainWindow::MainWindow(const ModularSynthOptions& mso)
{
  /*make sure the window destructor is called on program exit*/
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowIcon(QPixmap(ams_32_xpm));
  setObjectName("MainWindow");
  fileName = "";
  rcFd = mso.rcFd;

  /*init synthesizer*/
  modularSynth = new ModularSynth(this, mso);

  /*init window*/
  ScrollArea *scrollArea = new ScrollArea();
  scrollArea->setWidget(modularSynth);
  setCentralWidget(scrollArea);

  QMenu *filePopup = menuBar()->addMenu(tr("&File"));
  QMenu *synthesisPopup = menuBar()->addMenu(tr("&Synthesis"));
  QMenu *modulePopup = menuBar()->addMenu(tr("&Module"));
  QMenu *midiMenu = menuBar()->addMenu(tr("&View"));
  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

  QMenu *newModulePopup = modulePopup->addMenu(tr("&New"));
  modularSynth->contextMenu = newModulePopup;

  filePopup->addAction(tr("&New"), this, SLOT(fileNew()),
          Qt::CTRL + Qt::Key_N);
  filePopup->addAction(tr("&Open..."), this, SLOT(fileOpen()),
          Qt::CTRL + Qt::Key_O);
  fileRecentlyOpenedFiles = filePopup->addMenu(tr("&Recently opened files"));
  filePopup->addAction(tr("&Save"), this, SLOT(fileSave()),
          Qt::CTRL + Qt::Key_S);
  filePopup->addAction(tr("Save &as..."), this, SLOT(fileSaveAs()));
  filePopup->addSeparator();
  filePopup->addAction(tr("&Load Colors..."), modularSynth, SLOT(loadColors()));
  filePopup->addAction(tr("Save &Colors as..."), modularSynth, SLOT(saveColors()));
  filePopup->addSeparator();
  filePopup->addAction(tr("&Quit"), qApp, SLOT(closeAllWindows()),
          Qt::CTRL + Qt::Key_Q);
  connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

  synthesisPopup->addAction(tr("&Start"), modularSynth, SLOT(startSynth()),
          Qt::CTRL + Qt::Key_B);
  synthesisPopup->addAction(tr("Sto&p"), modularSynth, SLOT(stopSynth()),
          Qt::CTRL + Qt::Key_H);
  synthesisPopup->addAction(tr("&All Voices Off"), modularSynth,
          SLOT(allVoicesOff()));

  newModulePopup->addAction(tr("Amplifier"), modularSynth, SLOT(newM_amp()));
  newModulePopup->addAction(tr("Analog Driver (2 Out)"), modularSynth, SLOT(newM_ad_2()));
  newModulePopup->addAction(tr("Analog Driver (4 Out)"), modularSynth, SLOT(newM_ad_4()));
  newModulePopup->addAction(tr("Analog Driver (6 Out)"), modularSynth, SLOT(newM_ad_6()));
  newModulePopup->addAction(tr("Advanced ENV"), modularSynth, SLOT(newM_advenv()));
  newModulePopup->addAction(tr("Advanced MCV"), modularSynth, SLOT(newM_advmcv()));
  newModulePopup->addAction(tr("Comment"), modularSynth, SLOT(new_textEdit()));
  newModulePopup->addAction(tr("Converter"), modularSynth, SLOT(newM_conv()));
  newModulePopup->addAction(tr("CVS"), modularSynth, SLOT(newM_cvs()));
  newModulePopup->addAction(tr("Delay"), modularSynth, SLOT(newM_delay()));
  newModulePopup->addAction(tr("Dynamic Waves (4 Oscillators)"), modularSynth, SLOT(newM_dynamicwaves_4()));
  newModulePopup->addAction(tr("Dynamic Waves (6 Oscillators)"), modularSynth, SLOT(newM_dynamicwaves_6()));
  newModulePopup->addAction(tr("Dynamic Waves (8 Oscillators)"), modularSynth, SLOT(newM_dynamicwaves_8()));
  newModulePopup->addAction(tr("ENV"), modularSynth, SLOT(newM_env()));
  newModulePopup->addAction(tr("Function 1 --> 1"), modularSynth, SLOT(newM_function_1()));
  newModulePopup->addAction(tr("Function 1 --> 2"), modularSynth, SLOT(newM_function_2()));
  newModulePopup->addAction(tr("Function 1 --> 4"), modularSynth, SLOT(newM_function_4()));
  newModulePopup->addAction(tr("INV"), modularSynth, SLOT(newM_inv()));
  newModulePopup->addAction(tr("LFO"), modularSynth, SLOT(newM_lfo()));
  newModulePopup->addAction(tr("MCV"), modularSynth, SLOT(newM_mcv()));
  newModulePopup->addAction(tr("MIDI Out"), modularSynth, SLOT(newM_midiout()));
  newModulePopup->addAction(tr("Mixer 2 -> 1"), modularSynth, SLOT(newM_mix_2()));
  newModulePopup->addAction(tr("Mixer 4 -> 1"), modularSynth, SLOT(newM_mix_4()));
  newModulePopup->addAction(tr("Mixer 8 -> 1"), modularSynth, SLOT(newM_mix_8()));
  newModulePopup->addAction(tr("Multiphase LFO"), modularSynth, SLOT(newM_mphlfo()));
  newModulePopup->addAction(tr("Noise / Random"), modularSynth, SLOT(newM_noise()));
  newModulePopup->addAction(tr("Noise / Random 2"), modularSynth, SLOT(newM_noise2()));
  newModulePopup->addAction(tr("PCM Out"), modularSynth, SLOT(newM_pcmout()));
  newModulePopup->addAction(tr("PCM In"), modularSynth, SLOT(newM_pcmin()));  
  newModulePopup->addAction(tr("Quantizer"), modularSynth, SLOT(newM_quantizer()));
  newModulePopup->addAction(tr("Quantizer 2"), modularSynth, SLOT(newM_vquant()));
  newModulePopup->addAction(tr("Ring Modulator"), modularSynth, SLOT(newM_ringmod()));
  newModulePopup->addAction(tr("Sample && Hold"), modularSynth, SLOT(newM_sh()));
  newModulePopup->addAction(tr("Scala MCV"), modularSynth, SLOT(newM_scmcv()));  
  newModulePopup->addAction(tr("Scala Quantizer"), modularSynth, SLOT(newM_scquantizer()));
  newModulePopup->addAction(tr("Scope View"), modularSynth, SLOT(newM_scope()));
  newModulePopup->addAction(tr("SEQ  8"), modularSynth, SLOT(newM_seq_8()));
  newModulePopup->addAction(tr("SEQ 12"), modularSynth, SLOT(newM_seq_12()));
  newModulePopup->addAction(tr("SEQ 16"), modularSynth, SLOT(newM_seq_16()));
  newModulePopup->addAction(tr("SEQ 24"), modularSynth, SLOT(newM_seq_24()));
  newModulePopup->addAction(tr("SEQ 32"), modularSynth, SLOT(newM_seq_32()));
  newModulePopup->addAction(tr("Slew Limiter"), modularSynth, SLOT(newM_slew()));
  newModulePopup->addAction(tr("Spectrum View"), modularSynth, SLOT(newM_spectrum()));
  newModulePopup->addAction(tr("Stereo Mixer 2"), modularSynth, SLOT(newM_stereomix_2()));
  newModulePopup->addAction(tr("Stereo Mixer 4"), modularSynth, SLOT(newM_stereomix_4())); 
  newModulePopup->addAction(tr("Stereo Mixer 8"), modularSynth, SLOT(newM_stereomix_8())); 
  newModulePopup->addAction(tr("VC Double Decay"), modularSynth, SLOT(newM_vcdoubledecay()));
  newModulePopup->addAction(tr("VC Envelope"), modularSynth, SLOT(newM_vcenv()));
  newModulePopup->addAction(tr("VC Envelope II"), modularSynth, SLOT(newM_vcenv2()));
  newModulePopup->addAction(tr("VC Organ (4 Oscillators)"), modularSynth, SLOT(newM_vcorgan_4()));
  newModulePopup->addAction(tr("VC Organ (6 Oscillators)"), modularSynth, SLOT(newM_vcorgan_6()));
  newModulePopup->addAction(tr("VC Organ (8 Oscillators)"), modularSynth, SLOT(newM_vcorgan_8()));
  newModulePopup->addAction(tr("VC Panning"), modularSynth, SLOT(newM_vcpanning()));
  newModulePopup->addAction(tr("VC Switch"), modularSynth, SLOT(newM_vcswitch()));
  newModulePopup->addAction(tr("VCA lin."), modularSynth, SLOT(newM_vca_lin()));
  newModulePopup->addAction(tr("VCA exp."), modularSynth, SLOT(newM_vca_exp()));
  newModulePopup->addAction(tr("VCF"), modularSynth, SLOT(newM_vcf()));
  newModulePopup->addAction(tr("VCO"), modularSynth, SLOT(newM_vco()));
  newModulePopup->addAction(tr("VCO2"), modularSynth, SLOT(newM_vco2()));
  newModulePopup->addAction(tr("WAV Out"), modularSynth, SLOT(newM_wavout()));

  modulePopup->addAction(tr("&Show Ladspa Browser..."), modularSynth,
          SLOT(displayLadspaPlugins()));

  midiMenu->addAction(tr("&Control Center..."), modularSynth,
          SLOT(displayMidiController()));
  midiMenu->addAction(tr("&Parameter View..."), modularSynth,
          SLOT(displayParameterView()));
  midiMenu->addAction(tr("Pre&ferences..."), modularSynth,
          SLOT(displayPreferences()));

  helpMenu->addAction(tr("&About AlsaModularSynth..."), modularSynth,
          SLOT(displayAbout()));
  helpMenu->addAction(tr("About &Qt..."), this,
          SLOT(helpAboutQt()));

  connect(filePopup, SIGNAL(aboutToShow()), this,
          SLOT(setupRecentFilesMenu()));
  connect(fileRecentlyOpenedFiles, SIGNAL(triggered(QAction*)), this,
        SLOT(recentFileActivated(QAction*)));


  if (pipe(pipeFd) < 0)
    return;

  QSocketNotifier *sigNotifier = new QSocketNotifier(pipeFd[0],
          QSocketNotifier::Read, this);
  QObject::connect(sigNotifier, SIGNAL(activated(int)), this,
          SLOT(unixSignal(int)));

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sighandler;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGUSR1, &action, NULL);

  readConfig();
  updateWindowTitle();

  if (mso.havePresetPath) {
    qWarning("%s", QObject::tr("Preset path now %1").arg(mso.presetPath)
            .toUtf8().constData()); 
    modularSynth->setPatchPath(mso.presetPath);
  }
  modularSynth->go(mso.forceJack, mso.forceAlsa);

  // autoload patch file
  if (mso.havePreset) {
    qWarning("%s", QObject::tr("Loading preset %1").arg(mso.presetName)
            .toUtf8().constData()); 
    openFile(mso.presetName);
  }

  if (mso.noGui)
      hide();
  else
      show();
}


MainWindow::~MainWindow()
{
    qWarning("%s", QObject::tr("Closing synthesizer...").toUtf8().constData());
    writeConfig();

    // remove file lock
    struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, 0};
    if (fcntl(rcFd, F_UNLCK, &lock) == -1) {
        qWarning("%s", QObject::tr("Could not unlock preferences file.")
                .toUtf8().constData());
    }
}

/*handle UNIX system signals*/
void MainWindow::sighandler(int s)
{
    ssize_t result;
    int pipeMessage = s;

    result = write(pipeFd[1], &pipeMessage, sizeof(pipeMessage));
    if (result == -1)
        qWarning("Error writing to pipe: %d", errno);
}

/*handle incoming unix signal messages*/
void MainWindow::unixSignal(int fd)
{
    int message;
    ssize_t result;

    result = read(fd, &message, sizeof(message));
    if (result == -1) {
        qWarning("Error reading signal message pipe: %d", errno);
        return;
    }

    switch(message) {
        case SIGINT:
            qApp->closeAllWindows();
            break;

        case SIGUSR1:
            saveFile();
            break;

        default:
            qWarning("Unexpected signal received: %d", message);
            break;
    }
}

/*check for changed file data*/
bool MainWindow::isModified()
{
    return modularSynth->isModified();
}

int MainWindow::querySaveChanges()
{
    QString queryStr;
    
    if (fileName.isEmpty())
        queryStr = tr("Unnamed file was changed.\nSave changes?");
    else
        queryStr = tr("File '%1' was changed.\n"
                "Save changes?").arg(fileName);
    
    return QMessageBox::warning(this, tr("Save changes"),
            queryStr, tr("&Yes"), tr("&No"), tr("Cancel"));
}


void MainWindow::chooseFile()
{
    QString fn = QFileDialog::getOpenFileName(this,
        tr("Open patch file"), modularSynth->getPatchPath(),
        tr("AlsaModularSynth patch files") + " (*" + PATCHEXT + ")");

    if (fn.isEmpty())
        return;
    else
        openFile(fn);
}


void MainWindow::fileNew()
{           
    if (isSave())
        newFile();
}

void MainWindow::newFile()
{  
    modularSynth->clearConfig(true);
 
    fileName = "";
    updateWindowTitle();
}


void MainWindow::fileOpen()
{
    if (isSave())
        chooseFile();
}


bool MainWindow::isSave()
{
    bool result = false;

    if (isModified()) {
        int choice = querySaveChanges();
        switch (choice) {
            case 0: //Yes
                if (saveFile())
                    result = true;
                break;
            case 1: //No
                result = true;
                break;
            case 2: //Cancel
            default:
                break;
        }
    }
    else
        result = true;
    return result;
}


void MainWindow::openFile(const QString& fn)
{
    QFile f(fn);

    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("%s", tr("Could not read file '%1'").arg(fn)
                .toUtf8().constData());
        return;
    }

    modularSynth->setPatchPath(fn.left(fn.lastIndexOf('/')));
    fileName = fn;
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    modularSynth->load(ts);
    f.close();
    
    addRecentlyOpenedFile(fileName, recentFiles);
    updateWindowTitle();
}


void MainWindow::fileSave()
{
    saveFile();
}


bool MainWindow::saveFile()
{
    if (fileName.isEmpty()) {
        fileSaveAs();
        return true;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning("%s", tr("Could not open file '%1'").arg(fileName)
                .toUtf8().constData());
        return false;
    }

    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    modularSynth->save(ts);
    
    f.close();
    updateWindowTitle();
    return true;
}


void MainWindow::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this,
            tr("Save patch file"), modularSynth->getPatchPath(),
            tr("AlsaModularSynth patch files") + " (*" + PATCHEXT + ")");

    if (!fn.isEmpty()) {
        /*check for file extension*/
        if (!fn.endsWith(PATCHEXT))
            fn.append(PATCHEXT);

        modularSynth->setPatchPath(fn.left(fn.lastIndexOf('/')));

        fileName = fn;
        saveFile();
    }
    else
        qWarning("%s", tr("Saving aborted").toUtf8().constData());
}


void MainWindow::updateWindowTitle()
{
    QString title = QString("%1 - (%2) - [%3]").
	arg(synthdata->name).
	arg(modularSynth->getSynthDataPoly()).
	arg(fileName.isEmpty() ? tr("noname") : fileName);
    setWindowTitle(title);
}


void MainWindow::closeEvent(QCloseEvent *e)
{
  if (isSave()) {
    modularSynth->clearConfig(false);
    e->accept();
  } else 
    e->ignore();
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::readConfig()
{
    QString s;
    QFile file;

    if (!file.open(rcFd, QIODevice::ReadOnly)) {
        qWarning("%s", "Could not open preferences file.");
        return;
    }
    if (!file.seek(0)) {
        qWarning("%s", "Could not seek start of preferences file.");
        file.close();
        return;
    }
    QTextStream ts(&file);

    while (!ts.atEnd()) {
        s = ts.readLine(); 
        if (s.startsWith("RecentFile"))
            appendRecentlyOpenedFile(s.section(' ', 1), recentFiles);
        else
            modularSynth->loadPreference(s);
    }
    file.close();
    
    modularSynth->refreshColors();
    modularSynth->setPreferencesWidgetColors();
}

void MainWindow::writeConfig()
{
    QFile file;

    if (!file.open(rcFd, QIODevice::WriteOnly)) {
        qWarning("%s", "Could not open preferences file.");
        return;
    }

    if (!file.resize(0)) {
        qWarning("%s", "Could not resize preferences file.");
        file.close();
        return;
    }

    QTextStream ts(&file);
    modularSynth->savePreferences(ts);

    // save recently opened files
    if (recentFiles.count() > 0) {
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            ts << "RecentFile " << *it << endl;
        }
    }
    file.close();
}

void MainWindow::setupRecentFilesMenu()
{
    fileRecentlyOpenedFiles->clear();

    if (recentFiles.count() > 0) {
        fileRecentlyOpenedFiles->setEnabled(true);
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            fileRecentlyOpenedFiles->addAction(*it);
        }
    } else {
        fileRecentlyOpenedFiles->setEnabled(false);
    }
}

void MainWindow::recentFileActivated(QAction *action)
{
    if (!action->text().isEmpty()) {
        if (isSave())
            openFile(action->text());
    }
}

void MainWindow::addRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    if (lst.contains(fi.absoluteFilePath()))
        return;
    if (lst.count() >= 6 )
        lst.removeLast();

    lst.prepend(fi.absoluteFilePath());
}

void MainWindow::appendRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    if (lst.contains(fi.absoluteFilePath()))
        return;
    if (lst.count() >= 6 )
        lst.removeFirst();

    lst.append(fi.absoluteFilePath());
}

#ifdef JACK_SESSION
void MainWindow::handleJackSessionEvent(int jsa)
{
    qWarning("JACK session action: %d", jsa);

    switch (jsa) {
        case -1:
            qApp->closeAllWindows();
            break;
        case SynthData::jsaSave:
        case SynthData::jsaSaveAndQuit:
            fileName = modularSynth->getJackSessionFilename();
            updateWindowTitle();
            if (!saveFile())
                qCritical("JACK session save file error");
            break;
        default:
            qWarning("Unsupported JACK session action: %d", jsa);
            break;
    }
}
#endif
