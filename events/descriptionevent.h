#ifndef DESCRIPTIONEVENT_H
#define DESCRIPTIONEVENT_H

#include <string>
#include "subject.h"

class DescriptionEvent{
  private:
    change_t mType;
    string mValue;
    subject<DescriptionEvent> * src;
    
  public:
    enum change_t{NAME_CHANGED,DESCRIPTION_CHANGED};
    
    change_t type() const;
    const char * value() const;
    subject<DescriptionEvent> * source() const;

    DescriptionEvent(subject<DescriptionEvent>* src,
                     const change_t type=NAME_CHANGED,
                     const char * val);
};
#endif
