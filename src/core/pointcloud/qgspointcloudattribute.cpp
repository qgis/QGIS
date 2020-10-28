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
#include "qgspointcloudattribute.h"

QgsPointCloudAttribute::QgsPointCloudAttribute( const QString &name, int size )
  : mName( name )
  , mSize( size )
{
}

QgsPointCloudAttributeCollection::QgsPointCloudAttributeCollection() = default;

QgsPointCloudAttributeCollection::QgsPointCloudAttributeCollection( const QVector<QgsPointCloudAttribute> &attributes )
  : mAttributes( attributes )
{
  for ( const QgsPointCloudAttribute &attr : mAttributes )
  {
    mSize += attr.size();
  }

}

void QgsPointCloudAttributeCollection::push_back( const QgsPointCloudAttribute &attribute )
{
  mAttributes.push_back( attribute );
  mSize += attribute.size();
}

QVector<QgsPointCloudAttribute> QgsPointCloudAttributeCollection::attributes() const
{
  return mAttributes;
}

int QgsPointCloudAttributeCollection::offset( const QString &attributeName, const QgsPointCloudAttribute *foundAttribute ) const
{

  int off = 0;

  for ( const QgsPointCloudAttribute &attr : mAttributes )
  {
    if ( attr.name() == attributeName )
    {
      foundAttribute = &attr;
      return off;
    }
    else
    {
      off += attr.size();
    }
  }

  // not found
  return -1;
}

QString QgsPointCloudAttribute::name() const
{
  return mName;
}

int QgsPointCloudAttribute::size() const
{
  return mSize;
}
