#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qdict.h>
#include <qtabwidget.h>

#include "parameter.h"
#include "parameterpanel.h"

class ConfigDialog : public QVBox
{
  Q_OBJECT

  private:
    QDict<ParameterPanel> mPanels;
    QTabWidget *tabWidget;
    QObject * parentModule;
  public:
    //! @todo ugly, but i am lazy:
    QPushButton *removeButton;//public so that it can be disabled from outside
    QVBox *headerBox;// public so you can add your custom headers like envelopes, scopes, etc.


    ConfigDialog(QObject *parentModule,QWidget* parent=0, const char *name=0);
    ~ConfigDialog();


//     int addEnvelope(float *delayRef, float *attackRef, float *holdRef,
//                     float *decayRef, float *sustainRef, float *releaseRef, QWidget *parent=0);
//     int addMultiEnvelope(int envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef, QWidget *parent=0);
//     int addLabel(QString label, QWidget *parent=0);
//     int addScopeScreen(float *timeScaleRef, int *modeRef, int *edgeRef, int *triggerModeRef,
//                        float *triggerThrsRef, float *zoomRef, QWidget *parent=0);
//     int addSpectrumScreen(QWidget *parent=0);

//     QHBox *addHBox(QWidget *parent=0);
//     QVBox *addVBox(QWidget *parent=0);
//     int addLineEdit(QString lineName, QWidget *parent=0);

    void addParameter(Parameter * param, const char * panelName=0);

    private:
//     int initTabWidget();
//     int addTab(QWidget *tabPage, QString tabLabel);

  signals:
    void removeModuleClicked();

  public slots:
    void removeButtonClicked();
};

#endif

