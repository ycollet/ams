#include "port_popup.h"

PopupMenu::PopupMenu()
  :port(NULL)
{
  QAction *a = new QAction(tr("Disconnect"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(disconnectClicked()));
  addAction(a);

  insertSeparator();
  a = new QAction(tr("Default Cable"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableDefaultClicked()));
  addAction(a);

  a = new QAction(tr("Gray Cable"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableGrayClicked()));
  addAction(a);

  a = new QAction(tr("Red Cable"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableRedClicked()));
  addAction(a);

  a = new QAction(tr("Green Cable"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableGreenClicked()));
  addAction(a);

  a = new QAction(tr("Blue Cable"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableBlueClicked()));
  addAction(a);

  a = new QAction(tr("Yellow Cable"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableYellowClicked()));
  addAction(a);

  insertSeparator();
  a = new QAction(tr("Set Jack Color"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(jackColorClicked()));
  addAction(a);

  a = new QAction(tr("Set Cable Color"), this);
  connect(a, SIGNAL(triggered()), this, SLOT(cableColorClicked()));
  addAction(a);
}
