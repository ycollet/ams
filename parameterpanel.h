#ifndef PARAMETERPANEL_H
#define PARAMETERPANEL_H

#include <qwidget.h>
#include <qobject.h>
#include <qlayout.h>
#include <qevent.h>
#include <qpoint.h>
#include <qmap.h>
#include <qlist.h>

#include "parameter.h"
#include "parametereditor.h"
#include "parametereditorfactory.h"
#include "enumparameter.h"
#include "actionparameter.h"

class ParameterPanel : public QWidget{

   Q_OBJECT

   private:
   //! to quickly find a editor for a given parameter
   QPtrDict<QWidget> mEditors;
   QVBoxLayout * mLayout;
   QWidget * mDraggedWidget;

   int mLastPreset;
   int mID;
   QMap<int,QMap<QString,QString> > mPresets;
   QList<Parameter> mParameters;
   protected:
   virtual void mouseMoveEvent ( QMouseEvent * e );
   virtual void mousePressEvent ( QMouseEvent * e );
   virtual void mouseReleaseEvent ( QMouseEvent * e );
   virtual void dragMoveEvent ( QDragMoveEvent * e );
   virtual void dropEvent ( QDropEvent * e );
   QWidget * directChildAt(const QPoint & pos);


   EnumParameter * mPreset;
   ActionParameter * mRestore;
   ActionParameter * mStore;
   ActionParameter * mNew;
   ActionParameter * mDelete;
   bool mEditable,mHavePresets,mIsNameChange;
   QPushButton *mRestoreButton,*mStoreButton,*mNewButton,*mDeleteButton,*mRemovePanelButton;
   QComboBox *mPresetCombo;
   QLineEdit *mNameEdit;
   QHBox *topBox;
   QVBox *bottomBox;
   public:
   ParameterPanel(QWidget * parent = 0, const char * name=0, bool editable=false
                                                           , bool havePresets=false
                                                           , int id=-1);
   ~ParameterPanel();
   void addParameter(Parameter * param, int position=-1);
   void removeParameter(Parameter * param);

   public slots:
   void parameterDestroyed(QObject *);
   void addPreset();
   void removePreset();
   void savePreset();
   void loadPreset(int);
   void loadPreset();
   void updatePresets();
   void setName(const QString &);

   protected slots:
   //the following slots are connected to the preset enum parameter
   void selectionChanged(int key);
   void itemAdded(int key);
   void itemRemoved(int key,const QString& name);
   void itemStateChanged(int key);

   //this slots are connected to the combo box
   void highlighted(const QString &);
   void textChanged(const QString &);
   
   void removeClicked();

   signals:
   void parameterAdded(Parameter * par);
   void parameterRemoved(Parameter * par);
   void removePanelButtonClicked(int id);
   void nameChanged(int,const QString &);

};

#endif
