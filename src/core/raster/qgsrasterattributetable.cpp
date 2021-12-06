/***************************************************************************
  qgsrasterattributetable.cpp - QgsRasterAttributeTable

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetable.h"

QgsRasterAttributeTable::QgsRasterAttributeTable()
{

}

const QgsRasterAttributeTable::RatType &QgsRasterAttributeTable::type() const
{
  return mType;
}

void QgsRasterAttributeTable::setType( const QgsRasterAttributeTable::RatType &newType )
{
  mType = newType;
}

bool QgsRasterAttributeTable::hasColor()
{
  return true;
}

QgsFields QgsRasterAttributeTable::fields()
{
  return mFields;
}

const QList<QgsRasterAttributeTable::FieldUsage> &QgsRasterAttributeTable::fieldUsages() const
{
  return mFieldUsages;
}

void QgsRasterAttributeTable::setFieldUsages( const QList<QgsRasterAttributeTable::FieldUsage> &newFieldUsages )
{
  mFieldUsages = newFieldUsages;
}

bool QgsRasterAttributeTable::isDirty() const
{
  return mIsDirty;
}

void QgsRasterAttributeTable::setIsDirty( bool newIsDirty )
{
  mIsDirty = newIsDirty;
}

bool QgsRasterAttributeTable::insertField( const QString &name, FieldUsage usage, QVariant::Type type, int position )
{
  if ( position < 0 )
  {
    return false;
  }

  QgsFields fields;
  QList<FieldUsage> usages;
  int realPos { std::min( mFields.count(), position ) };
  int fieldIdx = 0;
  for ( ; fieldIdx < realPos; ++fieldIdx )
  {
    fields.append( mFields.at( fieldIdx ) );
    usages.append( mFieldUsages[ fieldIdx ] );
  }

  fields.append( QgsField( name, type ) );
  usages.append( usage );
  fieldIdx++;

  for ( ; fieldIdx < mFields.count(); ++fieldIdx )
  {
    fields.append( mFields.at( fieldIdx ) );
    usages.append( mFieldUsages[ fieldIdx ] );
  }

  mFields = fields;
  mFieldUsages = usages;

  for ( auto it = mData.begin(); it != mData.end(); ++it )
  {
    mData.insert( realPos, QVariant( type ) );
  }

  return true;
}

bool QgsRasterAttributeTable::appendField( const QString &name, FieldUsage usage, QVariant::Type type )
{
  return insertField( name, usage, type, mFields.count() );
}

bool QgsRasterAttributeTable::insertRow( const QVariantList data, int position )
{
  if ( position < 0 )
  {
    return false;
  }
  mData.insert( position, data );
  return true;
}

bool QgsRasterAttributeTable::appendRow( const QVariantList data )
{
  return insertRow( data, mData.count() );
}

bool QgsRasterAttributeTable::isValid()
{
  // TODO: check for mandatory fields
  return mFields.count() > 0 && mData.count( ) > 0;
}
