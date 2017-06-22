/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Node.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class Node
{
%Docstring
 Node is a class used by Line3D. It represents a node in the single directed linked list. Associated QgsPoint objects are deleted when the node is deleted.*
%End

%TypeHeaderCode
#include "Node.h"
%End
  protected:
  public:
    Node();
    Node( const Node &n );
    ~Node();
    Node *getNext() const;
%Docstring
Returns a pointer to the next element in the linked list
 :rtype: Node
%End
    QgsPoint *getPoint() const;
%Docstring
Returns a pointer to the QgsPoint object associated with the node
 :rtype: QgsPoint
%End
    void setNext( Node *n );
%Docstring
Sets the pointer to the next node
%End
    void setPoint( QgsPoint *p );
%Docstring
Sets a new pointer to an associated QgsPoint object
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Node.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
