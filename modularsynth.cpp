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
#include <QTextBlock>
#include <QTextEdit>
#include "midislider.h"
#include "intmidislider.h"
#include "floatintmidislider.h"
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

ModularSynth::ModularSynth(QMainWindow *mainWindow, const char *p_pcmname,
			   int p_fsamp, int p_frsize, int p_nfrags,
			   int p_ncapt, int p_nplay, int poly, float edge) 
  : mainWindow(mainWindow)
  , pcmname (p_pcmname)
  , fsamp (p_fsamp)
  , frsize (p_frsize)
  , nfrags (p_nfrags)
  , ncapt (p_ncapt)
  , nplay (p_nplay)
  , paintFastly(false)
  , _zoomFactor(1.0)
{
  firstPort = true;
  connectingPort[0] = NULL;
  connectingPort[1] = NULL;
  connectorStyle = CONNECTOR_BEZIER;
  aboutWidget = new QMessageBox(this); 
  clientid = 0;
  portid = 0;

  synthdata = new SynthData (poly, edge);

  midiWidget = new MidiWidget(NULL);
  midiWidget->setWindowTitle("AlsaModularSynth Control Center");
  synthdata->midiWidget = midiWidget;
  guiWidget = new GuiWidget(NULL);
  guiWidget->setWindowTitle("AlsaModularSynth Parameter View");
  synthdata->guiWidget = guiWidget;
  prefWidget = new PrefWidget();
  prefWidget->setWindowTitle("AlsaModularSynth Preferences");
  synthdata->loadPath = "";
  ladspaDialog = new LadspaDialog();
  QObject::connect(static_cast<QWidget *>(ladspaDialog),
		   SIGNAL(createLadspaModule(int, int, bool, bool)),
                   this, SLOT(newM_ladspa(int, int, bool, bool)));
  QObject::connect(prefWidget, SIGNAL(prefChanged()),
                   this, SLOT(refreshColors()));
  setPalette(QPalette(QColor(240, 240, 255), QColor(240, 240, 255)));
  loadingPatch = false;

  synthdata->rcPath = QString(getenv("HOME")) + "/.alsamodular.cfg";
  prefWidget->loadPref(synthdata->rcPath);
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
  int width = std::max(childrenRect().right() + 30, ((QWidget*)parent())->width());
  int height = std::max(childrenRect().bottom() + 10, ((QWidget*)parent())->height());
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
    fprintf(stderr, " MIDI wont work!\n");

  if (withJack) {
    synthdata->initJack(ncapt, nplay);
    synthdata->doSynthesis = true;
  } else {
    synthdata->initAlsa(pcmname, fsamp, frsize, nfrags, ncapt, nplay);
    synthdata->doSynthesis = true;
  }
  return 0;
}

void ModularSynth::displayAbout() {
 
    aboutWidget->about(this, "About AlsaModularSynth", "AlsaModularSynth " AMS_VERSION 
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
    "answered many questions about QT. Thanks to Jörg Arndt for valuable\n"
    "hints regarding speed optimization. Torsten Rahn has helped to\n" 
    "improve the color scheme of the program. Thanks to Bernhard Kaindl\n"
    "for helpful discussion.\n");
    aboutWidget->raise();
}

void ModularSynth::displayMidiController() {
   
  midiWidget->show();  
  midiWidget->raise();
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

int ModularSynth::setPresetPath(QString name) {

  synthdata->loadPath = name;
  return(0);
}

int ModularSynth::setSavePath(QString name) {

  synthdata->savePath = name;
  return(0);
}

snd_seq_t *ModularSynth::open_seq() {

  snd_seq_t *seq_handle;
  int l1;
  QString qs;

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.");
    return NULL;
  }
  snd_seq_set_client_name(seq_handle, "AlsaModularSynth");
  clientid = snd_seq_client_id(seq_handle);
  if ((portid = snd_seq_create_simple_port(seq_handle, "ams",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer write port.");
    snd_seq_close(seq_handle);
    return NULL;
  }
  for (l1 = 0; l1 < 2; ++l1)
    if ((synthdata->midi_out_port[l1] = snd_seq_create_simple_port(seq_handle, "ams",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      fprintf(stderr, "Error creating sequencer read port.");
      snd_seq_close(seq_handle);
      return NULL;
    }

  qs.sprintf("AlsaModularSynth " AMS_VERSION " - %d:%d - (%d)", clientid, portid, synthdata->poly);
  mainWindow->setWindowTitle(qs);
  synthdata->jackName.sprintf("ams_%d_%d", clientid, portid);
  return seq_handle;
}

int ModularSynth::initSeqNotifier() {

  int alsaEventFd = 0;

  struct pollfd pfd[1];
  snd_seq_poll_descriptors(synthdata->seq_handle, pfd, 1, POLLIN);
  alsaEventFd = pfd[0].fd;
  seqNotifier = new QSocketNotifier(alsaEventFd, QSocketNotifier::Read);
  QObject::connect(seqNotifier, SIGNAL(activated(int)),
                   this, SLOT(midiAction(int)));
  return(0);
}

void ModularSynth::midiAction(int) {

  snd_seq_event_t *ev;
  QString qs;
  int l1, l2, osc, noteCount;
  bool foundOsc;

  do {
    snd_seq_event_input(synthdata->seq_handle, &ev);
    MidiController midiController(ev); 
    if (midiWidget->isVisible()) {
      if (ev->type == SND_SEQ_EVENT_CONTROLLER ||
	  ev->type == SND_SEQ_EVENT_CONTROL14 ||
	  ev->type == SND_SEQ_EVENT_PITCHBEND)
	midiWidget->addMidiController(midiController);

      if (midiWidget->noteControllerEnabled &&
	  (ev->type == SND_SEQ_EVENT_NOTEON || ev->type == SND_SEQ_EVENT_NOTEOFF))
	midiWidget->addMidiController(midiController);
    }

    const MidiController *c = midiWidget->midiController(midiController);
    if (c) {
      int value;
      switch (c->type()) {
      case SND_SEQ_EVENT_PITCHBEND:
	value = (ev->data.control.value + 8192) >> 7;
	break;
      case SND_SEQ_EVENT_CONTROL14:
	value = ev->data.control.value >> 7;
	break;
      case SND_SEQ_EVENT_CONTROLLER:
	value = ev->data.control.value;
	break;
      case SND_SEQ_EVENT_NOTEON:
	value = ev->data.note.velocity;
	break;
      default:
      case SND_SEQ_EVENT_NOTEOFF:
	value = 0;
	break;
      }
      emit c->context->sendMidiValue(value);
      if (midiWidget->followMidi)
	midiWidget->setSelectedController(*c);
    }

// Voice assignment

    if (((ev->type == SND_SEQ_EVENT_NOTEON) || (ev->type == SND_SEQ_EVENT_NOTEOFF)) 
         && ((synthdata->midiChannel < 0) || (synthdata->midiChannel == ev->data.control.channel))) {
      if ((ev->type == SND_SEQ_EVENT_NOTEON) && (ev->data.note.velocity > 0)) {

// Note On: Search for oldest voice to allocate new note.          
          
          osc = 0;
          noteCount = 0;
          foundOsc = false;
          for (l2 = 0; l2 < synthdata->poly; ++l2) {
            if (synthdata->noteCounter[l2] > noteCount) {
              noteCount = synthdata->noteCounter[l2];
              osc = l2;
              foundOsc = true;
            }
          }
          if (foundOsc) {
            synthdata->noteCounter[osc] = 0;
            synthdata->sustainNote[osc] = false;
            synthdata->velocity[osc] = ev->data.note.velocity;
            synthdata->channel[osc] = ev->data.note.channel;
            synthdata->notes[osc] = ev->data.note.note;   
            for (l1 = 0; l1 < listModule.count(); ++l1) {
              listModule.at(l1)->noteOnEvent(osc);
            }
          }  
      } else {
      
// Note Off      
      
        for (l2 = 0; l2 < synthdata->poly; ++l2) {
          if ((synthdata->notes[l2] == ev->data.note.note)
            && (synthdata->channel[l2] == ev->data.note.channel)) {
            if (synthdata->sustainFlag) {
              synthdata->sustainNote[l2] = true;
            } else {
              synthdata->noteCounter[l2] = 1000000; 
              for (l1 = 0; l1 < listModule.count(); ++l1) {
                listModule.at(l1)->noteOffEvent(l2);
              } 
            }  
          }   
        }     
      }       
    }
    
    if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
      if (ev->data.control.param == MIDI_CTL_ALL_NOTES_OFF) {
        for (l2 = 0; l2 < synthdata->poly; ++l2) {
          if ((synthdata->noteCounter[l2] < 1000000) && (synthdata->channel[l2] == ev->data.note.channel)) {
            synthdata->noteCounter[l2] = 1000000;
            for (l1 = 0; l1 < listModule.count(); ++l1) {
              listModule.at(l1)->noteOffEvent(l2);
            } 
          }   
        }     
      }  
      if (ev->data.control.param == MIDI_CTL_SUSTAIN) {
        synthdata->sustainFlag = ev->data.control.value > 63;
        if (!synthdata->sustainFlag) {
          for (l2 = 0; l2 < synthdata->poly; ++l2) {
            if (synthdata->sustainNote[l2]) {
              synthdata->noteCounter[l2] = 1000000;
              for (l1 = 0; l1 < listModule.count(); ++l1) {
                listModule.at(l1)->noteOffEvent(l2);
              }  
            } 
          }   
        }     
      }  
    }
    if (ev->type == SND_SEQ_EVENT_PGMCHANGE) {
      guiWidget->setCurrentPreset(ev->data.control.value);
    }
    for (l1 = 0; l1 < synthdata->listM_advmcv.count(); ++l1) {
      switch (ev->type) {
        case SND_SEQ_EVENT_CHANPRESS: 
          ((M_advmcv *)synthdata->listM_advmcv.at(l1))->aftertouchEvent(ev->data.note.channel, ev->data.control.value);
          break;
        case SND_SEQ_EVENT_PITCHBEND:
          ((M_advmcv *)synthdata->listM_advmcv.at(l1))->pitchbendEvent(ev->data.note.channel, ev->data.control.value); 
          break;
        case SND_SEQ_EVENT_CONTROLLER: 
          ((M_advmcv *)synthdata->listM_advmcv.at(l1))->controllerEvent(ev->data.note.channel, ev->data.control.param, ev->data.control.value);
          break;
      }
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(synthdata->seq_handle, 0) > 0);
}

void ModularSynth::initPorts(Module *m) {

  int l1;

  for (l1 = 0; l1 < m->portList.count(); ++l1) {
    QObject::connect(m->portList.at(l1), SIGNAL(portClicked()), 
                     this, SLOT(portSelected()));
    QObject::connect(m->portList.at(l1), SIGNAL(portDisconnected()), 
                     this, SLOT(updatePortConnections()));
  }
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
//############################################################# start add new modules
void ModularSynth::newM_seq(int seqLen) {

  M_seq *m = new M_seq(seqLen, this);
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
  initNewModule(m);
}

void ModularSynth::newM_advmcv() {

  M_advmcv *m = new M_advmcv(this);
  synthdata->listM_advmcv.append(m);
  initNewModule(m);
}

void ModularSynth::newM_scmcv() {

  M_scmcv *m = new M_scmcv(this);
  initNewModule(m);
}

void ModularSynth::newM_scmcv(QString *p_scalaName) {

  M_scmcv *m = new M_scmcv(this, p_scalaName);
  initNewModule(m);
}

void ModularSynth::newM_env() {

  M_env *m = new M_env(this);
  initNewModule(m);
}

void ModularSynth::newM_vcenv() {

  M_vcenv *m = new M_vcenv(this);
  initNewModule(m);
}

void ModularSynth::newM_vcenv2() {

  M_vcenv2 *m = new M_vcenv2(this);
  initNewModule(m);
}

void ModularSynth::newM_vcdoubledecay() {

  M_vcdoubledecay *m = new M_vcdoubledecay(this);
  initNewModule(m);
}

void ModularSynth::newM_vcpanning() {

  M_vcpanning *m = new M_vcpanning(this);
  initNewModule(m);
}

void ModularSynth::newM_advenv() {

  M_advenv *m = new M_advenv(this);
  initNewModule(m);
}

void ModularSynth::newM_vco() {

  M_vco *m = new M_vco(this);
  initNewModule(m);
}
void ModularSynth::newM_vco2() {

  M_vco2 *m = new M_vco2(this);
  initNewModule(m);
}

void ModularSynth::newM_vca_lin() {

  M_vca *m = new M_vca(false, this);
  initNewModule(m);
}

void ModularSynth::newM_vca_exp() {

  M_vca *m = new M_vca(true, this);
  initNewModule(m);
}

void ModularSynth::newM_lfo() {

  M_lfo *m = new M_lfo(this);
  initNewModule(m);
}

void ModularSynth::newM_mphlfo() {

  M_mphlfo *m = new M_mphlfo(this);
  initNewModule(m);
}

void ModularSynth::newM_noise() {

  M_noise *m = new M_noise(this);
  initNewModule(m);
}
void ModularSynth::newM_noise2() {

  M_noise2 *m = new M_noise2(this);
  initNewModule(m);
}
void ModularSynth::newM_ringmod() {

  M_ringmod *m = new M_ringmod(this);
  initNewModule(m);
}

void ModularSynth::newM_inv() {

  M_inv *m = new M_inv(this);
  initNewModule(m);
}

void ModularSynth::newM_amp() {

  M_amp *m = new M_amp(this);
  initNewModule(m);
}

void ModularSynth::newM_ad(int outCount) {

  M_ad *m = new M_ad(outCount, this);
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
  initNewModule(m);
}

void ModularSynth::newM_conv() {

  M_conv *m = new M_conv(this);
  initNewModule(m);
}

void ModularSynth::newM_cvs() {

  M_cvs *m = new M_cvs(this);
  initNewModule(m);
}

void ModularSynth::newM_sh() {

  M_sh *m = new M_sh(this);
  initNewModule(m);
}

void ModularSynth::newM_vcswitch() {

  M_vcswitch *m = new M_vcswitch(this);
  initNewModule(m);
}

void ModularSynth::newM_slew() {

  M_slew *m = new M_slew(this);
  initNewModule(m);
}

void ModularSynth::newM_quantizer() {

  M_quantizer *m = new M_quantizer(this);
  initNewModule(m);
}

void ModularSynth::newM_scquantizer(QString *p_scalaName) {

  M_scquantizer *m = new M_scquantizer(this, p_scalaName);
  initNewModule(m);
}

void ModularSynth::newM_scquantizer() {

  M_scquantizer *m = new M_scquantizer(this);
  initNewModule(m);
}

void ModularSynth::newM_delay() {

  M_delay *m = new M_delay(this);
  initNewModule(m);
}

void ModularSynth::newM_mix(int in_channels) {

  M_mix *m = new M_mix(in_channels, this);
  initNewModule(m);
}

void ModularSynth::newM_function(int functionCount) {

  M_function *m = new M_function(functionCount, this);
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

  qs.sprintf("%s", synthdata->ladspaLib.at(p_ladspaDesFuncIndex).desc.at(n)->Name);
  M_ladspa *m = new M_ladspa(this, p_ladspaDesFuncIndex, n, p_newLadspaPoly, p_extCtrlPorts);
  initNewModule(m);
}

void ModularSynth::newM_wavout() {

  M_wavout *m = new M_wavout(this);
  synthdata->wavoutModuleList.append(m);
  initNewModule(m);
}

void ModularSynth::newM_midiout() {

  M_midiout *m = new M_midiout(this);
  synthdata->midioutModuleList.append((QObject *)m);
  initNewModule(m);
}

void ModularSynth::newM_pcmout()
{
    int k;
    M_pcmout *M; 

    k = synthdata->find_play_mod (0);
    if (k >= 0) 
    {
        M = new M_pcmout (this, 2 * k);
        initNewModule (M);
        synthdata->set_play_mod (k, M);
    } 
    else fprintf (stderr, "All available output ports are in use\n");
}

void ModularSynth::newM_pcmin()
{
    int k;
    M_pcmin *M; 

    k = synthdata->find_capt_mod (0);
    if (k >= 0) 
    {
        M = new M_pcmin(this, 2 * k);
        initNewModule (M);
        synthdata->set_capt_mod (k, M);
    } 
    else fprintf (stderr, "All available input ports are in use\n");
}

void ModularSynth::newM_scope() {

  M_scope *m = new M_scope(this);
  synthdata->scopeModuleList.append((QObject *)m);
  initNewModule(m);
}

void ModularSynth::newM_spectrum() {

  M_spectrum *m = new M_spectrum(this);
#ifdef OUTDATED_CODE
  synthdata->spectrumModuleList.append((QObject *)m);
#endif
  initNewModule(m);
}

void ModularSynth::newM_vcf() {
 
  M_vcf *m = new M_vcf(this);
  initNewModule(m);
}
//==================================================== End of adding module functions

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
    if ((((connectingPort[0]->dir == PORT_IN) && (connectingPort[1]->dir == PORT_OUT))
      ||((connectingPort[1]->dir == PORT_IN) && (connectingPort[0]->dir == PORT_OUT)))
      && (connectingPort[0]->module != connectingPort[1]->module)) {
        connectingPort[0]->connectTo(connectingPort[1]);
        connectingPort[1]->connectTo(connectingPort[0]);
	update();//      repaintContents(false);
    } else {
      printf("Connection refused.\n");
      connectingPort[0] = NULL;
      connectingPort[1] = NULL;
    }
  } 
}

void ModularSynth::updatePortConnections()
{
  update();  //  repaintContents(false);
}

void ModularSynth::deleteModule() {

  Module *m;
 
  connectingPort[0] = NULL;
  connectingPort[1] = NULL;
  firstPort = true;
  m = (Module *)sender();
  listModule.removeAll(m);
  deleteModule (m);
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

void ModularSynth::clearConfig() {

  int l1, l2;
  bool restartSynth;
  QString qs;

  for (l2 = 0; l2 < synthdata->poly; ++l2) {
    synthdata->noteCounter[l2] = 1000000;
  }
  qs.sprintf("AlsaModularSynth " AMS_VERSION " - %d:%d - (%d)", clientid, portid, synthdata->poly);
  mainWindow->setWindowTitle(qs);
  restartSynth = synthdata->doSynthesis;
  synthdata->doSynthesis = false;
  sleep (1);
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
  if (restartSynth)
    synthdata->doSynthesis = true;

  update();
}

//############################################################################# Start persistence
void ModularSynth::load() {

  QString config_fn;

  config_fn = QFileDialog::getOpenFileName(this, tr("Load Patch"),
					   synthdata->loadPath, tr("AlsaModularSynth files (*.ams)"));
  if (config_fn.isEmpty())
    return;

  load(&config_fn);
}


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
					   synthdata->loadPath, tr("AlsaModularSynth Color files (*.acs)"));
  if (config_fn.isEmpty())
    return;

  if (!(f = fopen(config_fn.toLatin1().constData(), "r"))) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not open file.");
  } else {
    while(Fscanf(f, "%s", sc) != EOF) {
      qs = QString(sc);
      if (qs.contains("ColorBackground", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorBackground);
      }        
      if (qs.contains("ColorModuleBackground", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorModuleBackground);
      }        
      if (qs.contains("ColorModuleBorder", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorModuleBorder);
      }        
      if (qs.contains("ColorModuleFont", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorModuleFont);
      }        
      if (qs.contains("ColorJack", Qt::CaseInsensitive)) {
        setColor(f, synthdata->colorJack);
      }        
      if (qs.contains("ColorCable", Qt::CaseInsensitive)) {
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
					   synthdata->savePath, tr("AlsaModularSynth Color files (*.acs)"));
  if (config_fn.isEmpty())
    return;

  if (!(f = fopen(config_fn.toLatin1().constData(), "w"))) { 
    QMessageBox::information( this, "AlsaModularSynth", "Could not save file.");
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

void ModularSynth::load(QString *presetName) {

  int l1;
  int M_type, moduleID, index, value, x, y, w, h, subID1, subID2;
  int index1, index2, moduleID1, moduleID2, midiSign;
  int index_read1, index_read2, moduleID_read1, moduleID_read2;
  int type, ch, param, isLogInt, sliderMin, sliderMax;
  int red1, green1, blue1, red2, green2, blue2;
  FILE *f;
  QString config_fn, qs, qs2, ladspaLibName, pluginName, para, scalaName;
  char sc[2048];
  bool restartSynth, isLog, ladspaLoadErr, commentFlag, followConfig;
  int newLadspaPolyFlag, textEditID;
  Module *m;
  int currentProgram;

  restartSynth = synthdata->doSynthesis; 
  synthdata->doSynthesis = false;
  followConfig = midiWidget->followConfig;
  midiWidget->followConfig = false;
  config_fn = *presetName;
  currentProgram = -1;
  if (!(f = fopen(config_fn.toLatin1().constData(), "r"))) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not open file.");  
  } else {
    clearConfig();
    qs2 = config_fn.mid(config_fn.lastIndexOf('/') + 1);
    qs.sprintf("AlsaModularSynth " AMS_VERSION " - %d:%d - (%d) - %s", clientid, portid, synthdata->poly, qs2.toLatin1().constData());
    mainWindow->setWindowTitle(qs);
    ladspaLoadErr = false;
    commentFlag = false;
    loadingPatch = true;
    while(Fscanf(f, "%s", sc) != EOF) {
      qs = QString(sc);
      if (qs.contains("#PARA#", Qt::CaseInsensitive)) {
        commentFlag = true;
      }
      if (qs.contains("#ARAP#", Qt::CaseInsensitive)) {
        commentFlag = false;
      }
      if (qs.contains("Module", Qt::CaseInsensitive) && !commentFlag) {
        Fscanf(f, "%d", &M_type);
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &newBoxPos.rx());
        Fscanf(f, "%d", &newBoxPos.ry());

        switch ((M_typeEnum)M_type) {
          case M_type_ladspa: 
            Fscanf(f, "%d", &newLadspaPolyFlag);
            Fscanf(f, "%s", sc);
            ladspaLibName = QString(sc);
            fgets(sc, 2048, f);
            sc[strlen(sc)-1] = '\0';
            pluginName = QString(sc+1);
            StdErr << "Loading LADSPA plugin \"" << pluginName <<
	      "\" from library \"" << ladspaLibName << "\"." << endl;
            if (!synthdata->getLadspaIDs(ladspaLibName, pluginName, &subID1, &subID2)) {
              QMessageBox::information(this, "AlsaModularSynth",
				       "Could not find LADSPA plugin \"" +
				       pluginName + "\" from library \"" +
				       ladspaLibName + "\".");
              ladspaLoadErr = true;
            }
            break;
          case M_type_scquantizer: 
          case M_type_scmcv: 
            Fscanf(f, "%s", sc);
            scalaName = QString(sc);
            break;
          default: 
            Fscanf(f, "%d", &subID1);
            Fscanf(f, "%d", &subID2);
            break;
        }

        switch((M_typeEnum)M_type) {
          case M_type_custom: 
            break;
          case M_type_vco: 
            newM_vco();
            break;
          case M_type_vco2: 
            newM_vco2();
            break;

	 case M_type_vca: 
            if (subID1) newM_vca_exp();
            else        newM_vca_lin();
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
              newM_ladspa(subID1, subID2, newLadspaPolyFlag & 2, newLadspaPolyFlag & 1);
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
        m = listModule.at(listModule.count()-1);
        m->moduleID=moduleID;
        qs = m->configDialog->windowTitle();
        qs2 = qs.left(qs.lastIndexOf(" "));
        qs.sprintf(" %d", moduleID);
//        fprintf(stderr, "load() setCaption %s%s\n", qs2.latin1(), qs.latin1());
        m->configDialog->setWindowTitle(qs2+qs);
        midiWidget->addModule(m);
        if (synthdata->moduleID <= moduleID) {
          synthdata->moduleID = moduleID+1;
        }
      }
      if (qs.contains("Comment", Qt::CaseInsensitive) && !commentFlag) {
        Fscanf(f, "%d", &textEditID);
        Fscanf(f, "%d", &textEditID); // TODO textEditID is not needed yet
        Fscanf(f, "%d", &newBoxPos.rx());
        Fscanf(f, "%d", &newBoxPos.ry());
        Fscanf(f, "%d", &w);
        Fscanf(f, "%d", &h);
        new_textEdit(w, h);
      }
    }
    rewind(f);
    while((Fscanf(f, "%s", sc) != EOF) && !ladspaLoadErr) {
      qs = QString(sc);
      if (qs.contains("Port", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &index1); 
        Fscanf(f, "%d", &index2);
        Fscanf(f, "%d", &moduleID1);
        Fscanf(f, "%d", &moduleID2); 
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
      }
      if (qs.contains("ColorP", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &index1); 
        Fscanf(f, "%d", &index2);
        Fscanf(f, "%d", &moduleID1);
        Fscanf(f, "%d", &moduleID2); 
        Fscanf(f, "%d", &red1); 
        Fscanf(f, "%d", &green1); 
        Fscanf(f, "%d", &blue1); 
        Fscanf(f, "%d", &red2); 
        Fscanf(f, "%d", &green2); 
        Fscanf(f, "%d", &blue2); 
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
      if (qs.contains("FSlider", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &value);
        Fscanf(f, "%d", &isLogInt);
        Fscanf(f, "%d", &sliderMin);
        Fscanf(f, "%d", &sliderMax);
        Fscanf(f, "%d", &midiSign);
        isLog = isLogInt == 1;
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setLogMode(isLog);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->updateValue(value);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setLogMode(isLog);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setNewMin(sliderMin);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->setNewMax(sliderMax);
            listModule.at(l1)->configDialog->midiSliderList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("ISlider", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &value);
        Fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->intMidiSliderList.at(index)->midiSign = midiSign;
            listModule.at(l1)->configDialog->intMidiSliderList.at(index)->updateValue((int)value);
            listModule.at(l1)->configDialog->intMidiSliderList.at(index)->slider->setValue((int)value);
            break;
          }
        }
      }
      if (qs.contains("LSlider", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &value);
        Fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->midiSign = midiSign;
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->updateValue((int)value);
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->slider->setValue((int)value);
            break;
          }
        }
      }
      if (qs.contains("ComboBox", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &value);
        Fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->midiComboBoxList.at(index)->comboBox->setCurrentIndex(value);
            listModule.at(l1)->configDialog->midiComboBoxList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("CheckBox", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &value);
        Fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->checkBox->setChecked(value==1);
            listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("Function", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &subID1);
        Fscanf(f, "%d", &subID2);
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->functionList.at(index)->setPointCount(subID2);
            break;
          }
        }
      }
      if (qs.contains("Point", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &subID1);
        Fscanf(f, "%d", &subID2); 
        Fscanf(f, "%d", &x); 
        Fscanf(f, "%d", &y); 
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->functionList.at(index)->setPoint(subID1, subID2, x, y);
          }
        }
      }
      if (qs.contains("MIDI", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);   
        Fscanf(f, "%d", &type);   
        Fscanf(f, "%d", &ch);
        Fscanf(f, "%d", &param);
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
        if (qs.contains("FSMIDI", Qt::CaseInsensitive)) {
          listModule.at(l1)->configDialog->midiSliderList.at(index)->connectToController(mck);
        }
        if (qs.contains("ISMIDI", Qt::CaseInsensitive)) {
          listModule.at(l1)->configDialog->intMidiSliderList.at(index)->connectToController(mck);
        }
        if (qs.contains("LSMIDI", Qt::CaseInsensitive)) {
          listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->connectToController(mck);
        }
        if (qs.contains("CMIDI", Qt::CaseInsensitive)) {
          listModule.at(l1)->configDialog->midiComboBoxList.at(index)->connectToController(mck);
        }
        if (qs.contains("TMIDI", Qt::CaseInsensitive)) {
          listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->connectToController(mck);
        }
      }
      if (qs.contains("#PARA#", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &textEditID);
        Fscanf(f, "%d", &textEditID);
        Fscanf(f, "%d", &index);
        Fscanf(f, "%s", sc);
        qs = QString(sc);
        if (!qs.contains("#ARAP#", Qt::CaseInsensitive)) {
           para = QString(sc) + " ";
        } else {
          para = " ";
        }
        while (!qs.contains("#ARAP#", Qt::CaseInsensitive)) {
          Fscanf(f, "%s", sc);
          qs = QString(sc);
          if (!qs.contains("#ARAP#", Qt::CaseInsensitive)) {
            para.append(qs+" "); 
          }
        }
	//	printf("%i:%s:\n", index, para.toLatin1().data());
        listTextEdit.at(textEditID)->textEdit->append(para);
      }
      if (qs.contains("Tab", Qt::CaseInsensitive)) {
        qs.truncate(0);
        do {
          Fscanf(f, "%s", sc);
          qs += QString(sc);
          if (qs.right(1) != "\"") {
            qs += " ";
          }
        } while (qs.right(1) != "\"");
        qs = qs.mid(1, qs.length()-2);
        guiWidget->addTab(qs);
      }
      if (qs.contains("Frame", Qt::CaseInsensitive)) {
        qs.truncate(0);
        do {
          Fscanf(f, "%s", sc);
          qs += QString(sc);
          if (qs.right(1) != "\"") {
            qs += " ";
          }
        } while (qs.right(1) != "\"");
        qs = qs.mid(1, qs.length()-2);
        Fscanf(f, "%d", &index);
        guiWidget->setTab(index);
        guiWidget->addFrame(qs);
      }
      if (qs.contains("Parameter", Qt::CaseInsensitive)) {
        qs.truncate(0);
        do {
          Fscanf(f, "%s", sc);
          qs += QString(sc);
          if (qs.right(1) != "\"") {
            qs += " ";
          }
        } while (qs.right(1) != "\"");
        qs = qs.mid(1, qs.length()-2);
        Fscanf(f, "%d", &moduleID);
        Fscanf(f, "%d", &index);
        for (l1 = 0; l1 < listModule.count(); ++l1) {
          if (listModule.at(l1)->moduleID == moduleID) {
            guiWidget->addParameter(listModule.at(l1)->configDialog->midiGUIcomponentList.at(index), qs);
            if (listModule.at(l1)->configDialog->midiGUIcomponentList.at(index)->componentType == GUIcomponentType_slider) {
              Fscanf(f, "%d", &sliderMin);
              Fscanf(f, "%d", &sliderMax);
              Fscanf(f, "%d", &isLogInt);
              ((MidiSlider *)guiWidget->parameterList.last())->setNewMin(sliderMin);
              ((MidiSlider *)guiWidget->parameterList.last())->setNewMax(sliderMax);
              ((MidiSlider *)guiWidget->parameterList.last())->setLogMode(isLogInt == 1);
            }
            break;
          }
        }
      }  
      if (qs.contains("Program", Qt::CaseInsensitive)) {
        Fscanf(f, "%d", &index);
        Fscanf(f, "%d", &value);
        if (index != currentProgram) {
          currentProgram = index;
          guiWidget->setPresetCount(currentProgram + 1);
        }
        guiWidget->presetList[currentProgram].append(value);
      }
      if (qs.contains("PresetName", Qt::CaseInsensitive)) {
        qs.truncate(0);
        do {
          Fscanf(f, "%s", sc);
          qs += QString(sc);
          if (qs.right(1) != "\"") {
            qs += " ";
          }
        } while (qs.right(1) != "\"");
        qs = qs.mid(1, qs.length()-2);
        qs2.sprintf("%3d", guiWidget->presetNameList.count());
        guiWidget->presetNameList.append(qs2+qs);
      }
    }
    if (guiWidget->presetCount) {
      guiWidget->setCurrentPreset(0);
    }
    fclose(f);
    loadingPatch = false;
  }
  resize();
  update();
  synthdata->doSynthesis = true;
  midiWidget->followConfig = followConfig;
  guiWidget->refreshGui();
}

void ModularSynth::save()
{
  int l1, l2, value;
  FILE *f;
  QString config_fn, qs;
  QStringList::iterator presetit;
   
  StdErr << "synthdata->savePath: " << synthdata->savePath << endl;
  config_fn = QFileDialog::getSaveFileName(this, tr("Save Patch"),
					   synthdata->savePath, tr("AlsaModularSynth files (*.ams)"));
  if (config_fn.isEmpty())
    return;

  if (!(f = fopen(config_fn.toLatin1().constData(), "w"))) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not save file.");
  } else {
    int offX = 0, offY = 0;
    if (childrenRect().left() > 100)
      offX = childrenRect().left() - 100;
    if (childrenRect().top() > 66)
      offY = childrenRect().top() - 66;
    for (l1 = 0; l1 < listModule.count(); ++l1) {
      fprintf(f, "Module %d %d %d %d ", (int)listModule.at(l1)->M_type, listModule.at(l1)->moduleID, 
              listModule.at(l1)->x() - offX,  listModule.at(l1)->y() - offY);

      switch(listModule.at(l1)->M_type)
      {
        case M_type_custom: 
          break;
        case M_type_vca: 
          fprintf(f, "%d 0\n", (int)((M_vca *)listModule.at(l1))->expMode); 
          break;
        case M_type_ad: 
        case M_type_function: 
          fprintf(f, "%d 0\n", listModule.at(l1)->outPortCount);
          break;
        case M_type_mix: 
          fprintf(f, "%d 0\n", ((M_mix *)listModule.at(l1))->in_channels);
          break;
        case M_type_stereomix: 
          fprintf(f, "%d 0\n", ((M_stereomix *)listModule.at(l1))->in_channels);
          break;
        case M_type_vcorgan: 
          fprintf(f, "%d 0\n", ((M_vcorgan *)listModule.at(l1))->oscCount);
          break;
        case M_type_dynamicwaves: 
          fprintf(f, "%d 0\n", ((M_dynamicwaves *)listModule.at(l1))->oscCount);
          break;
        case M_type_seq: 
          fprintf(f, "%d 0\n", ((M_seq *)listModule.at(l1))->seqLen);
          break;
        case M_type_ladspa: 
          fprintf(f, "%d %s %s\n", 2 * (int)((M_ladspa *)listModule.at(l1))->isPoly
                  + (int)((M_ladspa *)listModule.at(l1))->hasExtCtrlPorts, 
                  synthdata->ladspaLib.at(((M_ladspa *)listModule.at(l1))->ladspaDesFuncIndex).
		  name.toLatin1().constData(), 
                  ((M_ladspa *)listModule.at(l1))->pluginName.toLatin1().constData());
          break;
        case M_type_scquantizer: 
          qs = ((M_scquantizer *)listModule.at(l1))->sclname.toLatin1().constData();
          if (qs.contains("/")) {
            qs = qs.mid(qs.lastIndexOf("/") + 1);             
          }
          fprintf(f, "%s\n", qs.toLatin1().constData());
          break;
        case M_type_scmcv: 
          qs = ((M_scmcv *)listModule.at(l1))->sclname.toLatin1().constData();
          if (qs.contains("/")) {
            qs = qs.mid(qs.lastIndexOf("/") + 1);             
          }
          fprintf(f, "%s\n", qs.toLatin1().constData());
          break;
        default:
          fprintf(f, "0 0\n");  
          break; 
      }
      listModule.at(l1)->save(f);
    }
    for (l1 = 0; l1 < listTextEdit.count(); ++l1)
      fprintf(f, "Comment %d %d %d %d %d %d\n", listTextEdit.at(l1)->textEditID, l1, 
	      listTextEdit.at(l1)->x() - offX,
	      listTextEdit.at(l1)->y() - offY, 
	      listTextEdit.at(l1)->width(), listTextEdit.at(l1)->height());

    for (l1 = 0; l1 < listTextEdit.count(); ++l1) {
      TextEdit *tE = listTextEdit.at(l1);
      QTextDocument *tD = tE->textEdit->document();
      QTextBlock tB = tD->begin();
      for (l2 = 0; l2 < tD->blockCount(); ++l2, tB = tB.next()) {
        fprintf(f, "#PARA# %d %d %d\n", tE->textEditID, l1, l2);
	fprintf(f, "%s\n", tB.text().toLatin1().constData());
        fprintf(f, "#ARAP#\n");
      }
    }
//    fprintf(stderr, "Saving Tabs\n");
//    fprintf(stderr, "TabName count: %d Tab count: %d\n", guiWidget->tabNameList.count(), guiWidget->tabList.count());
    for (l1 = 0; l1 < guiWidget->tabList.count(); ++l1)
      fprintf(f, "Tab \"%s\"\n", guiWidget->tabList.at(l1)->parentWidget()->objectName().toAscii().constData());

//    fprintf(stderr, "Saving Frames\n");
    for (l1 = 0; l1 < guiWidget->frameBoxList.count(); ++l1) {
      fprintf(f, "Frame \"%s\" %d\n", guiWidget->frameBoxList.at(l1)->frameBox->parentWidget()->objectName().toAscii().constData(),
                                      guiWidget->frameBoxList.at(l1)->tabIndex);
      for (l2 = 0; l2 < guiWidget->parameterList.count(); ++l2) {
        if (guiWidget->parameterList.at(l2)->parent() == guiWidget->frameBoxList.at(l1)->frameBox->parentWidget()) {
          fprintf(f, "Parameter \"%s\" %d %d ", guiWidget->parameterList.at(l2)->objectName().toAscii().constData(), 
                                         ((Module *)guiWidget->parameterList.at(l2)->parentModule)->moduleID, 
                                         guiWidget->parameterList.at(l2)->midiGUIcomponentListIndex);
          if (guiWidget->parameterList.at(l2)->componentType == GUIcomponentType_slider) {
            fprintf(f, "%d %d %d\n", ((MidiSlider *)guiWidget->parameterList.at(l2))->slider->minimum(), 
                                   ((MidiSlider *)guiWidget->parameterList.at(l2))->slider->maximum(), 
                                   ((MidiSlider *)guiWidget->parameterList.at(l2))->isLog);
          } else {
            fprintf(f, "\n");
          }
        }
      }
    }

    for (l1 = 0; l1 < guiWidget->presetCount; ++l1) {
      for (int p = 0; p < guiWidget->presetList[l1].count(); p++) {
        value = guiWidget->presetList[l1][p];
        fprintf(f, "Program %d %d\n", l1, value);
      } 
    }
    for (presetit = guiWidget->presetNameList.begin(); presetit != guiWidget->presetNameList.end(); ++presetit) {
      fprintf(f, "PresetName \"%s\"\n", (*presetit).mid(3).toLatin1().constData());
    } 
    fclose(f);
  }
}
//==================================================================== End persistence
void ModularSynth::allVoicesOff() {

  int l1, l2;

  for (l2 = 0; l2 < synthdata->poly; ++l2) {
    if (synthdata->noteCounter[l2] < 1000000) {
      synthdata->noteCounter[l2] = 1000000;
      for (l1 = 0; l1 < listModule.count(); ++l1) {      
        listModule.at(l1)->noteOffEvent(l2);
      } 
    }
  } 
}

void ModularSynth::cleanUpSynth()
{
  prefWidget->savePref(synthdata->rcPath);
  fprintf(stderr, "Closing synth...\n");
  delete this;
}

void ModularSynth::showContextMenu(QPoint pos) {
  
    contextMenu->popup(mapToGlobal(pos));
}

void ModularSynth::refreshColors() {

  int l1, l2;
  for (l1 = 0; l1 < listModule.count(); ++l1) {
    listModule[l1]->getColors();
    for (l2 = 0; l2 < listModule.at(l1)->portList.count(); ++l2) {
      listModule.at(l1)->portList.at(l2)->setPalette(QPalette(synthdata->colorModuleBackground, synthdata->colorModuleBackground));
      listModule.at(l1)->portList.at(l2)->colorFont1 = synthdata->colorPortFont1;
      listModule.at(l1)->portList.at(l2)->colorFont2 = synthdata->colorPortFont2;
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

void ModularSynth::moveAllBoxes(QPoint const &delta)
{
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
