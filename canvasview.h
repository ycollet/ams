#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <qcanvas.h>
#include <qevent.h>

class CanvasView : public QCanvasView
{
  Q_OBJECT

  public:
    CanvasView(QCanvas *canvas, QWidget* parent=0, const char *name=0, WFlags f=0);
    ~CanvasView();

  protected:
    virtual void contentsMousePressEvent (QMouseEvent* );
    virtual void contentsMouseReleaseEvent (QMouseEvent* );
    virtual void contentsMouseMoveEvent (QMouseEvent* );
    
};
  
#endif
