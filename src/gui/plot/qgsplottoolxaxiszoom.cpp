/***************************************************************************
                          qgsplottoolxaxiszoom.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#include "qgsplottoolxaxiszoom.h"
#include "moc_qgsplottoolxaxiszoom.cpp"
#include "qgsplotcanvas.h"
#include "qgselevationprofilecanvas.h"
#include <QWheelEvent>

QgsPlotToolXAxisZoom::QgsPlotToolXAxisZoom( QgsElevationProfileCanvas *canvas )
  : QgsPlotToolZoom( canvas )
  , mElevationCanvas( canvas )
{
  mToolName = tr( "Zoom X Axis" );
}

QgsPlotToolXAxisZoom::~QgsPlotToolXAxisZoom() = default;

QPointF QgsPlotToolXAxisZoom::constrainStartPoint( QPointF scenePoint ) const
{
  if ( mElevationCanvas->lockAxisScales() )
  {
    return scenePoint;
  }
  else
  {
    // force the rubber band to take the whole vertical area
    QRectF plotArea = mElevationCanvas->plotArea();
    return QPointF( scenePoint.x(), plotArea.top() );
  }
}

QPointF QgsPlotToolXAxisZoom::constrainMovePoint( QPointF scenePoint ) const
{
  const QRectF plotArea = mElevationCanvas->plotArea();
  if ( mElevationCanvas->lockAxisScales() )
  {
    const double selectedDistanceRange = std::fabs( scenePoint.x() - mRubberBandStartPos.x() );
    // ensure the same aspect ratio is retained
    const double calculatedElevationRange = plotArea.height() / plotArea.width() * selectedDistanceRange;
    return QPointF( scenePoint.x(), mRubberBandStartPos.y() + calculatedElevationRange );
  }
  else
  {
    // force the rubber band to take the whole vertical area
    return QPointF( scenePoint.x(), plotArea.bottom() );
  }
}

QRectF QgsPlotToolXAxisZoom::constrainBounds( const QRectF &sceneBounds ) const
{
  const QRectF plotArea = mElevationCanvas->plotArea();

  if ( mElevationCanvas->lockAxisScales() )
  {
    // constraint has already been applied
    return sceneBounds;
  }
  else
  {
    // retain current vertical rect
    return QRectF( sceneBounds.left(), plotArea.top(), sceneBounds.width(), plotArea.height() );
  }
}

void QgsPlotToolXAxisZoom::zoomOutClickOn( QPointF scenePoint )
{
  //just a click, so zoom to clicked point and recenter
  const QRectF plotArea = mElevationCanvas->plotArea();
  canvas()->centerPlotOn( scenePoint.x(), plotArea.center().y() );
  mElevationCanvas->scalePlot( 0.5, 1 );
}

void QgsPlotToolXAxisZoom::zoomInClickOn( QPointF scenePoint )
{
  //just a click, so zoom to clicked point and recenter
  const QRectF plotArea = mElevationCanvas->plotArea();
  canvas()->centerPlotOn( scenePoint.x(), plotArea.center().y() );
  mElevationCanvas->scalePlot( 2.0, 1 );
}
