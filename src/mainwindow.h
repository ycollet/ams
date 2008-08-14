#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <signal.h>
#include <QApplication>
#include <QCloseEvent>
#include <QMainWindow>
#include <QSocketNotifier>

#include "modularsynth.h"
#include "msoptions.h"

class MainWindow: public QMainWindow {
  Q_OBJECT

  static int pipeFd[2];
  static void sighandler(int);

private:
    QString fileName;
    ModularSynth *modularSynth;
    bool saveFile();
    void newFile();
    void openFile(const QString&);
    void chooseFile();
    int querySaveChanges();
    bool isModified();

private slots:
    void unixSignal(int fd);
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void updateWindowTitle();

public:
  MainWindow(const ModularSynthOptions&);

protected:
  void closeEvent(QCloseEvent *e);
};

#endif
