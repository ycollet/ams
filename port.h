#ifndef PORT_H
#define PORT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qlist.h>
#include <qstring.h>
#include <qcolor.h>
#include <qpoint.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include "synthdata.h"

#define PORT_DEFAULT_WIDTH  80
#define PORT_DEFAULT_HEIGHT 15

enum dirType { PORT_IN, PORT_OUT };
enum outTypeEnum { outType_audio, outType_control };

class Port : public QWidget
{
  Q_OBJECT

    QString portName;
    static class PopupMenu *contextMenu;
    int portWidth;

  public:
    Port(const QString &p_portName, dirType p_dir, int p_index, class Module *parent,
         int p_portWidth = PORT_DEFAULT_WIDTH, int p_color = 0);
    ~Port();
    int connectTo(Port *port);
    float **getinputdata (void);

  private:
    void showContextMenu(QPoint pos);

  public:
    class Module *module;
    QList<Port*> connectedPortList;
    dirType dir;
    int index, fontColor;
    QColor jackColor, cableColor, colorFont1, colorFont2;
    outTypeEnum outType;
    bool highlighted;
    QList<outTypeEnum> outTypeAcceptList;

  signals:
    void portClicked();
    void portDisconnected();
    
  protected:
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent (QMouseEvent* );
    virtual void mouseReleaseEvent (QMouseEvent* );
    virtual void mouseMoveEvent (QMouseEvent* );
    
  public:
    void disconnectClicked();  
    void cableGrayClicked();
    void cableRedClicked();
    void cableGreenClicked();
    void cableBlueClicked();
    void cableYellowClicked();
    void cableDefaultClicked();
    void jackColorClicked();
    void cableColorClicked();
    void checkConnectionStatus();
};
  
#endif
