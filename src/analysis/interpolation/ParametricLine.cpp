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
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

void ParametricLine::calcFirstDer( float t, Vector3D *v )
{
  Q_UNUSED( t )
  Q_UNUSED( v )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

void ParametricLine::calcSecDer( float t, Vector3D *v )
{
  Q_UNUSED( t )
  Q_UNUSED( v )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

void ParametricLine::calcPoint( float t, QgsPoint *p )
{
  Q_UNUSED( t )
  Q_UNUSED( p )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

ParametricLine *ParametricLine::getParent() const
{
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
  return nullptr;
}

void ParametricLine::remove( int i )
{
  Q_UNUSED( i )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

void ParametricLine::setControlPoly( QVector<QgsPoint *> *cp )
{
  Q_UNUSED( cp )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

void ParametricLine::setParent( ParametricLine *paral )
{
  Q_UNUSED( paral )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
}

int ParametricLine::getDegree() const
{
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
  return mDegree;
}

const QgsPoint *ParametricLine::getControlPoint( int number ) const
{
  Q_UNUSED( number )
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
  return nullptr;
}

const QVector<QgsPoint *> *ParametricLine::getControlPoly() const
{
  QgsDebugError( u"warning, derive a class from ParametricLine"_s );
  return nullptr;
}
