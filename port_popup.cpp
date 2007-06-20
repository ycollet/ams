#include "port_popup.h"

PopupMenu::PopupMenu()
  :port(NULL)
{
  QAction *a = new QAction("Disconnect", this);
  connect(a, SIGNAL(triggered()), this, SLOT(disconnectClicked()));
  addAction(a);

  insertSeparator();
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

  insertSeparator();
  a = new QAction("Set Jack Color", this);
  connect(a, SIGNAL(triggered()), this, SLOT(jackColorClicked()));
  addAction(a);

  a = new QAction("Set Cable Color", this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableColorClicked()));
  addAction(a);
}
