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
#include "canvas.h"
#include "canvasfunction.h"

Function::Function(int p_functionCount, int *p_mode, int *p_editIndex, QPointArray *p_points[], int p_pointCount, SynthData *p_synthdata, QWidget *parent, const char *name) 
             : QCanvasView (parent, name)
{
  int l1;
  QString qs;

  Canvas *functionCanvas = new Canvas(this);
  setCanvas(functionCanvas);
  ((Canvas *)canvas())->setGrid(FUNCTION_BORDER_L, FUNCTION_BORDER_R, FUNCTION_BORDER_B, FUNCTION_BORDER_T, FUNCTION_SCALE, 
                                FUNCTION_WIDTH, FUNCTION_HEIGHT, FUNCTION_GRID, FUNCTION_GRID);
  synthdata = p_synthdata;
  functionCount = p_functionCount;
  mode = p_mode;
  editIndex = p_editIndex;
  mousePressed = false, 
  activeFunction = -1;
  activePoint = -1;
  zoom = 1.0;
  updateScale();
  for (l1 = 0; l1 < MAX_FUNCTIONS; l1++) {
    points[l1] = p_points[l1];
    setPointCount(MAX_POINTS);
    screenPoints[l1] = new QPointArray(MAX_POINTS);
  }
  setMinimumWidth(FUNCTION_MINIMUM_WIDTH);
  setMinimumHeight(FUNCTION_MINIMUM_HEIGHT);
  setPalette(QPalette(QColor(FUNCTION_COLOR_FG), QColor(FUNCTION_COLOR_BG)));
  colorTable[0] = QColor(FUNCTION_COLOR_1);
  colorTable[1] = QColor(FUNCTION_COLOR_2);
  colorTable[2] = QColor(FUNCTION_COLOR_3);
  colorTable[3] = QColor(FUNCTION_COLOR_4);
//  colorTable[4] = QColor(FUNCTION_COLOR_5);
//  colorTable[5] = QColor(FUNCTION_COLOR_6);
//  colorTable[6] = QColor(FUNCTION_COLOR_7);
//  colorTable[7] = QColor(FUNCTION_COLOR_8);
  canvas()->setBackgroundColor(QColor(FUNCTION_COLOR_BG));
  canvas()->resize(width(), height());
  for (l1 = 0; l1 < functionCount; l1++) {
    CanvasFunction *canvasFunction = new CanvasFunction(canvas(), 1000 + l1, MAX_POINTS, colorTable[l1], this);
    canvasFunctionList.append(canvasFunction);
    updateFunction(l1);
    qs.sprintf("Out %d", l1);
    QCanvasText *canvasText = new QCanvasText(qs, canvas());
    canvasText->move(8 + 50 * l1, 4);
    canvasText->setColor(colorTable[l1]);
    canvasText->setFont(QFont("Helvetica", 10));
    canvasText->setVisible(TRUE);
    canvasTextList.append(canvasText);
  }
}

Function::~Function()
{
}

void Function::updateFunction(int index) {

  int l1;
  QPoint qp;

  *screenPoints[index] = matrix.map(*points[index]);
  f[0][index][0] = -1e30;
  f[0][index][pointCount+1] = 1e30;
  f[1][index][0] = 0;
  f[1][index][pointCount+1] = 0;
  for (l1 = 0; l1 < pointCount; l1++) {
    qp = screenPoints[index]->point(l1);
    f[0][index][l1 + 1] = (double)(points[index]->point(l1).x() - FUNCTION_CENTER_X) / (double)FUNCTION_SCALE;
    f[1][index][l1 + 1] = (double)(FUNCTION_HEIGHT - points[index]->point(l1).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE;
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
  updateScale();
  canvas()->resize(zoom * width(), zoom * height());
//  redrawGrid();
  redrawFunction();
  updateContents();
}

void Function::redrawFunction() {

  int l1;

  if (canvasFunctionList.count() == functionCount) {
    for (l1 = 0; l1 < functionCount; l1++) {
      updateFunction(l1);
    }
  }
}

void Function::redrawGrid() {

  int l1, l2, x0, y0, ix0, iy0;
  QPoint qp_in[2], qp_out[2];
  QWMatrix invMatrix, zoomMatrix;

  if (matrix.isInvertible()) {
    invMatrix = matrix.invert();
  } else {
    fprintf(stderr, "Function::redrawGrid: Could not invert Matrix.\n");
    return;
  }
  contentsToViewport(0, 0, x0, y0);
  qp_in[0] = QPoint(x0, y0);
  qp_out[0] = invMatrix.map(qp_in[0]);
  ix0 =  qp_out[0].x() / (FUNCTION_GRID * 2);
  iy0 =  qp_out[0].y() / (FUNCTION_GRID * 2);
//  fprintf(stderr, "x0: %d  y0: %d qp.x(): %d, qp.y(): %d  ix0: %d  iy0: %d\n", x0, y0, qp_out[0].x(), qp_out[0].y(), ix0, iy0);
  qp_in[0].setX(0);
  qp_in[1].setX(FUNCTION_WIDTH);
  qp_in[1].setY(0);
  zoomMatrix = matrix;
  zoomMatrix.scale(1.0/zoom, 1.0/zoom);
  zoomMatrix.translate(-ix0 * zoom * 2 * FUNCTION_GRID, -iy0 * zoom * 2 * FUNCTION_GRID);
  for (l1 = 0; l1 <gridX.count(); l1++) {
    qp_in[0].setY(l1 * FUNCTION_GRID);
    for (l2 = 0; l2 < 2; l2++) {
      qp_out[l2] = zoomMatrix.map(qp_in[l2]);
    }
    gridX.at(l1)->setPoints(qp_out[0].x(), qp_out[0].y(), qp_out[1].x(), qp_out[0].y());
  }
  qp_in[0].setY(0);
  qp_in[1].setY(FUNCTION_HEIGHT);
  qp_in[1].setX(0);
  for (l1 = 0; l1 <gridY.count(); l1++) {
    qp_in[0].setX(l1 * FUNCTION_GRID);
    for (l2 = 0; l2 < 2; l2++) {
      qp_out[l2] = zoomMatrix.map(qp_in[l2]);
    }
    gridY.at(l1)->setPoints(qp_out[0].x(), qp_out[0].y(), qp_out[0].x(), qp_out[1].y());
  }
}

void Function::contentsMousePressEvent(QMouseEvent *ev) {

  int l1, l2;
  QCanvasItemList hitList;
  
  mousePressPos = ev->pos();
  mousePressed = true;
  hitList = canvas()->collisions(ev->pos());
  if (hitList.count()) {
    for (l1 = 0; l1 < functionCount; l1++) {
      for (l2 = 0; l2 < pointCount; l2++) {
        for(QCanvasItemList::Iterator it=hitList.begin(); it!=hitList.end(); it++) {
          if (canvasFunctionList.at(l1)->canvasPoints.at(l2) == *it) {
//            fprintf(stderr, "Hit %d %d\n", l1, l2);
            if (!*editIndex || (l1 == *editIndex - 1)) {
              activeFunction = l1;
              activePoint = l2;
            }
            break;
          }
        }
      }  
    }
    if (*mode == 3) {
      for (l1 = 0; l1 < pointCount; l1++) {
        deltaArray[l1] = (double)points[activeFunction]->point(l1).x() - FUNCTION_CENTER_X;
      }
    } else if (*mode == 4) {
      for (l1 = 0; l1 < pointCount; l1++) {
        deltaArray[l1] = (double)points[activeFunction]->point(l1).y() - FUNCTION_CENTER_Y;
      }
    }                                                                      
  } else {
    activePoint = -1;
  }
}

void Function::contentsMouseReleaseEvent(QMouseEvent *ev) {

  QPoint qp;
  int l1;
  
  mousePressed = false;
  if ((activeFunction >=0) && (activePoint >= 0)) {
    switch (*mode) {
      case 5:
        for (l1 = 0; l1 < pointCount; l1++) {
          qp = QPoint(l1 * FUNCTION_WIDTH / (pointCount - 1), FUNCTION_HEIGHT >> 1);
          points[activeFunction]->setPoint(l1, qp);                              
        }
        break;
      case 6:
        for (l1 = 0; l1 < pointCount; l1++) {
          qp = QPoint(l1 * FUNCTION_WIDTH / (pointCount - 1), (pointCount - 1 - l1) * FUNCTION_WIDTH / (pointCount - 1));
          points[activeFunction]->setPoint(l1, qp);                              
        }
        break;
    } 
    updateFunction(activeFunction);
    repaintContents(false);
  }  
}

void Function::contentsMouseMoveEvent(QMouseEvent *ev) {

  QWMatrix invMatrix;
  QPoint qp;
  int l1, delta;
  float scaleFactor;

  if (matrix.isInvertible()) {
    invMatrix = matrix.invert();
    qp = invMatrix.map(ev->pos());
    emit mousePos(qp.x(), qp.y());
    if (mousePressed && (activeFunction >=0) && (activePoint >= 0)) {
//    fprintf(stderr, "mouseMoveEvent scale[0] = %f, scale[1] = %f\n", scale[0], scale[1]);
      switch (*mode) {
        case 0:
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
          break;
        case 1:
          delta = qp.x() - points[activeFunction]->point(activePoint).x();
          for (l1 = 0; l1 < pointCount; l1++) {
            qp = QPoint(points[activeFunction]->point(l1).x() + delta, points[activeFunction]->point(l1).y());
            if (qp.x() < 0) qp.setX(0);
            if (qp.x() > FUNCTION_WIDTH) qp.setX(FUNCTION_WIDTH);
            points[activeFunction]->setPoint(l1, qp);                              
          }
          break;
        case 2:
          delta = qp.y() - points[activeFunction]->point(activePoint).y();
          for (l1 = 0; l1 < pointCount; l1++) {
            qp = QPoint(points[activeFunction]->point(l1).x(), points[activeFunction]->point(l1).y()+delta);
            if (qp.y() < 0) qp.setY(0);
            if (qp.y() > FUNCTION_HEIGHT) qp.setY(FUNCTION_HEIGHT);
            points[activeFunction]->setPoint(l1, qp);                              
          }
          break;
        case 3:
          delta = qp.x() - FUNCTION_CENTER_X;
          scaleFactor = (deltaArray[activePoint] != 0) 
                      ? (double)delta / (double)deltaArray[activePoint] : 1.0;
          for (l1 = 0; l1 < pointCount; l1++) {
            qp = QPoint((double)FUNCTION_CENTER_X + scaleFactor * (double)deltaArray[l1], 
                        points[activeFunction]->point(l1).y());
            if (qp.x() < 0) qp.setX(0);
            if (qp.x() > FUNCTION_WIDTH) qp.setX(FUNCTION_WIDTH);
            points[activeFunction]->setPoint(l1, qp);                              
          }
          break;  
        case 4:
          delta = qp.y() - FUNCTION_CENTER_Y;
          scaleFactor = (deltaArray[activePoint] != 0) 
                      ? (double)delta / (double)deltaArray[activePoint] : 1.0;
          for (l1 = 0; l1 < pointCount; l1++) {
            qp = QPoint(points[activeFunction]->point(l1).x(), 
                        (double)FUNCTION_CENTER_Y + scaleFactor * (double)deltaArray[l1]);
            if (qp.y() < 0) qp.setY(0);
            if (qp.y() > FUNCTION_HEIGHT) qp.setY(FUNCTION_HEIGHT);
            points[activeFunction]->setPoint(l1, qp);                              
          }
          break;
      } 
//      fprintf(stderr, "mouseMoveEvent points[%d]->point(%d) = %d %d\n", activeFunction, activePoint, 
//              points[activeFunction]->point(activePoint).x(), points[activeFunction]->point(activePoint).y());
      updateFunction(activeFunction);
//      fprintf(stderr, "mouseMoveEvent f[0][%d][%d+1] = %f f[1][%d][%d+1] = %f\n", activeFunction, activePoint, f[0][activeFunction][activePoint+1],
//              activeFunction, activePoint, f[1][activeFunction][activePoint+1]);
      repaintContents(false);
    }
  } else {
    fprintf(stderr, "Matrix not invertible !\n");
  }
}

void Function::setZoom(float p_zoom) {

  int l1;

  zoom = p_zoom;
  ((Canvas *)canvas())->setZoom(zoom);
  canvas()->resize(zoom * width(), zoom * height());
  updateScale();
//  redrawGrid();
  redrawFunction();
  updateContents();
}

void Function::updateScale() {

  double scale[2];

  scale[0] = (zoom * (double)width()-(double)(FUNCTION_BORDER_L+FUNCTION_BORDER_R)) / (double)FUNCTION_WIDTH;
  scale[1] = (zoom * (double)height()-(double)(FUNCTION_BORDER_B+FUNCTION_BORDER_T)) / (double)FUNCTION_HEIGHT;
  matrix.reset();
  matrix.scale(scale[0], scale[1]);
  matrix.translate((double)FUNCTION_BORDER_L/scale[0], (double)FUNCTION_BORDER_B/scale[1]);
  ((Canvas *)canvas())->setMatrix(matrix);
}

void Function::setPoint(int f_index, int p_index, int x, int y) {

  points[f_index]->setPoint(p_index, QPoint(x, y));
}

QPoint Function::getPoint(int f_index, int p_index) {

  return(points[f_index]->point(p_index));
}
        