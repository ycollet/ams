#include <stdio.h>
#include <stdlib.h>
#include "canvasview.h"


CanvasView::CanvasView(QCanvas *canvas, QWidget* parent, const char *name, WFlags f)
           : QCanvasView(canvas, parent, name, f) {

}

CanvasView::~CanvasView() {

}

void CanvasView::contentsMousePressEvent(QMouseEvent *ev) {
  
}  
   
void CanvasView::contentsMouseReleaseEvent(QMouseEvent *ev) {
  
}  
   
void CanvasView::contentsMouseMoveEvent(QMouseEvent *ev) {

}      
