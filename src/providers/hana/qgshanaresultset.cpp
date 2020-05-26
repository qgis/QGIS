/***************************************************************************
   qgshanaresultset.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaresultset.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include <QString>

QgsHanaResultSet::QgsHanaResultSet( ResultSetRef &&resultSet )
  : mResultSet( resultSet )
  , mMetadata( mResultSet->getMetaData() )
{
}

QgsHanaResultSetRef QgsHanaResultSet::create( StatementRef &stmt, const QString &sql )
{
  QgsHanaResultSetRef ret( new QgsHanaResultSet( stmt->executeQuery( reinterpret_cast<const char16_t *>( sql.unicode() ) ) ) );
  return ret;
}

QgsHanaResultSetRef QgsHanaResultSet::create( PreparedStatementRef &stmt )
{
  QgsHanaResultSetRef ret( new QgsHanaResultSet( stmt->executeQuery() ) );
  return ret;
}

void QgsHanaResultSet::close()
{
  mResultSet->close();
}

bool QgsHanaResultSet::next()
{
  return mResultSet->next();
}

QString QgsHanaResultSet::getString( unsigned short columnIndex )
{
  return QgsHanaUtils::toQString( mResultSet->getNString( columnIndex ) );
}

QVariant QgsHanaResultSet::getValue( unsigned short columnIndex )
{
  switch ( mMetadata->getColumnType( columnIndex ) )
  {
    case SQLDataTypes::Bit:
    case SQLDataTypes::Boolean:
      return QgsHanaUtils::toVariant( mResultSet->getBoolean( columnIndex ), QVariant::Bool );
    case SQLDataTypes::Char:
      return QgsHanaUtils::toVariant( mResultSet->getByte( columnIndex ) );
    case SQLDataTypes::WChar:
      return QgsHanaUtils::toVariant( mResultSet->getUByte( columnIndex ) );
    case SQLDataTypes::SmallInt:
      if ( mMetadata ->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getShort( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getUShort( columnIndex ) );
    case SQLDataTypes::Integer:
      if ( mMetadata ->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getInt( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getUInt( columnIndex ) );
    case SQLDataTypes::BigInt:
      if ( mMetadata ->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getLong( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getULong( columnIndex ) );
    case SQLDataTypes::Double:
    case SQLDataTypes::Decimal:
    case SQLDataTypes::Numeric:
      return QgsHanaUtils::toVariant( mResultSet->getDouble( columnIndex ) );
    case SQLDataTypes::Date:
    case SQLDataTypes::TypeDate:
      return QgsHanaUtils::toVariant( mResultSet->getDate( columnIndex ) );
    case SQLDataTypes::Time:
    case SQLDataTypes::TypeTime:
      return QgsHanaUtils::toVariant( mResultSet->getTime( columnIndex ) );
    case SQLDataTypes::Timestamp:
    case SQLDataTypes::TypeTimestamp:
      return QgsHanaUtils::toVariant( mResultSet->getTimestamp( columnIndex ) );
    case SQLDataTypes::VarChar:
    case SQLDataTypes::LongVarChar:
      return QgsHanaUtils::toVariant( mResultSet->getString( columnIndex ) );
    case SQLDataTypes::WVarChar:
    case SQLDataTypes::WLongVarChar:
      return QgsHanaUtils::toVariant( mResultSet->getNString( columnIndex ) );
    case SQLDataTypes::Binary:
    case SQLDataTypes::VarBinary:
    case SQLDataTypes::LongVarBinary:
      return QgsHanaUtils::toVariant( mResultSet->getBinary( columnIndex ) );
    default:
      QgsDebugMsg( QStringLiteral( "Unhandled HANA type %1" ).arg( QString::fromStdString( mMetadata->getColumnTypeName( columnIndex ) ) ) );
      return QVariant();
  }
}

QgsGeometry QgsHanaResultSet::getGeometry( unsigned short columnIndex )
{
  size_t bufLength = mResultSet->getBinaryLength( columnIndex );
  unsigned char *bufPtr = nullptr;

  if ( bufLength == ResultSet::UNKNOWN_LENGTH )
  {
    Binary wkb = mResultSet->getBinary( columnIndex );
    if ( wkb.isNull() || wkb->size() == 0 )
      bufLength = 0;
    else
      bufPtr = reinterpret_cast<unsigned char *>( wkb->data() );
  }
  else if ( bufLength != 0 && bufLength != odbc::ResultSet::NULL_DATA )
  {
    ensureBufferCapacity( bufLength );
    mResultSet->getBinaryData( columnIndex, mBuffer.data(), bufLength );
    bufPtr = mBuffer.data();
  }

  QgsGeometry geom;
  if ( !( bufLength == 0 || bufPtr == nullptr ) )
  {
    unsigned char *wkbGeom = new unsigned char[bufLength];
    memcpy( wkbGeom, bufPtr, bufLength );
    geom.fromWkb( wkbGeom, static_cast<int>( bufLength ) );
  }

  return geom;
}

void QgsHanaResultSet::ensureBufferCapacity( std::size_t capacity )
{
  if ( capacity > mBuffer.size() )
  {
    mBuffer.reserve( capacity );
  }
}
