#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qpointarray.h>
#include <qptrlist.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qcanvas.h>
#include <qevent.h>
#include "canvasview.h"
#include "canvasfunction.h"
#include "synthdata.h"

#define FUNCTION_MINIMUM_WIDTH        100
#define FUNCTION_MINIMUM_HEIGHT        50
#define MAX_FUNCTIONS                   8
#define MAX_POINTS                      8
#define FUNCTION_WIDTH              10000 // Function points range from 0 to 10000.
#define FUNCTION_HEIGHT             10000 // Width == Height 
#define FUNCTION_SCALE                500 // 500 in point coordinates correspond to 1 V
#define FUNCTION_CENTER_X            5000 // Origin of the point coordinate system. This Origin
#define FUNCTION_CENTER_Y            5000 // is subtracted from the point coordinate to get negative CVs.
#define FUNCTION_BORDER_1               5
#define FUNCTION_BORDER_2              25

class Function : public QWidget
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    int functionCount;
    QPointArray *screenPoints[MAX_FUNCTIONS];
    QColor colorTable[MAX_FUNCTIONS];
    QCanvas *canvas;
    QList<CanvasFunction> canvasFunctionList;
    QList<QCanvasText> canvasTextList;
    CanvasView *canvasView;
    float zoomMin[2], zoomMax[2];             // Ranges for zoomed display of functions
    bool mousePressed;
    int activeFunction, activePoint;

  public:
    QPointArray *points[MAX_FUNCTIONS];
    float fx[MAX_FUNCTIONS+2][MAX_POINTS+2];    
    int pointCount;
    
  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent (QResizeEvent* );            
    
  public:
    Function(int p_functionCount, QPointArray *p_points[], int p_pointCount, SynthData *p_synthdata, QWidget* parent=0, const char *name=0);
    ~Function();
    void setPointCount(int count);
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  public slots: 
    void updateFunction(int index);
    void mousePressEvent(QMouseEvent* );
    void mouseReleaseEvent(QMouseEvent* ); 
    void mouseMoveEvent(QMouseEvent* );
};
  
#endif
