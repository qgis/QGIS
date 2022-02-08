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
#include "qgsgcpcanvasitem.h"
#include "qgscoordinatereferencesystem.h"


#include "qgsgeorefdatapoint.h"

//
// QgsGcpPoint
//

QgsGcpPoint::QgsGcpPoint( const QgsPointXY &sourcePoint, const QgsPointXY &destinationPoint, const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled )
  : mSourcePoint( sourcePoint )
  , mDestinationPoint( destinationPoint )
  , mDestinationCrs( destinationPointCrs )
  , mEnabled( enabled )
{

}

QgsCoordinateReferenceSystem QgsGcpPoint::destinationPointCrs() const
{
  return mDestinationCrs;
}

void QgsGcpPoint::setDestinationPointCrs( const QgsCoordinateReferenceSystem &crs )
{
  mDestinationCrs = crs;
}

QgsPointXY QgsGcpPoint::transformedDestinationPoint( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const
{
  const QgsCoordinateTransform transform( mDestinationCrs, targetCrs, context );
  try
  {
    return transform.transform( mDestinationPoint );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming destination point" ) );
    return mDestinationPoint;
  }
}


//
// QgsGeorefDataPoint
//

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas,
                                        const QgsPointXY &sourceCoordinates, const QgsPointXY &destinationPoint,
                                        const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled )
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

QgsGeorefDataPoint::QgsGeorefDataPoint( const QgsGeorefDataPoint &p )
  : QObject( nullptr )
  , mGcpPoint( p.mGcpPoint )
{
  // we share item representation on canvas between all points
//  mGCPSourceItem = new QgsGCPCanvasItem(p.srcCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
//  mGCPDestinationItem = new QgsGCPCanvasItem(p.dstCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
  mResidual = p.residual();
  mId = p.id();
}

QgsGeorefDataPoint::~QgsGeorefDataPoint()
{
  delete mGCPSourceItem;
  delete mGCPDestinationItem;
}

void QgsGeorefDataPoint::setSourcePoint( const QgsPointXY &p )
{
  mGcpPoint.setSourcePoint( p );
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
}

void QgsGeorefDataPoint::setDestinationPoint( const QgsPointXY &p )
{
  mGcpPoint.setDestinationPoint( p );
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
    mGCPDestinationItem->update();
  }
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
  mId = id;
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
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

bool QgsGeorefDataPoint::contains( QPoint p, bool isMapPlugin )
{
  if ( isMapPlugin )
  {
    const QPointF pnt = mGCPSourceItem->mapFromScene( p );
    return mGCPSourceItem->shape().contains( pnt );
  }
  else
  {
    const QPointF pnt = mGCPDestinationItem->mapFromScene( p );
    return mGCPDestinationItem->shape().contains( pnt );
  }
}

void QgsGeorefDataPoint::moveTo( QPoint canvasPixels, bool isMapPlugin )
{
  if ( isMapPlugin )
  {
    const QgsPointXY pnt = mGCPSourceItem->toMapCoordinates( canvasPixels );
    mGcpPoint.setSourcePoint( pnt );
  }
  else
  {
    mGcpPoint.setDestinationPoint( mGCPDestinationItem->toMapCoordinates( canvasPixels ) );
    if ( mSrcCanvas && mSrcCanvas->mapSettings().destinationCrs().isValid() )
      mGcpPoint.setDestinationPointCrs( mSrcCanvas->mapSettings().destinationCrs() );
    else
      mGcpPoint.setDestinationPointCrs( mGCPDestinationItem->canvas()->mapSettings().destinationCrs() );
  }
  if ( !mGcpPoint.destinationPointCrs().isValid() )
    mGcpPoint.setDestinationPointCrs( QgsProject::instance()->crs() );
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
  updateCoords();
}
