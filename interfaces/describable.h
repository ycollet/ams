#ifndef DESCRIBABLE_H
#define DESCRIBABLE_H

/*! @brief an objsect that has a queriable name and description
 *
 * This stores name and description per instance, and supports methods
 * for changing each.
 *
 *
 */
class Describable{
  public:
    virtual const char * name() =0;
    virtual const char * description() =0;
    virtual void setName(const char * nm)=0;
    virtual void setDescription(const char *ds)=0;
};

#endif
