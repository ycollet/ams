#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qpoint.h>
#include <qlist.h>
#include "synthdata.h"

#define TEXTEDIT_DEFAULT_WIDTH                200
#define TEXTEDIT_DEFAULT_HEIGHT               170 
#define TEXTEDIT_NEW_X                         50
#define TEXTEDIT_NEW_Y                         50

class TextEdit : public QVBox
{
  Q_OBJECT

  private:
    bool drag, sizeDrag;
    QPoint mousePressPos;
    
  public: 
    int textEditID;
    QTextEdit *textEdit;
    
  protected:
    SynthData *synthdata;

  public:
    TextEdit(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    virtual  ~TextEdit();
    QPoint getMousePressPos();
    
  protected:
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent (QMouseEvent* );
    virtual void mouseReleaseEvent (QMouseEvent* );
    virtual void mouseMoveEvent (QMouseEvent* );

  signals:
    void dragged(QPoint pos);
    void sizeDragged(QPoint pos);
    void removeTextEdit();
                        
  public slots: 
    virtual void removeThisTextEdit();
};
  
#endif
