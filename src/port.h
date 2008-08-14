#ifndef PORT_H
#define PORT_H

#include <QColor>
#include <QList>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QString>
#include <QWidget>

#include "synthdata.h"
#include "port_popup.h"

#define PORT_DEFAULT_WIDTH  80
#define PORT_DEFAULT_HEIGHT 15

enum dirType { PORT_IN, PORT_OUT };
enum outTypeEnum { outType_audio, outType_control };

class Port : public QWidget
{
  Q_OBJECT

    QString portName;
    int portNameWidth;

    public:
    Port(const QString &p_portName, dirType p_dir, int p_index,
            class Module *parent, int p_color = 0);
    ~Port();
    int connectTo(Port *port);
    float **getinputdata (void);

  public:
    class Module *module;
    QList<Port*> connectedPortList;
    dirType dir;
    int index;
    QColor jackColor, cableColor, &colorFont;
    outTypeEnum outType;
    bool highlighted;
    QList<outTypeEnum> outTypeAcceptList;
    void popupMenuClicked(PopupMenu::portAction);
    bool hasConnectedPort();
    Port* needsConnectionToPort();

  signals:
    void portClicked();
    void portDisconnected();
    
  protected:
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent(QMouseEvent* );
    
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
    bool isInPort();
    bool hasIndex(int idx);
};
  
#endif
