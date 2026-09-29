/***************************************************************************
    qgslabelsearchtree.cpp
    ---------------------
    begin                : November 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslabelsearchtree.h"

QgsLabelSearchTree::QgsLabelSearchTree() = default;

QgsLabelSearchTree::~QgsLabelSearchTree() = default;

void QgsLabelSearchTree::label( const QgsPointXY &point, QList<QgsLabelPosition *> &posList ) const
{
  const QgsPointXY p( point );

  QList<QgsLabelPosition *> searchResults;
  mSpatialIndex.intersects( QgsRectangle( p.x() - 0.1, p.y() - 0.1, p.x() + 0.1, p.y() + 0.1 ), [&searchResults]( const QgsLabelPosition *pos ) -> bool {
    searchResults.push_back( const_cast< QgsLabelPosition * >( pos ) );
    return true;
  } );

  //tolerance +-0.1 could be high in case of degree crs, so check if p is really contained in the results
  posList.clear();
  QList<QgsLabelPosition *>::const_iterator resultIt = searchResults.constBegin();
  for ( ; resultIt != searchResults.constEnd(); ++resultIt )
  {
    if ( ( *resultIt )->labelGeometry.contains( &p ) )
    {
      posList.push_back( *resultIt );
    }
  }
}

QList<QgsLabelPosition> QgsLabelSearchTree::allLabels() const
{
  return {};
}

void QgsLabelSearchTree::labelsInRect( const QgsRectangle &r, QList<QgsLabelPosition *> &posList ) const
{
  QList<QgsLabelPosition *> searchResults;
  mSpatialIndex.intersects( r, [&searchResults]( const QgsLabelPosition *pos ) -> bool {
    searchResults.push_back( const_cast< QgsLabelPosition * >( pos ) );
    return true;
  } );

  posList.clear();
  QList<QgsLabelPosition *>::const_iterator resultIt = searchResults.constBegin();
  for ( ; resultIt != searchResults.constEnd(); ++resultIt )
  {
    if ( ( *resultIt )->labelGeometry.intersects( r ) )
    {
      posList.push_back( *resultIt );
    }
  }
}

QList<const QgsCalloutPosition *> QgsLabelSearchTree::calloutsInRectangle( const QgsRectangle &rectangle ) const
{
  QList<const QgsCalloutPosition *> searchResults;
  mCalloutIndex.intersects( rectangle, [&searchResults]( const QgsCalloutPosition *pos ) -> bool {
    searchResults.push_back( pos );
    return true;
  } );

  std::sort( searchResults.begin(), searchResults.end() );
  searchResults.erase( std::unique( searchResults.begin(), searchResults.end() ), searchResults.end() );

  return searchResults;
}

void QgsLabelSearchTree::setMapSettings( const QgsMapSettings & )
{}

void QgsLabelSearchTree::clear()
{}
