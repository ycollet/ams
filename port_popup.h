#include <QMenu>
#include "modularsynth.h"
#include "module.h"
#include "port.h"

class PopupMenu: public QMenu {
  Q_OBJECT

  Port *port;

public:

  void popup(Port *p, QPoint pos) {
    port = p;
    QMenu::popup(pos);
  }

  PopupMenu()
    :port(NULL)
  {
    QAction *a = new QAction("Disconnect", this);
    connect(a, SIGNAL(triggered()), this, SLOT(disconnectClicked()));
    addAction(a);

    addSeparator();
    a = new QAction("Default Cable", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableDefaultClicked()));
    addAction(a);

    a = new QAction("Gray Cable", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableGrayClicked()));
    addAction(a);

    a = new QAction("Red Cable", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableRedClicked()));
    addAction(a);

    a = new QAction("Green Cable", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableGreenClicked()));
    addAction(a);

    a = new QAction("Blue Cable", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableBlueClicked()));
    addAction(a);

    a = new QAction("Yellow Cable", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableYellowClicked()));
    addAction(a);

    addSeparator();
    a = new QAction("Set Jack Color", this);
    connect(a, SIGNAL(triggered()), this, SLOT(jackColorClicked()));
    addAction(a);

    a = new QAction("Set Cable Color", this);
    connect(a, SIGNAL(triggered()), this, SLOT(cableColorClicked()));
    addAction(a);
  }

public slots:
  void disconnectClicked() {
    port->disconnectClicked();
  }
  void cableGrayClicked() {
    port->cableGrayClicked();
    port->module->modularSynth()->update();
  }
  void cableRedClicked() {
    port->cableRedClicked();
    port->module->modularSynth()->update();
  }
  void cableGreenClicked() {
    port->cableGreenClicked();
    port->module->modularSynth()->update();
  }
  void cableBlueClicked() {
    port->cableBlueClicked();
    port->module->modularSynth()->update();
  }
  void cableYellowClicked() {
    port->cableYellowClicked();
    port->module->modularSynth()->update();
  }
  void cableDefaultClicked() {
    port->cableDefaultClicked();
    port->module->modularSynth()->update();
  }
  void jackColorClicked() {
    port->jackColorClicked();
    port->module->modularSynth()->update();
  }
  void cableColorClicked() {
    port->cableColorClicked();
    port->module->modularSynth()->update();
  }
};
