#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QScrollArea>

#include "mainwindow.h"
#include "synthdata.h"

#include "pixmaps/ams_32.xpm"

/*some constants*/
#define APPNAME  "AlsaModularSynth"
#define PATCHEXT ".ams"


class ScrollArea: public QScrollArea {
  void resizeEvent(QResizeEvent *ev)
  {
    QScrollArea::resizeEvent(ev);
    ((ModularSynth*)widget())->resize();
  }
};


int MainWindow::pipeFd[2];

MainWindow::MainWindow(const ModularSynthOptions& mso)
{
  setWindowIcon(QPixmap(ams_32_xpm));

  /*init synthesizer*/
  modularSynth = new ModularSynth(this, mso);
  if (mso.havePresetPath) {
    qWarning(QObject::tr("Preset path now %1").arg(mso.presetPath).toUtf8()); 
    modularSynth->setLoadPath(mso.presetPath);
  }
  modularSynth->go(mso.enableJack);
  // TODO: check autoload option
  if (mso.havePreset) {
    qWarning(QObject::tr("Loading preset %1").arg(mso.presetName).toUtf8()); 
    openFile(mso.presetName);
  }

  fileName = "";
  updateWindowTitle();

  /*init window*/
  ScrollArea *scrollArea = new ScrollArea();
  scrollArea->setWidget(modularSynth);
  setCentralWidget(scrollArea);

  QObject::connect(qApp, SIGNAL(aboutToQuit()), modularSynth,
          SLOT(cleanUpSynth()));

  QMenu *filePopup = menuBar()->addMenu(tr("&File"));
  QMenu *synthesisPopup = menuBar()->addMenu(tr("&Synthesis"));
  QMenu *modulePopup = menuBar()->addMenu(tr("&Module"));
  QMenu *midiMenu = menuBar()->addMenu(tr("&View"));
  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

  QMenu *newModulePopup = modulePopup->addMenu(tr("&New"));
  modularSynth->contextMenu = newModulePopup;

  filePopup->addAction(tr("&New"), this, SLOT(fileNew()));
  filePopup->addAction(tr("&Open..."), this, SLOT(fileOpen()));
  filePopup->addAction(tr("&Save"), this, SLOT(fileSave()));
  filePopup->addAction(tr("Save &as..."), this, SLOT(fileSaveAs()));
  filePopup->addSeparator();
  filePopup->addAction(tr("&Load Colors"), modularSynth, SLOT(loadColors()));
  filePopup->addAction(tr("&Save Colors"), modularSynth, SLOT(saveColors()));
  filePopup->addSeparator();
  filePopup->addAction(tr("&Quit"), qApp, SLOT(closeAllWindows()));

  synthesisPopup->addAction(tr("&Start"), modularSynth, SLOT(startSynth()));
  synthesisPopup->addAction(tr("Sto&p"), modularSynth, SLOT(stopSynth()));
  synthesisPopup->addAction(tr("&All Voices Off"), modularSynth,
          SLOT(allVoicesOff()));

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

  modulePopup->addAction(tr("&Show Ladspa Browser..."), modularSynth, SLOT(displayLadspaPlugins()));

  midiMenu->addAction(tr("&Control Center..."), modularSynth,
          SLOT(displayMidiController()));
  midiMenu->addAction(tr("&Parameter View..."), modularSynth,
          SLOT(displayParameterView()));
  midiMenu->addAction(tr("Pre&ferences..."), modularSynth,
          SLOT(displayPreferences()));

  helpMenu->addAction(tr("&About AlsaModularSynth..."), modularSynth,
          SLOT(displayAbout()));

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

  if (mso.noGui)
      hide();
  else
      show();
}

void MainWindow::sighandler(int s)
{
  char pipeMessage = s;
  write(pipeFd[1], &pipeMessage, 1);
}

void MainWindow::unixSignal(int /*fd*/)
{
    //char pipeIn[4];
    //int pipeRed = read(fd, pipeIn, sizeof(pipeIn));
    //StdOut << __PRETTY_FUNCTION__ << (int)pipeIn[0]  << endl;
    //qApp->quit();
    qApp->closeAllWindows();
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
        QString(tr("Open patch file")), modularSynth->getLoadPath(),
        QString(tr("Patch files")) + " (*" + PATCHEXT + ")");

    if (fn.isEmpty())
        return;
    else
        openFile(fn);
}


void MainWindow::fileNew()
{           
    if (isModified()) {
        int choice = querySaveChanges();
        switch (choice) {
            case 0: 
                if (saveFile())
                    newFile();
                break;
            case 1:
                newFile();
                break;
            case 2: 
            default: 
                break;
        }
    }       
    else
        newFile();
}

void MainWindow::newFile()
{  
    modularSynth->clearConfig();
 
    fileName = "";
    updateWindowTitle();
    qWarning(tr("New patch file created").toUtf8());
}


void MainWindow::fileOpen()
{
    if (isModified()) {
        int choice = querySaveChanges();
        switch (choice) {
            case 0:
                if (saveFile())
                    chooseFile();
                break;
            case 1:
                chooseFile();
                break;
            case 2:
            default:
                break;
        }
    }
    else
        chooseFile();
}


void MainWindow::openFile(const QString& fn)
{
    modularSynth->setLoadPath(fn.left(fn.lastIndexOf('/')));

    QFile f(fn);

    if (!f.open(QIODevice::ReadOnly)) {
        qWarning(tr("Could not read file '%1'").arg(fn).toUtf8());
        return;
    }

    modularSynth->clearConfig();
    fileName = fn;
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    modularSynth->load(ts);
    f.close();
    
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
        qWarning(tr("Could not open file '%1'").arg(fileName).toUtf8());
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
            QString(tr("Save patch file")), modularSynth->getSavePath(),
            QString(tr("Patch files")) + " (*" + PATCHEXT + ")");

    if (!fn.isEmpty()) {
        /*check for file extension*/
        if (!fn.endsWith(PATCHEXT))
            fn.append(PATCHEXT);

        modularSynth->setSavePath(fn.left(fn.lastIndexOf('/')));

        fileName = fn;
        saveFile();
    }
    else
        qWarning(tr("Saving aborted").toUtf8());
}


void MainWindow::updateWindowTitle()
{
    int poly = modularSynth->getSynthDataPoly();

    if (fileName.isEmpty())
        setWindowTitle(QString(APPNAME) + QString(" - (%1) - ").arg(poly) +
                " [" + tr("noname") + "]");
    else
        setWindowTitle(QString(APPNAME) + QString(" - (%1) - ").arg(poly) +
                " [" + fileName + "]");
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    if (!isModified())
        e->accept();
    else {
        int choice = querySaveChanges();
        switch (choice) {
            case 0:
                if (saveFile())
                    e->accept();
                else 
                    e->ignore();
                break;
            case 1:
                e->accept();
                break;
            case 2:
            default:
                e->ignore();
                break;
        }
    }
}

