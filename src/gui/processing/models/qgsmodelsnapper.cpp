/***************************************************************************
                             qgsmodelsnapper.cpp
                             --------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelsnapper.h"
#include "qgssettings.h"

QgsModelSnapper::QgsModelSnapper()
{
  QgsSettings s;
  mTolerance = s.value( QStringLiteral( "LayoutDesigner/defaultSnapTolerancePixels" ), 5, QgsSettings::Gui ).toInt();
}

void QgsModelSnapper::setSnapTolerance( const int snapTolerance )
{
  mTolerance = snapTolerance;
}

void QgsModelSnapper::setSnapToGrid( bool enabled )
{
  mSnapToGrid = enabled;
}

QPointF QgsModelSnapper::snapPoint( QPointF point, double scaleFactor, bool &snapped ) const
{
  snapped = false;

  bool snappedXToGrid = false;
  bool snappedYToGrid = false;
  QPointF res = snapPointToGrid( point, scaleFactor, snappedXToGrid, snappedYToGrid );
  if ( snappedXToGrid )
  {
    snapped = true;
    point.setX( res.x() );
  }
  if ( snappedYToGrid )
  {
    snapped = true;
    point.setY( res.y() );
  }

  return point;
}

QRectF QgsModelSnapper::snapRect( const QRectF &rect, double scaleFactor, bool &snapped ) const
{
  snapped = false;
  QRectF snappedRect = rect;

  QList< double > xCoords;
  xCoords << rect.left() << rect.center().x() << rect.right();
  QList< double > yCoords;
  yCoords << rect.top() << rect.center().y() << rect.bottom();

  bool snappedXToGrid = false;
  bool snappedYToGrid = false;
  QList< QPointF > points;
  points << rect.topLeft() << rect.topRight() << rect.bottomLeft() << rect.bottomRight();
  QPointF res = snapPointsToGrid( points, scaleFactor, snappedXToGrid, snappedYToGrid );
  if ( snappedXToGrid )
  {
    snapped = true;
    snappedRect.translate( res.x(), 0 );
  }
  if ( snappedYToGrid )
  {
    snapped = true;
    snappedRect.translate( 0, res.y() );
  }

  return snappedRect;
}

QPointF QgsModelSnapper::snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX, bool &snappedY ) const
{
  QPointF delta = snapPointsToGrid( QList< QPointF >() << point, scaleFactor, snappedX, snappedY );
  return point + delta;
}

QPointF QgsModelSnapper::snapPointsToGrid( const QList<QPointF> &points, double scaleFactor, bool &snappedX, bool &snappedY ) const
{
  snappedX = false;
  snappedY = false;
  if ( !mSnapToGrid )
  {
    return QPointF( 0, 0 );
  }
#if 0
  const QgsLayoutGridSettings &grid = mLayout->gridSettings();
  if ( grid.resolution().length() <= 0 )
    return QPointF( 0, 0 );
#endif

  double deltaX = 0;
  double deltaY = 0;
  double smallestDiffX = std::numeric_limits<double>::max();
  double smallestDiffY = std::numeric_limits<double>::max();
  for ( QPointF point : points )
  {
    //snap x coordinate
    double gridRes = 10; //mLayout->convertToLayoutUnits( grid.resolution() );
    int xRatio = static_cast< int >( ( point.x() ) / gridRes + 0.5 ); //NOLINT
    int yRatio = static_cast< int >( ( point.y() ) / gridRes + 0.5 ); //NOLINT

    double xSnapped = xRatio * gridRes;
    double ySnapped = yRatio * gridRes;

    double currentDiffX = std::abs( xSnapped - point.x() );
    if ( currentDiffX < smallestDiffX )
    {
      smallestDiffX = currentDiffX;
      deltaX = xSnapped - point.x();
    }

    double currentDiffY = std::abs( ySnapped - point.y() );
    if ( currentDiffY < smallestDiffY )
    {
      smallestDiffY = currentDiffY;
      deltaY = ySnapped - point.y();
    }
  }

  //convert snap tolerance from pixels to layout units
  double alignThreshold = mTolerance / scaleFactor;

  QPointF delta( 0, 0 );
  if ( smallestDiffX <= alignThreshold )
  {
    //snap distance is inside of tolerance
    snappedX = true;
    delta.setX( deltaX );
  }
  if ( smallestDiffY <= alignThreshold )
  {
    //snap distance is inside of tolerance
    snappedY = true;
    delta.setY( deltaY );
  }

  return delta;
}
