#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qpoint.h>
#include <qwmatrix.h>
#include "function.h"
#include "canvasview.h"
#include "canvasfunction.h"

Function::Function(int p_functionCount, QPointArray *p_points[], int p_pointCount, SynthData *p_synthdata, QWidget *parent, const char *name) 
             : QWidget (parent, name)
{
  int l1;
  QString qs;

  synthdata = p_synthdata;
  functionCount = p_functionCount;
  mousePressed = false, 
  activeFunction = -1;
  activePoint = -1;
  for (l1 = 0; l1 < functionCount; l1++) {
    points[l1] = p_points[l1];
    setPointCount(MAX_POINTS);
    screenPoints[l1] = new QPointArray(MAX_POINTS);
  }
  setPalette(QPalette(QColor(20, 20, 80), QColor(20, 20, 80)));
  setMinimumHeight(125);
  colorTable[0].setRgb(255, 255, 255);
  colorTable[1].setRgb(255, 0, 0);
  colorTable[2].setRgb(0, 255, 0);
  colorTable[3].setRgb(50, 150, 255); 
  colorTable[4].setRgb(255, 255, 0);
  colorTable[5].setRgb(0, 255, 255);
  colorTable[6].setRgb(255, 100, 255);
  colorTable[7].setRgb(255, 200, 50);
  canvas = new QCanvas(this);
  canvas->resize(width(), height());
  canvas->setBackgroundColor(QColor(20, 20, 80));
  canvasView = new CanvasView(canvas, this);
  connect(canvasView, SIGNAL(mousePressed(QMouseEvent *)), this, SLOT(mousePressEvent(QMouseEvent *)));
  connect(canvasView, SIGNAL(mouseReleased(QMouseEvent *)), this, SLOT(mouseReleaseEvent(QMouseEvent *)));
  connect(canvasView, SIGNAL(mouseMoved(QMouseEvent *)), this, SLOT(mouseMoveEvent(QMouseEvent *)));
  for (l1 = 0; l1 < functionCount; l1++) {
    CanvasFunction *canvasFunction = new CanvasFunction(canvas, 1000 + l1, MAX_POINTS, colorTable[l1], this);
    canvasFunctionList.append(canvasFunction);
    updateFunction(l1);
    qs.sprintf("Out %d", l1);
    QCanvasText *canvasText = new QCanvasText(qs, canvas);
    canvasText->move(8 + 50 * l1, 4);
    canvasText->setColor(colorTable[l1]);
    canvasText->setFont(QFont("Helvetica", 10));
    canvasText->setVisible(TRUE);
    canvasTextList.append(canvasText);
  }
  for (l1 = 0; l1 <= FUNCTION_HEIGHT / 500; l1++) {
    QCanvasLine *canvasLineX = new QCanvasLine(canvas);
    canvasLineX->setPoints(0, 0, 0, 0);
    canvasLineX->setZ(-10);
    if (l1 & 1) {
      canvasLineX->setPen(QPen(QColor(70, 110, 70), 1));
    } else {
      canvasLineX->setPen(QPen(QColor(90, 180, 90), 1));
    }
    canvasLineX->setVisible(TRUE);
    gridX.append(canvasLineX);
  }
  for (l1 = 0; l1 <= FUNCTION_WIDTH / 500; l1++) {
    QCanvasLine *canvasLineY = new QCanvasLine(canvas);
    canvasLineY->setPoints(0, 0, 0, 0);            
    canvasLineY->setZ(-10);
    if (l1 & 1) {
      canvasLineY->setPen(QPen(QColor(70, 110, 70), 1));
    } else {
      canvasLineY->setPen(QPen(QColor(90, 180, 90), 1));
    }
    canvasLineY->setVisible(TRUE);
    gridY.append(canvasLineY);
  }
}

Function::~Function()
{
}

void Function::paintEvent(QPaintEvent *) {

//  fprintf(stderr,"Function::paintEvent\n");
  canvas->update();
}

void Function::updateFunction(int index) {

  int l1;
  QPoint qp;
  QWMatrix matrix;
  double scale[2];

  scale[0] = (double)(width()-FUNCTION_BORDER_1-FUNCTION_BORDER_2) / (double)FUNCTION_WIDTH;
  scale[1] = -(double)(height()-FUNCTION_BORDER_1-FUNCTION_BORDER_2) / (double)FUNCTION_HEIGHT;
//  fprintf(stderr, "updateFunction scale[0] = %f, scale[1] = %f\n", scale[0], scale[1]);
  matrix.scale(scale[0], scale[1]);
  matrix.translate((double)FUNCTION_BORDER_1/scale[0], (double)FUNCTION_BORDER_1/scale[1]-(double)FUNCTION_HEIGHT);
  *screenPoints[index] = matrix.map(*points[index]);
  f[0][index][0] = -1e30;
  f[0][index][pointCount+1] = 1e30;
  f[1][index][0] = 0;
  f[1][index][pointCount+1] = 0;
  for (l1 = 0; l1 < pointCount; l1++) {
    qp = screenPoints[index]->point(l1);
    f[0][index][l1 + 1] = (double)(points[index]->point(l1).x() - FUNCTION_CENTER_X) / (double)FUNCTION_SCALE;
    f[1][index][l1 + 1] = (double)(points[index]->point(l1).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE;
    canvasFunctionList.at(index)->setPoint(l1, qp.x(), qp.y());
  }
  repaint(false);
}

void Function::setPointCount(int count) {
  
  pointCount = count;
}

QSize Function::sizeHint() const {

  return QSize(FUNCTION_MINIMUM_WIDTH, FUNCTION_MINIMUM_HEIGHT); 
}

QSizePolicy Function::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void Function::resizeEvent (QResizeEvent* )
{
  int l1, l2;
  double scale[2];
  QWMatrix matrix;
  QPoint qp_in[2], qp_out[2];

  canvas->resize(width(), height());
  canvasView->resize(width(), height());
  if (canvasFunctionList.count() == functionCount) {
    for (l1 = 0; l1 < functionCount; l1++) {
      updateFunction(l1);
    }
  }
  scale[0] = (double)(width()-FUNCTION_BORDER_1-FUNCTION_BORDER_2) / (double)FUNCTION_WIDTH;  
  scale[1] = -(double)(height()-FUNCTION_BORDER_1-FUNCTION_BORDER_2) / (double)FUNCTION_HEIGHT;  
  matrix.scale(scale[0], scale[1]);
  matrix.translate((double)FUNCTION_BORDER_1/scale[0], (double)FUNCTION_BORDER_1/scale[1]-(double)FUNCTION_HEIGHT);
  qp_in[0].setX(0);
  qp_in[1].setX(FUNCTION_WIDTH);
  qp_in[1].setY(0);
  for (l1 = 0; l1 <gridX.count(); l1++) {
    qp_in[0].setY(l1 * 500);
    for (l2 = 0; l2 < 2; l2++) {
      qp_out[l2] = matrix.map(qp_in[l2]);
    }
    gridX.at(l1)->setPoints(qp_out[0].x(), qp_out[0].y(), qp_out[1].x(), qp_out[0].y());
  }
  qp_in[0].setY(0);
  qp_in[1].setY(FUNCTION_HEIGHT);
  qp_in[1].setX(0);
  for (l1 = 0; l1 <gridY.count(); l1++) {
    qp_in[0].setX(l1 * 500);
    for (l2 = 0; l2 < 2; l2++) {
      qp_out[l2] = matrix.map(qp_in[l2]);
    }
    gridY.at(l1)->setPoints(qp_out[0].x(), qp_out[0].y(), qp_out[0].x(), qp_out[1].y());
  }
  repaint(true);
}

void Function::mousePressEvent(QMouseEvent *ev) {

  int l1, l2;
  QCanvasItemList hitList;
  
  mousePressed = true;
  hitList = canvas->collisions(ev->pos());
  if (hitList.count()) {
    for (l1 = 0; l1 < functionCount; l1++) {
      for (l2 = 0; l2 < pointCount; l2++) {
        for(QCanvasItemList::Iterator it=hitList.begin(); it!=hitList.end(); it++) {
          if (canvasFunctionList.at(l1)->canvasPoints.at(l2) == *it) {
//            fprintf(stderr, "Hit %d %d\n", l1, l2);
            activeFunction = l1;
            activePoint = l2;
            break;
          }
        }
      }  
    }
  } else {
    activePoint = -1;
  }
}

void Function::mouseReleaseEvent(QMouseEvent *ev) {
  
  mousePressed = false;
}

void Function::mouseMoveEvent(QMouseEvent *ev) {

  double scale[2];
  QWMatrix matrix, invMatrix;
  QPoint qp;

  if (mousePressed && (activeFunction >=0) && (activePoint >= 0)) {
    scale[0] = (double)(width()-FUNCTION_BORDER_1-FUNCTION_BORDER_2) / (double)FUNCTION_WIDTH;   
    scale[1] = -(double)(height()-FUNCTION_BORDER_1-FUNCTION_BORDER_2) / (double)FUNCTION_HEIGHT;
//    fprintf(stderr, "mouseMoveEvent scale[0] = %f, scale[1] = %f\n", scale[0], scale[1]);
    matrix.scale(scale[0], scale[1]);    
    matrix.translate((double)FUNCTION_BORDER_1/scale[0], (double)FUNCTION_BORDER_1/scale[1]-(double)FUNCTION_HEIGHT);
    if (matrix.isInvertible()) {
      invMatrix = matrix.invert();
      qp = invMatrix.map(ev->pos());
      if ((activePoint > 0) && (qp.x() < points[activeFunction]->point(activePoint - 1).x())) {
        qp.setX(points[activeFunction]->point(activePoint-1).x() + 1);     // a minimum dx of 1 corresponds to 0.002 V
      } else if ((activePoint < pointCount - 1) && (qp.x() > points[activeFunction]->point(activePoint + 1).x())) {
        qp.setX(points[activeFunction]->point(activePoint+1).x() - 1); 
      }
      if (qp.x() < 0) qp.setX(0);
      if (qp.x() > FUNCTION_WIDTH) qp.setX(FUNCTION_WIDTH);
      if (qp.y() < 0) qp.setY(0);
      if (qp.y() > FUNCTION_HEIGHT) qp.setY(FUNCTION_HEIGHT);
      points[activeFunction]->setPoint(activePoint, qp);
//      fprintf(stderr, "mouseMoveEvent points[%d]->point(%d) = %d %d\n", activeFunction, activePoint, 
//              points[activeFunction]->point(activePoint).x(), points[activeFunction]->point(activePoint).y());
      updateFunction(activeFunction);
//      fprintf(stderr, "mouseMoveEvent f[0][%d][%d+1] = %f f[1][%d][%d+1] = %f\n", activeFunction, activePoint, f[0][activeFunction][activePoint+1],
//              activeFunction, activePoint, f[1][activeFunction][activePoint+1]);
    } else {
      fprintf(stderr, "Matrix not invertible !\n");
    }
  }
}
