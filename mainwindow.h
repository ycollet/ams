#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <signal.h>
#include <QApplication>
#include <QMainWindow>
#include <QSocketNotifier>

class MainWindow: public QMainWindow {
  Q_OBJECT

  static int pipeFd[2];
  static void sighandler(int);

private slots:
  void unixSignal(int fd);

public:
  MainWindow();

protected:
  void closeEvent(QCloseEvent */*event*/);
};

#endif
