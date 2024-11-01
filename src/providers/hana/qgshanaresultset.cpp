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
#include "qgshanadatatypes.h"
#include "qgshanaexception.h"
#include "qgshanaresultset.h"
#include "qgshanautils.h"
#include "qgsvariantutils.h"
#include "qgslogger.h"
#include <QString>

#include "odbc/Exception.h"
#include "odbc/PreparedStatement.h"
#include "odbc/Statement.h"

#include <cstring>

using namespace NS_ODBC;

namespace
{
  QString fvecsToString( const char *data, size_t )
  {
    uint32_t numElements;
    memcpy( &numElements, data, sizeof( numElements ) );

    const char *ptr = static_cast<const char *>( data ) + sizeof( numElements );
    const float *elements = reinterpret_cast<const float *>( ptr );

    QString res;
    res += QStringLiteral( "[" ) + QString::number( elements[0], 'g', 7 );
    for ( uint32_t i = 1; i < numElements; ++i )
      res += QStringLiteral( "," ) + QString::number( elements[i], 'g', 7 );
    res += QLatin1Char( ']' );

    return res;
  }
} // namespace

QgsHanaResultSet::QgsHanaResultSet( ResultSetRef &&resultSet )
  : mResultSet( std::move( resultSet ) )
  , mMetadata( mResultSet->getMetaDataUnicode() )
{
}

QgsHanaResultSetRef QgsHanaResultSet::create( StatementRef &stmt, const QString &sql )
{
  try
  {
    QgsHanaResultSetRef ret( new QgsHanaResultSet( stmt->executeQuery( QgsHanaUtils::toUtf16( sql ) ) ) );
    return ret;
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }
}

QgsHanaResultSetRef QgsHanaResultSet::create( PreparedStatementRef &stmt )
{
  try
  {
    QgsHanaResultSetRef ret( new QgsHanaResultSet( stmt->executeQuery() ) );
    return ret;
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }
}

void QgsHanaResultSet::close()
{
  try
  {
    mResultSet->close();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }
}

bool QgsHanaResultSet::next()
{
  try
  {
    return mResultSet->next();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }
}

double QgsHanaResultSet::getDouble( unsigned short columnIndex )
{
  return *mResultSet->getDouble( columnIndex );
}

int QgsHanaResultSet::getInt( unsigned short columnIndex )
{
  return *mResultSet->getInt( columnIndex );
}

short QgsHanaResultSet::getShort( unsigned short columnIndex )
{
  return *mResultSet->getShort( columnIndex );
}

QString QgsHanaResultSet::getString( unsigned short columnIndex )
{
  return QgsHanaUtils::toQString( mResultSet->getNString( columnIndex ) );
}

QVariant QgsHanaResultSet::getValue( unsigned short columnIndex )
{
  QgsHanaDataType type = QgsHanaDataTypeUtils::fromInt( mMetadata->getColumnType( columnIndex ) );
  if ( type == QgsHanaDataType::VarBinary && mMetadata->getColumnTypeName( columnIndex ) == QLatin1String( "REAL_VECTOR" ) )
    type = QgsHanaDataType::RealVector;

  switch ( type )
  {
    case QgsHanaDataType::Bit:
    case QgsHanaDataType::Boolean:
      return QgsHanaUtils::toVariant( mResultSet->getBoolean( columnIndex ) );
    case QgsHanaDataType::Char:
    {
      String str = mResultSet->getString( columnIndex );
      if ( mMetadata->getColumnLength( columnIndex ) == 1 )
      {
        if ( str.isNull() || str->empty() )
          return QgsVariantUtils::createNullVariant( QMetaType::Type::QChar );
        else
          return QVariant( QChar( str->at( 0 ) ) );
      }
      else
        return QgsHanaUtils::toVariant( str );
    }
    case QgsHanaDataType::WChar:
    {
      NString str = mResultSet->getNString( columnIndex );
      if ( mMetadata->getColumnLength( columnIndex ) == 1 )
      {
        if ( str.isNull() || str->empty() )
          return QgsVariantUtils::createNullVariant( QMetaType::Type::QChar );
        else
          return QVariant( QChar( str->at( 0 ) ) );
      }
      else
        return QgsHanaUtils::toVariant( str );
    }
    case QgsHanaDataType::TinyInt:
      if ( mMetadata->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getByte( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getUByte( columnIndex ) );
    case QgsHanaDataType::SmallInt:
      if ( mMetadata->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getShort( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getUShort( columnIndex ) );
    case QgsHanaDataType::Integer:
      if ( mMetadata->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getInt( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getUInt( columnIndex ) );
    case QgsHanaDataType::BigInt:
      if ( mMetadata->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getLong( columnIndex ) );
      else
        return QgsHanaUtils::toVariant( mResultSet->getULong( columnIndex ) );
    case QgsHanaDataType::Real:
      return QgsHanaUtils::toVariant( mResultSet->getFloat( columnIndex ) );
    case QgsHanaDataType::Double:
    case QgsHanaDataType::Decimal:
    case QgsHanaDataType::Float:
    case QgsHanaDataType::Numeric:
      return QgsHanaUtils::toVariant( mResultSet->getDouble( columnIndex ) );
    case QgsHanaDataType::Date:
    case QgsHanaDataType::TypeDate:
      return QgsHanaUtils::toVariant( mResultSet->getDate( columnIndex ) );
    case QgsHanaDataType::Time:
    case QgsHanaDataType::TypeTime:
      return QgsHanaUtils::toVariant( mResultSet->getTime( columnIndex ) );
    case QgsHanaDataType::Timestamp:
    case QgsHanaDataType::TypeTimestamp:
      return QgsHanaUtils::toVariant( mResultSet->getTimestamp( columnIndex ) );
    case QgsHanaDataType::VarChar:
    case QgsHanaDataType::LongVarChar:
      return QgsHanaUtils::toVariant( mResultSet->getString( columnIndex ) );
    case QgsHanaDataType::WVarChar:
    case QgsHanaDataType::WLongVarChar:
      return QgsHanaUtils::toVariant( mResultSet->getNString( columnIndex ) );
    case QgsHanaDataType::Binary:
    case QgsHanaDataType::VarBinary:
    case QgsHanaDataType::LongVarBinary:
    case QgsHanaDataType::Geometry:
      return QgsHanaUtils::toVariant( mResultSet->getBinary( columnIndex ) );
    case QgsHanaDataType::RealVector:
    {
      const size_t bufLength = mResultSet->getBinaryLength( columnIndex );
      if ( bufLength == ResultSet::UNKNOWN_LENGTH )
      {
        Binary vec = mResultSet->getBinary( columnIndex );
        if ( !vec.isNull() && vec->size() > 0 )
          return fvecsToString( vec->data(), vec->size() );
      }
      else if ( bufLength != 0 && bufLength != ResultSet::NULL_DATA )
      {
        if ( bufLength > static_cast<size_t>( std::numeric_limits<int>::max() ) )
          throw QgsHanaException( "RealVector size is larger than maximum integer value" );

        QByteArray vec( static_cast<int>( bufLength ), '0' );
        mResultSet->getBinaryData( columnIndex, vec.data(), bufLength );
        return fvecsToString( vec.data(), vec.size() );
      }

      return QVariant();
    }
    default:
      QgsDebugError( QStringLiteral( "Unhandled HANA data type %1" ).arg( QString::fromStdU16String( mMetadata->getColumnTypeName( columnIndex ) ) ) );
      return QVariant();
  }
}

QgsGeometry QgsHanaResultSet::getGeometry( unsigned short columnIndex )
{
  auto toWkbSize = []( size_t size ) {
    if ( size > static_cast<size_t>( std::numeric_limits<int>::max() ) )
      throw QgsHanaException( "Geometry size is larger than maximum integer value" );
    return static_cast<int>( size );
  };

  const size_t bufLength = mResultSet->getBinaryLength( columnIndex );
  if ( bufLength == ResultSet::UNKNOWN_LENGTH )
  {
    Binary wkb = mResultSet->getBinary( columnIndex );
    if ( !wkb.isNull() && wkb->size() > 0 )
    {
      const QByteArray wkbBytes( wkb->data(), toWkbSize( wkb->size() ) );
      QgsGeometry geom;
      geom.fromWkb( wkbBytes );
      return geom;
    }
  }
  else if ( bufLength != 0 && bufLength != ResultSet::NULL_DATA )
  {
    QByteArray wkbBytes( toWkbSize( bufLength ), '0' );
    mResultSet->getBinaryData( columnIndex, wkbBytes.data(), bufLength );
    QgsGeometry geom;
    geom.fromWkb( wkbBytes );
    return geom;
  }

  return QgsGeometry();
}
