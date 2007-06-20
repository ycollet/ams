#include <QList>
#include "module.h"
#include "box.h"
#include "modularsynth.h"

QPoint Box::pressPos, Box::mousePressPos;

Box::Box(QWidget *parent, char const *name)
  : QWidget(parent)
{
  setObjectName(name);
  setAutoFillBackground(true);
}

void Box::mousePressEvent(QMouseEvent *ev)
{
  switch (ev->button()) {
  case Qt::LeftButton:   
    raise();
    drag = true;
    mousePressPos = ev->globalPos();
    pressPos = pos();
    break;
  default:
    break;
  }
}  

void Box::mouseReleaseEvent(QMouseEvent *ev)
{
  switch (ev->button()) {
  case Qt::LeftButton:   
    drag = false;
    modularSynth()->setPaintFastly(false);
    modularSynth()->update();
  default:
    break;
  }
}  
   
void Box::mouseMoveEvent(QMouseEvent *ev)
{
  if (drag) {
    QPoint newPos = pressPos + ev->globalPos() - mousePressPos;
    QPoint moveAll;
    move(newPos);
    if (newPos.x() < 0)
      moveAll.rx() = - newPos.x();
    if (newPos.y() < 0)
      moveAll.ry() = - newPos.y();

    QPoint topLeft = modularSynth()->childrenRect().topLeft();

    if (topLeft.x() > 60)
      moveAll.rx() = - (topLeft.x() - 60);
    if (topLeft.y() > 20)
      moveAll.ry() = - (topLeft.y() - 20);

    modularSynth()->moveAllBoxes(moveAll);
    modularSynth()->setPaintFastly(true);
    modularSynth()->resize();
    modularSynth()->update();
  }
}  
