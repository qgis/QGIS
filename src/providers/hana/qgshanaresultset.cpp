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
#include "qgshanaexception.h"
#include "qgshanaresultset.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include <QString>

#include "odbc/Exception.h"
#include "odbc/PreparedStatement.h"
#include "odbc/Statement.h"

using namespace NS_ODBC;

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
  switch ( mMetadata->getColumnType( columnIndex ) )
  {
    case SQLDataTypes::Bit:
    case SQLDataTypes::Boolean:
      return QgsHanaUtils::toVariant( mResultSet->getBoolean( columnIndex ) );
    case SQLDataTypes::Char:
    {
      String str = mResultSet->getString( columnIndex );
      if ( mMetadata->getColumnLength( columnIndex ) == 1 )
      {
        if ( str.isNull() || str->empty() )
          return QVariant( QVariant::Char );
        else
          return QVariant( QChar( str->at( 0 ) ) );
      }
      else
        return QgsHanaUtils::toVariant( str );
    }
    case SQLDataTypes::WChar:
    {
      NString str = mResultSet->getNString( columnIndex );
      if ( mMetadata->getColumnLength( columnIndex ) == 1 )
      {
        if ( str.isNull() || str->empty() )
          return QVariant( QVariant::Char );
        else
          return QVariant( QChar( str->at( 0 ) ) );
      }
      else
        return QgsHanaUtils::toVariant( str );
    }
    case SQLDataTypes::TinyInt:
      if ( mMetadata ->isSigned( columnIndex ) )
        return QgsHanaUtils::toVariant( mResultSet->getByte( columnIndex ) );
      else
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
    case SQLDataTypes::Real:
      return QgsHanaUtils::toVariant( mResultSet->getFloat( columnIndex ) );
    case SQLDataTypes::Double:
    case SQLDataTypes::Decimal:
    case SQLDataTypes::Float:
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
    case 29812: /* ST_GEOMETRY, ST_POINT */
      return QgsHanaUtils::toVariant( mResultSet->getBinary( columnIndex ) );
    default:
      QgsDebugMsg( QStringLiteral( "Unhandled HANA type %1" ).arg( QString::fromStdU16String( mMetadata->getColumnTypeName( columnIndex ) ) ) );
      return QVariant();
  }
}

QgsGeometry QgsHanaResultSet::getGeometry( unsigned short columnIndex )
{
  auto toWkbSize = []( size_t size )
  {
    if ( size > static_cast<size_t>( std::numeric_limits<int>::max() ) )
      throw QgsHanaException( "Geometry size is larger than maximum integer value" );
    return  static_cast<int>( size );
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
