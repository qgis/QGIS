/***************************************************************************
                          Node.h  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NODE_H
#define NODE_H

#include "Point3D.h"

/** Node is a class used by Line3D. It represents a node in the single directed linked list. Associated Point3D objects are deleted when the node is deleted.*/
class ANALYSIS_EXPORT Node
{
  protected:
    /** Pointer to the Point3D object associated with the node*/
    Point3D* mPoint;
    /** Pointer to the next Node in the linked list*/
    Node* mNext;
  public:
    Node();
    Node( const Node& n );
    ~Node();
    Node& operator=( const Node& n );
    /** Returns a pointer to the next element in the linked list*/
    Node* getNext() const;
    /** Returns a pointer to the Point3D object associated with the node*/
    Point3D* getPoint() const;
    /** Sets the pointer to the next node*/
    void setNext( Node* n );
    /** Sets a new pointer to an associated Point3D object*/
    void setPoint( Point3D* p );
};

inline Node::Node() : mPoint( 0 ), mNext( 0 )
{

}

inline Node::~Node()
{
  delete mPoint;
}

inline Node* Node::getNext() const
{
  return mNext;
}

inline Point3D* Node::getPoint() const
{
  return mPoint;
}

inline void Node::setNext( Node* n )
{
  mNext = n;
}

inline void Node::setPoint( Point3D* p )
{
  mPoint = p;
}

#endif
