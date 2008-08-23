/***************************************************************************
                          Node.cc  -  description
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

#include "Node.h"

Node::Node( const Node& n )
{
  if ( n.getPoint() )
  {
    Point3D* point = new Point3D( n.getPoint()->getX(), n.getPoint()->getY(), n.getPoint()->getZ() );
    mPoint = point;
  }
  else
  {
    mPoint = 0;
  }

  mNext = n.getNext();
}


Node& Node::operator=( const Node & n )
{
  Point3D* tmp = mPoint;

  if ( n.getPoint() )//mPoint of n is not a null pointer
  {
    mPoint = new Point3D( n.getPoint()->getX(), n.getPoint()->getY(), n.getPoint()->getZ() );
    if ( mPoint == 0 )//no memory
    {
      mPoint = tmp;
      mNext = n.getNext();
      return ( *this );
    }

  }
  else//mPoint of n is a null pointer
  {
    mPoint = 0;
  }

  if ( tmp )
  {
    delete tmp;
  }

  mNext = n.getNext();
  return ( *this );
}


