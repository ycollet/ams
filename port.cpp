#include <qstring.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qcolordialog.h> 
#include <QPaintEvent>
#include <QMouseEvent>
#include "modularsynth.h"
#include "port.h"
#include "synthdata.h"
#include "module.h"
#include "main.h"
#include "port_popup.h"

PopupMenu *Port::contextMenu;

Port::Port(const QString &p_portName, dirType p_dir, int p_index, Module *module, int p_color) 
           : QWidget(module)
	   , portNameWidth(0)
	   , module(module)
	   , colorFont(p_color ? synthdata->colorPortFont2 : synthdata->colorPortFont1)
{
  module->portList.append(this);

  portName = p_portName;
  
  dir = p_dir;
  if (dir == PORT_IN)
    outTypeAcceptList.append(outType_audio);
  else
    outType = outType_audio;

  jackColor = synthdata->colorJack;
  cableColor = synthdata->colorCable;
  highlighted = false;
  index = p_index;
  int width = 6 + QFontMetrics(font()).boundingRect(p_portName).width();
  if (width < 30)
    width = 30;
  setFixedWidth(width);
  setFixedHeight(PORT_DEFAULT_HEIGHT);
}

Port::~Port() {
}

int Port::connectTo(Port *port)
{
  synthdata->port_sem.acquire();

  if (dir == PORT_OUT) {
    if (port->outTypeAcceptList.contains(outType)) {
      connectedPortList.append(port);
      module->incConnections();
      port->jackColor = synthdata->colorJack;
      port->cableColor = synthdata->colorCable;
    }
  } else {
    if (outTypeAcceptList.contains(port->outType)) {
      if (connectedPortList.count()) {
        connectedPortList.at(0)->connectedPortList.removeAll(this);
        connectedPortList.at(0)->module->decConnections();
        connectedPortList.clear();
      } else
	module->incConnections();

      connectedPortList.append(port);
    }
  }
  synthdata->port_sem.release();
  return(0);
}

void Port::paintEvent(QPaintEvent *) {

  QPainter p(this);  
  QRect textRect;
  //int l1;

//   p.setPen(synthdata->colorModuleBorder);
//   for (l1 = 0; l1 < 4; ++l1) {
//     p.setPen(synthdata->colorModuleBorder.light(100 + 15 * l1));
//     if (dir == PORT_IN) {
//       p.drawLine(l1, 0, l1, height());
//     } else {
//       p.drawLine(width() - l1 - 1, 0, width() - l1 - 1, height());
//     }
//   }
  p.setFont(synthdata->smallFont);
//  p.setPen(QColor(255, 220, 60));
  p.setPen(colorFont);

  if (dir == PORT_IN) { 
    if (highlighted) {
      p.fillRect(0, height()/2 - 2, 3, 5, QBrush(QColor(240, 0, 0)));
    } else {
      p.fillRect(0, height()/2 - 2, 3, 5, QBrush(QColor(10, 10, 10)));
    }
    p.drawText(5, 11, portName);
  } else {
    if (highlighted) {
      p.fillRect(width() - 3, height()/2 - 2, 3, 5, QBrush(QColor(240, 0, 0)));
    } else {
      p.fillRect(width() - 3, height()/2 - 2, 3, 5, QBrush(QColor(10, 10, 10)));
    }
    if (!portNameWidth) {
      textRect = p.boundingRect(0, 0, width(), height(), Qt::AlignLeft, portName);
      portNameWidth = textRect.width();
    }
    p.drawText(width() - portNameWidth - 5, 11, portName);
  }
}  
   
void Port::mousePressEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    emit portClicked();
    break;
  case Qt::RightButton:
    if (dir == PORT_IN)
      showContextMenu(ev->pos());

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
    break;
  case Qt::MidButton:
    break;
  default:
    break;
  }
}  
   
void Port::mouseMoveEvent(QMouseEvent *) {

}      

void Port::showContextMenu(QPoint pos)
{
  if (!contextMenu)
    contextMenu = new PopupMenu();

  contextMenu->popup(this, mapToGlobal(pos));
}

void Port::disconnectClicked() {

  synthdata->port_sem.acquire();
  if (connectedPortList.count()) {
    connectedPortList.at(0)->connectedPortList.removeAll(this);
    connectedPortList.at(0)->module->decConnections();
    connectedPortList.clear();
    module->decConnections();
  }
  synthdata->port_sem.release();
  emit portDisconnected();
}

float **Port::getinputdata (void)
{
  Module *M;

  if (connectedPortList.count()) {
    M = connectedPortList.at(0)->module;
    return M->getData(connectedPortList.at(0)->index);
  } else
    return synthdata->zeroModuleData;
}

void Port::cableGrayClicked() {

  jackColor = QColor(250, 200, 50);
  cableColor = QColor(180, 180, 180);
  update();
}

void Port::cableRedClicked() {

  jackColor = QColor(200, 150, 150);
  cableColor = QColor(190, 0, 60);
  update();
}

void Port::cableGreenClicked() {

  jackColor = QColor(130, 190, 130);
  cableColor = QColor(60, 170, 60);
  update();
}

void Port::cableBlueClicked() {

  jackColor = QColor(150, 150, 190);
  cableColor = QColor(90, 90, 210);
  update();
}

void Port::cableYellowClicked() {

  jackColor = QColor(220, 170, 100);
  cableColor = QColor(220, 195, 10);
  update();
}

void Port::cableDefaultClicked() {

  jackColor = synthdata->colorJack;
  cableColor = synthdata->colorCable;
  update();
}

void Port::jackColorClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor (jackColor);
  if (tmp.isValid()) {
    jackColor = tmp;
  }
  update();
}

void Port::cableColorClicked() {

  QColor tmp;
  
  tmp = QColorDialog::getColor (cableColor);
  if (tmp.isValid()) {
    cableColor = tmp;
  }
  update();
}
