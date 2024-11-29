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
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgsvariantutils.h"

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
} // namespace

QString QgsHanaUtils::connectionInfo( const QgsDataSourceUri &uri )
{
  QStringList connectionItems;
  auto addItem = [&connectionItems]( const char *key, const QString &value, bool quoted = true ) {
    if ( quoted )
      connectionItems << QStringLiteral( "%1='%2'" ).arg( key, value );
    else
      connectionItems << QStringLiteral( "%1=%2" ).arg( key, value );
  };

  QgsHanaConnectionType connType = QgsHanaConnectionType::HostPort;
  if ( uri.hasParam( "connectionType" ) )
    connType = static_cast<QgsHanaConnectionType>( uri.param( "connectionType" ).toUInt() );

  addItem( "connectionType", QString::number( static_cast<uint>( connType ) ) );
  switch ( connType )
  {
    case QgsHanaConnectionType::Dsn:
      if ( uri.hasParam( "dsn" ) )
        addItem( "dsn", escape( uri.param( "dsn" ) ) );
      break;
    case QgsHanaConnectionType::HostPort:
      if ( !uri.database().isEmpty() )
        addItem( "dbname", escape( uri.database() ) );
      if ( !uri.host().isEmpty() )
        addItem( "host", escape( uri.host() ), false );
      if ( !uri.port().isEmpty() )
        addItem( "port", uri.port(), false );
      if ( !uri.driver().isEmpty() )
        addItem( "driver", escape( uri.driver() ) );
      break;
  }

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

  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();
    case QMetaType::Type::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );
    case QMetaType::Type::QString:
    default:
      return quotedString( value.toString() );
  }
}

QString QgsHanaUtils::toConstant( const QVariant &value, QMetaType::Type type )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( type )
  {
    case QMetaType::Type::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Double:
      return value.toString();
    case QMetaType::Type::QChar:
    case QMetaType::Type::QString:
      return QgsHanaUtils::quotedString( value.toString() );
    case QMetaType::Type::QDate:
      return QStringLiteral( "date'%1'" ).arg( value.toDate().toString( QStringLiteral( "yyyy-MM-dd" ) ) );
    case QMetaType::Type::QDateTime:
      return QStringLiteral( "timestamp'%1'" ).arg( value.toDateTime().toString( QStringLiteral( "yyyy-MM-dd hh:mm:ss.zzz" ) ) );
    case QMetaType::Type::QTime:
      return QStringLiteral( "time'%1'" ).arg( value.toTime().toString( QStringLiteral( "hh:mm:ss.zzz" ) ) );
    case QMetaType::Type::QByteArray:
      return QStringLiteral( "x'%1'" ).arg( QString( value.toByteArray().toHex() ) );
    default:
      return value.toString();
  }
}

QString QgsHanaUtils::toString( Qgis::DistanceUnit unit )
{
  // We need to translate the distance unit to the name used in HANA's
  // SYS.ST_UNITS_OF_MEASURE view. These names are different from the names
  // returned by QgsUnitTypes::encodeUnit(). Hence, we need our own translation
  // method.
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
      return QStringLiteral( "meter" );
    case Qgis::DistanceUnit::Kilometers:
      return QStringLiteral( "kilometer" );
    case Qgis::DistanceUnit::Feet:
      return QStringLiteral( "foot" );
    case Qgis::DistanceUnit::Yards:
      return QStringLiteral( "yard" );
    case Qgis::DistanceUnit::Miles:
      return QStringLiteral( "mile" );
    case Qgis::DistanceUnit::Degrees:
      return QStringLiteral( "degree" );
    case Qgis::DistanceUnit::Centimeters:
      return QStringLiteral( "centimeter" );
    case Qgis::DistanceUnit::Millimeters:
      return QStringLiteral( "millimeter" );
    case Qgis::DistanceUnit::NauticalMiles:
      return QStringLiteral( "nautical mile" );
    case Qgis::DistanceUnit::Inches:
      return QStringLiteral( "inch" );
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
    case Qgis::DistanceUnit::MilesUSSurvey:
    case Qgis::DistanceUnit::Fathoms:
    case Qgis::DistanceUnit::MetersGermanLegal:
    case Qgis::DistanceUnit::Unknown:
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
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Bool );
  else
    return QVariant( *value );
}

QVariant QgsHanaUtils::toVariant( const Byte &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const UByte &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::UInt );
  else
    return QVariant( static_cast<uint>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Short &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const UShort &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::UInt );
  else
    return QVariant( static_cast<uint>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Int &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  else
    return QVariant( static_cast<int>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const UInt &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::UInt );
  else
    return QVariant( static_cast<uint>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Long &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
  else
    return QVariant( static_cast<qlonglong>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const ULong &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::ULongLong );
  else
    return QVariant( static_cast<qulonglong>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Float &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  else
    return QVariant( static_cast<double>( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Double &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  else
    return QVariant( *value );
}

QVariant QgsHanaUtils::toVariant( const Date &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::QDate );
  else
    return QVariant( QDate( value->year(), value->month(), value->day() ) );
}

QVariant QgsHanaUtils::toVariant( const Time &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::QTime );
  else
    return QVariant( QTime( value->hour(), value->minute(), value->second(), 0 ) );
}

QVariant QgsHanaUtils::toVariant( const Timestamp &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::QDateTime );
  else
    return QVariant( QDateTime( QDate( value->year(), value->month(), value->day() ), QTime( value->hour(), value->minute(), value->second(), value->milliseconds() ) ) );
}

QVariant QgsHanaUtils::toVariant( const String &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::QString );
  else
    return QVariant( QString::fromUtf8( value->c_str() ) );
}

QVariant QgsHanaUtils::toVariant( const NString &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::QString );
  else
    return QVariant( QString::fromStdU16String( *value ) );
}

QVariant QgsHanaUtils::toVariant( const Binary &value )
{
  if ( value.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::QByteArray );

  if ( value->size() > static_cast<size_t>( std::numeric_limits<int>::max() ) )
    throw QgsHanaException( "Binary size is larger than maximum integer value" );

  return QByteArray( value->data(), static_cast<int>( value->size() ) );
}

const char16_t *QgsHanaUtils::toUtf16( const QString &sql )
{
  return reinterpret_cast<const char16_t *>( sql.utf16() );
}

bool QgsHanaUtils::isGeometryTypeSupported( Qgis::WkbType wkbType )
{
  switch ( QgsWkbTypes::flatType( wkbType ) )
  {
    case Qgis::WkbType::Point:
    case Qgis::WkbType::LineString:
    case Qgis::WkbType::Polygon:
    case Qgis::WkbType::MultiPoint:
    case Qgis::WkbType::MultiLineString:
    case Qgis::WkbType::MultiPolygon:
    case Qgis::WkbType::CircularString:
    case Qgis::WkbType::GeometryCollection:
      return true;
    default:
      return false;
  }
}

Qgis::WkbType QgsHanaUtils::toWkbType( const NS_ODBC::String &type, const NS_ODBC::Int &hasZ, const NS_ODBC::Int &hasM )
{
  if ( type.isNull() )
    return Qgis::WkbType::Unknown;

  const bool hasZValue = hasZ.isNull() ? false : *hasZ == 1;
  const bool hasMValue = hasM.isNull() ? false : *hasM == 1;
  const QString hanaType( type->c_str() );

  if ( hanaType == QLatin1String( "ST_POINT" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::Point, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_MULTIPOINT" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::MultiPoint, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_LINESTRING" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::LineString, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_MULTILINESTRING" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::MultiLineString, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_POLYGON" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::Polygon, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_MULTIPOLYGON" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::MultiPolygon, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_GEOMETRYCOLLECTION" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::GeometryCollection, hasZValue, hasMValue );
  else if ( hanaType == QLatin1String( "ST_CIRCULARSTRING" ) )
    return QgsWkbTypes::zmType( Qgis::WkbType::CircularString, hasZValue, hasMValue );
  return Qgis::WkbType::Unknown;
}

QVersionNumber QgsHanaUtils::toHANAVersion( const QString &dbVersion )
{
  QString version = dbVersion;
  QStringList strs = version.replace( '-', '.' ).replace( ' ', '.' ).split( '.' );

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
  return srid < PLANAR_SRID_OFFSET ? PLANAR_SRID_OFFSET + srid : srid;
}

bool QgsHanaUtils::convertField( QgsField &field )
{
  QString fieldType = QStringLiteral( "NVARCHAR(5000)" ); //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QMetaType::Type::Bool:
      fieldType = QStringLiteral( "BOOLEAN" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::Int:
      fieldType = QStringLiteral( "INTEGER" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::UInt:
      fieldType = QStringLiteral( "DECIMAL" );
      fieldSize = 10;
      fieldPrec = 0;
      break;
    case QMetaType::Type::LongLong:
      fieldType = QStringLiteral( "BIGINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::ULongLong:
      fieldType = QStringLiteral( "DECIMAL" );
      fieldSize = 20;
      fieldPrec = 0;
      break;
    case QMetaType::Type::QDate:
      fieldType = QStringLiteral( "DATE" );
      fieldPrec = -1;
      break;
    case QMetaType::Type::QTime:
      fieldType = QStringLiteral( "TIME" );
      fieldPrec = -1;
      break;
    case QMetaType::Type::QDateTime:
      fieldType = QStringLiteral( "TIMESTAMP" );
      fieldPrec = -1;
      break;
    case QMetaType::Type::Double:
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
    case QMetaType::Type::QChar:
      fieldType = QStringLiteral( "NCHAR(1)" );
      fieldSize = 1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::QString:
      if ( field.typeName() == QLatin1String( "REAL_VECTOR" ) )
      {
        if ( fieldSize > 0 )
          fieldType = QStringLiteral( "REAL_VECTOR(%1)" ).arg( QString::number( fieldSize ) );
        else
          fieldType = QStringLiteral( "REAL_VECTOR" );
      }
      else if ( field.typeName() == QLatin1String( "ST_GEOMETRY" ) )
      {
        QVariant srid = field.metadata( Qgis::FieldMetadataProperty::CustomProperty );
        if ( srid.isValid() && srid.toInt() >= 0 )
          fieldType = QStringLiteral( "ST_GEOMETRY(%1)" ).arg( QString::number( srid.toInt() ) );
        else
          fieldType = QStringLiteral( "ST_GEOMETRY" );
      }
      else
      {
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
      }
      break;
    case QMetaType::Type::QByteArray:
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
