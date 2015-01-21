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

#include "qgisapp.h"
#include "qgssnappingutils.h"

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QMouseEvent* event , bool doSnap )
    :  QMouseEvent( event->type(), event->pos(), event->globalPos(), event->button(), event->buttons(), event->modifiers() )
    , mMapPoint( mapTool->canvas()->mapSettings().mapToPixel().toMapCoordinates( event->pos() ) )
    , mMapTool( mapTool )
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
    , mMapTool( mapTool )
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
  mSnapMatch = mMapTool->canvas()->snappingUtils()->snapToMap( mMapPoint );
  mMapPoint = mSnapMatch.isValid() ? mSnapMatch.point() : mOriginalPoint;
  return mSnapMatch.isValid();
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
    *snappedPoint = mSnapMatch.isValid();
  }
  return mMapPoint;
}

struct EdgesOnlyFilter : public QgsPointLocator::MatchFilter
{
  bool acceptMatch( const QgsPointLocator::Match& m ) { return m.hasEdge(); }
};

QList<QgsPoint> QgsMapMouseEvent::snappedSegment( bool* snapped ) const
{
  QList<QgsPoint> segment =  QList<QgsPoint>();
  if ( mSnapMatch.hasEdge() )
  {
    QgsPoint pt1, pt2;
    mSnapMatch.edgePoints( pt1, pt2 );
    segment << pt1 << pt2;
  }
  else
  {
    // run snapToMap with only segments
    EdgesOnlyFilter filter;
    mMapTool->canvas()->snappingUtils()->snapToMap( mMapPoint, &filter );
  }

  if ( snapped )
  {
    *snapped = segment.count() == 2;
  }

  return segment;
}


