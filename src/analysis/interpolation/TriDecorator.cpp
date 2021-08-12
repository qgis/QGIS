/***************************************************************************
                             TriDecorator.cpp
                             ----------------
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

#include "TriDecorator.h"
#include "qgslogger.h"

void TriDecorator::addLine( const QVector<QgsPoint> &points, QgsInterpolator::SourceType lineType )
{
  if ( mTIN )
  {
    mTIN->addLine( points, lineType );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

int TriDecorator::addPoint( const QgsPoint &p )
{
  if ( mTIN )
  {
    const unsigned int number = mTIN->addPoint( p );
    return number;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

void TriDecorator::performConsistencyTest()
{
  if ( mTIN )
  {
    mTIN->performConsistencyTest();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

bool TriDecorator::calcNormal( double x, double y, QgsPoint &result )
{
  if ( mTIN )
  {
    const bool b = mTIN->calcNormal( x, y, result );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool TriDecorator::calcPoint( double x, double y, QgsPoint &result )
{
  if ( mTIN )
  {
    const bool b = mTIN->calcPoint( x, y, result );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

QgsPoint *TriDecorator::point( int i ) const
{
  if ( mTIN )
  {
    QgsPoint *p = mTIN->point( i );
    return p;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return nullptr;
  }
}

bool TriDecorator::triangleVertices( double x, double y, QgsPoint &p1, int &n1, QgsPoint &p2, int &n2, QgsPoint &p3, int &n3 )
{
  if ( mTIN )
  {
    const bool b = mTIN->triangleVertices( x, y, p1, n1, p2, n2, p3, n3 );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool TriDecorator::triangleVertices( double x, double y, QgsPoint &p1, QgsPoint &p2, QgsPoint &p3 )
{
  if ( mTIN )
  {
    const bool b = mTIN->triangleVertices( x, y, p1, p2, p3 );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

int TriDecorator::pointsCount() const
{
  if ( mTIN )
  {
    return ( mTIN->pointsCount() );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

int TriDecorator::oppositePoint( int p1, int p2 )
{
  if ( mTIN )
  {
    const int i = mTIN->oppositePoint( p1, p2 );
    return i;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

QList<int> TriDecorator::surroundingTriangles( int pointno )
{
  if ( mTIN )
  {
    return mTIN->surroundingTriangles( pointno );
  }
  else
  {
    return QList< int >();
  }
}

double TriDecorator::xMax() const
{
  if ( mTIN )
  {
    const double d = mTIN->xMax();
    return d;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

double TriDecorator::xMin() const
{
  if ( mTIN )
  {
    const double d = mTIN->xMin();
    return d;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}
double TriDecorator::yMax() const
{
  if ( mTIN )
  {
    const double d = mTIN->yMax();
    return d;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

double TriDecorator::yMin() const
{
  if ( mTIN )
  {
    const double d = mTIN->yMin();
    return d;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

void TriDecorator::setForcedCrossBehavior( QgsTriangulation::ForcedCrossBehavior b )
{
  if ( mTIN )
  {
    mTIN->setForcedCrossBehavior( b );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

void TriDecorator::setTriangleInterpolator( TriangleInterpolator *interpolator )
{
  if ( mTIN )
  {
    mTIN->setTriangleInterpolator( interpolator );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

void TriDecorator::eliminateHorizontalTriangles()
{
  if ( mTIN )
  {
    mTIN->eliminateHorizontalTriangles();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

void TriDecorator::ruppertRefinement()
{
  if ( mTIN )
  {
    mTIN->ruppertRefinement();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

bool TriDecorator::pointInside( double x, double y )
{
  if ( mTIN )
  {
    const bool b = mTIN->pointInside( x, y );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool TriDecorator::swapEdge( double x, double y )
{
  if ( mTIN )
  {
    const bool b = mTIN->swapEdge( x, y );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

QList<int> TriDecorator::pointsAroundEdge( double x, double y )
{
  if ( mTIN )
  {
    return mTIN->pointsAroundEdge( x, y );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return QList<int>();
  }
}
