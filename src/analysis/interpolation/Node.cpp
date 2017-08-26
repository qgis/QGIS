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

Node::Node( const Node &n )
{
  if ( n.getPoint() )
  {
    QgsPoint *point = new QgsPoint( n.getPoint()->x(), n.getPoint()->y(), n.getPoint()->z() );
    mPoint = point;
  }
  else
  {
    mPoint = nullptr;
  }

  mNext = n.getNext();
}


Node &Node::operator=( const Node &n )
{
  QgsPoint *tmp = mPoint;

  if ( n.getPoint() )//mPoint of n is not a null pointer
  {
    mPoint = new QgsPoint( n.getPoint()->x(), n.getPoint()->y(), n.getPoint()->z() );
    if ( !mPoint )//no memory
    {
      mPoint = tmp;
      mNext = n.getNext();
      return ( *this );
    }

  }
  else//mPoint of n is a nullptr
  {
    mPoint = nullptr;
  }

  delete tmp;


  mNext = n.getNext();
  return ( *this );
}


