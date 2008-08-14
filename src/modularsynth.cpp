#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <QAbstractScrollArea>
#include <qcolordialog.h>
#include <qsocketnotifier.h>
#include <QFileDialog>
#include <qpen.h>
#include <qstringlist.h>
#include <QPainterPath>
#include <QMouseEvent>
#include <QMenu>
#include <QPaintEvent>
#include <QStringList>
#include <QTextBlock>
#include <QTextEdit>
#include "midislider.h"
#include "intmidislider.h"
#include "midicombobox.h"
#include "midicheckbox.h"
#include "modularsynth.h"
#include "port.h"
#include "midiwidget.h"
#include "guiwidget.h"
#include "midicontroller.h"
#include "ladspadialog.h"
#include "textedit.h"
#include "m_spectrum.h"
#include "m_vco.h"
#include "m_vco2.h"
#include "m_vca.h"
#include "m_delay.h"
#include "m_seq.h"
#include "m_env.h"
#include "m_vcenv.h"
#include "m_vcenv2.h"
#include "m_advenv.h"
#include "m_mcv.h"
#include "m_advmcv.h"
#include "m_scmcv.h"
#include "m_ringmod.h"
#include "m_mix.h"
#include "m_stereomix.h"
#include "m_pcmout.h"
#include "m_pcmin.h"
#include "m_wavout.h"
#include "m_lfo.h"
#include "m_noise.h"
#include "m_noise2.h"
#include "m_ladspa.h"
#include "m_vcf.h"
#include "m_inv.h"
#include "m_amp.h"
#include "m_ad.h"
#include "m_vquant.h"
#include "m_conv.h"
#include "m_cvs.h"
#include "m_vcswitch.h"
#include "m_slew.h"
#include "m_sh.h"
#include "m_vcpanning.h"
#include "m_midiout.h"
#include "m_scope.h"
#include "m_vcorgan.h"
#include "m_dynamicwaves.h"
#include "m_quantizer.h"
#include "m_scquantizer.h"
#include "m_function.h"
#include "m_vcdoubledecay.h"
#include "m_mphlfo.h"
#include "prefwidget.h"

SynthData *synthdata;

ModularSynth::ModularSynth(QWidget* parent, const ModularSynthOptions& mso) 
  : QWidget(parent)
  , pcmname(mso.pcmname)
  , fsamp(mso.fsamp)
  , frsize(mso.frsize)
  , nfrags(mso.nfrags)
  , ncapt(mso.ncapt)
  , nplay(mso.nplay)
  , paintFastly(false)
  , _zoomFactor(1.0)
  , rcFd(mso.rcFd)
{
  firstPort = true;
  modified = false;
  connectingPort[0] = NULL;
  connectingPort[1] = NULL;
  connectorStyle = CONNECTOR_BEZIER;
  aboutWidget = new QMessageBox(parent); 

  synthdata = new SynthData(mso.synthName, mso.poly, mso.edge);

  midiWidget = new MidiWidget(NULL);
  midiWidget->setWindowTitle(tr("AlsaModularSynth Control Center"));
  synthdata->midiWidget = midiWidget;
  
  guiWidget = new GuiWidget(NULL);
  guiWidget->setWindowTitle(tr("AlsaModularSynth Parameter View"));
  synthdata->guiWidget = guiWidget;
  
  prefWidget = new PrefWidget();
  prefWidget->setWindowTitle(tr("AlsaModularSynth Preferences"));
  QObject::connect(prefWidget, SIGNAL(prefChanged()),
                   this, SLOT(refreshColors()));

  ladspaDialog = new LadspaDialog();
  QObject::connect(static_cast<QWidget *>(ladspaDialog),
		   SIGNAL(createLadspaModule(int, int, bool, bool)),
                   this, SLOT(newM_ladspa(int, int, bool, bool)));

  setPalette(QPalette(QColor(240, 240, 255), QColor(240, 240, 255)));
  loadingPatch = false;

  prefWidget->loadPref(rcFd);
  refreshColors();
}

ModularSynth::~ModularSynth()
{
  synthdata->stopPCM();
  synthdata->midiWidget->clearAllClicked();
  for (int l1 = 0; l1 < listModule.count(); ++l1)
    deleteModule(listModule.at(l1));

  listModule.clear();

  delete synthdata;
}

void ModularSynth::resize()
{
//   cout << childrenRect().right() << childrenRect().bottom() << endl;
  int width = std::max(childrenRect().right() + 30,
          ((QWidget*)parent())->width());
  int height = std::max(childrenRect().bottom() + 10,
          ((QWidget*)parent())->height());
  QWidget::resize(width, height);
}

void ModularSynth::paintEvent(QPaintEvent *) {
  
  QPainter p(this);
  int l1, l2;
  Port *port[2];
  int port_x[2], port_y[2];
  QPoint port_pos[2];
  QColor cableColor, jackColor;

  if (!paintFastly)
    p.setRenderHint(QPainter::Antialiasing);

  p.setPen(jackColor.light(170));
  for (l1 = 0; l1 < listModule.count(); ++l1) {
    for (l2 = 0; l2 < listModule.at(l1)->portList.count(); ++l2) {
      port[0] = listModule.at(l1)->portList.at(l2);
      cableColor = port[0]->cableColor;
      jackColor = port[0]->jackColor;
      if ((port[0]->dir == PORT_IN) && port[0]->connectedPortList.count()) {
        port[1] = port[0]->connectedPortList.at(0);
        port_pos[0] = port[0]->pos();
        port_pos[1] = port[1]->pos();
	port_x[0] = port_pos[0].x() + port[0]->module->x() - 10;
        port_x[1] = port_pos[1].x() + port[1]->width() + port[1]->module->x() + 10;
        port_y[0] = port_pos[0].y() + port[0]->module->y() + port[0]->height()/2;
        port_y[1] = port_pos[1].y() + port[1]->module->y() + port[1]->height()/2;
        if (connectorStyle == CONNECTOR_BEZIER) {
	  QPen pen;
	  QPainterPath path;
	  int xShift = 30;
	  if (port_x[1] > port_x[0])
	    xShift += (port_x[1] - port_x[0]) >> 3;
	  path.moveTo(port_x[0], port_y[0]);
	  path.cubicTo(port_x[0] - xShift, port_y[0] + 3,
		       port_x[1] + xShift, port_y[1] + 3,
		       port_x[1], port_y[1]);
          pen.setWidth(5);
          pen.setColor(cableColor.dark(120));
	  p.strokePath(path, pen);

          pen.setWidth(3);
          pen.setColor(cableColor);
	  p.strokePath(path, pen);

          pen.setWidth(1);
          pen.setColor(cableColor.light(120));
	  p.strokePath(path, pen);

          p.fillRect(port_x[0], port_y[0] - 3, 11, 7, QBrush(jackColor.dark(120)));
          p.fillRect(port_x[1] - 11, port_y[1] - 3, 11, 7, QBrush(jackColor.dark(120)));
          p.fillRect(port_x[0], port_y[0] - 2, 11, 5, QBrush(jackColor));
          p.fillRect(port_x[1] - 11, port_y[1] - 2, 11, 5, QBrush(jackColor));
          p.fillRect(port_x[0], port_y[0] - 1, 11, 3, QBrush(jackColor.light(120)));
          p.fillRect(port_x[1] - 11, port_y[1] - 1, 11, 3, QBrush(jackColor.light(120)));
	  //          pen.setWidth(1);
	  //          pen.setColor(jackColor.light(170));
	  //          p.setPen(pen);
//           p.drawLine(port_x[0], port_y[0], port_x[0] + 11, port_y[0]);
//           p.drawLine(port_x[1] - 11, port_y[1], port_x[1], port_y[1]);
        }
      }
    }
  }
}
 
void ModularSynth::mousePressEvent(QMouseEvent *ev)
{
  
  switch (ev->button()) {
  case Qt::LeftButton:  
    break;
  case Qt::RightButton:
    newBoxPos = ev->pos();
    showContextMenu(newBoxPos);  
    break;
  case Qt::MidButton:
    break;
  default:
    break;
  }
}  
   
void ModularSynth::mouseReleaseEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    break;
  case Qt::RightButton:
    break;
  case Qt::MidButton:
//    connectorStyle = (connectorStyle == CONNECTOR_STRAIGHT) ? CONNECTOR_BEZIER : CONNECTOR_STRAIGHT;
//    repaintContents(false);
    break;
  default:
    break;
  }
}  

int ModularSynth::go(bool withJack)
{
  if ((synthdata->seq_handle = open_seq()))
    initSeqNotifier();
  else
    qWarning(QObject::tr(" MIDI wont work!").toUtf8());

  midiWidget->setActiveMidiControllers();

  if (withJack) {
    synthdata->initJack(ncapt, nplay);
    synthdata->doSynthesis = true;
  } else {
    synthdata->initAlsa(pcmname.toLocal8Bit(), fsamp, frsize, nfrags, ncapt, nplay);
    synthdata->doSynthesis = true;
  }
  return 0;
}

void ModularSynth::displayAbout() {
 
    aboutWidget->about(this, "About " AMS_LONGNAME, AMS_LONGNAME " " AMS_VERSION 
                     "\nby Matthias Nagorni and Fons Adriaensen\n"
                     "(c)2002-2003 SuSE AG Nuremberg\n"
                     "(c)2003 Fons Adriaensen\n\n"
		     "additional programming:\n"
		     "2007 Malte Steiner\n"
		     "2007 Karsten Wiese\n\n"
                     "Documentation and examples can be found in\n"
                     "/usr/share/doc/packages/kalsatools\n\n"
                     "More presets and updates are available from\n"
                     "http://alsamodular.sourceforge.net"
    "\n\nAcknowledgements\n"
    "----------------------\n\n"
    "The VCF Module uses the resonant low-pass filter by Paul Kellett\n" 
    "and the Cookbook formulae for audio EQ biquad filter coefficients\n"
    "by Robert Bristow-Johnson. The experimental 24 dB Lowpass filters have\n" 
    "been taken from http://musicdsp.org. They are based on the CSound source\n"
    "code, the paper by Stilson/Smith and modifications by Paul Kellett\n" 
    "and Timo Tossavainen. The pink noise conversion formula is by Paul\n" 
    "Kellett and has been taken from http://musicdsp.org as well.\n\n"
    "The author is grateful to Takashi Iwai for instructions about ALSA.\n"
    "Klaas Freitag, Helmut Herold, Stefan Hundhammer and Arvin Schnell\n"
    "answered many questions about Qt. Thanks to J�rg Arndt for valuable\n"
    "hints regarding speed optimization. Torsten Rahn has helped to\n" 
    "improve the color scheme of the program. Thanks to Bernhard Kaindl\n"
    "for helpful discussion.\n");
    aboutWidget->raise();
}

void ModularSynth::displayMidiController()
{
  if (!midiWidget->isVisible())
    midiWidget->show();
  else {
    midiWidget->raise();
    midiWidget->activateWindow();
  }
}

void ModularSynth::displayParameterView() {

  guiWidget->show();
  guiWidget->raise();
}

void ModularSynth::displayPreferences() {

  prefWidget->show();
  prefWidget->raise();
}

void ModularSynth::displayLadspaPlugins() {
   
  ladspaDialog->show();  
  ladspaDialog->raise();
}

int ModularSynth::getSynthDataPoly()
{
    return synthdata->poly;
}

snd_seq_t *ModularSynth::open_seq() {

  snd_seq_t *seq_handle;
  int l1;
  QString qs;

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK) < 0) {
    qWarning(QObject::tr("Error opening ALSA sequencer.").toUtf8());
    return NULL;
  }

  snd_seq_set_client_name(seq_handle, (synthdata->name + " Midi").toLatin1().constData());
  if (snd_seq_create_simple_port(seq_handle, "in",
				 SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
				 SND_SEQ_PORT_TYPE_APPLICATION) < 0) {
    qWarning(QObject::tr("Error creating sequencer write port.").toUtf8());
    snd_seq_close(seq_handle);
    return NULL;
  }
  for (l1 = 0; l1 < 2; ++l1)
    if ((synthdata->midi_out_port[l1] = snd_seq_create_simple_port(seq_handle, "out",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      qWarning(QObject::tr("Error creating sequencer read port.").toUtf8());
      snd_seq_close(seq_handle);
      return NULL;
    }

  return seq_handle;
}

int ModularSynth::initSeqNotifier()
{

//   int alsaEventFd = 0;

//   struct pollfd pfd[1];
//   snd_seq_poll_descriptors(synthdata->seq_handle, pfd, 1, POLLIN);
//   alsaEventFd = pfd[0].fd;
  seqNotifier = new QSocketNotifier(synthdata->pipeFd[0], QSocketNotifier::Read, this);
  QObject::connect(seqNotifier, SIGNAL(activated(int)),
		   this, SLOT(midiAction(int)));
  return(0);
}

void ModularSynth::midiAction(int fd)
{
  char pipeIn[16];

  ssize_t pipeRed = read(fd, pipeIn, sizeof(pipeIn));

  if (pipeRed < 0) {
    StdErr << __PRETTY_FUNCTION__ << ": read() " << endl;
    perror(NULL);
    exit(-1);
  }
  if (pipeRed < 1 || pipeRed >= (ssize_t)sizeof(pipeIn))
    StdErr << __PRETTY_FUNCTION__ << ": read() " << pipeRed << " bytes" << endl;
  if (pipeRed == 0)
    return;

  while (pipeRed > 1)
    pipeIn[0] |= pipeIn[--pipeRed];

  MidiControllableBase *mcAble = NULL;
  for (int mCs = synthdata->mcSet.count(); mCs; --mCs) {
    mcAble = synthdata->mcSet.get();
    mcAble->updateMGCs(NULL);
  }
  if (mcAble)
    midiWidget->midiTouched(*mcAble);

  for (int mcKs = synthdata->mckRed.count();
       mcKs; --mcKs) {
    MidiControllerKey mcK = synthdata->mckRed.get();
    if (midiWidget->isVisible()) {
      if (mcK.type() == SND_SEQ_EVENT_CONTROLLER ||
	  mcK.type() == SND_SEQ_EVENT_CONTROL14 ||
	  mcK.type() == SND_SEQ_EVENT_PITCHBEND ||
	  (midiWidget->noteControllerEnabled &&
	   (mcK.type() == SND_SEQ_EVENT_NOTEON ||
	    mcK.type() == SND_SEQ_EVENT_NOTEOFF)))
	midiWidget->addMidiController(mcK);
    }
  }

  if (pipeIn[0] & 4)
    guiWidget->setCurrentPresetText();

  return;

//   snd_seq_event_t *ev;
//   QString qs;
//   int l1, l2, osc, noteCount;
//   bool foundOsc;

//   do {
//     snd_seq_event_input(synthdata->seq_handle, &ev);
//     MidiController midiController(ev); 
//     if (midiWidget->isVisible()) {
//       if (ev->type == SND_SEQ_EVENT_CONTROLLER ||
// 	  ev->type == SND_SEQ_EVENT_CONTROL14 ||
// 	  ev->type == SND_SEQ_EVENT_PITCHBEND)
// 	midiWidget->addMidiController(midiController);

//       if (midiWidget->noteControllerEnabled &&
// 	  (ev->type == SND_SEQ_EVENT_NOTEON || ev->type == SND_SEQ_EVENT_NOTEOFF))
// 	midiWidget->addMidiController(midiController);
//     }

//     const MidiController *c = midiWidget->midiController(midiController);
//     if (c) {
//       int value;
//       switch (c->type()) {
//       case SND_SEQ_EVENT_PITCHBEND:
// 	value = (ev->data.control.value + 8192) >> 7;
// 	break;
//       case SND_SEQ_EVENT_CONTROL14:
// 	value = ev->data.control.value >> 7;
// 	break;
//       case SND_SEQ_EVENT_CONTROLLER:
// 	value = ev->data.control.value;
// 	break;
//       case SND_SEQ_EVENT_NOTEON:
// 	value = ev->data.note.velocity;
// 	break;
//       default:
//       case SND_SEQ_EVENT_NOTEOFF:
// 	value = 0;
// 	break;
//       }
//       //!!      emit c->context->sendMidiValue(value);
//       if (midiWidget->followMidi)
// 	midiWidget->setSelectedController(*c);
//     }

// // Voice assignment

//     if ((ev->type == SND_SEQ_EVENT_NOTEON || ev->type == SND_SEQ_EVENT_NOTEOFF) &&
// 	(synthdata->midiChannel < 0 || synthdata->midiChannel == ev->data.control.channel))
//       if ((ev->type == SND_SEQ_EVENT_NOTEON) && (ev->data.note.velocity > 0)) {

// // Note On: Search for oldest voice to allocate new note.          
          
//           osc = 0;
//           noteCount = 0;
//           foundOsc = false;
//           for (l2 = 0; l2 < synthdata->poly; ++l2)
//             if (synthdata->noteCounter[l2] > noteCount) {
//               noteCount = synthdata->noteCounter[l2];
//               osc = l2;
//               foundOsc = true;
//             }

//           if (foundOsc) {
//             synthdata->noteCounter[osc] = 0;
//             synthdata->sustainNote[osc] = false;
//             synthdata->velocity[osc] = ev->data.note.velocity;
//             synthdata->channel[osc] = ev->data.note.channel;
//             synthdata->notes[osc] = ev->data.note.note;
//           }  
//       } else {
      
// // Note Off      
      
//         for (l2 = 0; l2 < synthdata->poly; ++l2)
//           if ((synthdata->notes[l2] == ev->data.note.note)
//             && (synthdata->channel[l2] == ev->data.note.channel))
//             if (synthdata->sustainFlag)
//               synthdata->sustainNote[l2] = true;
//             else
//               synthdata->noteCounter[l2] = 1000000; 

//       }       

    
//     if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
//       if (ev->data.control.param == MIDI_CTL_ALL_NOTES_OFF)
//         for (l2 = 0; l2 < synthdata->poly; ++l2)
//           if ((synthdata->noteCounter[l2] < 1000000) && (synthdata->channel[l2] == ev->data.note.channel))
//             synthdata->noteCounter[l2] = 1000000;

//       if (ev->data.control.param == MIDI_CTL_SUSTAIN) {
//         synthdata->sustainFlag = ev->data.control.value > 63;
//         if (!synthdata->sustainFlag)
//           for (l2 = 0; l2 < synthdata->poly; ++l2)
//             if (synthdata->sustainNote[l2])
//               synthdata->noteCounter[l2] = 1000000;
//       }
//     }
//     if (ev->type == SND_SEQ_EVENT_PGMCHANGE)
//       guiWidget->setCurrentPreset(ev->data.control.value);

//     for (l1 = 0; l1 < synthdata->listM_advmcv.count(); ++l1)
//       switch (ev->type) {
//         case SND_SEQ_EVENT_CHANPRESS: 
//           synthdata->listM_advmcv.at(l1)->aftertouchEvent(ev->data.control.value);
//           break;
//         case SND_SEQ_EVENT_PITCHBEND:
//           synthdata->listM_advmcv.at(l1)->pitchbendEvent(ev->data.control.value); 
//           break;
//         case SND_SEQ_EVENT_CONTROLLER: 
//           synthdata->listM_advmcv.at(l1)->controllerEvent(ev->data.control.param, ev->data.control.value);
//           break;
//       }

//     snd_seq_free_event(ev);
//   } while (snd_seq_event_input_pending(synthdata->seq_handle, 0) > 0);
}

void ModularSynth::initPorts(Module *m) {

  int l1;

  for (l1 = 0; l1 < m->portList.count(); ++l1) {
    QObject::connect(m->portList.at(l1), SIGNAL(portClicked()), 
                     this, SLOT(portSelected()));
    QObject::connect(m->portList.at(l1), SIGNAL(portDisconnected()), 
                     this, SLOT(redrawPortConnections()));
  }
}

/* redraws complete module area when a module connection is changed*/
void ModularSynth::redrawPortConnections()
{
    update();
    modified = true;
}

void ModularSynth::initNewModule(Module *m)
{
  m->move(newBoxPos);
  m->show();

  QObject::connect(m, SIGNAL(removeModule()), this, SLOT(deleteModule()));
  listModule.append(m);
  if (!loadingPatch) {
    midiWidget->addModule(m);
  }
  initPorts(m);
  modified = true;
}

void ModularSynth::new_textEdit()
{
  TextEdit *te = new TextEdit(this, "textEdit");
  te->move(newBoxPos);
  te->show();
  //  QObject::connect(te, SIGNAL(dragged(QPoint)), this, SLOT(moveTextEdit(QPoint)));
  QObject::connect(te, SIGNAL(sizeDragged(QPoint)), this, SLOT(resizeTextEdit(QPoint)));
  QObject::connect(te, SIGNAL(removeTextEdit()), this, SLOT(deleteTextEdit()));
  listTextEdit.append(te);
  modified = true;
}

void ModularSynth::new_textEdit(int w, int h) {

  TextEdit *te = new TextEdit(this, "textEdit");
  te->setFixedSize(w, h);
  te->move(newBoxPos);
  te->show();
  //  QObject::connect(te, SIGNAL(dragged(QPoint)), this, SLOT(moveTextEdit(QPoint)));
  QObject::connect(te, SIGNAL(sizeDragged(QPoint)), this, SLOT(resizeTextEdit(QPoint)));
  QObject::connect(te, SIGNAL(removeTextEdit()), this, SLOT(deleteTextEdit()));
  listTextEdit.append(te);
}

void ModularSynth::startSynth()
{
  synthdata->doSynthesis = true;
}

void ModularSynth::stopSynth()
{
  synthdata->doSynthesis = false;
}
//################################################ start add new modules
void ModularSynth::newM_seq(int seqLen) {

    M_seq *m = new M_seq(seqLen, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_seq_8() {

    newM_seq(8);
}

void ModularSynth::newM_seq_12() {

    newM_seq(12);
}

void ModularSynth::newM_seq_16() {

    newM_seq(16);
}

void ModularSynth::newM_seq_24() {

    newM_seq(24);
}

void ModularSynth::newM_seq_32() {

    newM_seq(32);
}

void ModularSynth::newM_vcorgan(int oscCount) {

    M_vcorgan *m = new M_vcorgan(oscCount, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vcorgan_4() {

    newM_vcorgan(4);
}

void ModularSynth::newM_vcorgan_6() {

    newM_vcorgan(6);
}

void ModularSynth::newM_vcorgan_8() {

    newM_vcorgan(8);
}

void ModularSynth::newM_dynamicwaves(int oscCount) {

    M_dynamicwaves *m = new M_dynamicwaves(oscCount, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_dynamicwaves_4() {

    newM_dynamicwaves(4);
}

void ModularSynth::newM_dynamicwaves_6() {

    newM_dynamicwaves(6);
}

void ModularSynth::newM_dynamicwaves_8() {

    newM_dynamicwaves(8);
}

void ModularSynth::newM_mcv() {

    M_mcv *m = new M_mcv(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_advmcv() {

    M_advmcv *m = new M_advmcv(this);
    if (m != NULL) {
        synthdata->listM_advmcv.append(m);
        initNewModule(m);
    }
}

void ModularSynth::newM_scmcv() {

    M_scmcv *m = new M_scmcv(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_scmcv(QString *p_scalaName) {

    M_scmcv *m = new M_scmcv(this, p_scalaName);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_env() {

    M_env *m = new M_env(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vcenv() {

    M_vcenv *m = new M_vcenv(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vcenv2() {

    M_vcenv2 *m = new M_vcenv2(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vcdoubledecay() {

    M_vcdoubledecay *m = new M_vcdoubledecay(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vcpanning() {

    M_vcpanning *m = new M_vcpanning(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_advenv() {

    M_advenv *m = new M_advenv(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vco() {

    M_vco *m = new M_vco(this);
    if (m != NULL)
        initNewModule(m);
}
void ModularSynth::newM_vco2() {

    M_vco2 *m = new M_vco2(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vca_lin() {

    M_vca *m = new M_vca(false, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vca_exp() {

    M_vca *m = new M_vca(true, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_lfo() {

    M_lfo *m = new M_lfo(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_mphlfo() {

    M_mphlfo *m = new M_mphlfo(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_noise() {

    M_noise *m = new M_noise(this);
    if (m != NULL)
        initNewModule(m);
}
void ModularSynth::newM_noise2() {

    M_noise2 *m = new M_noise2(this);
    if (m != NULL)
        initNewModule(m);
}
void ModularSynth::newM_ringmod() {

    M_ringmod *m = new M_ringmod(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_inv() {

    M_inv *m = new M_inv(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_amp() {

    M_amp *m = new M_amp(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_ad(int outCount) {

    M_ad *m = new M_ad(outCount, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_ad_2() { 

    newM_ad(2);
}

void ModularSynth::newM_ad_4() { 

    newM_ad(4);
}

void ModularSynth::newM_ad_6() { 

    newM_ad(6);
}

void ModularSynth::newM_vquant() {

    M_vquant *m = new M_vquant(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_conv() {

    M_conv *m = new M_conv(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_cvs() {

    M_cvs *m = new M_cvs(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_sh() {

    M_sh *m = new M_sh(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_vcswitch() {

    M_vcswitch *m = new M_vcswitch(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_slew() {

    M_slew *m = new M_slew(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_quantizer() {

    M_quantizer *m = new M_quantizer(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_scquantizer(QString *p_scalaName) {

    M_scquantizer *m = new M_scquantizer(this, p_scalaName);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_scquantizer() {

    M_scquantizer *m = new M_scquantizer(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_delay() {

    M_delay *m = new M_delay(this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_mix(int in_channels) {

    M_mix *m = new M_mix(in_channels, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_function(int functionCount) {

    M_function *m = new M_function(functionCount, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_function_1() {

    newM_function(1);
}

void ModularSynth::newM_function_2() {

    newM_function(2);
}

void ModularSynth::newM_function_4() {

    newM_function(4);
}

void ModularSynth::newM_stereomix(int in_channels) {

    M_stereomix *m = new M_stereomix(in_channels, this);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_mix_2() {

    newM_mix(2);
}

void ModularSynth::newM_mix_4() {

    newM_mix(4);
}

void ModularSynth::newM_mix_8() {

    newM_mix(8);
}

void ModularSynth::newM_stereomix_2() {

    newM_stereomix(2);
}

void ModularSynth::newM_stereomix_4() {

    newM_stereomix(4);
}

void ModularSynth::newM_stereomix_8() {

    newM_stereomix(8);
}

void ModularSynth::newM_ladspa(int p_ladspaDesFuncIndex, int n, bool p_newLadspaPoly, bool p_extCtrlPorts) {

    QString qs;

    qs = synthdata->ladspaLib.at(p_ladspaDesFuncIndex).desc.at(n)->Name;
    M_ladspa *m = new M_ladspa(this, p_ladspaDesFuncIndex, n,
            p_newLadspaPoly, p_extCtrlPorts);
    if (m != NULL)
        initNewModule(m);
}

void ModularSynth::newM_wavout() {

    M_wavout *m = new M_wavout(this);
    if (m != NULL) {
        synthdata->wavoutModuleList.append(m);
        initNewModule(m);
    }
}

void ModularSynth::newM_midiout() {

    M_midiout *m = new M_midiout(this);
    if (m != NULL) {
        synthdata->midioutModuleList.append(m);
        initNewModule(m);
    }
}

void ModularSynth::newM_pcmout()
{
    int k;
    M_pcmout *m; 

    k = synthdata->find_play_mod (0);
    if (k >= 0) 
    {
        m = new M_pcmout (this, 2 * k);
        if (m != NULL) {
            initNewModule(m);
            synthdata->set_play_mod (k, m);
        }
    } 
    else qWarning(QObject::tr("All available output ports are in use").toUtf8());
}

void ModularSynth::newM_pcmin()
{
    int k;
    M_pcmin *m; 

    k = synthdata->find_capt_mod (0);
    if (k >= 0) 
    {
        m = new M_pcmin(this, 2 * k);
        if (m != NULL) {
            initNewModule(m);
            synthdata->set_capt_mod (k, m);
        }
    } 
    else qWarning(QObject::tr("All available input ports are in use").toUtf8());
}

void ModularSynth::newM_scope() {

    M_scope *m = new M_scope(this);
    if (m != NULL) {
        synthdata->scopeModuleList.append(m);
        initNewModule(m);
    }
}

void ModularSynth::newM_spectrum() {

    M_spectrum *m = new M_spectrum(this);
    if (m != NULL) {
#ifdef OUTDATED_CODE
        synthdata->spectrumModuleList.append(m);
#endif
        initNewModule(m);
    }
}

void ModularSynth::newM_vcf() {

    M_vcf *m = new M_vcf(this);
    if (m != NULL)
        initNewModule(m);
}
//========================================== End of adding module functions

void ModularSynth::resizeTextEdit(QPoint pos) {

  int l1;
  TextEdit *te;
  int cx, cy;
  
  for (l1 = 0; l1 < listTextEdit.count(); ++l1) {
    if ((te=listTextEdit.at(l1)) == sender()) {
      cx = pos.x();
      cy = pos.y();
      if ((cx > 200) && (cy > 170)) { 
        te->setFixedSize(cx + 3, cy + 3);
      }
    }
  }
}
// selecting and connecting ports:
void ModularSynth::portSelected() {

  if (firstPort) {
    firstPort = false;
    connectingPort[0] = (Port *)sender();
    connectingPort[0]->highlighted = true;
    connectingPort[0]->update();
  } else {
    firstPort = true;
    connectingPort[1] = (Port *)sender();
    connectingPort[0]->highlighted = false;  
    connectingPort[0]->update();
    //connectingPort[0]->cableColor = LastCableColor;  
    connectingPort[1]->highlighted = false;
    connectingPort[1]->update();
    if ((((connectingPort[0]->dir == PORT_IN) &&
                    (connectingPort[1]->dir == PORT_OUT))
                || ((connectingPort[1]->dir == PORT_IN) &&
                    (connectingPort[0]->dir == PORT_OUT)))
            && (connectingPort[0]->module != connectingPort[1]->module)) {
        connectingPort[0]->connectTo(connectingPort[1]);
        connectingPort[1]->connectTo(connectingPort[0]);
        redrawPortConnections();
    } else {
      qWarning(QObject::tr("Connection refused.").toUtf8());
      connectingPort[0] = NULL;
      connectingPort[1] = NULL;
    }
  } 
}

void ModularSynth::deleteModule() {

  Module *m;
 
  connectingPort[0] = NULL;
  connectingPort[1] = NULL;
  firstPort = true;
  m = (Module *)sender();
  listModule.removeAll(m);
  deleteModule(m);
  modified = true;
}

void ModularSynth::deleteTextEdit() {

  listTextEdit.removeAll((TextEdit *)sender());
  delete((TextEdit *)sender());
}

void ModularSynth::deleteTextEdit(TextEdit *te) {

  delete(te);
}

void ModularSynth::deleteModule(Module *m)
{
  if (m->M_type == M_type_pcmout)
  {
      int k = synthdata->find_play_mod (m);
      if (k >= 0) synthdata->set_play_mod (k, 0);
  }
  if (m->M_type == M_type_pcmin)
  {
      int k = synthdata->find_capt_mod (m);
      if (k >= 0) synthdata->set_capt_mod (k, 0);
  }
#ifdef OUTDATED_CODE
  if (m->M_type == M_type_spectrum) {
    synthdata->spectrumModuleList.remove((QObject *)m);
  }
#endif
  delete(m);
}

bool ModularSynth::clearConfig(bool restart)
{
  int l1;

  bool restartSynth = synthdata->doSynthesis;
  synthdata->doSynthesis = false;
  if (restartSynth)
    sleep(1);

  synthdata->initVoices();
  guiWidget->clearGui();
  for (l1 = 0; l1 < listModule.count(); ++l1) {
    deleteModule(listModule.at(l1));
  }
  listModule.clear();
  for (l1 = 0; l1 < listTextEdit.count(); ++l1) {
    deleteTextEdit(listTextEdit.at(l1));
  }
  listTextEdit.clear();
  synthdata->moduleID = 0;
  synthdata->moduleCount = 0;
  if (restartSynth && restart)
    synthdata->doSynthesis = true;

  update();
  return restartSynth;
}

void ModularSynth::clearConfig()
{
  clearConfig(true);
}

//#################################################### Start persistence
static int Fscanf(FILE *f, const char *format, ...)
{
   va_list argList;
   va_start(argList, format);

   return vfscanf(f, format, argList);
}

static void setColor(FILE *f, QColor &c)
{
  int r, g, b;
  Fscanf(f, "%d", &r);   
  Fscanf(f, "%d", &g);   
  Fscanf(f, "%d", &b);   
  c = QColor(r, g, b);
}

void ModularSynth::loadColors() {

  QString config_fn, qs;
  FILE *f;
  char sc[2048];

  config_fn = QFileDialog::getOpenFileName(this, tr("Load Colors"),
					   synthdata->loadPath,
                                           tr("AlsaModularSynth Color files (*.acs)"));
  if (config_fn.isEmpty())
    return;

  if (!(f = fopen(config_fn.toLatin1().constData(), "r"))) {
    QMessageBox::information( this, "AlsaModularSynth",
            tr("Could not open file."));
  } else {
    while(Fscanf(f, "%s", sc) != EOF) {
      qs = QString(sc);
      if (qs.contains("ColorBackground", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorBackground);
      }        
      else if (qs.contains("ColorModuleBackground", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorModuleBackground);
      }        
      else if (qs.contains("ColorModuleBorder", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorModuleBorder);
      }        
      else if (qs.contains("ColorModuleFont", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorModuleFont);
      }        
      else if (qs.contains("ColorJack", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorJack);
      }        
      else if (qs.contains("ColorCable", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorCable);
      }        
    }      
    updateColors();
    fclose(f);
  }        
}

void ModularSynth::saveColors() {

  FILE *f;
  QString config_fn, qs;
  
  config_fn = QFileDialog::getSaveFileName(this, tr("Save Colors"),
					   synthdata->savePath,
                                           tr("AlsaModularSynth Color files (*.acs)"));
  if (config_fn.isEmpty())
    return;

  if (!(f = fopen(config_fn.toLatin1().constData(), "w"))) { 
    QMessageBox::information( this, "AlsaModularSynth",
            tr("Could not save file."));
  } else {
    fprintf(f, "ColorBackground %d %d %d\n", synthdata->colorBackground.red(), synthdata->colorBackground.green(), synthdata->colorBackground.blue());
    fprintf(f, "ColorModuleBackground %d %d %d\n", synthdata->colorModuleBackground.red(), synthdata->colorModuleBackground.green(), synthdata->colorModuleBackground.blue());
    fprintf(f, "ColorModuleBorder %d %d %d\n", synthdata->colorModuleBorder.red(), synthdata->colorModuleBorder.green(), synthdata->colorModuleBorder.blue());
    fprintf(f, "ColorModuleFont %d %d %d\n", synthdata->colorModuleFont.red(), synthdata->colorModuleFont.green(), synthdata->colorModuleFont.blue());
    fprintf(f, "ColorJack %d %d %d\n", synthdata->colorJack.red(), synthdata->colorJack.green(), synthdata->colorJack.blue());
    fprintf(f, "ColorCable %d %d %d\n", synthdata->colorCable.red(), synthdata->colorCable.green(), synthdata->colorCable.blue());
    fclose(f);
  }
}                             

void ModularSynth::load(QTextStream& ts)
{
  int l1 = 0;
  int moduleID = 0;
  int index = 0;
  M_typeEnum M_type;
  int value, x, y, w, h, subID1, subID2;
  int index1, index2, moduleID1, moduleID2, midiSign;
  int index_read1, index_read2, moduleID_read1, moduleID_read2;
  int type, ch, param, isLogInt, sliderMin, sliderMax;
  int red1, green1, blue1, red2, green2, blue2;
  QString qs, qs2, ladspaLibName, pluginName, para, scalaName;
  bool isLog, ladspaLoadErr, commentFlag, followConfig;
  int newLadspaPolyFlag = 0;
  int textEditID;
  Module *m;
  int currentProgram;
  QStringList tokens;

  followConfig = midiWidget->followConfig;
  midiWidget->followConfig = false;
  currentProgram = -1;

  bool restartSynth = clearConfig(false);
  ladspaLoadErr = false;
  commentFlag = false;
  loadingPatch = true;

  while (!ts.atEnd()) {
    qs = ts.readLine();

    if (qs.startsWith("#PARA#", Qt::CaseInsensitive))
      commentFlag = true;

    else if (qs.startsWith("#ARAP#", Qt::CaseInsensitive))
      commentFlag = false;

    else if (qs.startsWith("Module", Qt::CaseInsensitive) && !commentFlag) {
        tokens = qs.split(' ');
      M_type = (M_typeEnum)tokens[1].toInt();
      moduleID = tokens[2].toInt();
      newBoxPos.setX(tokens[3].toInt());
      newBoxPos.setY(tokens[4].toInt());

      switch (M_type) {
          case M_type_ladspa: 
              newLadspaPolyFlag = tokens[5].toInt();
              ladspaLibName = tokens[6];
              pluginName = qs.section(' ', 7);
              StdErr << "Loading LADSPA plugin \"" << pluginName <<
                  "\" from library \"" << ladspaLibName << "\"." << endl;

              if (!synthdata->getLadspaIDs(ladspaLibName, pluginName,
                          &subID1, &subID2)) {
                  QMessageBox::information(this, "AlsaModularSynth",
                          tr("Could not find LADSPA plugin "
                              "\"%1\" from library \"%2\".")
                          .arg(pluginName).arg(ladspaLibName));
                  ladspaLoadErr = true;
              }
              break;
          case M_type_scquantizer: 
          case M_type_scmcv: 
              scalaName = tokens[5];
              break;
          default: 
              subID1 = tokens[5].toInt();
              subID2 = tokens[6].toInt();
              break;
      }

      switch(M_type) {
          case M_type_custom: 
              break;
          case M_type_vco: 
              newM_vco();
              break;
          case M_type_vco2: 
              newM_vco2();
              break;

          case M_type_vca: 
              if (subID1) 
                  newM_vca_exp();
              else
                  newM_vca_lin();
              break;
          case M_type_vcf: 
              newM_vcf();
              break;
          case M_type_lfo: 
              newM_lfo();
              break;
          case M_type_mphlfo: 
              newM_mphlfo();
              break;
          case M_type_noise: 
              newM_noise();
              break;
          case M_type_noise2: 
              newM_noise2();
              break;

          case M_type_delay: 
              newM_delay();
              break;
          case M_type_seq: 
              newM_seq(subID1);
              break;
          case M_type_env: 
              newM_env();
              break;
          case M_type_vcenv: 
              newM_vcenv();
              break;
          case M_type_vcenv2: 
              newM_vcenv2();
              break;
          case M_type_vcdoubledecay: 
              newM_vcdoubledecay();
              break;
          case M_type_vcpanning: 
              newM_vcpanning();
              break;
          case M_type_advenv: 
              newM_advenv();
              break;
          case M_type_mcv: 
              newM_mcv();
              break;
          case M_type_advmcv: 
              newM_advmcv();
              break;
          case M_type_scmcv: 
              newM_scmcv(&scalaName);
              break;
          case M_type_ringmod: 
              newM_ringmod();
              break;
          case M_type_inv: 
              newM_inv();
              break;
          case M_type_amp: 
              newM_amp();
              break;
          case M_type_vquant: 
              newM_vquant();
              break;
          case M_type_conv: 
              newM_conv();
              break;
          case M_type_sh: 
              newM_sh();
              break;
          case M_type_vcswitch: 
              newM_vcswitch();
              break;
          case M_type_cvs: 
              newM_cvs();
              break;
          case M_type_slew: 
              newM_slew();
              break;
          case M_type_quantizer: 
              newM_quantizer();
              break;
          case M_type_scquantizer: 
              newM_scquantizer(&scalaName);
              break;
          case M_type_ad: 
              newM_ad(subID1);
              break;
          case M_type_mix: 
              newM_mix(subID1);
              break;
          case M_type_function: 
              newM_function(subID1);
              break;
          case M_type_stereomix: 
              newM_stereomix(subID1);
              break;
          case M_type_vcorgan: 
              newM_vcorgan(subID1);
              break;
          case M_type_dynamicwaves: 
              newM_dynamicwaves(subID1);
              break;
          case M_type_ladspa: 
              if (!ladspaLoadErr) {
                  newM_ladspa(subID1, subID2, newLadspaPolyFlag & 2,
                          newLadspaPolyFlag & 1);
              } 
              break;
          case M_type_pcmout:
          case M_type_jackout:
              newM_pcmout();
              break;
          case M_type_pcmin:
          case M_type_jackin:
              newM_pcmin();
              break;
          case M_type_wavout:
              newM_wavout(); 
              break;
          case M_type_midiout:
              newM_midiout(); 
              break;
          case M_type_scope:
              newM_scope(); 
              break;
          case M_type_spectrum:
              newM_spectrum(); 
              break;
      }
      // TODO:
      //m = listModule.last();
      m = listModule.at(listModule.count() - 1);
      // TODO:
      // m->setModuleId(moduleID);
      m->moduleID = moduleID;
      qs = m->configDialog->windowTitle();
      qs2 = qs.left(qs.lastIndexOf(' '));
      qs.sprintf(" %d", moduleID);
      m->configDialog->setWindowTitle(qs2+qs);

      midiWidget->addModule(m);
      if (synthdata->moduleID <= moduleID) {
          synthdata->moduleID = moduleID + 1;
      }
    }
    else if (qs.startsWith("Comment", Qt::CaseInsensitive) && !commentFlag) {
        tokens = qs.split(' ');
      textEditID = tokens[1].toInt(); 
      //textEditID = tokens[2].toInt(); 
      newBoxPos.setX(tokens[3].toInt());
      newBoxPos.setY(tokens[4].toInt());
      w = tokens[5].toInt();
      h = tokens[6].toInt();
      new_textEdit(w, h);
    }
  }
  // end of file reached, we start again looking for other tags
  ts.seek(0);

  while (!ts.atEnd() && !ladspaLoadErr) {
    qs = ts.readLine();
    tokens = qs.split(' ');

    if (qs.startsWith("Port", Qt::CaseInsensitive)) {
      index1 = tokens[1].toInt(); 
      index2 = tokens[2].toInt(); 
      moduleID1 = tokens[3].toInt();
      moduleID2 = tokens[4].toInt();

      moduleID_read1 = 0;
      moduleID_read2 = 0;
      index_read1 = 0;
      index_read2 = 0;
      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID1)
	  moduleID_read1 = l1;

	if (listModule.at(l1)->moduleID == moduleID2)
	  moduleID_read2 = l1;

      }
      for (l1 = 0; l1 < listModule.at(moduleID_read1)->portList.count(); ++l1)
	if ((listModule.at(moduleID_read1)->portList.at(l1)->index == index1) 
            && (listModule.at(moduleID_read1)->portList.at(l1)->dir == PORT_IN)) {
	  index_read1 = l1;
	}

      for (l1 = 0; l1 < listModule.at(moduleID_read2)->portList.count(); ++l1)
	if ((listModule.at(moduleID_read2)->portList.at(l1)->index == index2)
            && (listModule.at(moduleID_read2)->portList.at(l1)->dir == PORT_OUT)) {
	  index_read2 = l1;
	} 

      listModule.at(moduleID_read1)->portList.at(index_read1)->connectTo(listModule.at(moduleID_read2)->portList.at(index_read2));
      listModule.at(moduleID_read2)->portList.at(index_read2)->connectTo(listModule.at(moduleID_read1)->portList.at(index_read1));
    }

    else if (qs.startsWith("ColorP", Qt::CaseInsensitive)) {
      index1 = tokens[1].toInt(); 
      index2 = tokens[2].toInt(); 
      moduleID1 = tokens[3].toInt();
      moduleID2 = tokens[4].toInt();
      red1 = tokens[5].toInt(); 
      green1 = tokens[6].toInt(); 
      blue1 = tokens[7].toInt(); 
      red2 = tokens[8].toInt(); 
      green2 = tokens[9].toInt(); 
      blue2 = tokens[10].toInt(); 

      moduleID_read1 = 0;
      moduleID_read2 = 0;
      index_read1 = 0;
      index_read2 = 0;
      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID1) {
	  moduleID_read1 = l1;
	}
	if (listModule.at(l1)->moduleID == moduleID2) {
	  moduleID_read2 = l1;
	}
      }
      for (l1 = 0; l1 < listModule.at(moduleID_read1)->portList.count(); ++l1) {
	if ((listModule.at(moduleID_read1)->portList.at(l1)->index == index1) 
            && (listModule.at(moduleID_read1)->portList.at(l1)->dir == PORT_IN)) {
	  index_read1 = l1;
	}
      } 
      for (l1 = 0; l1 < listModule.at(moduleID_read2)->portList.count(); ++l1) {
	if ((listModule.at(moduleID_read2)->portList.at(l1)->index == index2)
            && (listModule.at(moduleID_read2)->portList.at(l1)->dir == PORT_OUT)) {
	  index_read2 = l1;
	} 
      }   
      listModule.at(moduleID_read1)->portList.at(index_read1)->connectTo(listModule.at(moduleID_read2)->portList.at(index_read2));
      listModule.at(moduleID_read2)->portList.at(index_read2)->connectTo(listModule.at(moduleID_read1)->portList.at(index_read1));
      listModule.at(moduleID_read1)->portList.at(index_read1)->jackColor = QColor(red1, green1, blue1);
      listModule.at(moduleID_read1)->portList.at(index_read1)->cableColor = QColor(red2, green2, blue2);
    }
    else if (qs.startsWith("FSlider", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      value = tokens[3].toInt();
      isLogInt = tokens[4].toInt();
      sliderMin = tokens[5].toInt(); 
      sliderMax = tokens[6].toInt(); 
      midiSign = tokens[7].toInt(); 

      isLog = isLogInt == 1;
      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  //	    StdOut << __PRETTY_FUNCTION__ << " moduleID " << moduleID << " index " << index << endl;
	  MidiControllableFloat &mcAbleF =
	    dynamic_cast<MidiControllableFloat &>(listModule.at(l1)->configDialog->midiSliderList.at(index)->mcAble);
	  mcAbleF.setLog(isLog);
	  mcAbleF.setVal(value, NULL);
	  mcAbleF.setNewMin(sliderMin);
	  mcAbleF.setNewMax(sliderMax);
	  mcAbleF.midiSign = midiSign;
	  /*listModule.at(l1)->configDialog->midiSliderList.at(index)->setLogMode(isLog);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->updateValue(value);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setLogMode(isLog);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setNewMin(sliderMin);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setNewMax(sliderMax);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->midiSign = midiSign;*/
	  break;
	}
      }
    }
    else if (qs.startsWith("ISlider", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      value = tokens[3].toInt();
      midiSign = tokens[4].toInt();

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  MidiControllableBase &mcAble =
	    listModule.at(l1)->configDialog->intMidiSliderList.at(index)->mcAble;
	  mcAble.midiSign = midiSign;
	  mcAble.setVal(value, NULL);
	  /*            listModule.at(l1)->configDialog->intMidiSliderList.at(index)->midiSign = midiSign;
			listModule.at(l1)->configDialog->intMidiSliderList.at(index)->updateValue((int)value);
			listModule.at(l1)->configDialog->intMidiSliderList.at(index)->slider->setValue((int)value);*/
	  break;
	}
      }
    }
    else if (qs.startsWith("LSlider", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      value = tokens[3].toInt();
      midiSign = tokens[4].toInt();

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  MidiControllableBase &mcAble =
	    listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->mcAble;
	  mcAble.midiSign = midiSign;
	  mcAble.setVal(value, NULL);
	  /*listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->midiSign = midiSign;
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->updateValue((int)value);
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->slider->setValue((int)value);*/
	  break;
	}
      }
    }
    else if (qs.startsWith("ComboBox", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      value = tokens[3].toInt();
      midiSign = tokens[4].toInt();

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  MidiControllableNames &mcAble =
	    dynamic_cast<MidiControllableNames &>(listModule.at(l1)->configDialog->midiComboBoxList.at(index)->mcAble);
	  mcAble.midiSign = midiSign;
	  mcAble.setVal(value, NULL);
	  break;
	}
      }
    }
    else if (qs.startsWith("CheckBox", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      value = tokens[3].toInt();
      midiSign = tokens[4].toInt();

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->checkBox->setChecked(value==1);
	  listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->mcAble.midiSign = midiSign;
	  break;
	}
      }
    }
    else if (qs.startsWith("Function", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      subID1 = tokens[3].toInt();
      subID2 = tokens[4].toInt();

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  listModule.at(l1)->configDialog->functionList.at(index)->setPointCount(subID2);
	  break;
	}
      }
    }
    else if (qs.startsWith("Point", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      subID1 = tokens[3].toInt();
      subID2 = tokens[4].toInt();
      x = tokens[5].toInt();
      y = tokens[6].toInt();

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  listModule.at(l1)->configDialog->functionList.at(index)->setPoint(subID1, subID2, x, y);
	}
      }
    }
    else if (qs.contains("MIDI", Qt::CaseInsensitive)) {
      moduleID = tokens[1].toInt(); 
      index = tokens[2].toInt(); 
      type = tokens[3].toInt();
      ch = tokens[4].toInt();
      param = tokens[5].toInt();

      MidiControllerKey mck(type, ch, param);
      midiWidget->addMidiController(mck);
      //         MidiControllerKey const midiController = //new MidiController(type, ch, param);
      // 	  //        if (!midiWidget->midiControllerList.contains(midiController)) {
      //           *midiWidget->midiController(MidiController(type, ch, param));
      // //         } else {
      // //           QList<MidiController*>::iterator midiIndex = midiWidget->midiControllerList.find(midiController);
      // //           delete(midiController);
      // //           midiController = *midiIndex;
      // //         }
      if (qs.startsWith("FSMIDI", Qt::CaseInsensitive)) {
	listModule.at(l1)->configDialog->midiSliderList.at(index)->mcAble.connectToController(mck);
      }
      else if (qs.startsWith("ISMIDI", Qt::CaseInsensitive)) {
	listModule.at(l1)->configDialog->intMidiSliderList.at(index)->mcAble.connectToController(mck);
      }
      else if (qs.startsWith("LSMIDI", Qt::CaseInsensitive)) {
	listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->mcAble.connectToController(mck);
      }
      else if (qs.startsWith("CMIDI", Qt::CaseInsensitive)) {
	listModule.at(l1)->configDialog->midiComboBoxList.at(index)->mcAble.connectToController(mck);
      }
      else if (qs.startsWith("TMIDI", Qt::CaseInsensitive)) {
	listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->mcAble.connectToController(mck);
      }
      else
          qWarning(tr("Unknown MIDI controller tag found: %1").arg(qs).toUtf8());
    }

    else if (qs.startsWith("#PARA#", Qt::CaseInsensitive)) {
      textEditID = tokens[1].toInt(); 
      //textEditID = tokens[2].toInt(); 
      index = tokens[3].toInt();
      qs = ts.readLine();

      if (!qs.startsWith("#ARAP#", Qt::CaseInsensitive)) {
	para = qs + ' ';
      } else {
	para = " ";
      }
      while (!qs.startsWith("#ARAP#", Qt::CaseInsensitive)) {
        qs = ts.readLine();

	if (!qs.startsWith("#ARAP#", Qt::CaseInsensitive)) {
	  para.append(qs + " "); 
	}
      }
      listTextEdit.at(textEditID)->textEdit->append(para);
    }

    // Instrument tags
    // Tab "TabName"
    else if (qs.startsWith("Tab", Qt::CaseInsensitive)) {
        QRegExp rx("\"([^\"]+)\"");
        int pos = rx.indexIn(qs);
        if (pos != -1)
            guiWidget->addTab(rx.cap(1));
        else
            qWarning(QObject::tr("No title for tab '%1' found.")
                    .arg(qs).toUtf8());
    }
    // Frame "FrameName" <frame number>
    else if (qs.startsWith("Frame", Qt::CaseInsensitive)) {
        QRegExp rx("\"([^\"]+)\"");
        int pos = rx.indexIn(qs);
        if (pos != -1) {
            guiWidget->addFrame(rx.cap(1));
            pos += rx.matchedLength();
            QString number = qs.mid(pos);
            number = number.trimmed();
            guiWidget->setTab(number.toInt());
        }
        else
            qWarning(QObject::tr("No data for frame '%1' found.")
                    .arg(qs).toUtf8());
    }
    else if (qs.startsWith("Parameter", Qt::CaseInsensitive)) {
        QRegExp rx("\"([^\"]+)\"");
        int pos = rx.indexIn(qs);
        if (pos != -1) {
            pos += rx.matchedLength();
            QString numbers = qs.mid(pos);
            numbers = numbers.trimmed();
            tokens = numbers.split(' ');
            if (tokens.isEmpty()) {
                qWarning(QObject::tr("No parameter values found.").toUtf8());
                continue;
            }

            qs = rx.cap(1);
            moduleID = tokens[0].toInt();
            index = tokens[1].toInt();
        }
        else
            qWarning(QObject::tr("No parameter name '%1' found.")
                    .arg(qs).toUtf8());

      for (l1 = 0; l1 < listModule.count(); ++l1) {
	if (listModule.at(l1)->moduleID == moduleID) {
	  guiWidget->addParameter(listModule.at(l1)->midiControllables.at(index), qs);
	  MidiControllableFloat *mcAbleF =
	    dynamic_cast<MidiControllableFloat *>(listModule.at(l1)->midiControllables.at(index));

	  if (mcAbleF)
	    /*listModule.at(l1)->configDialog->midiGUIcomponentList.at(index)->componentType == GUIcomponentType_slider)*/ {
	    sliderMin = tokens[2].toInt();
	    sliderMax = tokens[3].toInt();
	    isLogInt = tokens[4].toInt();
	    //!!               ((MidiSlider *)guiWidget->parameterList.last())->setNewMin(sliderMin);
	    //               ((MidiSlider *)guiWidget->parameterList.last())->setNewMax(sliderMax);
	    //               ((MidiSlider *)guiWidget->parameterList.last())->setLogMode(isLogInt == 1);
	  }
	  break;
	}
      }
    }

    else if (qs.startsWith("Program", Qt::CaseInsensitive)) {

      index = tokens[1].toInt(); 
      value = tokens[2].toInt();

      if (index != currentProgram) {
	currentProgram = index;
	guiWidget->setPresetCount(currentProgram + 1);
      }
      guiWidget->presetList[currentProgram].append(value);
    }
    
    else if (qs.startsWith("PresetName", Qt::CaseInsensitive)) {
        QRegExp rx("\"([^\"]+)\"");
        int pos = rx.indexIn(qs);
        if (pos != -1)
            qs = rx.cap(1);
        else {
            qWarning(QObject::tr("No name for preset '%1' found.")
                    .arg(qs).toUtf8());
            continue;
        }

      qs2.sprintf("%3d", guiWidget->presetNameList.count());
      guiWidget->presetNameList.append(qs2+qs);
    }
    /*For debugging only, 'Module' and 'Comment' will also apear*/
    else
        qWarning(tr("Unknown tag found: %1").arg(qs).toUtf8());
  } // end while loop

  if (guiWidget->presetCount)
    guiWidget->setCurrentPreset(0);

  loadingPatch = false;

  resize();
  update();
  midiWidget->setActiveMidiControllers();
  synthdata->doSynthesis = restartSynth;
  StdOut << "synthdata->periodsize = " << synthdata->periodsize << endl;
  StdOut << "synthdata->cyclesize = " << synthdata->cyclesize << endl;
  StdOut << "Module::portmemAllocated = " << Module::portmemAllocated << endl;
  midiWidget->followConfig = followConfig;
  guiWidget->refreshGui();
  modified = false;
}

void ModularSynth::save(QTextStream& ts)
{
    int l1, l2, value;
    QString qs;
    QStringList::iterator presetit;
    int offX = 0, offY = 0;

    if (childrenRect().left() > 100)
      offX = childrenRect().left() - 100;

    if (childrenRect().top() > 66)
      offY = childrenRect().top() - 66;
    
    for (l1 = 0; l1 < listModule.count(); ++l1) {
      ts << "Module "
          << (int)listModule.at(l1)->M_type << ' '
          << listModule.at(l1)->moduleID << ' '
          << listModule.at(l1)->x() - offX << ' '
          << listModule.at(l1)->y() - offY << ' ';

      switch(listModule.at(l1)->M_type)
      {
        case M_type_custom: 
          break;
        case M_type_vca: 
          ts << (int)((M_vca *)listModule.at(l1))->expMode << " 0" << endl;
          break;
        case M_type_ad: 
        case M_type_function: 
          ts << listModule.at(l1)->outPortCount << " 0" << endl;
          break;
        case M_type_mix: 
          ts << ((M_mix *)listModule.at(l1))->in_channels << " 0" << endl;
          break;
        case M_type_stereomix: 
          ts << ((M_stereomix *)listModule.at(l1))->in_channels << " 0" << endl;
          break;
        case M_type_vcorgan: 
          ts << ((M_vcorgan *)listModule.at(l1))->oscCount << " 0" << endl;
          break;
        case M_type_dynamicwaves: 
          ts << ((M_dynamicwaves *)listModule.at(l1))->oscCount << " 0" << endl;
          break;
        case M_type_seq: 
          ts << ((M_seq *)listModule.at(l1))->seqLen << " 0" << endl;
          break;
        case M_type_ladspa: 
          ts << 2 * (int)((M_ladspa *)listModule.at(l1))->isPoly
                  + (int)((M_ladspa *)listModule.at(l1))->hasExtCtrlPorts
             << ' '
             << synthdata->ladspaLib.at(((M_ladspa *)listModule.at(l1))
                         ->ladspaDesFuncIndex).name
             << ' '
             << ((M_ladspa *)listModule.at(l1))->pluginName << endl;
          break;
        case M_type_scquantizer: 
          qs = ((M_scquantizer *)listModule.at(l1))->sclname;
          if (qs.contains('/')) {
            qs = qs.mid(qs.lastIndexOf('/') + 1);             
          }
          ts << qs << endl;
          break;
        case M_type_scmcv: 
          qs = ((M_scmcv *)listModule.at(l1))->sclname;
          if (qs.contains('/')) {
            qs = qs.mid(qs.lastIndexOf('/') + 1);             
          }
          ts << qs << endl;
          break;
        default:
          ts << "0 0" << endl;
          break; 
      }
      listModule.at(l1)->save(ts);
    }
    for (l1 = 0; l1 < listTextEdit.count(); ++l1)
        ts << "Comment " 
            << listTextEdit.at(l1)->textEditID << ' '
            << l1 << ' '
            << listTextEdit.at(l1)->x() - offX << ' '
            << listTextEdit.at(l1)->y() - offY << ' '
            << listTextEdit.at(l1)->width() << ' '
            << listTextEdit.at(l1)->height() << endl;

    for (l1 = 0; l1 < listTextEdit.count(); ++l1) {
      TextEdit *tE = listTextEdit.at(l1);
      QTextDocument *tD = tE->textEdit->document();
      QTextBlock tB = tD->begin();
      for (l2 = 0; l2 < tD->blockCount(); ++l2, tB = tB.next()) {
        ts << "#PARA# " << tE->textEditID << ' ' << l1 << ' ' << l2 << endl;
        ts << tB.text() << endl; 
        ts << "#ARAP#" << endl; 
      }
    }

    for (l1 = 0; l1 < guiWidget->tabList.count(); ++l1)
        ts << "Tab \"" << guiWidget->tabNameList.at(l1) << "\"" << endl; 

    for (l1 = 0; l1 < guiWidget->frameBoxList.count(); ++l1) {
      ts << "Frame \""
         << guiWidget->frameBoxList.at(l1)->frameBox->parentWidget()
            ->objectName() << "\" "
         << guiWidget->frameBoxList.at(l1)->tabIndex << endl;

      for (l2 = 0; l2 < guiWidget->parameterList.count(); ++l2)
        if (guiWidget->mgcs.at(l2)->parent() ==
                guiWidget->frameBoxList.at(l1)->frameBox->parentWidget()) {
          ts << "Parameter \""
              << guiWidget->mgcs.at(l2)->nameLabel.text() << "\" "
              << guiWidget->parameterList.at(l2)->module.moduleID << ' '
              << guiWidget->parameterList.at(l2)->midiControllableListIndex
              << ' ';

	  MidiControllableFloat *mcAbleF = dynamic_cast<MidiControllableFloat *>(guiWidget->parameterList.at(l2));
          if (mcAbleF)
              ts << mcAbleF->sliderMin() << ' '
                 << mcAbleF->sliderMax() << ' '
                 << mcAbleF->getLog() << endl;
          else
              ts << endl;
        }
    }

    for (l1 = 0; l1 < guiWidget->presetCount; ++l1) {
      for (int p = 0; p < guiWidget->presetList[l1].count(); p++) {
        value = guiWidget->presetList[l1][p];
        ts << "Program " << l1 << ' ' << value << endl;
      } 
    }
    for (presetit = guiWidget->presetNameList.begin();
            presetit != guiWidget->presetNameList.end(); ++presetit) {
        ts << "PresetName \"" << (*presetit).mid(3) << "\"" << endl;
    } 

  modified = false;
}
//====================================================== End persistence
void ModularSynth::allVoicesOff()
{
  int l2;

  for (l2 = 0; l2 < synthdata->poly; ++l2)
    if (synthdata->noteCounter[l2] < 1000000)
      synthdata->noteCounter[l2] = 1000000;

}

void ModularSynth::cleanUpSynth()
{
  prefWidget->savePref(rcFd);
  qWarning(QObject::tr("Closing synth...").toUtf8());
}

void ModularSynth::showContextMenu(QPoint pos) {
  
    contextMenu->popup(mapToGlobal(pos));
}

void ModularSynth::refreshColors() {

  int l1, l2;
  for (l1 = 0; l1 < listModule.count(); ++l1) {
    listModule[l1]->getColors();
    for (l2 = 0; l2 < listModule.at(l1)->portList.count(); ++l2) {
      listModule.at(l1)->portList.at(l2)->setPalette(QPalette(
                  synthdata->colorModuleBackground,
                  synthdata->colorModuleBackground));
    }
  }      
  prefWidget->refreshColors();

  QPalette p = palette();
  p.setColor(backgroundRole(), synthdata->colorBackground);
  setAutoFillBackground(true);
  setPalette(p);
  update();
}

void ModularSynth::updateColors()
{
  synthdata->colorPortFont1 = synthdata->colorModuleFont;
  synthdata->colorPortFont2 = QColor(255, 240, 140);
  prefWidget->recallColors();
  refreshColors();
}

void ModularSynth::moveAllBoxes(const QPoint &delta)
{
  modified = true;

  if (delta.isNull())
    return;

  int i;
  for (i = 0; i < listModule.count(); ++i) {
    Box *b = listModule[i];
    b->move(b->pos() + delta);
  }
  for (i = 0; i < listTextEdit.count(); ++i) {
    Box *b = listTextEdit[i];
    b->move(b->pos() + delta);
  }
}

bool ModularSynth::isModified()
{
   return modified;
}

QString ModularSynth::getLoadPath()
{
    return synthdata->loadPath;
}

void ModularSynth::setLoadPath(const QString& sp)
{
    synthdata->loadPath = sp;
}

QString ModularSynth::getSavePath()
{
    return synthdata->savePath;
}

void ModularSynth::setSavePath(const QString& sp)
{
    synthdata->savePath = sp;
}