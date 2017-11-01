/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Line3D.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class Line3D /NoDefaultCtors/
{
%Docstring
 This class represents a line. It is implemented as a single directed linked list of nodes (with related QgsPoint objects). Attention: the points inserted in a line are not deleted from Line3D*
%End

%TypeHeaderCode
#include "Line3D.h"
%End
  private:
    Line3D( const Line3D & ) ;
  private:
    Line3D &operator=( const Line3D & ) ;
%Docstring
 :rtype: Line3D
%End
  protected:

  public:
    Line3D();
    ~Line3D();
    bool empty() const;
%Docstring
Returns true, if the Line contains no QgsPoint, otherwise false
 :rtype: bool
%End
    void insertPoint( QgsPoint *p );
%Docstring
Inserts a node behind the current position and sets the current position to this new node
%End
    void removePoint();
%Docstring
Removes the point behind the current position
%End
    QgsPoint *getPoint() const;
%Docstring
Gets the point at the current position
 :rtype: QgsPoint
%End
    unsigned int getCurrent() const;
%Docstring
Returns the current position
%End
    unsigned int getSize() const;
%Docstring
Returns the size of the line (the numbero of inserted Nodes without 'head' and 'z'
%End
    void goToBegin();
%Docstring
Sets the current Node to head
%End
    void goToNext();
%Docstring
Goes to the next Node
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Line3D.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
