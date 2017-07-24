/***************************************************************************
                             qgslayoutsnapper.cpp
                             --------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutsnapper.h"
#include "qgslayout.h"

QgsLayoutSnapper::QgsLayoutSnapper( QgsLayout *layout )
  : mLayout( layout )
  , mGridResolution( QgsLayoutMeasurement( 10 ) )
{
  mGridPen = QPen( QColor( 190, 190, 190, 100 ), 0 );
  mGridPen.setCosmetic( true );
}

QPointF QgsLayoutSnapper::snapPoint( QPointF point, double scaleFactor, bool &snapped ) const
{
  snapped = false;

  // highest priority - grid
  bool snappedToGrid = false;
  QPointF res = snapPointToGrid( point, scaleFactor, snappedToGrid );
  if ( snappedToGrid )
  {
    snapped = true;
    return res;
  }

  return point;
}

QPointF QgsLayoutSnapper::snapPointToGrid( QPointF point, double scaleFactor, bool &snapped ) const
{
  snapped = false;
  if ( !mLayout || !mSnapToGrid || mGridResolution.length() <= 0 )
  {
    return point;
  }

  //calculate y offset to current page
  QPointF pagePoint = mLayout->pageCollection()->positionOnPage( point );

  double yPage = pagePoint.y(); //y-coordinate relative to current page
  double yAtTopOfPage = mLayout->pageCollection()->page( mLayout->pageCollection()->pageNumberForPoint( point ) )->pos().y();

  //snap x coordinate
  double gridRes = mLayout->convertToLayoutUnits( mGridResolution );
  QPointF gridOffset = mLayout->convertToLayoutUnits( mGridOffset );
  int xRatio = static_cast< int >( ( point.x() - gridOffset.x() ) / gridRes + 0.5 ); //NOLINT
  int yRatio = static_cast< int >( ( yPage - gridOffset.y() ) / gridRes + 0.5 ); //NOLINT

  double xSnapped = xRatio * gridRes + gridOffset.x();
  double ySnapped = yRatio * gridRes + gridOffset.y() + yAtTopOfPage;

  //convert snap tolerance from pixels to layout units
  double alignThreshold = mTolerance / scaleFactor;

  if ( fabs( xSnapped - point.x() ) > alignThreshold )
  {
    //snap distance is outside of tolerance
    xSnapped = point.x();
  }
  else
  {
    snapped = true;
  }
  if ( fabs( ySnapped - point.y() ) > alignThreshold )
  {
    //snap distance is outside of tolerance
    ySnapped = point.y();
  }
  else
  {
    snapped = true;
  }

  return QPointF( xSnapped, ySnapped );
}
