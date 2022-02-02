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

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas,
                                        const QgsPointXY &sourceCoordinates, const QgsPointXY &destinationMapCoords,
                                        const QgsCoordinateReferenceSystem &destinationCrs, bool enable )
  : mSrcCanvas( srcCanvas )
  , mDstCanvas( dstCanvas )
  , mSourceCoords( sourceCoordinates )
  , mDestinationMapCoords( destinationMapCoords )
  , mId( -1 )
  , mDestinationCrs( destinationCrs )
  , mEnabled( enable )
{
  mTransCoords = QgsPointXY( destinationMapCoords );
  mGCPSourceItem = new QgsGCPCanvasItem( srcCanvas, this, true );
  mGCPDestinationItem = new QgsGCPCanvasItem( dstCanvas, this, false );
  mGCPSourceItem->setEnabled( enable );
  mGCPDestinationItem->setEnabled( enable );
  mGCPSourceItem->show();
  mGCPDestinationItem->show();
}

QgsGeorefDataPoint::QgsGeorefDataPoint( const QgsGeorefDataPoint &p )
  : QObject( nullptr )
{
  // we share item representation on canvas between all points
//  mGCPSourceItem = new QgsGCPCanvasItem(p.srcCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
//  mGCPDestinationItem = new QgsGCPCanvasItem(p.dstCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
  mSourceCoords = p.sourceCoords();
  mDestinationMapCoords = p.destinationMapCoords();
  mTransCoords = p.transCoords();
  mEnabled = p.isEnabled();
  mResidual = p.residual();
  mDestinationInCanvasPixels = p.destinationInCanvasPixels();
  mDestinationCrs = p.destinationCrs();
  mId = p.id();
}

QgsGeorefDataPoint::~QgsGeorefDataPoint()
{
  delete mGCPSourceItem;
  delete mGCPDestinationItem;
}

void QgsGeorefDataPoint::setSourceCoords( const QgsPointXY &p )
{
  mSourceCoords = p;
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
}

void QgsGeorefDataPoint::setDestinationMapCoords( const QgsPointXY &p )
{
  mDestinationMapCoords = p;
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
    mGCPDestinationItem->update();
  }
}

void QgsGeorefDataPoint::setTransCoords( const QgsPointXY &p )
{
  mTransCoords = p;
  if ( mGCPSourceItem )
  {
    mGCPSourceItem->update();
  }
  if ( mGCPDestinationItem )
  {
    mGCPDestinationItem->update();
  }
}

QgsPointXY QgsGeorefDataPoint::transCoords() const
{
  return mTransCoords.isEmpty() ? mDestinationMapCoords : mTransCoords;
}

void QgsGeorefDataPoint::setDestinationInCanvasPixels( const QgsPointXY &p )
{
  mDestinationInCanvasPixels = p;
}

QgsPointXY QgsGeorefDataPoint::destinationInCanvasPixels() const
{
  return mDestinationInCanvasPixels;
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
    mSourceCoords = pnt;
  }
  else
  {
    mDestinationInCanvasPixels = canvasPixels;
    mDestinationMapCoords = mGCPDestinationItem->toMapCoordinates( canvasPixels );
    if ( mSrcCanvas && mSrcCanvas->mapSettings().destinationCrs().isValid() )
      mDestinationCrs = mSrcCanvas->mapSettings().destinationCrs();
    else
      mDestinationCrs = mGCPDestinationItem->canvas()->mapSettings().destinationCrs();
  }
  if ( !mDestinationCrs.isValid() )
    mDestinationCrs =  QgsProject::instance()->crs();
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
  updateCoords();
}
