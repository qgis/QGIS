/***************************************************************************
  qgslabelingresults.cpp
  -------------------
   begin                : February 2021
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingresults.h"
#include "qgslabelsearchtree.h"

QgsLabelingResults::QgsLabelingResults()
  : mLabelSearchTree( std::make_unique< QgsLabelSearchTree >() )
{
}

QgsLabelingResults::~QgsLabelingResults() = default;


QList<QgsLabelPosition> QgsLabelingResults::allLabels() const
{
  return mLabelSearchTree ? mLabelSearchTree->allLabels() : QList<QgsLabelPosition>();
}

QList<QgsLabelPosition> QgsLabelingResults::labelsAtPosition( const QgsPointXY &p ) const
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition *> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->label( p, positionPointers );
    QList<QgsLabelPosition *>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}

QList<QgsLabelPosition> QgsLabelingResults::labelsWithinRect( const QgsRectangle &r ) const
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition *> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->labelsInRect( r, positionPointers );
    QList<QgsLabelPosition *>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}

QList<QgsLabelPosition> QgsLabelingResults::groupedLabelPositions( long long groupId ) const
{
  QList<QgsLabelPosition> positions;
  if ( mLabelSearchTree )
  {
    const QList<QgsLabelPosition *> positionPointers = mLabelSearchTree->groupedLabelPositions( groupId );
    positions.reserve( positionPointers.size() );
    for ( const QgsLabelPosition *pos : positionPointers )
      positions.push_back( QgsLabelPosition( *pos ) );
  }
  return positions;
}

QList<QgsCalloutPosition> QgsLabelingResults::calloutsWithinRectangle( const QgsRectangle &rectangle ) const
{
  QList<QgsCalloutPosition> positions;

  if ( mLabelSearchTree )
  {
    const QList<const QgsCalloutPosition *>positionPointers  = mLabelSearchTree->calloutsInRectangle( rectangle );
    for ( const QgsCalloutPosition *pos : positionPointers )
    {
      positions.push_back( QgsCalloutPosition( *pos ) );
    }
  }

  return positions;
}

void QgsLabelingResults::setMapSettings( const QgsMapSettings &settings )
{
  mLabelSearchTree->setMapSettings( settings );
}
