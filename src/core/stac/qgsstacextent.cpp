/***************************************************************************
    qgsstacextent.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacextent.h"


void QgsStacExtent::setSpatialExtent( QgsBox3D extent )
{
  mSpatialExtent = extent;
}

void QgsStacExtent::setSpatialExtent( QgsRectangle extent )
{
  setSpatialExtent( extent.toBox3d( 0, 0 ) );
}

void QgsStacExtent::addDetailedSpatialExtent( QgsBox3D extent )
{
  mDetailedSpatialExtents.append( extent );
}

void QgsStacExtent::addDetailedSpatialExtent( QgsRectangle extent )
{
  addDetailedSpatialExtent( extent.toBox3d( 0, 0 ) );
}

QgsBox3D QgsStacExtent::spatialExtent() const
{
  return mSpatialExtent;
}

void QgsStacExtent::setTemporalExtent( QgsDateTimeRange extent )
{
  mTemporalExtent = extent;
}

void QgsStacExtent::addDetailedTemporalExtent( QgsDateTimeRange extent )
{
  mDetailedTemporalExtents.append( extent );
}

QgsDateTimeRange QgsStacExtent::temporalExtent() const
{
  return mTemporalExtent;
}

QVector<QgsBox3D> QgsStacExtent::detailedSpatialExtents() const
{
  return mDetailedSpatialExtents;
}

QVector<QgsDateTimeRange> QgsStacExtent::detailedTemporalExtents() const
{
  return mDetailedTemporalExtents;
}

bool QgsStacExtent::hasDetailedSpatialExtents() const
{
  return !mDetailedSpatialExtents.isEmpty();
}

bool QgsStacExtent::hasDetailedTemporalExtents() const
{
  return !mDetailedTemporalExtents.isEmpty();
}
