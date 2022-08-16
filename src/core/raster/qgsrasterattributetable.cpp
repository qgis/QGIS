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

QgsRasterAttributeTable::QgsRasterAttributeTable( Origin origin )
  : mOrigin( origin )
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

QList<QgsRasterAttributeTable::Field> QgsRasterAttributeTable::fields() const
{
  return mFields;
}

bool QgsRasterAttributeTable::isDirty() const
{
  return mIsDirty;
}

void QgsRasterAttributeTable::setIsDirty( bool newIsDirty )
{
  mIsDirty = newIsDirty;
}

bool QgsRasterAttributeTable::insertField( const Field field, int position )
{
  if ( position < 0 )
  {
    return false;
  }

  int realPos { std::min( mFields.count(), position ) };

  mFields.insert( realPos, field );

  for ( auto it = mData.begin(); it != mData.end(); ++it )
  {
    it->insert( realPos, QVariant( field.type ) );
  }

  setIsDirty( true );

  return true;
}

bool QgsRasterAttributeTable::insertField( const QString &name, FieldUsage usage, QVariant::Type type, int position )
{
  return insertField( { name, usage, type}, position );
}

bool QgsRasterAttributeTable::appendField( const QString &name, FieldUsage usage, QVariant::Type type )
{
  return insertField( name, usage, type, mFields.count() );
}

bool QgsRasterAttributeTable::appendField( const Field &field )
{
  return insertField( field, mFields.count() );
}

bool QgsRasterAttributeTable::removeField( const QString &name )
{
  const auto toRemove { std::find_if( mFields.begin(), mFields.end(), [ &name ]( Field & f ) -> bool {
      return f.name == name;
    } )};

  if ( toRemove != mFields.end() )
  {
    const auto idx { std::distance( mFields.begin(), toRemove ) };
    mFields.erase( toRemove, mFields.end() );
    for ( auto it = mData.begin(); it != mData.end(); ++it )
    {
      it->removeAt( idx );
    }
    setIsDirty( true );
    return true;
  }

  return false;
}

bool QgsRasterAttributeTable::insertRow( const QVariantList data, int position )
{
  if ( position < 0 )
  {
    return false;
  }

  QVariantList dataValid;

  if ( dataValid.size() > mFields.size() )
  {
    for ( int colIdx = 0; colIdx < mFields.size(); colIdx++ )
    {
      dataValid.append( data[ colIdx ] );
    }
  }
  else
  {
    dataValid = data;
  }
  mData.insert( position, dataValid );
  setIsDirty( true );
  return true;
}

bool QgsRasterAttributeTable::appendRow( const QVariantList data )
{
  return insertRow( data, mData.count() );
}

bool QgsRasterAttributeTable::saveToFile( const QString &path, int bandNoInt )
{
  return false;
}

bool QgsRasterAttributeTable::loadFromFile( const QString &path )
{
  return false;
}

bool QgsRasterAttributeTable::isValid() const
{
  // TODO: check for mandatory fields
  return mFields.count() > 0 && mData.count( ) > 0;
}

QgsRasterAttributeTable::Origin QgsRasterAttributeTable::origin() const
{
  return mOrigin;
}

const QList<QList<QVariant> > &QgsRasterAttributeTable::data() const
{
  return mData;
}
