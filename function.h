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
#include <qwmatrix.h>
#include "canvasfunction.h"
#include "synthdata.h"
#include "canvas.h"

#define FUNCTION_MINIMUM_WIDTH        550
#define FUNCTION_MINIMUM_HEIGHT       350
#define MAX_FUNCTIONS                   4
#define MAX_POINTS                     11
#define FUNCTION_WIDTH              10000 // Function points range from 0 to 10000.
#define FUNCTION_HEIGHT             10000 // Width == Height 
#define FUNCTION_SCALE               1000 // 1000 in point coordinates correspond to 1 V
#define FUNCTION_CENTER_X            5000 // Origin of the point coordinate system. This Origin
#define FUNCTION_CENTER_Y            5000 // is subtracted from the point coordinate to get negative CVs.
#define FUNCTION_BORDER_L              50
#define FUNCTION_BORDER_R              35
#define FUNCTION_BORDER_B              35
#define FUNCTION_BORDER_T              40
#define FUNCTION_GRID                1000

#define FUNCTION_COLOR_1 0xF02020
#define FUNCTION_COLOR_2 0xF0F000
#define FUNCTION_COLOR_3 0x00F0F0
#define FUNCTION_COLOR_4 0x00F000
#define FUNCTION_COLOR_5 0xF00000
#define FUNCTION_COLOR_6 0x00F000
#define FUNCTION_COLOR_7 0xF000F0
#define FUNCTION_COLOR_8 0x8080FF
#define FUNCTION_COLOR_BG 0x141450
#define FUNCTION_COLOR_FG 0x505080

typedef float float_function[2][MAX_FUNCTIONS+2][MAX_POINTS+2];

class Function : public QCanvasView
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    int *mode, *editIndex;
    QPointArray *screenPoints[MAX_FUNCTIONS];
    QPointArray *points[MAX_FUNCTIONS];
    QColor colorTable[MAX_FUNCTIONS];
    int deltaArray[MAX_POINTS];
    QList<QCanvasLine> gridX, gridY;
    QList<CanvasFunction> canvasFunctionList;
    QList<QCanvasText> canvasTextList;
    QWMatrix matrix;
    float zoom;
    bool mousePressed;
    int activeFunction, activePoint;
    QPoint mousePressPos;

  public:
    float_function f;
    int pointCount;
    int functionCount;
    
  protected:
    virtual void resizeEvent (QResizeEvent* );            
    virtual void contentsMousePressEvent (QMouseEvent* );
    virtual void contentsMouseReleaseEvent (QMouseEvent* );
    virtual void contentsMouseMoveEvent (QMouseEvent* );
                
  public:
    Function(int p_functionCount, int *p_mode, int *p_editIndex, QPointArray *p_points[], int p_pointCount, SynthData *p_synthdata, QWidget* parent=0, const char *name=0);
    ~Function();
    void setPointCount(int count);
    void setFunctionCount(int count);
    void setPoint(int f_index, int p_index, int x, int y);
    QPoint getPoint(int f_index, int p_index);
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    void redrawFunction();
    void redrawGrid();
    void updateScale();

  signals:
    void mousePos(int, int);

  public slots: 
    void updateFunction(int index);
    void setZoom(float p_zoom);
};
  
#endif
