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
#include "port.h"
#include "synthdata.h"
#include "module.h"

Port::Port(const QString &p_portName, dirType p_dir, int p_index, QWidget* parent, SynthData *p_synthdata) 
           : QWidget(parent) {

  parentModule = parent;
  portName = p_portName;
  synthdata = p_synthdata;
  dir = p_dir;
  highlighted = false;
  index = p_index;
  setPalette(QPalette(QColor(77, 70, 64), QColor(77, 70, 64)));
  setFixedSize(PORT_DEFAULT_WIDTH, PORT_DEFAULT_HEIGHT);    
  contextMenu = new QPopupMenu(this);
  contextMenu->insertItem("disconnect", this, SLOT(disconnectClicked()));
}

Port::~Port() {
}

int Port::connectTo(Port *port) {

  synthdata->port_sem++;
  ((Module *)parentModule)->configDialog->removeButton->setEnabled(false);
  if (dir == PORT_OUT) {
    if (port->outTypeAcceptList.contains(outType)) {
      connectedPortList.append(port);
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

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    if (dir == PORT_IN) {
      p.drawLine(l1, 0, l1, height());
    } else {
      p.drawLine(width() - l1 - 1, 0, width() - l1 - 1, height());
    }
  }
  p.setFont(QFont("Helvetica", 8));
//  p.setPen(QColor(255, 220, 60));
  p.setPen(QColor(255, 255, 255));
  if (dir == PORT_IN) { 
    if (highlighted) {
      p.fillRect(0, height()/2-2, 3, 5, QBrush(QColor(240, 0, 0)));
    } else {
      p.fillRect(0, height()/2-2, 3, 5, QBrush(QColor(10, 10, 10)));
    }
    p.drawText(5, 12, portName);
  } else {
    if (highlighted) {
      p.fillRect(width() - 3, height()/2-2, 3, 5, QBrush(QColor(240, 0, 0)));
    } else {
      p.fillRect(width() - 3, height()/2-2, 3, 5, QBrush(QColor(10, 10, 10)));
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

  int l1;
  bool moduleConnected;

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
  for (l1 = 0; l1 < ((Module *)parentModule)->portList.count(); l1++) {
    if (((Module *)parentModule)->portList.at(l1)->connectedPortList.count()) {
      moduleConnected = true;
    }
  }
  if (!moduleConnected) {
    ((Module *)parentModule)->configDialog->removeButton->setEnabled(true);
  }
}
