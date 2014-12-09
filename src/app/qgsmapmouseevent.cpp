/***************************************************************************
    qgsmapmouseevent.cpp  -  mouse event in map coordinates and ability to snap
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmapmouseevent.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmapcanvas.h"


QgsMapMouseEvent::QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QMouseEvent* event , bool doSnap )
    :  QMouseEvent( event->type(), event->pos(), event->globalPos(), event->button(), event->buttons(), event->modifiers() )
    , mMapPoint( mapTool->canvas()->mapSettings().mapToPixel().toMapCoordinates( event->pos() ) )
    , mSnapped( false )
    , mMapTool( mapTool )
    , mSnapResults()
{
  mOriginalPoint = mMapPoint;

  if ( doSnap )
    snapPoint();
}

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QgsPoint point,
                                    Qt::MouseButton button, Qt::KeyboardModifiers modifiers,
                                    QEvent::Type eventType, bool doSnap )
    : QMouseEvent( eventType,
                   mapToPixelCoordinates( mapTool->canvas(), point ),
                   mapTool->canvas()->mapToGlobal( mapToPixelCoordinates( mapTool->canvas(), point ) ),
                   button, button, modifiers )
    , mMapPoint( point )
    , mOriginalPoint( point )
    , mSnapped( false )
    , mMapTool( mapTool )
    , mSnapResults()
{
  if ( doSnap )
    snapPoint();
}

void QgsMapMouseEvent::setPoint( const QgsPoint& point )
{
  mMapPoint.set( point.x(), point.y() );
}

bool QgsMapMouseEvent::snapPoint()
{
  QgsMapCanvasSnapper snapper;
  snapper.setMapCanvas( mMapTool->canvas() );
  mSnapped = false;
  if ( snapper.snapToBackgroundLayers( mMapPoint, mSnapResults ) == 0 )
  {
    mSnapped = !mSnapResults.isEmpty();
  }
  if ( mSnapped )
  {
    mMapPoint = mSnapResults.constBegin()->snappedVertex;
  }
  else
  {
    mMapPoint = mOriginalPoint;
  }
  return mSnapped;
}

QPoint QgsMapMouseEvent::mapToPixelCoordinates( QgsMapCanvas* canvas, const QgsPoint& point )
{
  double x = point.x();
  double y = point.y();

  canvas->mapSettings().mapToPixel().transformInPlace( x, y );

  return QPoint( qRound( x ), qRound( y ) );
}

QgsPoint QgsMapMouseEvent::mapPoint( bool* snappedPoint ) const
{
  if ( snappedPoint )
  {
    *snappedPoint = mSnapped && mSnapResults.constBegin()->snappedVertexNr != -1;
  }
  return mMapPoint;
}

QList<QgsPoint> QgsMapMouseEvent::snappedSegment( bool* snapped ) const
{
  QList<QgsPoint> segment =  QList<QgsPoint>();
  if ( mSnapped )
  {
    foreach ( const QgsSnappingResult result, mSnapResults )
    {
      if ( result.beforeVertexNr != -1 && result.afterVertexNr != -1 )
      {
        segment << result.beforeVertex << result.afterVertex;
        break;
      }
    }
  }

  if ( snapped )
  {
    *snapped = segment.count() == 2;
  }

  return segment;
}


