/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs@list.ru                                                     *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file simplegraphbuilder.cpp
 * \brief implementation of RgSimpleGraphBuilder
 */

#include "simplegraphbuilder.h"
#include "utils.h"

// Qgis includes
#include <qgscoordinatetransform.h>
#include <qgsdistancearea.h>

#include <iostream>

RgSimpleGraphBuilder::RgSimpleGraphBuilder()
{
  mCoordinateTransform = new QgsCoordinateTransform();
  mDistanceArea = new QgsDistanceArea();
  mDistanceArea->setProjectionsEnabled( true );
}

RgSimpleGraphBuilder::~RgSimpleGraphBuilder()
{
  delete mCoordinateTransform;
  delete mDistanceArea;
}

void RgSimpleGraphBuilder::setSourceCrs( const QgsCoordinateReferenceSystem& crs )
{
  mCoordinateTransform->setSourceCrs( crs );
}

void RgSimpleGraphBuilder::setDestinationCrs( const QgsCoordinateReferenceSystem& crs )
{
  mCoordinateTransform->setDestCRS( crs );
  mDistanceArea->setSourceCrs( crs.srsid() );
}

void RgSimpleGraphBuilder::addVertex( const QgsPoint& pt )
{
  mMatrix[ mCoordinateTransform->transform( pt )];
}

void RgSimpleGraphBuilder::addArc( const QgsPoint& pt1, const QgsPoint& pt2, double speed )
{
  QgsPoint p1 = mCoordinateTransform->transform( pt1 );
  QgsPoint p2 = mCoordinateTransform->transform( pt2 );

  double distance = mDistanceArea->measureLine( p1, p2 );
  double time = distance / speed;
  mMatrix[ p1 ][ p2 ] = ArcAttributes( distance, time, 0 );
}

QgsPoint RgSimpleGraphBuilder::tiePoint( const QgsPoint &pt, bool &b )
{
  b = false;
  AdjacencyMatrix::iterator it1;
  AdjacencyMatrixString::iterator it2;

  double min = infinity();
  QgsPoint minP1, minP2;
  QgsPoint center;
  for ( it1 = mMatrix.begin(); it1 != mMatrix.end(); ++it1 )
  {
    for ( it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 )
    {
      QgsPoint c;
      double d = distance( it1->first, it2->first, pt, c );
      if ( d < min )
      {
        minP1 = it1->first;
        minP2 = it2->first;
        min = d;
        center = c;
      }
    }
  }
  if ( min >= infinity() )
    return center;

  double c = mMatrix[ minP1 ][ minP2 ].mCost;
  double t = mMatrix[ minP1 ][ minP2 ].mTime;

  double newArcLength = mDistanceArea->measureLine( minP1, center );
  mMatrix[ minP1 ][ center ] = ArcAttributes( newArcLength, t * newArcLength / c, 0 );
  newArcLength = mDistanceArea->measureLine( center, minP2 );
  mMatrix[ center ][ minP2 ] = ArcAttributes( newArcLength, t * newArcLength / c, 0 );

  if ( mMatrix[ minP2 ].find( minP1 ) != mMatrix[ minP2 ].end() )
  {
    c = mMatrix[ minP2 ][ minP1 ].mCost;
    t = mMatrix[ minP2 ][ minP1 ].mTime;
    newArcLength = mDistanceArea->measureLine( center, minP1 );
    mMatrix[ center ][ minP1 ] = ArcAttributes( newArcLength, t * newArcLength / c, 0 );
    newArcLength = mDistanceArea->measureLine( minP2, center );
    mMatrix[ minP2 ][ center ] = ArcAttributes( newArcLength, t * newArcLength / c, 0 );
  }

  mMatrix[minP1].erase( minP2 );
  mMatrix[minP2].erase( minP1 );
  b = true;
  return center;
}

AdjacencyMatrix RgSimpleGraphBuilder::adjacencyMatrix()
{
  return mMatrix;
}
