#include <stdio.h>
#include <stdlib.h>
#include "canvasview.h"


CanvasView::CanvasView(QCanvas *canvas, QWidget* parent, const char *name, WFlags f)
           : QCanvasView(canvas, parent, name, f) {

}

CanvasView::~CanvasView() {

}

void CanvasView::contentsMousePressEvent(QMouseEvent *ev) {

  emit(mousePressed(ev));  
}  
   
void CanvasView::contentsMouseReleaseEvent(QMouseEvent *ev) {
  
  emit(mouseReleased(ev));
}  
   
void CanvasView::contentsMouseMoveEvent(QMouseEvent *ev) {

  emit(mouseMoved(ev));
}      
