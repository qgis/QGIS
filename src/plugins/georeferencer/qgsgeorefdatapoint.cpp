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

#include "qgsgeorefdatapoint.h"

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas* srcCanvas, QgsMapCanvas *dstCanvas,
                                        const QgsPoint& pixelCoords, const QgsPoint& mapCoords,
                                        bool enable )
    : mSrcCanvas( srcCanvas )
    , mDstCanvas( dstCanvas )
    , mPixelCoords( pixelCoords )
    , mMapCoords( mapCoords )
    , mId( -1 )
    , mEnabled( enable )
{
  mGCPSourceItem = new QgsGCPCanvasItem( srcCanvas, this, true );
  mGCPDestinationItem = new QgsGCPCanvasItem( dstCanvas, this, false );

  mGCPSourceItem->setEnabled( enable );
  mGCPDestinationItem->setEnabled( enable );
  mGCPSourceItem->show();
  mGCPDestinationItem->show();
}

QgsGeorefDataPoint::QgsGeorefDataPoint( const QgsGeorefDataPoint &p )
    : QObject()
    , mSrcCanvas( nullptr )
    , mDstCanvas( nullptr )
    , mGCPSourceItem( nullptr )
    , mGCPDestinationItem( nullptr )
{
  Q_UNUSED( p );
  // we share item representation on canvas between all points
//  mGCPSourceItem = new QgsGCPCanvasItem(p.srcCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
//  mGCPDestinationItem = new QgsGCPCanvasItem(p.dstCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
  mPixelCoords = p.pixelCoords();
  mMapCoords = p.mapCoords();
  mEnabled = p.isEnabled();
  mResidual = p.residual();
  mId = p.id();
}

QgsGeorefDataPoint::~QgsGeorefDataPoint()
{
  delete mGCPSourceItem;
  delete mGCPDestinationItem;
}

void QgsGeorefDataPoint::setPixelCoords( const QgsPoint &p )
{
  mPixelCoords = p;
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
}

void QgsGeorefDataPoint::setMapCoords( const QgsPoint &p )
{
  mMapCoords = p;
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
    mGCPDestinationItem->update();
  }
}

void QgsGeorefDataPoint::setEnabled( bool enabled )
{
  mEnabled = enabled;
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
    QPointF pnt = mGCPSourceItem->mapFromScene( p );
    return mGCPSourceItem->shape().contains( pnt );
  }
  else
  {
    QPointF pnt = mGCPDestinationItem->mapFromScene( p );
    return mGCPDestinationItem->shape().contains( pnt );
  }
}

void QgsGeorefDataPoint::moveTo( QPoint p, bool isMapPlugin )
{
  if ( isMapPlugin )
  {
    QgsPoint pnt = mGCPSourceItem->toMapCoordinates( p );
    mPixelCoords = pnt;
  }
  else
  {
    QgsPoint pnt = mGCPDestinationItem->toMapCoordinates( p );
    mMapCoords = pnt;
  }
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
  updateCoords();
}
