/***************************************************************************
                         qgspointcloudblock.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"


QgsPointCloudBlock::QgsPointCloudBlock(
  int count,
  const QgsPointCloudAttributeCollection &attributes,
  const QByteArray &data
)
  : mPointCount( count )
  , mAttributes( attributes )
  , mStorage( data )
{
}

QgsPointCloudBlock::~QgsPointCloudBlock() = default;

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
