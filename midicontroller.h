#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include <qobject.h>
#include <qlist.h>
#include <qlistview.h>

/**
  *@author Matthias Nagorni
  */
    
class MidiController : public QObject  {
    
  Q_OBJECT
    
  public:
    int type;
    int ch;
    int param;
    QListViewItem *listViewItem;
          
  public:
    MidiController();
    MidiController(int p_type, int p_ch, int p_param);
    ~MidiController();
    void sendMidiValue(int value);
    void setListViewItem(QListViewItem *p_listViewItem);

  signals:
    void midiValueChanged(int);
};
                              
#endif
                              