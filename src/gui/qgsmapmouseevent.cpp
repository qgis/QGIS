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
#include "qgsmapcanvas.h"

#include "qgssnappingutils.h"
#include "qgssnappingconfig.h"

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapCanvas *mapCanvas, QMouseEvent *event )
  : QMouseEvent( event->type(), event->pos(), event->button(), event->buttons(), event->modifiers() )
  , mHasCachedSnapResult( false )
  , mOriginalMapPoint( mapCanvas ? mapCanvas->mapSettings().mapToPixel().toMapCoordinates( event->pos() ) : QgsPointXY() )
  , mMapPoint( mOriginalMapPoint )
  , mPixelPoint( event->pos() )
  , mMapCanvas( mapCanvas )
{
}

QgsMapMouseEvent::QgsMapMouseEvent( QgsMapCanvas *mapCanvas, QEvent::Type type, QPoint pos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers )
  : QMouseEvent( type, pos, button, buttons, modifiers )
  , mHasCachedSnapResult( false )
  , mOriginalMapPoint( mapCanvas ? mapCanvas->mapSettings().mapToPixel().toMapCoordinates( pos ) : QgsPointXY() )
  , mMapPoint( mOriginalMapPoint )
  , mPixelPoint( pos )
  , mMapCanvas( mapCanvas )
{
}

QgsPointXY QgsMapMouseEvent::snapPoint()
{
  // Use cached result
  if ( mHasCachedSnapResult )
    return mMapPoint;

  mHasCachedSnapResult = true;

  QgsSnappingUtils *snappingUtils = mMapCanvas->snappingUtils();
  mSnapMatch = snappingUtils->snapToMapRelaxed( mMapPoint );

  if ( mSnapMatch.isValid() )
  {
    mMapPoint = mSnapMatch.point();
    mPixelPoint = mapToPixelCoordinates( mMapPoint );
  }
  else
  {
    mMapPoint = mOriginalMapPoint;
    mPixelPoint = pos();
  }

  return mMapPoint;
}

void QgsMapMouseEvent::setMapPoint( const QgsPointXY &point )
{
  mMapPoint = point;
  mPixelPoint = mapToPixelCoordinates( point );
}

void QgsMapMouseEvent::snapToGrid( double precision, const QgsCoordinateReferenceSystem &crs )
{
  if ( precision <= 0 )
    return;

  try
  {
    QgsCoordinateTransform ct( mMapCanvas->mapSettings().destinationCrs(), crs, mMapCanvas->mapSettings().transformContext() );

    QgsPointXY pt = ct.transform( mMapPoint );

    pt.setX( std::round( pt.x() / precision ) * precision );
    pt.setY( std::round( pt.y() / precision ) * precision );

    pt = ct.transform( pt, QgsCoordinateTransform::ReverseTransform );

    setMapPoint( pt );
  }
  catch ( QgsCsException &e )
  {
    Q_UNUSED( e )
  }
}

QPoint QgsMapMouseEvent::mapToPixelCoordinates( const QgsPointXY &point )
{
  double x = point.x(), y = point.y();

  mMapCanvas->mapSettings().mapToPixel().transformInPlace( x, y );

  return QPoint( std::round( x ), std::round( y ) );
}
