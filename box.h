#ifndef BOX_H
#define BOX_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>


class Box: public QWidget
{
  Q_OBJECT

protected:
  bool drag;
  static QPoint pressPos, mousePressPos;
  
public:
  Box(QWidget* parent, const QString &name);
  virtual  ~Box() {}
  class ModularSynth *modularSynth() {
    return (ModularSynth*)parent();
  }
    
protected:
  virtual void mousePressEvent (QMouseEvent *);
  virtual void mouseReleaseEvent (QMouseEvent *);
  virtual void mouseMoveEvent (QMouseEvent *);

signals:
    void dragged(QPoint pos);
};
  
#endif
