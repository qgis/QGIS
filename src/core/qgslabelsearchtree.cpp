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
#include "labelposition.h"

bool searchCallback( QgsLabelPosition *pos, void *context )
{
  QList<QgsLabelPosition *> *list = static_cast< QList<QgsLabelPosition *>* >( context );
  list->push_back( pos );
  return true;
}

QgsLabelSearchTree::~QgsLabelSearchTree()
{
  clear();
}

void QgsLabelSearchTree::label( const QgsPointXY &point, QList<QgsLabelPosition *> &posList ) const
{
  QgsPointXY p( point );
  double c_min[2];
  c_min[0] = p.x() - 0.1;
  c_min[1] = p.y() - 0.1;
  double c_max[2];
  c_max[0] = p.x() + 0.1;
  c_max[1] = p.y() + 0.1;

  QList<QgsLabelPosition *> searchResults;
  mSpatialIndex.Search( c_min, c_max, searchCallback, &searchResults );

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

void QgsLabelSearchTree::labelsInRect( const QgsRectangle &r, QList<QgsLabelPosition *> &posList ) const
{
  double c_min[2];
  c_min[0] = r.xMinimum();
  c_min[1] = r.yMinimum();
  double c_max[2];
  c_max[0] = r.xMaximum();
  c_max[1] = r.yMaximum();

  QList<QgsLabelPosition *> searchResults;
  mSpatialIndex.Search( c_min, c_max, searchCallback, &searchResults );

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

bool QgsLabelSearchTree::insertLabel( pal::LabelPosition *labelPos, QgsFeatureId featureId, const QString &layerName, const QString &labeltext, const QFont &labelfont, bool diagram, bool pinned, const QString &providerId, bool isUnplaced )
{
  if ( !labelPos )
  {
    return false;
  }

  QVector<QgsPointXY> cornerPoints;
  cornerPoints.reserve( 4 );
  double xMin = std::numeric_limits< double >::max();
  double yMin = std::numeric_limits< double >::max();
  double xMax = std::numeric_limits< double >::lowest();
  double yMax = std::numeric_limits< double >::lowest();
  for ( int i = 0; i < 4; ++i )
  {
    // we have to transform the bounding box to convert pre-rotated label positions back to real world locations
    QPointF res = mTransform.map( QPointF( labelPos->getX( i ), labelPos->getY( i ) ) );
    cornerPoints.push_back( QgsPointXY( res ) );
    xMin = std::min( xMin, res.x() );
    xMax = std::max( xMax, res.x() );
    yMin = std::min( yMin, res.y() );
    yMax = std::max( yMax, res.y() );
  }
  double c_min[2];
  double c_max[2];
  c_min[0] = xMin;
  c_min[1] = yMin;
  c_max[0] = xMax;
  c_max[1] = yMax;

  QgsGeometry labelGeometry( QgsGeometry::fromPolygonXY( QVector<QgsPolylineXY>() << cornerPoints ) );
  std::unique_ptr< QgsLabelPosition > newEntry = qgis::make_unique< QgsLabelPosition >( featureId, labelPos->getAlpha() + mMapSettings.rotation(), cornerPoints, QgsRectangle( c_min[0], c_min[1], c_max[0], c_max[1] ),
      labelPos->getWidth(), labelPos->getHeight(), layerName, labeltext, labelfont, labelPos->getUpsideDown(), diagram, pinned, providerId, labelGeometry, isUnplaced );
  mSpatialIndex.Insert( c_min, c_max, newEntry.get() );
  mOwnedPositions.emplace_back( std::move( newEntry ) );

  if ( pal::LabelPosition *next = labelPos->getNextPart() )
  {
    return insertLabel( next, featureId, layerName, labeltext, labelfont, diagram, pinned, providerId, isUnplaced );
  }
  return true;
}

void QgsLabelSearchTree::setMapSettings( const QgsMapSettings &settings )
{
  mMapSettings = settings;

  if ( !qgsDoubleNear( mMapSettings.rotation(), 0.0 ) )
  {
    // build a transform to convert points from real world to pre-rotated label positions
    const QgsPointXY center = mMapSettings.visibleExtent().center();
    mTransform = QTransform::fromTranslate( center.x(), center.y() );
    mTransform.rotate( mMapSettings.rotation() );
    mTransform.translate( -center.x(), -center.y() );
  }
  else
  {
    mTransform = QTransform();
  }
}

void QgsLabelSearchTree::clear()
{
  mSpatialIndex.RemoveAll();

  //PAL rtree iterator is buggy and doesn't iterate over all items, so we can't iterate through the tree to delete positions
  mOwnedPositions.clear();
}
