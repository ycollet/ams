#ifndef OBSERVER_H
#define OBSERVER_H
//forward declaration
template <class Event_T> class observer;

#include <set>

#include "subject.h"

template <class Event_T> class observer{
  private:
    set<subject<Event_T> * > mSubjects;
    string debug;
  protected:
    void setDebug(const char * d){debug = d;};
  public:
    string getDebug(){return debug;};
    virtual void onNotify(const Event_T & e){
      //cout<<"observer("<<debug<<"): onNotify called."<<endl;
    };
    
    void attach(subject<Event_T> * s){
      //we have to insert a break condition to avoid endless mutual recursion
      //cout<<"observer("<<debug<<"): attach called. "<<s->getDebug()<<endl;
      //cout<<debug<<" s.c. before: "<<mSubjects.size()<<endl;

      if(mSubjects.find(s)!=mSubjects.end()) return;
  
      //this has to come FIRST! see above.  mSubjects.insert(s);
      mSubjects.insert(s);
      //ensure symmetrie of this connection.
      s->attach(this);
      //cout<<debug<<" s.c. after: "<<mSubjects.size()<<endl;
    };
    
    void detach(subject<Event_T> * s){
      //same as above, break recursion right here:
      //cout<<"observer("<<debug<<"): detach called. "<<s->getDebug()<<endl;
      //cout<<debug<<" s.c. before: "<<mSubjects.size()<<endl;

      if(mSubjects.find(s)==mSubjects.end()) return;
      mSubjects.erase(s);
      s->detach(this);
      //cout<<debug<<" s.c. after: "<<mSubjects.size()<<endl;
    };
    virtual ~observer(){
      //cout<<"observer("<<debug<<"): destructor called."<<endl;       
      //cout<<debug<<" s.c. before: "<<mSubjects.size()<<endl;
      //important to detach from all subjects!
      set<subject<Event_T> * >::iterator it=mSubjects.begin();
      while(it!=mSubjects.end()){
        detach(*it);
        it=mSubjects.begin();
      }
      //cout<<debug<<" s.c. after: "<<mSubjects.size()<<endl;
    };
};
#endif
