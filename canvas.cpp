#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <qpainter.h>
#include <qcanvas.h>
#include <qstring.h>
#include <qfont.h>
#include "canvas.h"


Canvas::Canvas(QObject* parent, const char *name)
           : QCanvas(parent, name) {

  zoom = 1.0;
  setDoubleBuffering(TRUE);
}

Canvas::~Canvas() {

}

void Canvas::drawBackground(QPainter & painter, const QRect & clip) {

  int l1;
  QPoint qp_in[2], qp_out[2];
  float zoom_dx, zoom_dy, val;
  QString qs;

  zoom_dx = dx / zoom;     
  zoom_dy = dy / zoom;  
  painter.fillRect(clip, QBrush(QColor(20, 20, 80)));
  painter.setPen(QColor(0, 220, 0));

  for (l1 = 0; l1 <= w / zoom_dx; l1++) {
    val =  ((float)l1 * zoom_dx - (float)w/2.0) / (float)scale;
    if (floor(val) == ceil(val)) {
      painter.setPen(QColor(0, 230, 0));
    } else {
      painter.setPen(QColor(60, 140, 60));
    }
    qp_in[0].setX(l1 * zoom_dx);
    qp_in[0].setY(0);
    qp_in[1].setX(l1 * zoom_dx);
    qp_in[1].setY(h);
    qp_out[0] = matrix.map(qp_in[0]);
    qp_out[1] = matrix.map(qp_in[1]);    
    painter.drawLine(qp_out[0], qp_out[1]);
    if (width()/zoom > 700) {
      qs.sprintf("%6.2f", val);
    } else if (width()/zoom > 600) {
      qs.sprintf("%6.1f", val);
    } else {
      qs.sprintf("%6.0f", val);
    }
    painter.setFont(QFont("Helvetica", 9));
    painter.drawText(qp_out[1].x() - 20, qp_out[1].y() + 15, qs);
  }
  for (l1 = 0; l1 <= h / zoom_dy; l1++) {
    val = -((float)l1 * zoom_dy - (float)w/2.0) / (float)scale;
    if (floor(val) == ceil(val)) {
      painter.setPen(QColor(0, 230, 0));
    } else {
      painter.setPen(QColor(60, 140, 60));
    }
    qp_in[0].setY(l1 * zoom_dy);
    qp_in[0].setX(0);
    qp_in[1].setY(l1 * zoom_dy);
    qp_in[1].setX(w);
    qp_out[0] = matrix.map(qp_in[0]);
    qp_out[1] = matrix.map(qp_in[1]);    
    painter.drawLine(qp_out[0], qp_out[1]);
    qs.sprintf("%6.2f", val);
    painter.setFont(QFont("Helvetica", 9));
    painter.drawText(qp_out[0].x() - 40, qp_out[0].y() + 4, qs);
  }
}  

void Canvas::setZoom(float p_zoom) {

  zoom = p_zoom;
}

void Canvas::setMatrix(QWMatrix p_matrix) {

  matrix = p_matrix;
}

void Canvas::setGrid(int p_border_l, int p_border_r, int p_border_b, int p_border_t, int p_scale, int p_w, int p_h, float p_dx, float p_dy) {

  border_l = p_border_l;
  border_r = p_border_r;
  border_b = p_border_b;
  border_t = p_border_t;
  scale = p_scale;
  w = p_w;
  h = p_h;
  dx = p_dx;
  dy = p_dy;
}
