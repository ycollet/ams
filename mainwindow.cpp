#include "mainwindow.h"
#include "synthdata.h"

int MainWindow::pipeFd[2];

void MainWindow::sighandler(int s)
{
  char pipeMessage = s;
  write(pipeFd[1], &pipeMessage, 1);
}

void MainWindow::unixSignal(int /*fd*/)
{
//   char pipeIn[4];
//   int pipeRed = read(fd, pipeIn, sizeof(pipeIn));
//   StdOut << __PRETTY_FUNCTION__ << (int)pipeIn[0]  << endl;
  qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
  qApp->quit();
}

MainWindow::MainWindow()
{
  setWindowTitle("AlsaModularSynth");
  if (pipe(pipeFd) < 0)
    return;

  QSocketNotifier *sigNotifier = new QSocketNotifier(pipeFd[0], QSocketNotifier::Read, this);
  QObject::connect(sigNotifier, SIGNAL(activated(int)), this, SLOT(unixSignal(int)));
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sighandler;
  sigaction(SIGINT, &action, NULL);
}
