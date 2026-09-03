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

#include "feature.h"
#include "labelposition.h"
#include "qgslabelsearchtree.h"
#include "qgslinestring.h"
#include "qgsmapsettings.h"
#include "qgspolygon.h"

QgsLabelingResults::QgsLabelingResults( bool enableSearchTree )
  : mLabelSearchTree( enableSearchTree ? std::make_unique< QgsLabelSearchTree >() : nullptr )
{}

QgsLabelingResults::~QgsLabelingResults() = default;

bool QgsLabelingResults::hasSearchTree() const
{
  return static_cast< bool >( mLabelSearchTree );
}

bool QgsLabelingResults::insertLabel(
  pal::LabelPosition *labelPos,
  QgsFeatureId featureId,
  const QString &layerName,
  const QString &labeltext,
  const QFont &labelfont,
  bool diagram,
  bool pinned,
  const QString &providerId,
  bool isUnplaced,
  long long linkedId
)
{
  if ( !labelPos )
  {
    return false;
  }

  QVector<QgsPointXY> cornerPoints;
  QVector< double > cornerPointsX;
  QVector< double > cornerPointsY;
  cornerPoints.resize( 4 );
  cornerPointsX.resize( 5 );
  cornerPointsY.resize( 5 );
  double xMin = std::numeric_limits< double >::max();
  double yMin = std::numeric_limits< double >::max();
  double xMax = std::numeric_limits< double >::lowest();
  double yMax = std::numeric_limits< double >::lowest();
  for ( int i = 0; i < 4; ++i )
  {
    // we have to transform the bounding box to convert pre-rotated label positions back to real world locations
    const QPointF res = mTransform.map( QPointF( labelPos->getX( i ), labelPos->getY( i ) ) );
    cornerPoints[i] = QgsPointXY( res );
    cornerPointsX[i] = res.x();
    cornerPointsY[i] = res.y();
    xMin = std::min( xMin, res.x() );
    xMax = std::max( xMax, res.x() );
    yMin = std::min( yMin, res.y() );
    yMax = std::max( yMax, res.y() );
  }
  cornerPointsX[4] = cornerPointsX[0];
  cornerPointsY[4] = cornerPointsY[0];

  pal::LabelPosition *next = labelPos->nextPart();
  long long uniqueLinkedId = 0;
  if ( linkedId != 0 )
    uniqueLinkedId = linkedId;
  else if ( next )
    uniqueLinkedId = mNextFeatureId++;

  const QgsRectangle bounds( xMin, yMin, xMax, yMax );
  const QgsGeometry labelGeometry( std::make_unique< QgsPolygon >( new QgsLineString( cornerPointsX, cornerPointsY ) ) );
  auto newEntry = std::make_unique< QgsLabelPosition >(
    featureId,
    -labelPos->getAlpha() * 180 / M_PI + mMapSettings.rotation(),
    cornerPoints,
    bounds,
    labelPos->getWidth(),
    labelPos->getHeight(),
    layerName,
    labeltext,
    labelfont,
    labelPos->getUpsideDown(),
    diagram,
    pinned,
    providerId,
    labelGeometry,
    isUnplaced
  );
  newEntry->groupedLabelId = uniqueLinkedId;

  if ( uniqueLinkedId != 0 )
  {
    mLinkedLabelHash[uniqueLinkedId].append( newEntry.get() );
  }

  if ( mLabelSearchTree )
  {
    mLabelSearchTree->mSpatialIndex.insert( newEntry.get(), bounds );
  }
  mOwnedPositions.emplace_back( std::move( newEntry ) );

  if ( next )
  {
    return insertLabel( next, featureId, layerName, labeltext, labelfont, diagram, pinned, providerId, isUnplaced, uniqueLinkedId );
  }
  return true;
}

bool QgsLabelingResults::insertCallout( const QgsCalloutPosition &position )
{
  const QPointF origin = position.origin();
  const QPointF destination = position.destination();

  auto newEntry = std::make_unique< QgsCalloutPosition >( position );

  if ( mLabelSearchTree )
  {
    mLabelSearchTree->mCalloutIndex.insert( newEntry.get(), QgsRectangle( origin.x(), origin.y(), origin.x(), origin.y() ) );
    mLabelSearchTree->mCalloutIndex.insert( newEntry.get(), QgsRectangle( destination.x(), destination.y(), destination.x(), destination.y() ) );
  }

  mOwnedCalloutPositions.emplace_back( std::move( newEntry ) );

  return true;
}

QList<QgsLabelPosition> QgsLabelingResults::allLabels() const
{
  QList<QgsLabelPosition> res;
  res.reserve( static_cast< qsizetype>( mOwnedPositions.size() ) );
  for ( const std::unique_ptr< QgsLabelPosition > &pos : mOwnedPositions )
  {
    res.append( *pos );
  }
  return res;
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
    const QList<QgsLabelPosition *> positionPointers = mLinkedLabelHash.value( groupId );
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
    const QList<const QgsCalloutPosition *> positionPointers = mLabelSearchTree->calloutsInRectangle( rectangle );
    for ( const QgsCalloutPosition *pos : positionPointers )
    {
      positions.push_back( QgsCalloutPosition( *pos ) );
    }
  }

  return positions;
}

void QgsLabelingResults::setMapSettings( const QgsMapSettings &settings )
{
  mMapSettings = settings;
  if ( !qgsDoubleNear( settings.rotation(), 0.0 ) )
  {
    // build a transform to convert points from real world to pre-rotated label positions
    const QgsPointXY center = settings.visibleExtent().center();
    mTransform = QTransform::fromTranslate( center.x(), center.y() );
    mTransform.rotate( mMapSettings.rotation() );
    mTransform.translate( -center.x(), -center.y() );
  }
  else
  {
    mTransform = QTransform();
  }
}
