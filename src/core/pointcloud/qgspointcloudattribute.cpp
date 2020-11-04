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

QgsPointCloudAttribute::QgsPointCloudAttribute( const QString &name, DataType type )
  : mName( name )
  , mType( type )
{
  updateSize();
}

QString QgsPointCloudAttribute::name() const
{
  return mName;
}

size_t QgsPointCloudAttribute::size() const
{
  return mSize;
}

QgsPointCloudAttribute::DataType QgsPointCloudAttribute::type() const
{
  return mType;
}

void QgsPointCloudAttribute::updateSize()
{
  switch ( mType )
  {
    case DataType::Char:
      mSize = 1;
      break;
    case DataType::Short:
      mSize = 2;
      break;
    case DataType::Float:
      mSize = 4;
      break;
    case DataType::Int32:
      mSize = 4;
      break;
    case DataType::Double:
      mSize = 8;
      break;
  }
}

// //////////////////

QgsPointCloudAttributeCollection::QgsPointCloudAttributeCollection() = default;

QgsPointCloudAttributeCollection::QgsPointCloudAttributeCollection( const QVector<QgsPointCloudAttribute> &attributes )
{
  mAttributes.reserve( attributes.size() );
  for ( const QgsPointCloudAttribute &attribute : attributes )
  {
    push_back( attribute );
  }
}

void QgsPointCloudAttributeCollection::push_back( const QgsPointCloudAttribute &attribute )
{
  mCachedAttributes.insert( attribute.name(), CachedAttributeData( mAttributes.size(), mSize ) );
  mAttributes.push_back( attribute );
  mSize += attribute.size();
}

QVector<QgsPointCloudAttribute> QgsPointCloudAttributeCollection::attributes() const
{
  return mAttributes;
}

const QgsPointCloudAttribute *QgsPointCloudAttributeCollection::find( const QString &attributeName, int &offset ) const
{
  auto it = mCachedAttributes.constFind( attributeName );
  if ( it != mCachedAttributes.constEnd() )
  {
    offset = it->offset;
    return &mAttributes.at( it->index );
  }

  // not found
  return nullptr;
}
