#ifndef CANVAS_H
#define CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <qcanvas.h>
#include <qwmatrix.h>

class Canvas : public QCanvas
{
  Q_OBJECT

  private:
    float zoom, dx, dy, w, h;
    int border_l, border_r, border_b, border_t, scale;
    QWMatrix matrix;

  public:
    Canvas(QObject* parent=0, const char *name=0);
    ~Canvas();
    void setZoom(float p_zoom);
    void setMatrix(QWMatrix p_matrix);
    void setGrid(int p_border_l, int p_border_r, int p_border_b, int p_border_t, int p_scale, int p_w, int p_h, float p_dx, float p_dy);

  protected:
    virtual void drawBackground(QPainter & painter, const QRect & clip);
  
};
  
#endif
