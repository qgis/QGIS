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

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QMouseEvent* event, SnappingMode mode )
    :  QMouseEvent( event->type(), event->pos(), event->globalPos(), event->button(), event->buttons(), event->modifiers() )
    , mMapPoint( mapTool->canvas()->mapSettings().mapToPixel().toMapCoordinates( event->pos() ) )
    , mMapTool( mapTool )
    , mSnapMatch( QgsPointLocator::Match() )
    , mSnappingMode( mode )
{
  mOriginalPoint = mMapPoint;
  snapPoint();
}

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
    int type;
    double tolerance;
    QgsTolerance::UnitType unit;
    snappingUtils->defaultSettings( type, tolerance, unit );
    snappingUtils->setSnapToMapMode( QgsSnappingUtils::SnapAllLayers );
    snappingUtils->setDefaultSettings( QgsPointLocator::Vertex | QgsPointLocator::Edge, tolerance, unit );
    mSnapMatch = snappingUtils->snapToMap( mMapPoint );
    snappingUtils->setSnapToMapMode( canvasMode );
    snappingUtils->setDefaultSettings( type, tolerance, unit );
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
    if ( mSnappingMode == SnapProjectConfig && !allLayers )
    {
      // run snapToMap with only segments
      EdgesOnlyFilter filter;
      match = mMapTool->canvas()->snappingUtils()->snapToMap( mOriginalPoint, &filter );
    }
    else if ( mSnappingMode == SnapAllLayers || allLayers )
    {
      // run snapToMap with only edges on all layers
      QgsSnappingUtils* snappingUtils = mMapTool->canvas()->snappingUtils();
      QgsSnappingUtils::SnapToMapMode canvasMode = snappingUtils->snapToMapMode();
      int type;
      double tolerance;
      QgsTolerance::UnitType unit;
      snappingUtils->defaultSettings( type, tolerance, unit );
      snappingUtils->setSnapToMapMode( QgsSnappingUtils::SnapAllLayers );
      snappingUtils->setDefaultSettings( QgsPointLocator::Edge, tolerance, unit );
      match = snappingUtils->snapToMap( mOriginalPoint );
      snappingUtils->setSnapToMapMode( canvasMode );
      snappingUtils->setDefaultSettings( type, tolerance, unit );
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


