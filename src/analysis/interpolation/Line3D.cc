/***************************************************************************
                          Line3D.cc  -  description
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

#include "Line3D.h"

Line3D::Line3D()
{
  head = new Node;
  z = new Node;
  head->setNext( z );
  z->setNext( z );
  currentNode = head;
  size = 0;
  current = 0;
}

Line3D::~Line3D()
{
  //First remove all the content
  goToBegin();
  unsigned int s = size;
  for ( unsigned int i = 1;i <= s;i++ )
  {
    removePoint();
  }
  //then remove head and z
  delete head;
  delete z;
}

bool Line3D::empty() const
{
  return ( head->getNext() == z );
}

void Line3D::insertPoint( Point3D* p )
{
  if ( currentNode != z )//we can't insert a node behind the z-node
  {
    //create a new Node after the current Node
    Node* thenode = new Node();
    thenode->setPoint( p );
    thenode->setNext( currentNode->getNext() );
    currentNode->setNext( thenode );
    size += 1;
    //set the current Node to the inserted node
    currentNode = currentNode->getNext();
    current += 1;
  }
}

void Line3D::removePoint()
{
  Node* x = currentNode->getNext();
  if ( x != z )//don't remove the end node of the list
  {
    currentNode->setNext( x->getNext() );
    delete x;
  }
  size -= 1;
}

void Line3D::goToBegin()
{
  currentNode = head;
  current = 0;
}

void Line3D::goToNext()
{
  if ( current < size )
  {
    currentNode = currentNode->getNext();
    current += 1;
  }
}
