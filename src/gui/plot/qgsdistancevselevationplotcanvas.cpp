/***************************************************************************
                          qgsdistancevselevationplotcanvas.cpp
                          -----------------
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

#include "qgsdistancevselevationplotcanvas.h"
#include "qgspoint.h"
#include "qgscurve.h"
#include "qgsgeos.h"
#include "qgsplot.h"




QgsDistanceVsElevationPlotCanvas::QgsDistanceVsElevationPlotCanvas( QWidget *parent )
  : QgsPlotCanvas( parent )
{
  mPlotContentsRect = rect();
}

QgsDistanceVsElevationPlotCanvas::~QgsDistanceVsElevationPlotCanvas() = default;

void QgsDistanceVsElevationPlotCanvas::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

void QgsDistanceVsElevationPlotCanvas::setProfileCurve( QgsCurve *curve )
{
  mProfileCurve.reset( curve );
}

QgsCurve *QgsDistanceVsElevationPlotCanvas::profileCurve() const
{
  return mProfileCurve.get();
}

double QgsDistanceVsElevationPlotCanvas::zToCanvasY( double z ) const
{
  const double yPercent = ( z - mZMin ) / ( mZMax - mZMin );
  return mPlotContentsRect.bottom() - mPlotContentsRect.height() * yPercent;
}

QgsCoordinateReferenceSystem QgsDistanceVsElevationPlotCanvas::crs() const
{
  return mCrs;
}

QgsPoint QgsDistanceVsElevationPlotCanvas::toMapCoordinates( const QgsPointXY &point ) const
{
  if ( !mPlotContentsRect.contains( point.x(), point.y() ) )
    return QgsPoint();

  if ( !mProfileCurve )
    return QgsPoint();

  const double dx = point.x() - mPlotContentsRect.left();
  const double distanceAlongCurvePercent = dx / mPlotContentsRect.width();
  double distanceAlongCurveLength = distanceAlongCurvePercent * mProfileCurve->length();
  std::unique_ptr< QgsPoint > mapXyPoint( mProfileCurve->interpolatePoint( distanceAlongCurveLength ) );

  const double mapZ = ( mZMax - mZMin ) / ( mPlotContentsRect.height() ) * point.y();

  return QgsPoint( mapXyPoint->x(), mapXyPoint->y(), mapZ );
}

QgsPointXY QgsDistanceVsElevationPlotCanvas::toCanvasCoordinates( const QgsPoint &point ) const
{
  if ( !mProfileCurve )
    return QgsPointXY();

  QgsGeos geos( mProfileCurve.get() );
  QString error;
  const double distanceAlongCurve = geos.lineLocatePoint( point, &error );
  const double distanceAlongCurvePercent = distanceAlongCurve / mProfileCurve->length();
  const double distanceAlongPlotRect = distanceAlongCurvePercent * mPlotContentsRect.width();

  const double canvasX = mPlotContentsRect.left() + distanceAlongPlotRect;

  double canvasY = 0;
  if ( std::isnan( point.z() ) || point.z() < mZMin || point.z() > mZMax )
  {
    canvasY = mPlotContentsRect.top();
  }
  else
  {
    canvasY = zToCanvasY( point.z() );
  }

  return QgsPointXY( canvasX, canvasY );
}

void QgsDistanceVsElevationPlotCanvas::resizeEvent( QResizeEvent *event )
{
  QgsPlotCanvas::resizeEvent( event );
  mPlotContentsRect = rect();
}
