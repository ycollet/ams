#ifndef KSCOPESCREEN_H
#define KSCOPESCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qscrollview.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qevent.h>
#include "synthdata.h"

#define MINIMUM_WIDTH                 100
#define MINIMUM_HEIGHT                 50
#define TRIGGER_RANGE               16384
#define SCOPE_BUFSIZE              128000 

enum modeType { MODE_NORMAL, MODE_SUM, MODE_DIFF };
enum edgeType { EDGE_RISING, EDGE_FALLING };
enum triggerModeType { TRIGGERMODE_CONTINUOUS, TRIGGERMODE_TRIGGERED, TRIGGERMODE_SINGLE, TRIGGERMODE_MIDI };

class ScopeScreen : public QWidget
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    modeType mode;
    edgeType edge;
    triggerModeType triggerMode;
    int ch1, ch2;
    float zoom;
    float triggerThrs;
    bool triggered;
    float timeScale, timeScaleLastTrigger;
    int timeScaleFrames;
    float *scopebuf;
    int scopebufValidFrames;

  public:
    float *scopedata;
    int readofs, writeofs;

  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void viewportResizeEvent (QResizeEvent *ev); 
    
  public:
    ScopeScreen(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~ScopeScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    modeType setMode(modeType p_mode);
    edgeType setEdge(edgeType p_edge);
    triggerModeType setTriggerMode(triggerModeType p_triggerMode);
    int setCh1(int p_ch1);
    int setCh2(int p_ch2);
    float setZoom(float p_zoom);
    float setTriggerThrs(float p_triggerThrs);
    float setTimeScale(float p_timeScale);
    modeType getMode();
    edgeType getEdge();
    triggerModeType getTriggerMode();
    int getCh1();
    int getCh2();
    float getZoom();
    float getTriggerThrs();
    float getTimeScale();

  public slots: 
    void refreshScope();
    void singleShot();
};
  
#endif
      
