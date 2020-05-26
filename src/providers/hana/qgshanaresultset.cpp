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
  : mResultSet( std::move( resultSet ) )
  , mMetadata( mResultSet->getMetaData() )
{
}

QgsHanaResultSetRef QgsHanaResultSet::create( StatementRef &stmt, const QString &sql )
{
  QgsHanaResultSetRef ret( new QgsHanaResultSet( stmt->executeQuery( reinterpret_cast<const char16_t *>( sql.utf16() ) ) ) );
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
    {
      if ( mMetadata->getColumnLength( columnIndex ) == 1 )
      {
        Byte value = mResultSet->getByte( columnIndex );
        if ( value.isNull() )
          return QVariant( QVariant::Char );
        else
          return QVariant( QChar( *value ) );
      }
      else
        return QgsHanaUtils::toVariant( mResultSet->getString( columnIndex ) );
    }
    case SQLDataTypes::WChar:
      if ( mMetadata->getColumnLength( columnIndex ) == 1 )
      {
        UByte value = mResultSet->getUByte( columnIndex );
        if ( value.isNull() )
          return QVariant( QVariant::Char );
        else
          return QVariant( QChar( *value ) );
      }
      else
        return QgsHanaUtils::toVariant( mResultSet->getNString( columnIndex ) );
    case SQLDataTypes::TinyInt:
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
  auto createGeometry = []( const char *data, size_t size )
  {
    if ( size == 0 || data == nullptr )
      return QgsGeometry();

    if ( size > static_cast<size_t>( std::numeric_limits<int>::max() ) )
      throw std::overflow_error( "Geometry size is larger than INT_MAX" );

    QByteArray wkbBytes( data, static_cast<int>( size ) );
    QgsGeometry geom;
    geom.fromWkb( wkbBytes );
    return geom;
  };

  size_t bufLength = mResultSet->getBinaryLength( columnIndex );
  if ( bufLength == ResultSet::UNKNOWN_LENGTH )
  {
    Binary wkb = mResultSet->getBinary( columnIndex );
    if ( wkb.isNull() || wkb->size() == 0 )
      return QgsGeometry();
    else
      return createGeometry( wkb->data(), wkb->size() );
  }
  else if ( bufLength != 0 && bufLength != odbc::ResultSet::NULL_DATA )
  {
    mBuffer.resize( bufLength );
    mResultSet->getBinaryData( columnIndex, mBuffer.data(), bufLength );
    return createGeometry( mBuffer.data(), bufLength );
  }

  return QgsGeometry();
}
