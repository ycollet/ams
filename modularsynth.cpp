#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>
#include <qregexp.h>
#include <qthread.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qvbox.h>
#include <qpopupmenu.h>
#include <qpointarray.h>
#include <qmessagebox.h>
#include <qsocketnotifier.h>
#include <qfiledialog.h>
#include <qmainwindow.h>
#include <qimage.h>
#include <qpen.h>
#include <qscrollview.h>
#include <qlistview.h>
#include <alsa/asoundlib.h>
#include <ladspa.h>
#include "modularsynth.h"
#include "port.h"
#include "midiwidget.h"
#include "guiwidget.h"
#include "midicontroller.h"
#include "ladspadialog.h"


ModularSynth::ModularSynth (QWidget *parent, const char *p_pcmname, int p_fsamp, int p_frsize, int p_nfrags,
                            int p_ncapt, int p_nplay, int poly, float edge) 
    : QScrollView (parent, "", Qt::WResizeNoErase | Qt::WRepaintNoErase | Qt::WStaticContents),
      pcmname (p_pcmname), fsamp (p_fsamp), frsize (p_frsize), nfrags (p_nfrags),
      ncapt (p_ncapt), nplay (p_nplay)
{
  firstPort = true;
  connectingPort[0] = NULL;
  connectingPort[1] = NULL;
  connectorStyle = CONNECTOR_BEZIER;
  aboutWidget = new QMessageBox(this); 
  mainWindow = (QMainWindow *)parent;
  clientid = 0;
  portid = 0;

  synthdata = new SynthData (poly, edge);

  midiWidget = new MidiWidget(synthdata, NULL);
  midiWidget->setCaption("AlsaModularSynth Control Center");
  synthdata->midiWidget = (QObject *)midiWidget;
  guiWidget = new GuiWidget(synthdata, NULL);
  guiWidget->setCaption("AlsaModularSynth Parameter View");
  synthdata->guiWidget = (QObject *)guiWidget;

  presetPath = "";
  ladspaDialog = new LadspaDialog(synthdata, NULL);
  QObject::connect(ladspaDialog, SIGNAL(createLadspaModule(int, int, bool, bool)),
                   this, SLOT(newM_ladspa(int, int, bool, bool)));
  setPalette(QPalette(QColor(240, 240, 255), QColor(240, 240, 255)));
  loadingPatch = false;
}

ModularSynth::~ModularSynth()
{
  delete synthdata;
}

void ModularSynth::viewportPaintEvent(QPaintEvent *pe) {
  
  QPixmap pm(visibleWidth(), visibleHeight());
  QPainter p(&pm);
  QPen *pen;
  QPointArray qpa(4);
  int l1, l2;
  Port *port[2];
  int port_x[2], port_y[2];
  QPoint port_pos[2];
  int moduleX[2], moduleY[2];

  pm.fill(QColor((unsigned int)COLOR_MAINWIN_BG));
  p.setPen(QColor((unsigned int)COLOR_CONNECT_LINE));
  pen = new QPen(QColor(220, 216, 216), 3);
  for (l1 = 0; l1 < listModule.count(); l1++) {
    for (l2 = 0; l2 < listModule.at(l1)->portList.count(); l2++) {
      port[0] = listModule.at(l1)->portList.at(l2);
      if ((port[0]->dir == PORT_IN) && port[0]->connectedPortList.count()) {
        port[1] = port[0]->connectedPortList.at(0);
        port_pos[0] = port[0]->pos();
        port_pos[1] = port[1]->pos();
        contentsToViewport(childX(port[0]->parentModule), childY(port[0]->parentModule), moduleX[0], moduleY[0]);
        contentsToViewport(childX(port[1]->parentModule), childY(port[1]->parentModule), moduleX[1], moduleY[1]);
        port_x[0] = port_pos[0].x() + moduleX[0];
        port_x[1] = port_pos[1].x() + port[1]->width() + moduleX[1];
        port_y[0] = port_pos[0].y() + moduleY[0] + port[0]->height()/2;
        port_y[1] = port_pos[1].y() + moduleY[1] + port[1]->height()/2;
        if (connectorStyle == CONNECTOR_BEZIER) {
          qpa.setPoint(0, port_x[0], port_y[0]);
          qpa.setPoint(1, (port_x[1] - port_x[0]) / 2 + port_x[0], 
                          (port_y[1] - port_y[0]) / 2 + port_y[0] + 50);
          qpa.setPoint(2, port_x[1], port_y[1]);
          qpa.setPoint(3, port_x[1], port_y[1]);
          pen->setWidth(5);
          pen->setColor(QColor(COLOR_CONNECT_BEZ1));
          p.setPen(*pen);
          p.drawCubicBezier(qpa);
          pen->setWidth(3);
          pen->setColor(QColor(COLOR_CONNECT_BEZ2));
          p.setPen(*pen);
          p.drawCubicBezier(qpa);
          pen->setWidth(1);
          pen->setColor(QColor(COLOR_CONNECT_BEZ3));
          p.setPen(*pen);          
          p.drawCubicBezier(qpa);
        }
        if (connectorStyle == CONNECTOR_STRAIGHT) {
          p.drawLine(port_x[0], port_y[0], port_x[1], port_y[1]);
        }
        if (port[0]->parentModule->x() < port[1]->parentModule->x()) {
          pen->setWidth(5);
          pen->setColor(QColor(COLOR_CONNECT_BEZ1));
          p.setPen(*pen);
          p.drawLine(port_x[0], port_y[0], port_x[0] - 5, port_y[0]);
          pen->setWidth(3);
          pen->setColor(QColor(COLOR_CONNECT_BEZ2));
          p.setPen(*pen);
          p.drawLine(port_x[0], port_y[0], port_x[0] - 6, port_y[0]);
          pen->setWidth(1);
          pen->setColor(QColor(COLOR_CONNECT_BEZ3));
          p.setPen(*pen);
          p.drawLine(port_x[0], port_y[0], port_x[0] - 7, port_y[0]);
        }
        if (port[1]->parentModule->x() > port[0]->parentModule->x()) {
          pen->setWidth(5);
          pen->setColor(QColor(COLOR_CONNECT_BEZ1));
          p.setPen(*pen);
          p.drawLine(port_x[1], port_y[1], port_x[1] + 5, port_y[1]);
          pen->setWidth(3);
          pen->setColor(QColor(COLOR_CONNECT_BEZ2));
          p.setPen(*pen);
          p.drawLine(port_x[1], port_y[1], port_x[1] + 6, port_y[1]);
          pen->setWidth(1);
          pen->setColor(QColor(COLOR_CONNECT_BEZ3));
          p.setPen(*pen);
          p.drawLine(port_x[1], port_y[1], port_x[1] + 7, port_y[1]);
        }
      }
    }
  }
  bitBlt(viewport(), 0, 0, &pm);
  delete pen;
}
 
void ModularSynth::mousePressEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:  
    break;
  case Qt::RightButton:
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
    connectorStyle = (connectorStyle == CONNECTOR_STRAIGHT) ? CONNECTOR_BEZIER : CONNECTOR_STRAIGHT;
    repaintContents(false);
    break;
  default:
    break;
  }
}  
   
QSize ModularSynth::sizeHint() const {

  return QSize(SYNTH_MINIMUM_WIDTH, SYNTH_MINIMUM_HEIGHT);
}

QSizePolicy ModularSynth::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}


int ModularSynth::go (bool withJack) {

  synthdata->seq_handle = open_seq();
  initSeqNotifier();

  if (withJack)
  {
    synthdata->initJack (ncapt, nplay);
    synthdata->doSynthesis = true;
  }
  else
  {
    synthdata->initAlsa (pcmname, fsamp, frsize, nfrags, ncapt, nplay);
    synthdata->doSynthesis = true;
  }
  return 0;
}

void ModularSynth::displayAbout() {
 
    aboutWidget->about(this, "About AlsaModularSynth", "AlsaModularSynth 1.7.2\n"
                     "by Matthias Nagorni and Fons Adriaensen\n"
                     "(c)2002-2003 SuSE AG Nuremberg\n"
                     "(c)2003 Fons Adriaensen\n\n" 
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

void ModularSynth::displayLadspaPlugins() {
   
  ladspaDialog->show();  
  ladspaDialog->raise();
}

int ModularSynth::setPresetPath(QString name) {

  presetPath = name;
  return(0);
}

int ModularSynth::setSavePath(QString name) {

  savePath = name;
  return(0);
}

snd_seq_t *ModularSynth::open_seq() {

  snd_seq_t *seq_handle;
  int l1;
  QString qs;

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "AlsaModularSynth");
  clientid = snd_seq_client_id(seq_handle);
  if ((portid = snd_seq_create_simple_port(seq_handle, "ams",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  for (l1 = 0; l1 < 2; l1++) {
    if ((synthdata->midi_out_port[l1] = snd_seq_create_simple_port(seq_handle, "ams",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      fprintf(stderr, "Error creating sequencer port.\n");
      exit(1);
    }
  }
  qs.sprintf("AlsaModularSynth 1.7.1 - %d:%d", clientid, portid);
  mainWindow->setCaption(qs);
  synthdata->jackName.sprintf("ams_%d_%d", clientid, portid);
  return(seq_handle);
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

void ModularSynth::midiAction(int fd) {

  snd_seq_event_t *ev;
  QString qs;
  int l1, l2, osc;
  bool noteActive, foundOsc;
  float min_e;
  MidiController *midiController; 

  do {
    snd_seq_event_input(synthdata->seq_handle, &ev);
    if (midiWidget->isVisible()) {
      if ((ev->type == SND_SEQ_EVENT_CONTROLLER)  
        ||(ev->type == SND_SEQ_EVENT_CONTROL14)   
        ||(ev->type == SND_SEQ_EVENT_PITCHBEND)) {
        MidiController *midiController = new MidiController();
        midiController->type = ev->type;
        midiController->ch = ev->data.control.channel;  
        midiController->param = (ev->type==SND_SEQ_EVENT_PITCHBEND) 
                              ? 0 : ev->data.control.param; 
        if (!midiWidget->midiControllerList.contains(midiController)) {
          midiWidget->addMidiController(midiController);
        } else {
          delete midiController;
        } 
      } 
      if (midiWidget->noteControllerEnabled &&((ev->type == SND_SEQ_EVENT_NOTEON)
        ||(ev->type == SND_SEQ_EVENT_NOTEOFF))) {
        MidiController *midiController = new MidiController();
        midiController->type = ev->type;
        midiController->ch = ev->data.control.channel;
        midiController->param = ev->data.note.note;
        if (!midiWidget->midiControllerList.contains(midiController)) {
          midiWidget->addMidiController(midiController);       
        } else {
          delete midiController;
        } 
      }
    }  
    if (midiWidget->followMidi) {
      for(l1 = 0; l1 < midiWidget->midiControllerList.count(); l1++) {
        midiController = midiWidget->midiControllerList.at(l1);
        if ((midiController->type == ev->type)
          &&(midiController->ch == ev->data.control.channel)) {
          if (ev->type == SND_SEQ_EVENT_PITCHBEND) {
            emit midiController->sendMidiValue((ev->data.control.value + 8192) / 128);
            midiWidget->setSelectedController(midiController);
          }
          if ((ev->type == SND_SEQ_EVENT_CONTROL14)
           && (midiController->param == ev->data.control.param)) {
            emit midiController->sendMidiValue(ev->data.control.value / 128);
            midiWidget->setSelectedController(midiController);
          }
          if ((ev->type == SND_SEQ_EVENT_CONTROLLER) 
           && (midiController->param == ev->data.control.param)) {
            emit midiController->sendMidiValue(ev->data.control.value);
            midiWidget->setSelectedController(midiController);
          }
          if ((ev->type == SND_SEQ_EVENT_NOTEON) 
           && (midiController->param == ev->data.note.note)) {
            emit midiController->sendMidiValue(ev->data.note.velocity);
            midiWidget->setSelectedController(midiController);
          }
          if ((ev->type == SND_SEQ_EVENT_NOTEOFF) 
           && (midiController->param == ev->data.note.note)) {
            emit midiController->sendMidiValue(0);
            midiWidget->setSelectedController(midiController);
          }
        }    
      }
    } else {
      for(l1 = 0; l1 < midiWidget->midiControllerList.count(); l1++) {
        midiController = midiWidget->midiControllerList.at(l1);
        if ((midiController->type == ev->type)
          &&(midiController->ch == ev->data.control.channel)) {
          if (ev->type == SND_SEQ_EVENT_PITCHBEND) {
            emit midiController->sendMidiValue((ev->data.control.value + 8192) / 128);
          }
          if ((ev->type == SND_SEQ_EVENT_CONTROL14)
           && (midiController->param == ev->data.control.param)) {
            emit midiController->sendMidiValue(ev->data.control.value / 128);
          }
          if ((ev->type == SND_SEQ_EVENT_CONTROLLER) 
           && (midiController->param == ev->data.control.param)) {
            emit midiController->sendMidiValue(ev->data.control.value);
          }
          if ((ev->type == SND_SEQ_EVENT_NOTEON) 
           && (midiController->param == ev->data.note.note)) {
            emit midiController->sendMidiValue(ev->data.note.velocity);
          }
          if ((ev->type == SND_SEQ_EVENT_NOTEOFF) 
           && (midiController->param == ev->data.note.note)) {
            emit midiController->sendMidiValue(0);
          }
        }    
      }
    }   
    if (((ev->type == SND_SEQ_EVENT_NOTEON) || (ev->type == SND_SEQ_EVENT_NOTEOFF)) 
         && ((synthdata->midiChannel < 0) || (synthdata->midiChannel == ev->data.control.channel))) {
      for (l2 = 0; l2 < synthdata->poly; l2++) {
        noteActive = false; 
        for (l1 = 0; l1 < synthdata->listM_env.count(); l1++) {
          if (((M_env *)synthdata->listM_env.at(l1))->noteActive[l2]) {   
            noteActive = true;
          }
        }  
        for (l1 = 0; l1 < synthdata->listM_vcenv.count(); l1++) {
          if (((M_vcenv *)synthdata->listM_vcenv.at(l1))->noteActive[l2]) {   
            noteActive = true;
          }
        }  
        for (l1 = 0; l1 < synthdata->listM_advenv.count(); l1++) {
          if (((M_advenv *)synthdata->listM_advenv.at(l1))->noteActive[l2]) {   
            noteActive = true;
          }
        }  
        for (l1 = 0; l1 < synthdata->listM_dynamicwaves.count(); l1++) {
          if (((M_dynamicwaves *)synthdata->listM_dynamicwaves.at(l1))->noteActive[l2]) {   
            noteActive = true;
          }
        }  
        synthdata->noteActive[l2] = noteActive || synthdata->notePressed[l2];
      }
      if ((ev->type == SND_SEQ_EVENT_NOTEON) && (ev->data.note.velocity > 0)) {
        foundOsc = false;
        for (l2 = 0; l2 < synthdata->poly; l2++) {
          if (!synthdata->noteActive[l2]) {
            foundOsc = true;
            synthdata->noteActive[l2] = true;
            synthdata->notePressed[l2] = true;
            synthdata->velocity[l2] = ev->data.note.velocity;
            synthdata->channel[l2] = ev->data.note.channel;  
            synthdata->notes[l2] = ev->data.note.note;
            for (l1 = 0; l1 < listModule.count(); l1++) {
              listModule.at(l1)->noteOnEvent(l2);  
            }
            break;
          }
        } 
        if ((synthdata->listM_advenv.count() 
          || synthdata->listM_env.count() 
          || synthdata->listM_vcenv.count() 
          || synthdata->listM_dynamicwaves.count())
          && !foundOsc) {
          min_e = 1.0;
          osc = 0;
          for (l2 = 0; l2 < synthdata->poly; l2++) {
            for (l1 = 0; l1 < synthdata->listM_dynamicwaves.count(); l1++) {
              if (((M_dynamicwaves *)synthdata->listM_dynamicwaves.at(l1))->noteActive[l2]) {
                if (((M_dynamicwaves *)synthdata->listM_dynamicwaves.at(l1))->e[l2][0] < min_e) {
                   min_e = ((M_dynamicwaves *)synthdata->listM_dynamicwaves.at(l1))->e[l2][0];
                   osc = l2;
                }
              }
            }
            for (l1 = 0; l1 < synthdata->listM_env.count(); l1++) {
              if (((M_env *)synthdata->listM_env.at(l1))->noteActive[l2]) {
                if (((M_env *)synthdata->listM_env.at(l1))->e[l2] < min_e) {
                   min_e = ((M_env *)synthdata->listM_env.at(l1))->e[l2];
                   osc = l2;
                }
              }
            }
            for (l1 = 0; l1 < synthdata->listM_vcenv.count(); l1++) {
              if (((M_vcenv *)synthdata->listM_vcenv.at(l1))->noteActive[l2]) {
                if (((M_vcenv *)synthdata->listM_vcenv.at(l1))->e[l2] < min_e) {
                   min_e = ((M_vcenv *)synthdata->listM_vcenv.at(l1))->e[l2];
                   osc = l2;
                }
              }
            }
            for (l1 = 0; l1 < synthdata->listM_advenv.count(); l1++) {
              if (((M_advenv *)synthdata->listM_advenv.at(l1))->noteActive[l2]) {
                if (((M_advenv *)synthdata->listM_advenv.at(l1))->e[l2] < min_e) {
                   min_e = ((M_advenv *)synthdata->listM_advenv.at(l1))->e[l2];
                   osc = l2;
                }
              }
            }
          }
          synthdata->noteActive[osc] = true; 
          synthdata->notePressed[osc] = true;
          synthdata->velocity[osc] = ev->data.note.velocity;
          synthdata->channel[osc] = ev->data.note.channel;
          synthdata->notes[osc] = ev->data.note.note;   
          for (l1 = 0; l1 < listModule.count(); l1++) {
            listModule.at(l1)->noteOnEvent(osc);
          }
        }
      } else {
        for (l2 = 0; l2 < synthdata->poly; l2++) {
          if ((synthdata->notes[l2] == ev->data.note.note)
            && (synthdata->channel[l2] == ev->data.note.channel)) {
            synthdata->notePressed[l2] = false;
            for (l1 = 0; l1 < listModule.count(); l1++) {
              listModule.at(l1)->noteOffEvent(l2);
            } 
          }   
        }     
      }       
    }
    if ((ev->type == SND_SEQ_EVENT_CONTROLLER) && (ev->data.control.param == MIDI_CTL_ALL_NOTES_OFF)) {
      for (l2 = 0; l2 < synthdata->poly; l2++) {
        if (synthdata->notePressed[l2] && (synthdata->channel[l2] == ev->data.note.channel)) {
          synthdata->notePressed[l2] = false;
          synthdata->noteActive[l2] = false;
          for (l1 = 0; l1 < listModule.count(); l1++) {
            listModule.at(l1)->noteOffEvent(l2);
          } 
        }   
      }     
    }
    if (ev->type == SND_SEQ_EVENT_PGMCHANGE) {
      guiWidget->setCurrentPreset(ev->data.control.value);
    }
    for (l1 = 0; l1 < synthdata->listM_advmcv.count(); l1++) {
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

  for (l1 = 0; l1 < m->portList.count(); l1++) {
    QObject::connect(m->portList.at(l1), SIGNAL(portClicked()), 
                     this, SLOT(portSelected()));
    QObject::connect(m->portList.at(l1), SIGNAL(portDisconnected()), 
                     this, SLOT(updatePortConnections()));
  }
}

void ModularSynth::initNewModule(Module *m) {

  int cx, cy;

  addChild(m);
  viewportToContents((visibleWidth()-m->width())>>1, (visibleHeight()-m->height())>>1, cx, cy);
  moveChild(m, cx, cy);
  m->show();
  QObject::connect(m, SIGNAL(dragged(QPoint)), this, SLOT(moveModule(QPoint)));
  QObject::connect(m, SIGNAL(removeModule()), this, SLOT(deleteModule()));
  listModule.append(m);
  if (!loadingPatch) {
    midiWidget->addModule(m);
  }
  initPorts(m);
}

void ModularSynth::new_textEdit() {

  int cx, cy;

  TextEdit *te = new TextEdit(viewport(), "textEdit", synthdata);
  addChild(te);
  viewportToContents((visibleWidth()-te->width())>>1, (visibleHeight()-te->height())>>1, cx, cy);
  moveChild(te, cx, cy);
  te->show();
  QObject::connect(te, SIGNAL(dragged(QPoint)), this, SLOT(moveTextEdit(QPoint)));
  QObject::connect(te, SIGNAL(sizeDragged(QPoint)), this, SLOT(resizeTextEdit(QPoint)));
  QObject::connect(te, SIGNAL(removeTextEdit()), this, SLOT(deleteTextEdit()));
  listTextEdit.append(te);
}

void ModularSynth::new_textEdit(int x, int y, int w, int h) {

  TextEdit *te = new TextEdit(viewport(), "textEdit", synthdata);
  addChild(te);
  te->setFixedSize(w, h);
  moveChild(te, x, y);
  te->show();
  QObject::connect(te, SIGNAL(dragged(QPoint)), this, SLOT(moveTextEdit(QPoint)));
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

void ModularSynth::newM_seq(int seqLen) {

  M_seq *m = new M_seq(seqLen, viewport(), "Sequencer", synthdata);
  initNewModule((Module *)m);
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
 
  M_vcorgan *m = new M_vcorgan(oscCount, viewport(), "VC_Organ", synthdata);
  initNewModule((Module *)m);
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
 
  M_dynamicwaves *m = new M_dynamicwaves(oscCount, viewport(), "DynamicWaves", synthdata);
  synthdata->listM_dynamicwaves.append(m);
  initNewModule((Module *)m);
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

  M_mcv *m = new M_mcv(viewport(), "MCV", synthdata);
  synthdata->listM_mcv.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_advmcv() {

  M_advmcv *m = new M_advmcv(viewport(), "Adv_MCV", synthdata);
  synthdata->listM_advmcv.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_scmcv() {

  M_scmcv *m = new M_scmcv(viewport(), "Scala_MCV", synthdata);
  synthdata->listM_scmcv.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_scmcv(QString *p_scalaName) {

  M_scmcv *m = new M_scmcv(viewport(), "Scala_MCV", synthdata, p_scalaName);
  synthdata->listM_scmcv.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_env() {

  M_env *m = new M_env(viewport(), "ENV", synthdata);
  synthdata->listM_env.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_vcenv() {

  M_vcenv *m = new M_vcenv(viewport(), "VC_ENV", synthdata);
  synthdata->listM_vcenv.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_advenv() {

  M_advenv *m = new M_advenv(viewport(), "Adv_ENV", synthdata);
  synthdata->listM_advenv.append(m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_vco() {

  M_vco *m = new M_vco(viewport(), "VCO", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_vca_lin() {

  M_vca *m = new M_vca(false, viewport(), "Lin VCA", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_vca_exp() {

  M_vca *m = new M_vca(true, viewport(), "Exp VCA", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_lfo() {

  M_lfo *m = new M_lfo(viewport(), "LFO", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_noise() {

  M_noise *m = new M_noise(viewport(), "Noise", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_ringmod() {

  M_ringmod *m = new M_ringmod(viewport(), "Ringmod", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_inv() {

  M_inv *m = new M_inv(viewport(), "INV", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_conv() {

  M_conv *m = new M_conv(viewport(), "Converter", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_cvs() {

  M_cvs *m = new M_cvs(viewport(), "CV Source", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_sh() {

  M_sh *m = new M_sh(viewport(), "Sample/Hold", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_vcswitch() {

  M_vcswitch *m = new M_vcswitch(viewport(), "VC_Switch", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_slew() {

  M_slew *m = new M_slew(viewport(), "Slew Limiter", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_quantizer() {

  M_quantizer *m = new M_quantizer(viewport(), "Quantizer", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_scquantizer(QString *p_scalaName) {

  M_scquantizer *m = new M_scquantizer(viewport(), "Scala Quant", synthdata, p_scalaName);
  initNewModule((Module *)m);
}

void ModularSynth::newM_scquantizer() {

  M_scquantizer *m = new M_scquantizer(viewport(), "Scala Quant", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_delay() {

  M_delay *m = new M_delay(viewport(), "Delay", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_mix(int in_channels) {

  M_mix *m = new M_mix(in_channels, viewport(), "Mixer", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::newM_stereomix(int in_channels) {

  M_stereomix *m = new M_stereomix(in_channels, viewport(), "Stero mix", synthdata);
  initNewModule((Module *)m);
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

  qs.sprintf("%s", synthdata->ladspa_dsc_func_list[p_ladspaDesFuncIndex](n)->Name);
  M_ladspa *m = new M_ladspa(viewport(), "M_ladspa", synthdata, p_ladspaDesFuncIndex, n, p_newLadspaPoly, p_extCtrlPorts);
  initNewModule((Module *)m);
}

void ModularSynth::newM_wavout() {

  M_wavout *m = new M_wavout(viewport(), "WAV Out", synthdata);
  synthdata->wavoutModuleList.append((QObject *)m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_midiout() {

  M_midiout *m = new M_midiout(viewport(), "Midi Out", synthdata);
  synthdata->midioutModuleList.append((QObject *)m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_pcmout()
{
    int k;
    M_pcmout *M; 

    k = synthdata->find_play_mod (0);
    if (k >= 0) 
    {
        M = new M_pcmout (viewport(), "PCM Out", synthdata, 2 * k);
        initNewModule ((Module *)M);
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
        M = new M_pcmin (viewport(), "PCM_In", synthdata, 2 * k);
        initNewModule ((Module *)M);
        synthdata->set_capt_mod (k, M);
    } 
    else fprintf (stderr, "All available input ports are in use\n");
}

void ModularSynth::newM_scope() {

  M_scope *m = new M_scope(viewport(), "Scope", synthdata);
  synthdata->scopeModuleList.append((QObject *)m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_spectrum() {

  M_spectrum *m = new M_spectrum(viewport(), "Spectrum", synthdata);
  synthdata->spectrumModuleList.append((QObject *)m);
  initNewModule((Module *)m);
}

void ModularSynth::newM_vcf() {
 
  M_vcf *m = new M_vcf(viewport(), "VCF", synthdata);
  initNewModule((Module *)m);
}

void ModularSynth::moveModule(QPoint pos) {

  int l1;
  Module *m;
  int cx, cy;
  
  for (l1 = 0; l1 < listModule.count(); l1++) {
    if ((m=listModule.at(l1)) == sender()) {
      viewportToContents(pos.x() - m->getMousePressPos().x() + m->pos().x(),
                         pos.y() - m->getMousePressPos().y() + m->pos().y(),
                         cx, cy);
      moveChild(m, cx, cy);
    }
  }
}

void ModularSynth::moveTextEdit(QPoint pos) {

  int l1;
  TextEdit *te;
  int cx, cy;
  
  for (l1 = 0; l1 < listTextEdit.count(); l1++) {
    if ((te=listTextEdit.at(l1)) == sender()) {
      viewportToContents(pos.x() - te->getMousePressPos().x() + te->pos().x(),
                         pos.y() - te->getMousePressPos().y() + te->pos().y(),
                         cx, cy);
      moveChild(te, cx, cy);
    }
  }
}

void ModularSynth::resizeTextEdit(QPoint pos) {

  int l1;
  TextEdit *te;
  int cx, cy;
  
  for (l1 = 0; l1 < listTextEdit.count(); l1++) {
    if ((te=listTextEdit.at(l1)) == sender()) {
      cx = pos.x();
      cy = pos.y();
      if ((cx > 200) && (cy > 170)) { 
        te->setFixedSize(cx + 3, cy + 3);
      }
    }
  }
}

void ModularSynth::portSelected() {

  if (firstPort) {
    firstPort = false;
    connectingPort[0] = (Port *)sender();
    connectingPort[0]->highlighted = true;
    connectingPort[0]->repaint(false);
  } else {
    firstPort = true;
    connectingPort[1] = (Port *)sender();
    connectingPort[0]->highlighted = false;  
    connectingPort[0]->repaint(false);
    connectingPort[1]->highlighted = false;
    connectingPort[1]->repaint(false);
    if ((((connectingPort[0]->dir == PORT_IN) && (connectingPort[1]->dir == PORT_OUT))
      ||((connectingPort[1]->dir == PORT_IN) && (connectingPort[0]->dir == PORT_OUT)))
      && (connectingPort[0]->parentModule != connectingPort[1]->parentModule)) {
        connectingPort[0]->connectTo(connectingPort[1]);
        connectingPort[1]->connectTo(connectingPort[0]);
      repaintContents(false);
    } else {
      printf("Connection refused.\n");
      connectingPort[0] = NULL;
      connectingPort[1] = NULL;
    }
  } 
}

void ModularSynth::updatePortConnections() {

  repaintContents(false);
}

void ModularSynth::deleteModule() {

  Module *m;
 
  connectingPort[0] = NULL;
  connectingPort[1] = NULL;
  firstPort = true;
  m = (Module *)sender();
  listModule.removeRef(m);
  deleteModule (m);
}

void ModularSynth::deleteTextEdit() {

  listTextEdit.removeRef((TextEdit *)sender());
  delete((TextEdit *)sender());
}

void ModularSynth::deleteTextEdit(TextEdit *te) {

  delete(te);
}

void ModularSynth::deleteModule(Module *m) {

  midiWidget->deleteModule(m);
  if (m->M_type == M_type_env) {
    synthdata->listM_env.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_vcenv) {
    synthdata->listM_vcenv.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_advenv) {
    synthdata->listM_advenv.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_dynamicwaves) {
    synthdata->listM_dynamicwaves.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_mcv) {
    synthdata->listM_mcv.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_advmcv) {
    synthdata->listM_advmcv.removeRef((QObject *)m);
  }
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
  if (m->M_type == M_type_wavout) {
    synthdata->wavoutModuleList.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_midiout) {
    synthdata->midioutModuleList.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_scope) {
    synthdata->scopeModuleList.removeRef((QObject *)m);
  }
  if (m->M_type == M_type_spectrum) {
    synthdata->spectrumModuleList.removeRef((QObject *)m);
  }

  delete(m);
}

void ModularSynth::clearConfig() {

  int l1;
  bool restartSynth;
  QString qs;

  qs.sprintf("AlsaModularSynth 1.7.1 - %d:%d", clientid, portid);
  mainWindow->setCaption(qs);
  restartSynth = synthdata->doSynthesis;
  synthdata->doSynthesis = false;
  sleep (1);
  guiWidget->clearGui();
  for (l1 = 0; l1 < listModule.count(); l1++) {
    deleteModule(listModule.at(l1));
  }
  listModule.clear();
  for (l1 = 0; l1 < listTextEdit.count(); l1++) {
    deleteTextEdit(listTextEdit.at(l1));
  }
  listTextEdit.clear();
  synthdata->moduleID = 0;
  synthdata->moduleCount = 0;
  if (restartSynth) synthdata->doSynthesis = true;    
}

void ModularSynth::load() {

  QString config_fn;

  if (presetPath.isEmpty()) {
    if (!(config_fn = QString(QFileDialog::getOpenFileName(QString::null, "AlsaModularSynth files (*.ams)")))) {
      return;
    }
  } else {
      if (!(config_fn = QString(QFileDialog::getOpenFileName(presetPath, "AlsaModularSynth files (*.ams)")))) {
      return;
    }
  }
  load(&config_fn);
}

void ModularSynth::load(QString *presetName) {

  int l1, l2;
  int M_type, moduleID, index, value, x, y, w, h, subID1, subID2;
  int index1, index2, moduleID1, moduleID2, midiSign;
  int index_read1, index_read2, moduleID_read1, moduleID_read2;
  int type, ch, param, isLogInt, midiIndex, sliderMin, sliderMax;
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
  if (!(f = fopen(config_fn, "r"))) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not open file.");  
  } else {
    clearConfig();
    qs2 = config_fn.mid(config_fn.findRev('/') + 1);
    qs.sprintf("AlsaModularSynth 1.7.1 - %d:%d - %s", clientid, portid, qs2.latin1());
    mainWindow->setCaption(qs);
    ladspaLoadErr = false;
    commentFlag = false;
    loadingPatch = true;
    while(fscanf(f, "%s", sc) != EOF) {
      qs = QString(sc);
      if (qs.contains("#PARA#", false)) {
        commentFlag = true;
      }
      if (qs.contains("#ARAP#", false)) {
        commentFlag = false;
      }
      if (qs.contains("Module", false) && !commentFlag) {
        fscanf(f, "%d", &M_type);
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &x);
        fscanf(f, "%d", &y);
        switch ((M_typeEnum)M_type) {
          case M_type_ladspa: 
            fscanf(f, "%d", &newLadspaPolyFlag);
            fscanf(f, "%s", sc);
            ladspaLibName = QString(sc);
            fgets(sc, 2048, f);
            sc[strlen(sc)-1] = '\0';
            pluginName = QString(sc+1);
            fprintf(stderr, "Loading LADSPA plugin \"%s\" from library \"%s\".\n", pluginName.latin1(), ladspaLibName.latin1());
            if (!synthdata->getLadspaIDs(ladspaLibName, pluginName, &subID1, &subID2)) {
              sprintf(sc, "Could not find LADSPA plugin \"%s\" from library \"%s\".\n", pluginName.latin1(), ladspaLibName.latin1());
              QMessageBox::information( this, "AlsaModularSynth", QString(sc));  
              ladspaLoadErr = true;
            }
            break;
          case M_type_scquantizer: 
            fscanf(f, "%s", sc);
            scalaName = QString(sc);
            break;
          case M_type_scmcv: 
            fscanf(f, "%s", sc);
            scalaName = QString(sc);
            break;
          default: 
            fscanf(f, "%d", &subID1);
            fscanf(f, "%d", &subID2);
            break;
        }

        switch((M_typeEnum)M_type) {
          case M_type_custom: 
            break;
          case M_type_vco: 
            newM_vco();
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
          case M_type_noise: 
            newM_noise();
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
          case M_type_mix: 
            newM_mix(subID1);
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
        moveChild(m, x, y);
        m->moduleID=moduleID;
        qs = m->configDialog->caption();
        qs2 = qs.left(qs.findRev(" "));
        qs.sprintf(" %d", moduleID);
        m->configDialog->setCaption(qs2+qs);
        midiWidget->addModule(m);
        if (synthdata->moduleID <= moduleID) {
          synthdata->moduleID = moduleID+1;
        }
      }
      if (qs.contains("Comment", false) && !commentFlag) {
        fscanf(f, "%d", &textEditID);
        fscanf(f, "%d", &textEditID); // TODO textEditID is not needed yet
        fscanf(f, "%d", &x);
        fscanf(f, "%d", &y);
        fscanf(f, "%d", &w);
        fscanf(f, "%d", &h);
        new_textEdit(x, y, w, h);
      }
    }
    rewind(f);
    while((fscanf(f, "%s", sc) != EOF) && !ladspaLoadErr) {
      qs = QString(sc);
      if (qs.contains("Port", false)) {
        fscanf(f, "%d", &index1); 
        fscanf(f, "%d", &index2);
        fscanf(f, "%d", &moduleID1);
        fscanf(f, "%d", &moduleID2); 
        moduleID_read1 = 0;
        moduleID_read2 = 0;
        index_read1 = 0;
        index_read2 = 0;
        for (l1 = 0; l1 < listModule.count(); l1++) {
          if (listModule.at(l1)->moduleID == moduleID1) {
            moduleID_read1 = l1;
          }
          if (listModule.at(l1)->moduleID == moduleID2) {
            moduleID_read2 = l1;
          }
        }
        for (l1 = 0; l1 < listModule.at(moduleID_read1)->portList.count(); l1++) {
          if ((listModule.at(moduleID_read1)->portList.at(l1)->index == index1) 
            && (listModule.at(moduleID_read1)->portList.at(l1)->dir == PORT_IN)) {
            index_read1 = l1;
          }
        } 
        for (l1 = 0; l1 < listModule.at(moduleID_read2)->portList.count(); l1++) {
          if ((listModule.at(moduleID_read2)->portList.at(l1)->index == index2)
            && (listModule.at(moduleID_read2)->portList.at(l1)->dir == PORT_OUT)) {
            index_read2 = l1;
          } 
        }   
        listModule.at(moduleID_read1)->portList.at(index_read1)->connectTo(listModule.at(moduleID_read2)->portList.at(index_read2));
        listModule.at(moduleID_read2)->portList.at(index_read2)->connectTo(listModule.at(moduleID_read1)->portList.at(index_read1));
      }
      if (qs.contains("FSlider", false)) {
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);
        fscanf(f, "%d", &value);
        fscanf(f, "%d", &isLogInt);
        fscanf(f, "%d", &sliderMin);
        fscanf(f, "%d", &sliderMax);
        fscanf(f, "%d", &midiSign);
        isLog = isLogInt == 1;
        for (l1 = 0; l1 < listModule.count(); l1++) {
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
      if (qs.contains("ISlider", false)) {
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);
        fscanf(f, "%d", &value);
        fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); l1++) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->intMidiSliderList.at(index)->slider->setValue((int)value);
            listModule.at(l1)->configDialog->intMidiSliderList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("LSlider", false)) {
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);
        fscanf(f, "%d", &value);
        fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); l1++) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->slider->setValue((int)value);
            listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("ComboBox", false)) {
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);
        fscanf(f, "%d", &value);
        fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); l1++) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->midiComboBoxList.at(index)->comboBox->setCurrentItem(value);
            listModule.at(l1)->configDialog->midiComboBoxList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("CheckBox", false)) {
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);
        fscanf(f, "%d", &value);
        fscanf(f, "%d", &midiSign); 
        for (l1 = 0; l1 < listModule.count(); l1++) {
          if (listModule.at(l1)->moduleID == moduleID) {
            listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->checkBox->setChecked(value==1);
            listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->midiSign = midiSign;
            break;
          }
        }
      }
      if (qs.contains("MIDI", false)) {
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);   
        fscanf(f, "%d", &type);   
        fscanf(f, "%d", &ch);
        fscanf(f, "%d", &param);
        MidiController *midiController = new MidiController(type, ch, param);
        if (!midiWidget->midiControllerList.contains(midiController)) {
          midiWidget->addMidiController(midiController);
        } else {
          midiIndex = midiWidget->midiControllerList.find(midiController);
          delete(midiController);
          midiController = midiWidget->midiControllerList.at(midiIndex);
        }
        if (qs.contains("FSMIDI", false)) {
          listModule.at(l1)->configDialog->midiSliderList.at(index)->connectToController(midiController);
        }
        if (qs.contains("ISMIDI", false)) {
          listModule.at(l1)->configDialog->intMidiSliderList.at(index)->connectToController(midiController);
        }
        if (qs.contains("LSMIDI", false)) {
          listModule.at(l1)->configDialog->floatIntMidiSliderList.at(index)->connectToController(midiController);
        }
        if (qs.contains("CMIDI", false)) {
          listModule.at(l1)->configDialog->midiComboBoxList.at(index)->connectToController(midiController);
        }
        if (qs.contains("TMIDI", false)) {
          listModule.at(l1)->configDialog->midiCheckBoxList.at(index)->connectToController(midiController);
        }
      }
      if (qs.contains("#PARA#", false)) {
        fscanf(f, "%d", &textEditID);
        fscanf(f, "%d", &textEditID);
        fscanf(f, "%d", &index);
        fscanf(f, "%s", sc);
        qs = QString(sc);
        if (!qs.contains("#ARAP#", false)) {
          para = QString(sc) + " ";
        } else {
          para = "\n";
        }
        while (!qs.contains("#ARAP#", false)) {
          fscanf(f, "%s", sc);
          qs = QString(sc);
          if (!qs.contains("#ARAP#", false)) {
            para.append(qs+" "); 
          }
        }
        listTextEdit.at(textEditID)->textEdit->insertParagraph(para, index);
      }
      if (qs.contains("Frame", false)) {
        qs.truncate(0);
        do {
          fscanf(f, "%s", sc);
          qs += QString(sc);
          if (qs.right(1) != "\"") {
            qs += " ";
          }
        } while (qs.right(1) != "\"");
        qs = qs.mid(1, qs.length()-2);
        guiWidget->addFrame(qs);
      }
      if (qs.contains("Parameter", false)) {
        qs.truncate(0);
        do {
          fscanf(f, "%s", sc);
          qs += QString(sc);
          if (qs.right(1) != "\"") {
            qs += " ";
          }
        } while (qs.right(1) != "\"");
        qs = qs.mid(1, qs.length()-2);
        fscanf(f, "%d", &moduleID);
        fscanf(f, "%d", &index);
        for (l1 = 0; l1 < listModule.count(); l1++) {
          if (listModule.at(l1)->moduleID == moduleID) {
            guiWidget->addParameter(listModule.at(l1)->configDialog->midiGUIcomponentList.at(index), qs);
            if (listModule.at(l1)->configDialog->midiGUIcomponentList.at(index)->componentType == GUIcomponentType_slider) {
              fscanf(f, "%d", &sliderMin);
              fscanf(f, "%d", &sliderMax);
              fscanf(f, "%d", &isLogInt);
              ((MidiSlider *)guiWidget->parameterList.last())->setNewMin(sliderMin);
              ((MidiSlider *)guiWidget->parameterList.last())->setNewMax(sliderMax);
              ((MidiSlider *)guiWidget->parameterList.last())->setLogMode(isLogInt == 1);
            }
            break;
          }
        }
      }  
      if (qs.contains("Program", false)) {
        fscanf(f, "%d", &index);
        fscanf(f, "%d", &value);
        if (index != currentProgram) {
          currentProgram = index;
          guiWidget->setPresetCount(currentProgram + 1);
        }
        guiWidget->presetList[currentProgram].append(value);
      }
    }
    if (guiWidget->presetCount) {
      guiWidget->setCurrentPreset(0);
    }
    fclose(f);
    loadingPatch = false;
  }
  synthdata->doSynthesis = true;
  midiWidget->followConfig = followConfig;
}

void ModularSynth::save() {

  Port *port[2];
  int l1, l2, l3, value;
  FILE *f;
  QString config_fn, qs;
  QValueList<int>::iterator it;
   
  if (!(config_fn = QString(QFileDialog::getSaveFileName(savePath, "AlsaModularSynth files (*.ams)")))) {
    return;
  }
  if (!(f = fopen(config_fn, "w"))) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not save file.");
  } else {
    for (l1 = 0; l1 < listModule.count(); l1++) {
      fprintf(f, "Module %d %d %d %d ", (int)listModule.at(l1)->M_type, listModule.at(l1)->moduleID, 
              listModule.at(l1)->x(),  listModule.at(l1)->y());

      switch(listModule.at(l1)->M_type)
      {
        case M_type_custom: 
          break;
        case M_type_vca: 
          fprintf(f, "%d 0\n", (int)((M_vca *)listModule.at(l1))->expMode); 
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
                  synthdata->ladspa_lib_name[((M_ladspa *)listModule.at(l1))->ladspaDesFuncIndex].latin1(), 
                  ((M_ladspa *)listModule.at(l1))->pluginName.latin1());
          break;
        case M_type_scquantizer: 
          qs = ((M_scquantizer *)listModule.at(l1))->sclname.latin1();
          if (qs.contains("/")) {
            qs = qs.mid(qs.findRev("/") + 1);             
          }
          fprintf(f, "%s\n", qs.latin1());
          break;
        case M_type_scmcv: 
          qs = ((M_scmcv *)listModule.at(l1))->sclname.latin1();
          if (qs.contains("/")) {
            qs = qs.mid(qs.findRev("/") + 1);             
          }
          fprintf(f, "%s\n", qs.latin1());
          break;
        default:
          fprintf(f, "0 0\n");  
          break; 
      }
      listModule.at(l1)->save(f);
    }
    for (l1 = 0; l1 < listTextEdit.count(); l1++) {
      fprintf(f, "Comment %d %d %d %d %d %d\n", listTextEdit.at(l1)->textEditID, l1, 
                  listTextEdit.at(l1)->x(), listTextEdit.at(l1)->y(), 
                  listTextEdit.at(l1)->width(), listTextEdit.at(l1)->height());
    }
    for (l1 = 0; l1 < listTextEdit.count(); l1++) {
      for (l2 = 0; l2 < listTextEdit.at(l1)->textEdit->paragraphs(); l2++) {
        fprintf(f, "#PARA# %d %d %d\n", listTextEdit.at(l1)->textEditID, l1, l2);
        fprintf(f, "%s\n", listTextEdit.at(l1)->textEdit->text(l2).latin1());
        fprintf(f, "#ARAP#\n");
      }
    }

    for (l1 = 0; l1 < guiWidget->frameBoxList.count(); l1++) {
      fprintf(f, "Frame \"%s\"\n", guiWidget->frameBoxList.at(l1)->name());
      for (l2 = 0; l2 < guiWidget->parameterList.count(); l2++) {
        if (guiWidget->parameterList.at(l2)->parent() == guiWidget->frameBoxList.at(l1)) {
          fprintf(f, "Parameter \"%s\" %d %d ", guiWidget->parameterList.at(l2)->name(), 
                                         ((Module *)guiWidget->parameterList.at(l2)->parentModule)->moduleID, 
                                         guiWidget->parameterList.at(l2)->midiGUIcomponentListIndex);
          if (guiWidget->parameterList.at(l2)->componentType == GUIcomponentType_slider) {
            fprintf(f, "%d %d %d\n", ((MidiSlider *)guiWidget->parameterList.at(l2))->slider->minValue(), 
                                   ((MidiSlider *)guiWidget->parameterList.at(l2))->slider->maxValue(), 
                                   ((MidiSlider *)guiWidget->parameterList.at(l2))->isLog);
          } else {
            fprintf(f, "\n");
          }
        }
      }
    }

    for (l1 = 0; l1 < guiWidget->presetCount; l1++) {
      for (it = guiWidget->presetList[l1].begin(); it != guiWidget->presetList[l1].end(); it++) {
        value = *it;
        fprintf(f, "Program %d %d\n", l1, value);
      } 
    }

    fclose(f);
  }
}

void ModularSynth::allVoicesOff() {

  int l1, l2;

  for (l2 = 0; l2 < synthdata->poly; l2++) {
    if (synthdata->notePressed[l2]) {
      synthdata->notePressed[l2] = false; 
      synthdata->noteActive[l2] = false;
      for (l1 = 0; l1 < listModule.count(); l1++) {      
        listModule.at(l1)->noteOffEvent(l2);
      } 
    }
  } 
}

void ModularSynth::cleanUpSynth()
{
  fprintf(stderr, "Closing synth...\n");
  delete this;
}
