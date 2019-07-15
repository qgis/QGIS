/***************************************************************************
   qgshanautils.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsdatasourceuri.h"
#include "qgshanautils.h"

#include <QDate>
#include <QTime>
#include <QDateTime>

using namespace std;
using namespace odbc;

namespace
{
  QString escape( const QString &val, QChar delim = '\'' )
  {
    QString escaped = val;

    escaped.replace( '\\', QLatin1String( "\\\\" ) );
    escaped.replace( delim, QStringLiteral( "\\%1" ).arg( delim ) );

    return escaped;
  }
}

QString QgsHanaUtils::connectionInfo( const QgsDataSourceUri &uri )
{
  QStringList connectionItems;

  if ( !uri.database().isEmpty() )
    connectionItems << "dbname='" + escape( uri.database() ) + '\'';

  if ( !uri.host().isEmpty() )
    connectionItems << "host=" + uri.host();

  if ( !uri.port().isEmpty() )
    connectionItems << "port=" + uri.port();

  if ( !uri.driver().isEmpty() )
    connectionItems << "driver='" + escape( uri.driver() ) + '\'';

  if ( !uri.username().isEmpty() )
  {
    connectionItems << "user='" + escape( uri.username() ) + '\'';

    if ( !uri.password().isEmpty() )
      connectionItems << "password='" + escape( uri.password() ) + '\'';
  }

  if ( uri.hasParam( QStringLiteral( "encrypt" ) ) )
    connectionItems << "encrypt='" + uri.param( QStringLiteral( "encrypt" ) ) + '\'';

  if ( uri.hasParam( QStringLiteral( "sslCryptoProvider" ) ) )
    connectionItems << "sslCryptoProvider='" + uri.param( QStringLiteral( "sslCryptoProvider" ) ) + '\'';

  if ( uri.hasParam( QStringLiteral( "sslValidateCertificate" ) ) )
    connectionItems << "sslValidateCertificate='" + uri.param( QStringLiteral( "sslValidateCertificate" ) ) + '\'';

  if ( uri.hasParam( QStringLiteral( "sslHostNameInCertificate" ) ) )
    connectionItems << "sslHostNameInCertificate='" + uri.param( QStringLiteral( "sslHostNameInCertificate" ) ) + '\'';

  if ( uri.hasParam( QStringLiteral( "sslKeyStore" ) ) )
    connectionItems << "sslKeyStore='" + uri.param( QStringLiteral( "sslKeyStore" ) ) + '\'';

  if ( uri.hasParam( QStringLiteral( "sslTrustStore" ) ) )
    connectionItems << "sslTrustStore='" + uri.param( QStringLiteral( "sslTrustStore" ) ) + '\'';

  return connectionItems.join( QStringLiteral( " " ) );
}

QString QgsHanaUtils::quotedIdentifier( const QString &str )
{
  QString result = str;
  result.replace( '"', QLatin1String( "\"\"" ) );
  return result.prepend( '\"' ).append( '\"' );
}

QString QgsHanaUtils::quotedString( const QString &str )
{
  QString result = str;
  result.replace( '\'', QLatin1String( "''" ) );
  return result.prepend( '\'' ).append( '\'' );
}

QString QgsHanaUtils::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();
    case QVariant::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );
    case QVariant::String:
    default:
      return quotedString( value.toString() );
  }
}

QString QgsHanaUtils::toQString( const String &str )
{
  if ( str.isNull() )
    return QString();
  else
    return QString( str->c_str() );
}

QVariant QgsHanaUtils::toVariant( const Byte &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Int );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const UByte &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Char );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Short &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Int );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const UShort &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::UInt );
  else
    return QVariant( static_cast<uint>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Long &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::LongLong );
  else
    return QVariant( static_cast<qlonglong>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const ULong &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::ULongLong );
  else
    return QVariant( static_cast<qulonglong>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Date &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Date );
  else
    return QVariant( QDate( value->year(), value->month(), value->day() ) );
}

QVariant QgsHanaUtils::toVariant( const Time &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Time );
  else
    return QVariant( QTime( value->hour(), value->minute(), value->second(), 0 ) );
}

QVariant QgsHanaUtils::toVariant( const Timestamp &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::DateTime );
  else
    return QVariant( QDateTime( QDate( value->year(), value->month(), value->day() ),
                                QTime( value->hour(), value->minute(), value->second(), value->milliseconds() ) ) );
}

QVariant QgsHanaUtils::toVariant( const String &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::String );
  else
    return QVariant( QString::fromStdString( *value ) );
}

QVariant QgsHanaUtils::toVariant( const String &value, int type )
{
  bool isNull = value.isNull();
  switch ( type )
  {
    case SQLDataTypes::Boolean:
      if ( isNull )
        return QVariant( QVariant::Bool );
      else
        return QVariant( ( *value == "true" || *value == "1" ) ? true : false );
    case SQLDataTypes::TinyInt:
      if ( isNull )
        return QVariant( QVariant::Int );
      else
        return QVariant( atoi( value->c_str() ) );
    case SQLDataTypes::BigInt:
      if ( isNull )
        return QVariant( QVariant::LongLong );
      else
        return QVariant( ( qlonglong )atoll( value->c_str() ) );
    case SQLDataTypes::Numeric:
    case SQLDataTypes::Decimal:
      if ( isNull )
        return QVariant( QVariant::Double );
      else
        return QVariant( value->c_str() );
    case SQLDataTypes::Double:
      if ( isNull )
        return QVariant( QVariant::Double );
      else
        return QVariant( stod( value->c_str() ) );
    case SQLDataTypes::Float:
    case SQLDataTypes::Real:
      if ( isNull )
        return QVariant( QVariant::Double );
      else
        return QVariant( stof( value->c_str() ) );
    case SQLDataTypes::Char:
    case SQLDataTypes::WChar:
      if ( isNull )
        return QVariant( QVariant::Char );
      else
        return QVariant( QChar( value->operator[]( 0 ) ) );
    case SQLDataTypes::VarChar:
    case SQLDataTypes::WVarChar:
    case SQLDataTypes::LongVarChar:
    case SQLDataTypes::WLongVarChar:
      if ( isNull )
        return QVariant( QVariant::String );
      else
        return QVariant( value->c_str() );
    case SQLDataTypes::Binary:
    case SQLDataTypes::VarBinary:
      return QVariant( QByteArray( value->c_str(), ( int )value->length() ) );
    case SQLDataTypes::Date:
    case SQLDataTypes::TypeDate:
      return QVariant( QDate::fromString( QString( value->c_str() ) ) );
    case SQLDataTypes::Time:
    case SQLDataTypes::TypeTime:
      return QVariant( QTime::fromString( QString( value->c_str() ) ) );
    case SQLDataTypes::Timestamp:
    case SQLDataTypes::TypeTimestamp:
      return QVariant( QDateTime::fromString( QString( value->c_str() ) ) );
    default:
      return QVariant( QVariant::String );
  }
}

QVariant QgsHanaUtils::toVariant( const NString &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::String );
  else
    return QVariant( QString::fromStdU16String( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Binary &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::BitArray );
  else
    return QVariant( toQByteArray( value ) );
}

QByteArray QgsHanaUtils::toQByteArray( const Binary &value )
{
  if ( value.isNull() )
    return QByteArray();
  else
    return QByteArray( value->data(), static_cast<int>( value->size() ) );
}

QgsWkbTypes::Type QgsHanaUtils::toWkbType( const QString &hanaType )
{
  if ( hanaType == QStringLiteral( "ST_POINT" ) )
    return QgsWkbTypes::Type::Point;
  else if ( hanaType == QStringLiteral( "ST_MULTIPOINT" ) )
    return QgsWkbTypes::Type::MultiPoint;
  else if ( hanaType == QStringLiteral( "ST_LINESTRING" ) )
    return QgsWkbTypes::Type::LineString;
  else if ( hanaType == QStringLiteral( "ST_MULTILINESTRING" ) )
    return QgsWkbTypes::Type::MultiLineString;
  else if ( hanaType == QStringLiteral( "ST_POLYGON" ) )
    return QgsWkbTypes::Type::Polygon;
  else if ( hanaType == QStringLiteral( "ST_MULTIPOLYGON" ) )
    return QgsWkbTypes::Type::MultiPolygon;
  else if ( hanaType == QStringLiteral( "ST_GEOMETRYCOLLECTION" ) )
    return QgsWkbTypes::Type::GeometryCollection;
  else if ( hanaType == QStringLiteral( "ST_CIRCULARSTRING" ) )
    return QgsWkbTypes::Type::CircularString;

  return QgsWkbTypes::Type::Unknown;
}

QVersionNumber QgsHanaUtils::toHANAVersion( const QString &dbVersion )
{
  QString version = dbVersion;
  version.replace( " ", "." );
  QStringList strs = version.replace( " ", "." ).split( "." );

  if ( strs.length() < 3 )
    return QVersionNumber( 0 );

  int maj = strs[0].toInt();
  int min = strs[1].toInt();
  int rev = strs[2].toInt();
  return QVersionNumber( maj, min, rev );
}

constexpr int PLANAR_SRID_OFFSET = 1000000000;

int QgsHanaUtils::toPlanarSRID( int srid )
{
  return srid  < PLANAR_SRID_OFFSET ? PLANAR_SRID_OFFSET + srid : srid;
}

bool QgsHanaUtils::convertField( QgsField &field )
{
  QString fieldType = QStringLiteral( "NVARCHAR(5000)" ); //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QVariant::Bool:
      fieldType = QStringLiteral( "BOOLEAN" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QVariant::Char:
      fieldType = QStringLiteral( "TINYINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QVariant::LongLong:
      fieldType = QStringLiteral( "BIGINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QVariant::Date:
      fieldType = QStringLiteral( "DATE" );
      fieldPrec = -1;
      break;
    case QVariant::Time:
      fieldType = QStringLiteral( "TIME" );
      fieldPrec = -1;
      break;
    case QVariant::DateTime:
      fieldType = QStringLiteral( "TIMESTAMP" );
      fieldPrec = -1;
      break;
    case QVariant::String:
      if ( fieldSize > 0 )
      {
        if ( fieldSize <= 5000 )
          fieldType = QStringLiteral( "NVARCHAR(%1)" ).arg( QString::number( fieldSize ) );
        else
          fieldType = QStringLiteral( "NCLOB" );
      }
      else
        fieldType = QStringLiteral( "NVARCHAR(5000)" );
      fieldPrec = -1;
      break;
    case QVariant::Int:
      fieldType = QStringLiteral( "INTEGER" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = QStringLiteral( "DOUBLE" );
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        if ( field.length() > 0 && field.precision() >= 0 )
          fieldType = QStringLiteral( "DECIMAL(%2,%3)" ).arg( field.length(), field.precision() );
        else
          fieldType = QStringLiteral( "DECIMAL" );
      }
      break;
    case QVariant::BitArray:
      if ( fieldSize > 0 )
        fieldType = QStringLiteral( "BLOB(%1)" ).arg( QString::number( fieldSize ) );
      else
        fieldType = QStringLiteral( "BLOB" );
      break;
    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}

int QgsHanaUtils::countFieldsInUppercase( const QgsFields &fields )
{
  int count = 0;
  for ( int i = 0, n = fields.size(); i < n; ++i )
  {
    QString name = fields.at( i ).name();
    if ( name.isEmpty() )
      continue;
    if ( isupper( name.at( 0 ).toAscii() ) )
      ++count;
  }
  return count;
}

QString QgsHanaUtils::formatErrorMessage( const char *message, bool withPrefix )
{
  if ( message == nullptr )
    return QString();

  QString ret( message );
  const QString mark = QStringLiteral( "[HDBODBC] " );
  int pos = ret.indexOf( mark );
  if ( pos != -1 )
    ret = ret.remove( 0, pos + mark.length() );
  if ( withPrefix )
    return QStringLiteral( "HANA: " ) + ret;
  return ret;
}
