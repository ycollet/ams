#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>
#include <qregexp.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h> 
#include <qlistbox.h> 
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qframe.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <ladspa.h>
#include "ladspadialog.h"
#include "synthdata.h"

LadspaDialog::LadspaDialog(SynthData *p_synthdata, QWidget* parent, const char *name) 
                : QVBox(parent, name) {

  char *error;
  QString ladspadir, ladspapath, qs;
  void *so_handle;                                                             
  LADSPA_Descriptor_Function ladspa_dsc_func;
  const LADSPA_Descriptor *ladspa_dsc;                   
  int l1, l2, colon, lastcolon;
  glob_t globbuf;
  QListViewItem *ladspaSetItem;

  setCaption("Ladspa Browser");  
  setMargin(10);
  setSpacing(10);
  synthdata = p_synthdata;
  ladspaList = new QListView(this);
  ladspaList->setRootIsDecorated(true);
  ladspaList->addColumn("Available Ladspa Plugins");

  ladspapath.sprintf("%s", getenv("LADSPA_PATH"));
  if (ladspapath.length() < 1) {
    fprintf(stderr, "\nYou did not set the environment variable LADSPA_PATH.\n");
    fprintf(stderr, "Assuming LADSPA_PATH=/usr/lib/ladspa:/usr/local/lib/ladspa\n");
    ladspapath = "/usr/lib/ladspa:/usr/local/lib/ladspa";
  } else {
    fprintf(stderr, "LADSPA_PATH: %s\n", ladspapath.latin1());
  }
  colon = -1;
  l2 = 0;
  do {   
    lastcolon = colon;
    colon = ladspapath.find(QRegExp(":"), lastcolon + 1);
    if (colon >= 0) {
      ladspadir = ladspapath.mid(lastcolon + 1, colon - lastcolon - 1);
    } else {
      ladspadir = (lastcolon) ? ladspapath.mid(lastcolon + 1, ladspapath.length() - lastcolon - 1)
                              : ladspapath;
    }
//    fprintf(stderr, "Searching for LADSPA plugins in %s\n", ladspadir.latin1());
    ladspadir += "/*.so";
    glob(ladspadir.latin1(), GLOB_MARK, NULL, &globbuf);
    for (l1 = 0; l1 < globbuf.gl_pathc; l1++) {
//      fprintf(stderr, "    found %s\n", globbuf.gl_pathv[l1]);
      if (!(so_handle = dlopen(globbuf.gl_pathv[l1], RTLD_LAZY))) {
        fprintf(stderr, "Error opening shared ladspa object %s.\n", globbuf.gl_pathv[l1]);
        continue;
      }
      ladspa_dsc_func =(LADSPA_Descriptor_Function)dlsym(so_handle, "ladspa_descriptor");
      if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error accessing ladspa descriptor in %s.\n", globbuf.gl_pathv[l1]);
        continue;
      } 
      synthdata->ladspa_dsc_func_list[l2] = ladspa_dsc_func;
      qs.sprintf("%s", globbuf.gl_pathv[l1]);
      synthdata->ladspa_lib_name[l2] = qs.mid(qs.findRev("/") + 1, qs.findRev(".") - qs.findRev("/") - 1);
      l2++;
    }
    globfree(&globbuf);
  } while (colon >= 0);
  l1 = 0;
  while(synthdata->ladspa_dsc_func_list[l1]!=NULL) {
    l2 = 0;
    QListViewItem *ladspaSetItem = new QListViewItem(ladspaList, QString(synthdata->ladspa_lib_name[l1]));
    ladspaSetItem->setSelectable(false);
    while((ladspa_dsc = synthdata->ladspa_dsc_func_list[l1](l2)) != NULL) {
      new QListViewItem(ladspaSetItem, QString(ladspa_dsc->Name));   
      l2++;
    }
    l1++;  
  }  

  QHBox *searchBox = new QHBox(this);
  searchBox->setSpacing(10);
  searchLine = new QLineEdit(searchBox);
  QPushButton *searchButton = new QPushButton("Search", searchBox);
  QObject::connect(searchButton, SIGNAL(clicked()), this, SLOT(searchClicked()));
  QObject::connect(searchLine, SIGNAL(returnPressed()), this, SLOT(searchClicked()));

  QVBox *labelBox = new QVBox(this);
  labelBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  labelBox->setSpacing(5);
  pluginLabel = new QLabel(labelBox);
  pluginLabel->setAlignment(Qt::WordBreak | Qt::AlignLeft);
  pluginMaker = new QLabel(labelBox);             
  pluginMaker->setAlignment(Qt::WordBreak | Qt::AlignLeft);
  pluginCopyright = new QLabel(labelBox);             
  pluginCopyright->setAlignment(Qt::WordBreak | Qt::AlignLeft);
  QObject::connect(ladspaList, SIGNAL(selectionChanged()), this, SLOT(pluginHighlighted()));

  QHBox *buttonBox = new QHBox(this);
  QPushButton *insertButton = new QPushButton("Create Plugin", buttonBox);
  QObject::connect(insertButton, SIGNAL(clicked()), this, SLOT(insertClicked()));  
  new QWidget(buttonBox);
  QPushButton *insertPolyButton = new QPushButton("Create Poly Plugin", buttonBox);  
  QObject::connect(insertPolyButton, SIGNAL(clicked()), this, SLOT(insertPolyClicked()));
}

LadspaDialog::~LadspaDialog() {
}

void LadspaDialog::insertClicked() {

  int index, n;

  if (ladspaList->selectedItem()) {
    if (synthdata->getLadspaIDs(ladspaList->selectedItem()->parent()->text(0),
                                ladspaList->selectedItem()->text(0), &index, &n)) {
      emit createLadspaModule(index, n, false);
    }
  }  
}  

void LadspaDialog::insertPolyClicked() {

  int index, n;

  if (ladspaList->selectedItem()) {
    if (synthdata->getLadspaIDs(ladspaList->selectedItem()->parent()->text(0), 
                                ladspaList->selectedItem()->text(0), &index, &n)) {
      emit createLadspaModule(index, n, true);
    }
  }
}

void LadspaDialog::pluginHighlighted() {

  const LADSPA_Descriptor *ladspa_dsc;
  int index, n;

  if (ladspaList->selectedItem()) {
    if (synthdata->getLadspaIDs(ladspaList->selectedItem()->parent()->text(0),
                                ladspaList->selectedItem()->text(0), &index, &n)) {
      ladspa_dsc = synthdata->ladspa_dsc_func_list[index](n);
      pluginLabel->setText(QString("Label: ")+QString(ladspa_dsc->Label));
      pluginMaker->setText(QString("Author: ")+QString(ladspa_dsc->Maker));
      pluginCopyright->setText(QString("Copyright: ")+QString(ladspa_dsc->Copyright));
    } 
  } else {
    pluginLabel->setText("");
    pluginMaker->setText("");
    pluginCopyright->setText("");
  }
}

void LadspaDialog::searchClicked() {

  QListViewItemIterator it(ladspaList), firstItem(ladspaList);

  while (it.current()) {
    if (it.current()->isSelected()) {
      ++it;
      break;
    }
    ++it;
  }
  if (!it.current()) {
    it = firstItem;
  }
  while (it.current()) {
    if ((it.current()->isSelectable()) && (it.current()->text(0).contains(searchLine->text(), false))) {
      break;
    }
    ++it;
  }
  if (!it.current()) {
    it = firstItem;   
    while (it.current()) {
      if ((it.current()->isSelectable()) && (it.current()->text(0).contains(searchLine->text(), false))) {
        break;
      }
      ++it;
    }
  }
  if (it.current()) {
    if (it.current()->isSelectable()) {
      ladspaList->setSelected(it.current(), true);
      it.current()->parent()->setOpen(true);
      ladspaList->ensureVisible(10, it.current()->itemPos());
    }
  }
}
