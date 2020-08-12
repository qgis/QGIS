/***************************************************************************
                             ParametricLine.cpp
                             ------------------
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
#include "qgslogger.h"

void ParametricLine::add( ParametricLine *pl )
{
  Q_UNUSED( pl )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

void ParametricLine::calcFirstDer( float t, Vector3D *v )
{
  Q_UNUSED( t )
  Q_UNUSED( v )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

void ParametricLine::calcSecDer( float t, Vector3D *v )
{
  Q_UNUSED( t )
  Q_UNUSED( v )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

void ParametricLine::calcPoint( float t, QgsPoint *p )
{
  Q_UNUSED( t )
  Q_UNUSED( p )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

ParametricLine *ParametricLine::getParent() const
{
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
  return nullptr;
}

void ParametricLine::remove( int i )
{
  Q_UNUSED( i )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

void ParametricLine::setControlPoly( QVector<QgsPoint *> *cp )
{
  Q_UNUSED( cp )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

void ParametricLine::setParent( ParametricLine *paral )
{
  Q_UNUSED( paral )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
}

int ParametricLine::getDegree() const
{
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
  return mDegree;
}

const QgsPoint *ParametricLine::getControlPoint( int number ) const
{
  Q_UNUSED( number )
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
  return nullptr;
}

const QVector<QgsPoint *> *ParametricLine::getControlPoly() const
{
  QgsDebugMsg( QStringLiteral( "warning, derive a class from ParametricLine" ) );
  return nullptr;
}
