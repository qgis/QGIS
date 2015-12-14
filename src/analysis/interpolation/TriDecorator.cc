/***************************************************************************
                          TriDecorator.cc  -  description
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

#include "TriDecorator.h"
#include "qgslogger.h"

void TriDecorator::addLine( Line3D* line, bool breakline )
{
  if ( mTIN )
  {
    mTIN->addLine( line, breakline );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}

int TriDecorator::addPoint( Point3D* p )
{
  if ( mTIN )
  {
    unsigned int number = mTIN->addPoint( p );
    return number;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
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
    QgsDebugMsg( "warning, null pointer" );
  }
}

bool TriDecorator::calcNormal( double x, double y, Vector3D* result )
{
  if ( mTIN )
  {
    bool b = mTIN->calcNormal( x, y, result );
    return b;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool TriDecorator::calcPoint( double x, double y, Point3D* result )
{
  if ( mTIN )
  {
    bool b = mTIN->calcPoint( x, y, result );
    return b;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

Point3D* TriDecorator::getPoint( unsigned int i ) const
{
  if ( mTIN )
  {
    Point3D* p = mTIN->getPoint( i );
    return p;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return nullptr;
  }
}

bool TriDecorator::getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 )
{
  if ( mTIN )
  {
    bool b = mTIN->getTriangle( x, y, p1, n1, p2, n2, p3, n3 );
    return b;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool TriDecorator::getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 )
{
  if ( mTIN )
  {
    bool b = mTIN->getTriangle( x, y, p1, p2, p3 );
    return b;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

int TriDecorator::getNumberOfPoints() const
{
  if ( mTIN )
  {
    return ( mTIN->getNumberOfPoints() );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

int TriDecorator::getOppositePoint( int p1, int p2 )
{
  if ( mTIN )
  {
    int i = mTIN->getOppositePoint( p1, p2 );
    return i;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

QList<int>* TriDecorator::getSurroundingTriangles( int pointno )
{
  if ( mTIN )
  {
    QList<int>* vl = mTIN->getSurroundingTriangles( pointno );
    return vl;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return nullptr;
  }
}

double TriDecorator::getXMax() const
{
  if ( mTIN )
  {
    double d = mTIN->getXMax();
    return d;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

double TriDecorator::getXMin() const
{
  if ( mTIN )
  {
    double d = mTIN->getXMin();
    return d;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}
double TriDecorator::getYMax() const
{
  if ( mTIN )
  {
    double d = mTIN->getYMax();
    return d;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

double TriDecorator::getYMin() const
{
  if ( mTIN )
  {
    double d = mTIN->getYMin();
    return d;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

void TriDecorator::setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b )
{
  if ( mTIN )
  {
    mTIN->setForcedCrossBehaviour( b );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}

void TriDecorator::setEdgeColor( int r, int g, int b )
{
  if ( mTIN )
  {
    mTIN->setEdgeColor( r, g, b );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}

void TriDecorator::setForcedEdgeColor( int r, int g, int b )
{
  if ( mTIN )
  {
    mTIN->setForcedEdgeColor( r, g, b );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}

void TriDecorator::setBreakEdgeColor( int r, int g, int b )
{
  if ( mTIN )
  {
    mTIN->setBreakEdgeColor( r, g, b );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}

void TriDecorator::setTriangleInterpolator( TriangleInterpolator* interpolator )
{
  if ( mTIN )
  {
    mTIN->setTriangleInterpolator( interpolator );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
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
    QgsDebugMsg( "warning, null pointer" );
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
    QgsDebugMsg( "warning, null pointer" );
  }
}

bool TriDecorator::pointInside( double x, double y )
{
  if ( mTIN )
  {
    bool b = mTIN->pointInside( x, y );
    return b;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool TriDecorator::swapEdge( double x, double y )
{
  if ( mTIN )
  {
    bool b = mTIN->swapEdge( x, y );
    return b;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

QList<int>* TriDecorator::getPointsAroundEdge( double x, double y )
{
  if ( mTIN )
  {
    return mTIN->getPointsAroundEdge( x, y );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return nullptr;
  }
}
