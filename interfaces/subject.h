#ifndef SUBJECT_H
#define SUBJECT_H
//forward declaration
template <class Event_T> class subject;

#include <set>

#include "observer.h"

template <class Event_T> class subject{
  private:
    set<observer<Event_T> * > mObservers;
    string debug;  
  protected:
    void setDebug(const char * d){debug = d;};
  public:
    string getDebug(){return debug;};
    void notify(const Event_T & e) const{
      //cout<<"subject("<<debug<<"): notify called"<<endl;
      for(set<observer<Event_T> * >::iterator it=mObservers.begin();
          it!=mObservers.end();
          ++it){
        (*it)->onNotify(e);
      }
    };
    
    void attach(observer<Event_T> * o){
      //cout<<"subject("<<debug<<"): attach called. "<<o->getDebug()<<endl;
      //cout<<debug<<" o.c. before: "<<mObservers.size()<<endl;
      //we have to insert a break condition to avoid endless mutual recursion
      if(mObservers.find(o)!=mObservers.end()) return;
  
      //this has to come FIRST! see above.
      mObservers.insert(o);
  
      //ensure symmetrie of this connection.
      o->attach(this);
      //cout<<debug<<" o.c. after: "<<mObservers.size()<<endl;

    };
    
    void detach(observer<Event_T> * o){
      //cout<<"subject("<<debug<<"): detach called. "<<o->getDebug()<<endl;
      //cout<<debug<<" o.c. before: "<<mObservers.size()<<endl;
      //same as above, break recursion right here:
      if(mObservers.find(o)==mObservers.end()) return;
      mObservers.erase(o);
      o->detach(this);
      //cout<<debug<<" o.c. after: "<<mObservers.size()<<endl;
    };
    virtual ~subject(){
      //important to detach from all observers!
      //cout<<"subject("<<debug<<"): destructor called."<<endl;
      //cout<<debug<<" o.c. before: "<<mObservers.size()<<endl;
      set<observer<Event_T> * >::iterator it=mObservers.begin();
      while(it!=mObservers.end()){
        detach(*it);
        it=mObservers.begin();
      }
      //cout<<debug<<" o.c. after: "<<mObservers.size()<<endl;
    };
};
#endif
