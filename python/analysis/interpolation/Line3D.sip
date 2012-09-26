class Line3D /NoDefaultCtors/
{
%TypeHeaderCode
#include <Line3D.h>
%End
  public:
    Line3D();
    ~Line3D();
    /**returns true, if the Line contains no Point3D, otherwise false*/
    bool empty() const;
    /**inserts a node behind the current position and sets the current position to this new node*/
    void insertPoint( Point3D* p );
    /**removes the point behind the current position*/
    void removePoint();
    /**gets the point at the current position*/
    Point3D* getPoint() const;
    /**returns the current position*/
    unsigned int getCurrent() const;
    /**returns the size of the line (the numbero of inserted Nodes without 'head' and 'z'*/
    unsigned int getSize() const;
    /**sets the current Node to head*/
    void goToBegin();
    /**goes to the next Node*/
    void goToNext();
};
