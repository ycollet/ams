#include <stdio.h>
#include <stdlib.h>
#include <qpoint.h>
#include <qbrush.h>
#include <qpen.h>
#include "canvasfunction.h"


CanvasFunction::CanvasFunction(QCanvas *canvas, int p_rtti_id, int p_pointCount, QColor p_color, QObject *parent, const char *name)
                               : QObject(parent, name) {

  int l1;

  rtti_id = p_rtti_id;
  pointCount = p_pointCount;
  color = p_color;
  points = new  QPointArray(pointCount);
  for (l1 = 0; l1 < pointCount; l1++) {
    QCanvasEllipse *canvasPoint = new QCanvasEllipse(canvas);
    canvasPoint->setBrush(QBrush(color));
    canvasPoint->setPen(QPen(color, 2));
    canvasPoint->setSize(7, 7);
    canvasPoint->setVisible(TRUE);
    canvasPoints.append(canvasPoint);
  }
  for (l1 = 0; l1 < pointCount - 1; l1++) {
    QCanvasLine *canvasLine = new QCanvasLine(canvas);
    canvasLine->setPoints(0, 0, 0, 0);
    canvasLine->setPen(QPen(color, 2));
    canvasLine->setVisible(TRUE);
    canvasLines.append(canvasLine);
  }
  for (l1 = 0; l1 < pointCount; l1++) {
    points->setPoint(l1, 0, 0);
  }
}

CanvasFunction::~CanvasFunction() {

}

void CanvasFunction::setColor(QColor p_color) {

  color = p_color;  
}

void CanvasFunction::setPoint(int index, int x, int y) {

  QPoint qp;

  points->setPoint(index, x, y);
  canvasPoints.at(index)->move(x, y);
  if (index > 0) {
    qp = points->point(index - 1);
    canvasLines.at(index - 1)->setPoints(qp.x(), qp.y(), x, y);
  }
  if (index < pointCount - 1) {
    qp = points->point(index);
    canvasLines.at(index)->setPoints(x, y, qp.x(), qp.y());
  }
}

int CanvasFunction::rtti() {
  
  return(rtti_id);
}  
