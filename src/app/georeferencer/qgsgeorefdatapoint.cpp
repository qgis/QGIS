/***************************************************************************
     qgsgeorefdatapoint.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:02:45 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QPainter>

#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsgcpcanvasitem.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeorefdatapoint.h"
#include "moc_qgsgeorefdatapoint.cpp"

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas, const QgsPointXY &sourceCoordinates, const QgsPointXY &destinationPoint, const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled )
  : mSrcCanvas( srcCanvas )
  , mDstCanvas( dstCanvas )
  , mGcpPoint( sourceCoordinates, destinationPoint, destinationPointCrs, enabled )
  , mId( -1 )
{
  mGCPSourceItem = new QgsGCPCanvasItem( srcCanvas, this, true );
  mGCPDestinationItem = new QgsGCPCanvasItem( dstCanvas, this, false );
  mGCPSourceItem->setEnabled( enabled );
  mGCPDestinationItem->setEnabled( enabled );
  mGCPSourceItem->show();
  mGCPDestinationItem->show();
}

// NOTE: copy constructor is only used to copy data points for the QgsResidualPlotItem layout item.
// Accordingly only some members are copied. Ideally things like the id and residual could be moved
// to another class (QgsGcpPoint?) so that QgsGeorefDataPoint can become a canvas associated GUI only
// class.
QgsGeorefDataPoint::QgsGeorefDataPoint( const QgsGeorefDataPoint &p )
  : QObject( nullptr )
  , mSrcCanvas( p.mSrcCanvas )
  , mDstCanvas( p.mDstCanvas )
  , mGCPSourceItem( nullptr )
  , mGCPDestinationItem( nullptr )
  , mGcpPoint( p.mGcpPoint )
  , mId( p.id() )
  , mResidual( p.residual() )
{
}

QgsGeorefDataPoint::~QgsGeorefDataPoint()
{
  delete mGCPSourceItem;
  delete mGCPDestinationItem;
}

void QgsGeorefDataPoint::setSourcePoint( const QgsPointXY &p )
{
  mGcpPoint.setSourcePoint( p );
  updateCoords();
}

void QgsGeorefDataPoint::setDestinationPoint( const QgsPointXY &p )
{
  mGcpPoint.setDestinationPoint( p );
  updateCoords();
}

void QgsGeorefDataPoint::setDestinationPointCrs( const QgsCoordinateReferenceSystem &crs )
{
  mGcpPoint.setDestinationPointCrs( crs );
  updateCoords();
}

QgsPointXY QgsGeorefDataPoint::transformedDestinationPoint( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const
{
  return mGcpPoint.transformedDestinationPoint( targetCrs, context );
}

void QgsGeorefDataPoint::setEnabled( bool enabled )
{
  mGcpPoint.setEnabled( enabled );
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->update();
  }
}

void QgsGeorefDataPoint::setId( int id )
{
  const bool noLongerTemporary = mId < 0 && id >= 0;
  mId = id;
  if ( mGCPSourceItem )
  {
    if ( noLongerTemporary )
      mGCPSourceItem->setPointColor( Qt::red );
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
    if ( noLongerTemporary )
      mGCPDestinationItem->setPointColor( Qt::red );
    mGCPDestinationItem->update();
  }
}

void QgsGeorefDataPoint::setResidual( QPointF r )
{
  mResidual = r;
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->checkBoundingRectChange();
  }
}

void QgsGeorefDataPoint::updateCoords()
{
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->updatePosition();
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
    mGCPDestinationItem->updatePosition();
    mGCPDestinationItem->update();
  }
}

bool QgsGeorefDataPoint::contains( QPoint p, QgsGcpPoint::PointType type, double &distance )
{
  const double searchRadiusMM = QgsMapTool::searchRadiusMM();
  const double pixelsPerMM = mGCPSourceItem->canvas()->logicalDpiX() / 25.4;
  const double searchRadiusPx = searchRadiusMM * pixelsPerMM;

  QPointF itemPos;
  switch ( type )
  {
    case QgsGcpPoint::PointType::Source:
    {
      itemPos = mGCPSourceItem->pos();
      break;
    }

    case QgsGcpPoint::PointType::Destination:
    {
      itemPos = mGCPDestinationItem->pos();
      break;
    }
  }

  const double dx = p.x() - itemPos.x();
  const double dy = p.y() - itemPos.y();
  distance = std::sqrt( dx * dx + dy * dy );
  return distance <= searchRadiusPx;
}

void QgsGeorefDataPoint::moveTo( QPoint canvasPixels, QgsGcpPoint::PointType type )
{
  switch ( type )
  {
    case QgsGcpPoint::PointType::Source:
    {
      const QgsPointXY pnt = mGCPSourceItem->toMapCoordinates( canvasPixels );
      mGcpPoint.setSourcePoint( pnt );
      break;
    }
    case QgsGcpPoint::PointType::Destination:
    {
      mGcpPoint.setDestinationPoint( mGCPDestinationItem->toMapCoordinates( canvasPixels ) );
      if ( mSrcCanvas && mSrcCanvas->mapSettings().destinationCrs().isValid() )
        mGcpPoint.setDestinationPointCrs( mSrcCanvas->mapSettings().destinationCrs() );
      else
        mGcpPoint.setDestinationPointCrs( mGCPDestinationItem->canvas()->mapSettings().destinationCrs() );

      if ( !mGcpPoint.destinationPointCrs().isValid() )
        mGcpPoint.setDestinationPointCrs( QgsProject::instance()->crs() );
      break;
    }
  }

  updateCoords();
}
