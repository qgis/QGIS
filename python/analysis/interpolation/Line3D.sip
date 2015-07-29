class Line3D /NoDefaultCtors/
{
%TypeHeaderCode
#include <Line3D.h>
%End
  public:
    Line3D();
    ~Line3D();
    /** Returns true, if the Line contains no Point3D, otherwise false*/
    bool empty() const;
    /** Inserts a node behind the current position and sets the current position to this new node*/
    void insertPoint( Point3D* p );
    /** Removes the point behind the current position*/
    void removePoint();
    /** Gets the point at the current position*/
    Point3D* getPoint() const;
    /** Returns the current position*/
    unsigned int getCurrent() const;
    /** Returns the size of the line (the numbero of inserted Nodes without 'head' and 'z'*/
    unsigned int getSize() const;
    /** Sets the current Node to head*/
    void goToBegin();
    /** Goes to the next Node*/
    void goToNext();
};
