#ifndef CANVASFUNCTION_H
#define CANVASFUNCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <qcanvas.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpointarray.h>
#include <qptrlist.h>

class CanvasFunction : public QObject
{
  Q_OBJECT

  private:
    int rtti_id, pointCount;
    QPointArray *points;
    QColor color;
    
  public:  
    QPtrList<QCanvasEllipse> canvasPoints;
    QPtrList<QCanvasLine> canvasLines;

  public:
    CanvasFunction(QCanvas *canvas, int p_rtti_id, int p_pointCount, QColor p_color, QObject * parent = 0, const char * name = 0);
    ~CanvasFunction();
    int rtti();
    void setColor(QColor p_color);
    void setPoint(int index, int x, int y, int z);

};
  
#endif
