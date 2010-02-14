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
/* $Id$ */
#include <QPainter>

#include "qgsmapcanvas.h"
#include "qgsgcpcanvasitem.h"

#include "qgsgeorefdatapoint.h"

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas* srcCanvas, QgsMapCanvas *dstCanvas,
                                        const QgsPoint& pixelCoords, const QgsPoint& mapCoords,
                                        bool enable )
    : mSrcCanvas(srcCanvas)
    , mDstCanvas(dstCanvas)
    , mPixelCoords( pixelCoords )
    , mMapCoords( mapCoords )
    , mId(-1)
    , mEnabled(enable)
{
  mGCPSourceItem = new QgsGCPCanvasItem(srcCanvas, pixelCoords, mapCoords, true);
  mGCPDestinationItem = new QgsGCPCanvasItem(dstCanvas, pixelCoords, mapCoords, false);

  mGCPSourceItem->setEnabled(enable);
  mGCPDestinationItem->setEnabled(enable);
  mGCPSourceItem->show();
  mGCPDestinationItem->show();
}

QgsGeorefDataPoint::QgsGeorefDataPoint(const QgsGeorefDataPoint &p)
{
  // we share item representation on canvas between all points
//  mGCPSourceItem = new QgsGCPCanvasItem(p.srcCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
//  mGCPDestinationItem = new QgsGCPCanvasItem(p.dstCanvas(), p.pixelCoords(), p.mapCoords(), p.isEnabled());
  mPixelCoords = p.pixelCoords();
  mMapCoords = p.mapCoords();
  mEnabled = p.isEnabled();
}

QgsGeorefDataPoint::~QgsGeorefDataPoint()
{
  delete mGCPSourceItem;
  delete mGCPDestinationItem;
}

void QgsGeorefDataPoint::setPixelCoords(const QgsPoint &p)
{
  mPixelCoords = p;
  mGCPSourceItem->setRasterCoords(p);
  mGCPDestinationItem->setRasterCoords(p);
}

void QgsGeorefDataPoint::setMapCoords(const QgsPoint &p)
{
  mMapCoords = p;
  mGCPSourceItem->setWorldCoords(p);
  mGCPDestinationItem->setWorldCoords(p);
}

void QgsGeorefDataPoint::setEnabled(bool enabled)
{
  mGCPSourceItem->setEnabled(enabled);
  mEnabled = enabled;
}

void QgsGeorefDataPoint::setId(int id) {
  mId = id;
  mGCPSourceItem->setId(id);
  mGCPDestinationItem->setId(id);
}

void QgsGeorefDataPoint::updateCoords()
{
  mGCPSourceItem->updatePosition();
  mGCPDestinationItem->updatePosition();
  mGCPSourceItem->update();
  mGCPDestinationItem->update();
}

bool QgsGeorefDataPoint::contains(const QPoint &p)
{
  QPointF pnt = mGCPSourceItem->mapFromScene(p);
  return mGCPSourceItem->shape().contains(pnt);
}

void QgsGeorefDataPoint::moveTo(const QPoint &p)
{
  QgsPoint pnt = mGCPSourceItem->toMapCoordinates(p);
  setPixelCoords(pnt);
  updateCoords();
}
