#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qpainter.h>
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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "textedit.h"

TextEdit::TextEdit(QWidget* parent, const char *name, SynthData *p_synthdata) : QVBox(parent, name) {

  synthdata = p_synthdata;
  setMargin(5);
  setSpacing(5);
//  synthdata->incTextEditCount();
//  textEditID = synthdata->getTextEditID();
//  synthdata->textEditList.append(this);
  textEditID = 0;  // TODO assign this 
  setPalette(QPalette(QColor(77, 70, 64), QColor(77, 70, 64)));
  setGeometry(TEXTEDIT_NEW_X, TEXTEDIT_NEW_Y, TEXTEDIT_DEFAULT_WIDTH, TEXTEDIT_DEFAULT_HEIGHT);
  drag = false;
  sizeDrag = false;
  QHBox *editBox = new QHBox(this);
  editBox->setMargin(5);
  textEdit = new QTextEdit(editBox);
//  textEdit->setColor(QColor(0, 0, 0));
//  textEdit->setPaper(QBrush(QColor(250, 250, 250)));
  textEdit->setPalette(QPalette(QColor(77, 70, 64), QColor(250, 250, 250)));
  QHBox *buttonBox = new QHBox(this);
  buttonBox->setMargin(5);
  new QWidget(buttonBox);
  QPushButton *removeButton = new QPushButton("Remove Comment", buttonBox);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeThisTextEdit()));
  new QWidget(buttonBox);
}

TextEdit::~TextEdit() {

//  synthdata->textEditListList.removeRef(this);
//  synthdata->decTextEditCount();
}

void TextEdit::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  int l1;

  for (l1 = 0; l1 < 4; l1++) { 
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
}

void TextEdit::mousePressEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    raise();
    drag = true;
    mousePressPos = ev->pos();
    break;
  case Qt::RightButton:
    break;
  case Qt::MidButton:
    sizeDrag = true;
    mousePressPos = ev->pos();
    break;
  default:
    break;
  }
}  

void TextEdit::mouseReleaseEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    drag = false;
    break;
  case Qt::RightButton:
    break;
  case Qt::MidButton:
    sizeDrag = false;
    break;
  default:
    break;
  }
}  
   
void TextEdit::mouseMoveEvent(QMouseEvent *ev) {

  if (drag) {
    emit dragged(ev->pos());
  }
  if (sizeDrag) {   
    emit sizeDragged(ev->pos());  
  }
}  
            
QPoint TextEdit::getMousePressPos() {

  return(mousePressPos);
}

void TextEdit::removeThisTextEdit() {

  emit removeTextEdit();
}
