/***************************************************************************
                         qgspointcloudattribute.cpp
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

QgsPointCloudAttribute::QgsPointCloudAttribute() = default;

QgsPointCloudAttribute::QgsPointCloudAttribute( const QString &name, int size )
  : mName( name )
  , mSize( size )
{
}

QgsPointCloudAttributeCollection::QgsPointCloudAttributeCollection() = default;

QgsPointCloudAttributeCollection::QgsPointCloudAttributeCollection( const QVector<QgsPointCloudAttribute> &attributes )
  : mAttributes( attributes )
{
  for ( int i = 0; i < mAttributes.size(); ++i )
  {
    const QgsPointCloudAttribute &attr = mAttributes.at( i );
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

bool QgsPointCloudAttributeCollection::offset( const QString &attributeName, int &offset, int &size ) const
{

  int off = 0;

  for ( int i = 0; i < mAttributes.size(); ++i )
  {
    const QgsPointCloudAttribute &attr = mAttributes.at( i );
    if ( attr.name() == attributeName )
    {
      offset = off;
      size = attr.size();
      return false;
    }
    else
    {
      off += attr.size();
    }
  }

  // not found
  return true;
}

QString QgsPointCloudAttribute::name() const
{
  return mName;
}

int QgsPointCloudAttribute::size() const
{
  return mSize;
}

void QgsPointCloudAttribute::setName(const QString& name)
{
  mName = name;
}

void QgsPointCloudAttribute::setSize(int size)
{
  mSize = size;
}
