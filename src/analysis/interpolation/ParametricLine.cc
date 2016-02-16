/***************************************************************************
                          ParametricLine.cc  -  description
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

#include "ParametricLine.h"
#include <qgslogger.h>

void ParametricLine::add( ParametricLine* pl )
{
  Q_UNUSED( pl );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

void ParametricLine::calcFirstDer( float t, Vector3D* v )
{
  Q_UNUSED( t );
  Q_UNUSED( v );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

void ParametricLine::calcSecDer( float t, Vector3D* v )
{
  Q_UNUSED( t );
  Q_UNUSED( v );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

void ParametricLine::calcPoint( float t, Point3D *p )
{
  Q_UNUSED( t );
  Q_UNUSED( p );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

ParametricLine* ParametricLine::getParent() const
{
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
  return nullptr;
}

void ParametricLine::remove( int i )
{
  Q_UNUSED( i );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

void ParametricLine::setControlPoly( QVector<Point3D*>* cp )
{
  Q_UNUSED( cp );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

void ParametricLine::setParent( ParametricLine* paral )
{
  Q_UNUSED( paral );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
}

int ParametricLine::getDegree() const
{
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
  return mDegree;
}

const Point3D* ParametricLine::getControlPoint( int number ) const
{
  Q_UNUSED( number );
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
  return nullptr;
}

const QVector<Point3D*>* ParametricLine::getControlPoly() const
{
  QgsDebugMsg( "warning, derive a class from ParametricLine" );
  return nullptr;
}
