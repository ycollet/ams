#include <stdio.h>
#include <stdlib.h>
#include <qpoint.h>
#include <qbrush.h>
#include <qpen.h>
#include "canvasfunction.h"


CanvasFunction::CanvasFunction(QCanvas *canvas, int p_rtti_id, int p_pointCount, QObject *parent, const char *name)
                               : QObject(parent, name) {

  int l1;

  fprintf(stderr, "CanvasFunction::CanvasFunction M1\n");
  rtti_id = p_rtti_id;
  pointCount = p_pointCount;
  points = new  QPointArray(pointCount);
  fprintf(stderr, "CanvasFunction::CanvasFunction points %p\n", points);
  for (l1 = 0; l1 < pointCount; l1++) {
    QCanvasEllipse *canvasPoint = new QCanvasEllipse(canvas);
    canvasPoint->setBrush(QBrush(QColor(100, 255, 100)));
    canvasPoint->setPen(QPen(QColor(0, 150, 0), 2));
    canvasPoint->setSize(5, 5);
    canvasPoint->setVisible(TRUE);
    canvasPoints.append(canvasPoint);
  }
  for (l1 = 0; l1 < pointCount - 1; l1++) {
    QCanvasLine *canvasLine = new QCanvasLine(canvas);
    canvasLine->setPoints(0, 0, 0, 0);
    canvasLine->setPen(QPen(QColor(0, 150, 0), 2));
    canvasLine->setVisible(TRUE);
    canvasLines.append(canvasLine);
  }
  for (l1 = 0; l1 < pointCount; l1++) {
    points->setPoint(l1, 0, 0);
  }
  fprintf(stderr, "CanvasFunction::CanvasFunction M2\n");
}

CanvasFunction::~CanvasFunction() {

}

void CanvasFunction::setPoint(int index, int x, int y) {

  QPoint qp;

  fprintf(stderr, "CanvasFunction::setPoint %d %d %d\n", index, x, y);
  fprintf(stderr, "CanvasFunction::setPoint points %p\n", points);
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
  fprintf(stderr, "CanvasFunction::setPoint finished\n");
}

int CanvasFunction::rtti() {
  
  return(rtti_id);
}  
