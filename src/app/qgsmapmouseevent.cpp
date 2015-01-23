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

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QMouseEvent* event , SnappingMode mode )
    :  QMouseEvent( event->type(), event->pos(), event->globalPos(), event->button(), event->buttons(), event->modifiers() )
    , mMapPoint( mapTool->canvas()->mapSettings().mapToPixel().toMapCoordinates( event->pos() ) )
    , mMapTool( mapTool )
    , mSnapMatch( QgsPointLocator::Match() )
    , mSnappingMode( mode )
{
  mOriginalPoint = mMapPoint;
  snapPoint();
}

struct VertexOnlyFilter : public QgsPointLocator::MatchFilter
{
  bool acceptMatch( const QgsPointLocator::Match& m ) override { return m.hasVertex(); }
};

struct EdgesOnlyFilter : public QgsPointLocator::MatchFilter
{
  bool acceptMatch( const QgsPointLocator::Match& m ) override { return m.hasEdge(); }
};

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QgsPoint point,
                                    Qt::MouseButton button, Qt::KeyboardModifiers modifiers,
                                    QEvent::Type eventType, SnappingMode mode )
    : QMouseEvent( eventType,
                   mapToPixelCoordinates( mapTool->canvas(), point ),
                   mapTool->canvas()->mapToGlobal( mapToPixelCoordinates( mapTool->canvas(), point ) ),
                   button, button, modifiers )
    , mMapPoint( point )
    , mOriginalPoint( point )
    , mMapTool( mapTool )
    , mSnapMatch( QgsPointLocator::Match() )
    , mSnappingMode( mode )
{
  snapPoint();
}

void QgsMapMouseEvent::setPoint( const QgsPoint& point )
{
  mMapPoint.set( point.x(), point.y() );
}

void QgsMapMouseEvent::snapPoint()
{
  if ( mSnappingMode == NoSnapping )
    return;

  QgsSnappingUtils* snappingUtils = mMapTool->canvas()->snappingUtils();
  QgsSnappingUtils::SnapToMapMode canvasMode = snappingUtils->snapToMapMode();
  if ( mSnappingMode == SnapAllLayers )
  {
    snappingUtils->setSnapToMapMode( QgsSnappingUtils::SnapAllLayers );
    VertexOnlyFilter filter;
    mSnapMatch = snappingUtils->snapToMap( mMapPoint, &filter );
    snappingUtils->setSnapToMapMode( canvasMode );
  }
  else
  {
    mSnapMatch = snappingUtils->snapToMap( mMapPoint );
  }
  mMapPoint = mSnapMatch.isValid() ? mSnapMatch.point() : mOriginalPoint;
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

QList<QgsPoint> QgsMapMouseEvent::snapSegment( bool* snapped, bool allLayers ) const
{
  QList<QgsPoint> segment =  QList<QgsPoint>();
  QgsPoint pt1, pt2;

  if ( mSnapMatch.hasEdge() )
  {
    mSnapMatch.edgePoints( pt1, pt2 );
    segment << pt1 << pt2;
  }

  else if ( mSnappingMode != NoSnapping )
  {
    QgsPointLocator::Match match;
    EdgesOnlyFilter filter;
    QgsPoint point;
    if ( mSnappingMode == SnapProjectConfig && !allLayers )
    {
      // run snapToMap with only segments
      match = mMapTool->canvas()->snappingUtils()->snapToMap( point, &filter );
    }
    else if ( mSnappingMode == SnapAllLayers || allLayers )
    {
      // run snapToMap with only segments on all layers
      QgsSnappingUtils* snappingUtils = mMapTool->canvas()->snappingUtils();
      QgsSnappingUtils::SnapToMapMode canvasMode = snappingUtils->snapToMapMode();
      snappingUtils->setSnapToMapMode( QgsSnappingUtils::SnapAllLayers );
      match = snappingUtils->snapToMap( point, &filter );
      snappingUtils->setSnapToMapMode( canvasMode );
    }
    if ( match.isValid() && match.hasEdge() )
    {
      match.edgePoints( pt1, pt2 );
      segment << pt1 << pt2;
    }
  }

  if ( snapped )
  {
    *snapped = segment.count() == 2;
  }

  return segment;
}


