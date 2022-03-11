/***************************************************************************
                         qgspointcloudblock.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"


QgsPointCloudBlock::QgsPointCloudBlock(
  int count,
  const QgsPointCloudAttributeCollection &attributes,
  const QByteArray &data, const QgsVector3D &scale, const QgsVector3D &offset
)
  : mPointCount( count )
  , mAttributes( attributes )
  , mStorage( data )
  , mScale( scale )
  , mOffset( offset )
{
}

const char *QgsPointCloudBlock::data() const
{
  return mStorage.data();
}

int QgsPointCloudBlock::pointCount() const
{
  return mPointCount;
}

QgsPointCloudAttributeCollection QgsPointCloudBlock::attributes() const
{
  return mAttributes;
}

QgsVector3D QgsPointCloudBlock::scale() const
{
  return mScale;
}

QgsVector3D QgsPointCloudBlock::offset() const
{
  return mOffset;
}

void QgsPointCloudBlock::setPointCount( int size )
{
  if ( size < 0 )
    return;
  mPointCount = size;
  mStorage.resize( size * mAttributes.pointRecordSize() );
}
