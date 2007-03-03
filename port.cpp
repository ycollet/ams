#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpopupmenu.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qcolordialog.h> 
#include "port.h"
#include "synthdata.h"
#include "module.h"
#include "main.h"


Port::Port(const QString &p_portName, dirType p_dir, int p_index, QWidget* parent, SynthData *p_synthdata, int p_portWidth, int p_color) 
           : QWidget(parent) {

  parentModule = parent;
  portName = p_portName;
  synthdata = p_synthdata;
  portWidth = p_portWidth;
  dir = p_dir;
  fontColor = p_color;
  jackColor = synthdata->colorJack;
  cableColor = synthdata->colorCable;
  highlighted = false;
  index = p_index;
  colorFont1 = synthdata->colorPortFont1;
  colorFont2 = synthdata->colorPortFont2;
  colorBackground = synthdata->colorModuleBackground;
  setPalette(QPalette(colorBackground, colorBackground));
  setFixedSize(portWidth, PORT_DEFAULT_HEIGHT);    
  contextMenu = new QPopupMenu(this);
  contextMenu->insertItem("Disconnect", this, SLOT(disconnectClicked()));
  contextMenu->insertSeparator();
  contextMenu->insertItem("Default Cable", this, SLOT(cableDefaultClicked()));
  contextMenu->insertItem("Gray Cable", this, SLOT(cableGrayClicked()));
  contextMenu->insertItem("Red Cable", this, SLOT(cableRedClicked()));
  contextMenu->insertItem("Green Cable", this, SLOT(cableGreenClicked()));
  contextMenu->insertItem("Blue Cable", this, SLOT(cableBlueClicked()));
  contextMenu->insertItem("Yellow Cable", this, SLOT(cableYellowClicked()));
  contextMenu->insertSeparator();
  contextMenu->insertItem("Set Jack Color", this, SLOT(jackColorClicked()));
  contextMenu->insertItem("Set Cable Color", this, SLOT(cableColorClicked()));
}

Port::~Port() {
}

int Port::connectTo(Port *port) {

  synthdata->port_sem++;
  ((Module *)parentModule)->configDialog->removeButton->setEnabled(false);
  if (dir == PORT_OUT) {
    if (port->outTypeAcceptList.contains(outType)) {
      connectedPortList.append(port);
      port->jackColor = synthdata->colorJack;
      port->cableColor = synthdata->colorCable;
    }
  } else {
    if (outTypeAcceptList.contains(port->outType)) {
      if (connectedPortList.count()) {
        connectedPortList.at(0)->connectedPortList.removeRef(this);
        connectedPortList.at(0)->checkConnectionStatus();
        connectedPortList.clear();
      }
      connectedPortList.append(port);
    }
  }
  synthdata->port_sem--;
  return(0);
}

void Port::paintEvent(QPaintEvent *ev) {

  QPainter p(this);  
  QRect textRect;
  int l1;

  p.setPen(synthdata->colorModuleBorder);
  for (l1 = 0; l1 < 4; ++l1) {
    p.setPen(synthdata->colorModuleBorder.light(100 + 15 * l1));
    if (dir == PORT_IN) {
      p.drawLine(l1, 0, l1, height());
    } else {
      p.drawLine(width() - l1 - 1, 0, width() - l1 - 1, height());
    }
  }
  p.setFont(QFont("Helvetica", 8));
//  p.setPen(QColor(255, 220, 60));
  if (fontColor) {
    p.setPen(colorFont2);
  } else {
    p.setPen(colorFont1);
  }
  if (dir == PORT_IN) { 
    if (highlighted) {
      p.fillRect(0, height()*0.5-2, 3, 5, QBrush(QColor(240, 0, 0)));
    } else {
      p.fillRect(0, height()*0.5-2, 3, 5, QBrush(QColor(10, 10, 10)));
    }
    p.drawText(5, 12, portName);
  } else {
    if (highlighted) {
      p.fillRect(width() - 3, height()*0.5-2, 3, 5, QBrush(QColor(240, 0, 0)));
    } else {
      p.fillRect(width() - 3, height()*0.5-2, 3, 5, QBrush(QColor(10, 10, 10)));
    }
    textRect = p.boundingRect(0, 0, width(), height(), QPainter::AlignLeft, portName);
    p.drawText(width() - textRect.width() - 5, 12, portName);
  }
}  
   
void Port::mousePressEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    emit portClicked();
    break;
  case Qt::RightButton:
    break;
  case Qt::MidButton:
    break;
  default:
    break;
  }
}  
   
void Port::mouseReleaseEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:
    break;
  case Qt::RightButton:
    if (dir == PORT_IN) {
      showContextMenu(ev->pos());
    }
    break;
  case Qt::MidButton:
    break;
  default:
    break;
  }
}  
   
void Port::mouseMoveEvent(QMouseEvent *ev) {

}      

void Port::showContextMenu(QPoint pos) {
  
  contextMenu->popup(mapToGlobal(pos));
}

void Port::disconnectClicked() {

  synthdata->port_sem++;
  if (connectedPortList.count()) {
    connectedPortList.at(0)->connectedPortList.removeRef(this);
    connectedPortList.at(0)->checkConnectionStatus();
    connectedPortList.clear();
  }
  checkConnectionStatus();
  synthdata->port_sem--;
  emit portDisconnected();
}

void Port::checkConnectionStatus() {

  int l1;
  bool moduleConnected;

  moduleConnected = false;
  for (l1 = 0; l1 < ((Module *)parentModule)->portList.count(); ++l1) {
    if (((Module *)parentModule)->portList.at(l1)->connectedPortList.count()) {
      moduleConnected = true;
    }
  }
  if (!moduleConnected) {
    ((Module *)parentModule)->configDialog->removeButton->setEnabled(true);
  }
}


float **Port::getinputdata (void)
{
    Module *M;

    if (connectedPortList.count())
    {
        M = (Module *)(connectedPortList.at (0)->parentModule);
        if (! M->cycleReady && ! M->cycleProcessing) M->generateCycle ();
        return M->data [connectedPortList.at (0)->index];
    }
    else return synthdata->zeroModuleData;
}

void Port::cableGrayClicked() {

  jackColor = QColor(250, 200, 50);
  cableColor = QColor(180, 180, 180);
}

void Port::cableRedClicked() {

  jackColor = QColor(200, 150, 150);
  cableColor = QColor(190, 0, 60);
}

void Port::cableGreenClicked() {

  jackColor = QColor(130, 190, 130);
  cableColor = QColor(60, 170, 60);
}

void Port::cableBlueClicked() {

  jackColor = QColor(150, 150, 190);
  cableColor = QColor(90, 90, 210);
}

void Port::cableYellowClicked() {

  jackColor = QColor(220, 170, 100);
  cableColor = QColor(220, 195, 10);
}

void Port::cableDefaultClicked() {

  jackColor = synthdata->colorJack;
  cableColor = synthdata->colorCable;
}

void Port::jackColorClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor (jackColor);
  if (tmp.isValid()) {
    jackColor = tmp;
  }
}

void Port::cableColorClicked() {

  QColor tmp;
  
  tmp = QColorDialog::getColor (cableColor);
  if (tmp.isValid()) {
    cableColor = tmp;
  }
}
