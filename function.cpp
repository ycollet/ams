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

Function::Function(int p_functionCount, QPointArray *p_points[], SynthData *p_synthdata, QWidget *parent, const char *name) 
             : QWidget (parent, name)
{
  int l1;

  synthdata = p_synthdata;
  functionCount = p_functionCount;
  for (l1 = 0; l1 < functionCount; l1++) {
    points[l1] = p_points[l1];
    fprintf(stderr, "points[%d] = %p\n", l1, points[l1]);
    setPointCount(l1, MAX_POINTS);
    screenPoints[l1] = new QPointArray(MAX_POINTS);
    fprintf(stderr, "screenPoints[%d] = %p\n", l1, screenPoints[l1]);
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
  fprintf(stderr, "Function::Function M1\n");
  canvas = new QCanvas(this);
  canvas->resize(width(), height());
  canvas->setBackgroundColor(QColor(20, 20, 80));
  fprintf(stderr, "Function::Function M2\n");
  canvasView = new CanvasView(canvas, this);
  for (l1 = 0; l1 < functionCount; l1++) {
    CanvasFunction *canvasFunction = new CanvasFunction(canvas, 1000 + l1, MAX_POINTS, this);
    canvasFunctionList.append(canvasFunction);
    updateFunction(l1);
  }
  fprintf(stderr, "Function::Function M3\n");
  fprintf(stderr, "--> points[0] = %p\n", points[0]);
}

Function::~Function()
{
}

void Function::paintEvent(QPaintEvent *) {

  fprintf(stderr,"Function::paintEvent\n");
  canvas->update();
}

void Function::updateFunction(int index) {

  int l1;
  QPoint qp;
  QWMatrix matrix;
  double scale[2];

  fprintf(stderr, "Function::updateFunction M1\n");
  scale[0] = (double)width() / (double)FUNCTION_WIDTH;
  scale[1] = -(double)height() / (double)FUNCTION_HEIGHT;
  fprintf(stderr, "scale: %f %f\n", scale[0], scale[1]);
  matrix.scale(scale[0], scale[1]);
  matrix.translate(0, -FUNCTION_HEIGHT);
  *screenPoints[index] = matrix.map(*points[index]);
  for (l1 = 0; l1 < pointCount[index]; l1++) {
    fprintf(stderr, "second loop points[%d] = %p\n", index, points[index]);
    qp = screenPoints[index]->point(l1);
    canvasFunctionList.at(index)->setPoint(l1, qp.x(), qp.y());
  }
  repaint(false);
  fprintf(stderr, "Function::updateFunction M2\n");
}

void Function::setPointCount(int index, int count) {
  
  fprintf(stderr, "setPointCount(%d, %d) --> points[0] = %p\n", index, count, points[0]);
  pointCount[index] = count;
}

QSize Function::sizeHint() const {

  return QSize(FUNCTION_MINIMUM_WIDTH, FUNCTION_MINIMUM_HEIGHT); 
}

QSizePolicy Function::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void Function::resizeEvent (QResizeEvent* )
{
  int l1;

  fprintf(stderr, "Function::resizeEvent M1 points[0] = %p\n", points[0]);
  fprintf(stderr, "screenPoints[0] = %p\n", screenPoints[0]);
  canvas->resize(width(), height());
  canvasView->resize(width(), height());
  if (canvasFunctionList.count() == functionCount) {
    for (l1 = 0; l1 < functionCount; l1++) {
      fprintf(stderr, "Function::resizeEvent M2 points[0] = %p\n", points[0]);
      updateFunction(l1);
    }
  }
  repaint(true);
}
