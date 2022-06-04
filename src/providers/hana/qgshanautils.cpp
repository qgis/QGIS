/***************************************************************************
   qgshanautils.cpp
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
#include "qgsdatasourceuri.h"
#include "qgshanaexception.h"
#include "qgshanautils.h"

#include <QDate>
#include <QTime>
#include <QDateTime>

using namespace NS_ODBC;

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
  auto addItem = [&connectionItems]( const char *key, const QString & value, bool quoted = true )
  {
    if ( quoted )
      connectionItems << QStringLiteral( "%1='%2'" ).arg( key, value );
    else
      connectionItems << QStringLiteral( "%1=%2" ).arg( key, value );
  };

  if ( !uri.database().isEmpty() )
    addItem( "dbname", escape( uri.database() ) );
  if ( !uri.host().isEmpty() )
    addItem( "host", escape( uri.host() ), false );
  if ( !uri.port().isEmpty() )
    addItem( "port", uri.port(), false );
  if ( !uri.driver().isEmpty() )
    addItem( "driver", escape( uri.driver() ) );

  if ( !uri.username().isEmpty() )
  {
    addItem( "user", escape( uri.username() ) );
    if ( !uri.password().isEmpty() )
      addItem( "password", escape( uri.password() ) );
  }

  if ( uri.hasParam( QStringLiteral( "sslEnabled" ) ) )
  {
    addItem( "sslEnabled", uri.param( QStringLiteral( "sslEnabled" ) ) );
    if ( uri.hasParam( QStringLiteral( "sslCryptoProvider" ) ) )
      addItem( "sslCryptoProvider", uri.param( QStringLiteral( "sslCryptoProvider" ) ) );
    if ( uri.hasParam( QStringLiteral( "sslValidateCertificate" ) ) )
      addItem( "sslValidateCertificate", uri.param( QStringLiteral( "sslValidateCertificate" ) ) );
    if ( uri.hasParam( QStringLiteral( "sslHostNameInCertificate" ) ) )
      addItem( "sslHostNameInCertificate", uri.param( QStringLiteral( "sslHostNameInCertificate" ) ) );
    if ( uri.hasParam( QStringLiteral( "sslKeyStore" ) ) )
      addItem( "sslKeyStore", uri.param( QStringLiteral( "sslKeyStore" ) ) );
    if ( uri.hasParam( QStringLiteral( "sslTrustStore" ) ) )
      addItem( "sslTrustStore", uri.param( QStringLiteral( "sslTrustStore" ) ) );
  }

  return connectionItems.join( QLatin1Char( ' ' ) );
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

QString QgsHanaUtils::toConstant( const QVariant &value, QVariant::Type type )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( type )
  {
    case QVariant::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
      return value.toString();
    case QVariant::Char:
    case QVariant::String:
      return QgsHanaUtils::quotedString( value.toString() );
    case QVariant::Date:
      return QStringLiteral( "date'%1'" ).arg( value.toDate().toString( QStringLiteral( "yyyy-MM-dd" ) ) );
    case QVariant::DateTime:
      return QStringLiteral( "timestamp'%1'" ).arg( value.toDateTime().toString( QStringLiteral( "yyyy-MM-dd hh:mm:ss.zzz" ) ) );
    case QVariant::Time:
      return QStringLiteral( "time'%1'" ).arg( value.toTime().toString( QStringLiteral( "hh:mm:ss.zzz" ) ) );
    case QVariant::ByteArray:
      return QStringLiteral( "x'%1'" ).arg( QString( value.toByteArray().toHex() ) );
    default:
      return value.toString();
  }
}

QString QgsHanaUtils::toString( QgsUnitTypes::DistanceUnit unit )
{
  // We need to translate the distance unit to the name used in HANA's
  // SYS.ST_UNITS_OF_MEASURE view. These names are different from the names
  // returned by QgsUnitTypes::encodeUnit(). Hence, we need our own translation
  // method.
  switch ( unit )
  {
    case QgsUnitTypes::DistanceMeters:
      return QStringLiteral( "meter" );
    case QgsUnitTypes::DistanceKilometers:
      return QStringLiteral( "kilometer" );
    case QgsUnitTypes::DistanceFeet:
      return QStringLiteral( "foot" );
    case QgsUnitTypes::DistanceYards:
      return QStringLiteral( "yard" );
    case QgsUnitTypes::DistanceMiles:
      return QStringLiteral( "mile" );
    case QgsUnitTypes::DistanceDegrees:
      return QStringLiteral( "degree" );
    case QgsUnitTypes::DistanceCentimeters:
      return QStringLiteral( "centimeter" );
    case QgsUnitTypes::DistanceMillimeters:
      return QStringLiteral( "millimeter" );
    case QgsUnitTypes::DistanceNauticalMiles:
      return QStringLiteral( "nautical mile" );
    case QgsUnitTypes::DistanceUnknownUnit:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

QString QgsHanaUtils::toQString( const NString &str )
{
  if ( str.isNull() )
    return QString();
  else
    return QString::fromStdU16String( *str );
}

QString QgsHanaUtils::toQString( const String &str )
{
  if ( str.isNull() )
    return QString();
  else
    return QString::fromUtf8( str->c_str() );
}

QVariant QgsHanaUtils::toVariant( const Boolean &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Bool );
  else
    return QVariant( *value );
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
    return QVariant( QVariant::UInt );
  else
    return QVariant( static_cast<uint>( *value ) );
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

QVariant QgsHanaUtils::toVariant( const Int &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Int );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const UInt &value )
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

QVariant QgsHanaUtils::toVariant( const Float &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Double );
  else
    return QVariant( static_cast<double>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Double &value )
{
  if ( value.isNull() )
    return QVariant( QVariant::Double );
  else
    return QVariant( *value );
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
    return QVariant( QString::fromUtf8( value->c_str() ) );
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
    return QVariant( QVariant::ByteArray );

  if ( value->size() > static_cast<size_t>( std::numeric_limits<int>::max() ) )
    throw QgsHanaException( "Binary size is larger than maximum integer value" );

  return QByteArray( value->data(), static_cast<int>( value->size() ) );
}

const char16_t *QgsHanaUtils::toUtf16( const QString &sql )
{
  return reinterpret_cast<const char16_t *>( sql.utf16() );
}

bool QgsHanaUtils::isGeometryTypeSupported( QgsWkbTypes::Type wkbType )
{
  switch ( QgsWkbTypes::flatType( wkbType ) )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::CircularString:
    case QgsWkbTypes::GeometryCollection:
      return true;
    default:
      return false;
  }
}

QgsWkbTypes::Type QgsHanaUtils::toWkbType( const String &type, const Int &hasZ, const Int &hasM )
{
  if ( type.isNull() )
    return QgsWkbTypes::Unknown;

  const bool hasZValue = hasZ.isNull() ? false : *hasZ == 1;
  const bool hasMValue = hasM.isNull() ? false : *hasM == 1;
  const QString hanaType( type->c_str() );

  if ( hanaType == QLatin1String( "ST_POINT" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::Point, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_MULTIPOINT" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::MultiPoint, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_LINESTRING" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::LineString, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_MULTILINESTRING" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::MultiLineString, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_POLYGON" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::Polygon, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_MULTIPOLYGON" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::MultiPolygon, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_GEOMETRYCOLLECTION" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::GeometryCollection, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_CIRCULARSTRING" ) )
    return QgsWkbTypes::zmType( QgsWkbTypes::CircularString, hasZValue, hasMValue );
  return QgsWkbTypes::Type::Unknown;
}

QVersionNumber QgsHanaUtils::toHANAVersion( const QString &dbVersion )
{
  QString version = dbVersion;
  QStringList strs = version.replace( ' ', '.' ).split( '.' );

  if ( strs.length() < 3 )
    return QVersionNumber( 0 );

  const int maj = strs[0].toInt();
  const int min = strs[1].toInt();
  const int rev = strs[2].toInt();
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
    case QVariant::Int:
      fieldType = QStringLiteral( "INTEGER" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QVariant::UInt:
      fieldType = QStringLiteral( "DECIMAL" );
      fieldSize = 10;
      fieldPrec = 0;
      break;
    case QVariant::LongLong:
      fieldType = QStringLiteral( "BIGINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QVariant::ULongLong:
      fieldType = QStringLiteral( "DECIMAL" );
      fieldSize = 20;
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
    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = QStringLiteral( "DOUBLE" );
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = QStringLiteral( "DECIMAL(%1,%2)" ).arg( QString::number( fieldSize ), QString::number( fieldPrec ) );
      }
      break;
    case QVariant::Char:
      fieldType = QStringLiteral( "NCHAR(1)" );
      fieldSize = 1;
      fieldPrec = 0;
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
    case QVariant::ByteArray:
      if ( fieldSize >= 1 && fieldSize <= 5000 )
        fieldType = QStringLiteral( "VARBINARY(%1)" ).arg( QString::number( fieldSize ) );
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

int QgsHanaUtils::countFieldsWithFirstLetterInUppercase( const QgsFields &fields )
{
  int count = 0;
  for ( int i = 0, n = fields.size(); i < n; ++i )
  {
    const QString name = fields.at( i ).name();
    if ( name.isEmpty() )
      continue;
    if ( name.at( 0 ).isUpper() )
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
  const int pos = ret.indexOf( mark );
  if ( pos != -1 )
    ret = ret.remove( 0, pos + mark.length() );
  if ( withPrefix && ret.indexOf( QLatin1String( "HANA" ) ) == -1 )
    return QStringLiteral( "SAP HANA: " ) + ret;
  return ret;
}
