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
#include "qgshanautils.h"

#include "qgsdatasourceuri.h"
#include "qgshanaexception.h"
#include "qgshanasettings.h"
#include "qgsvariantutils.h"

#include <QDate>
#include <QDateTime>
#include <QTime>

using namespace NS_ODBC;

namespace
{
  QString escape( const QString &val, QChar delim = '\'' )
  {
    QString escaped = val;

    escaped.replace( '\\', "\\\\"_L1 );
    escaped.replace( delim, u"\\%1"_s.arg( delim ) );

    return escaped;
  }
} // namespace

QString QgsHanaUtils::connectionInfo( const QgsDataSourceUri &uri )
{
  QStringList connectionItems;
  auto addItem = [&connectionItems]( const char *key, const QString &value, bool quoted = true ) {
    if ( quoted )
      connectionItems << u"%1='%2'"_s.arg( key, value );
    else
      connectionItems << u"%1=%2"_s.arg( key, value );
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

  if ( uri.hasParam( u"sslEnabled"_s ) )
  {
    addItem( "sslEnabled", uri.param( u"sslEnabled"_s ) );
    if ( uri.hasParam( u"sslCryptoProvider"_s ) )
      addItem( "sslCryptoProvider", uri.param( u"sslCryptoProvider"_s ) );
    if ( uri.hasParam( u"sslValidateCertificate"_s ) )
      addItem( "sslValidateCertificate", uri.param( u"sslValidateCertificate"_s ) );
    if ( uri.hasParam( u"sslHostNameInCertificate"_s ) )
      addItem( "sslHostNameInCertificate", uri.param( u"sslHostNameInCertificate"_s ) );
    if ( uri.hasParam( u"sslKeyStore"_s ) )
      addItem( "sslKeyStore", uri.param( u"sslKeyStore"_s ) );
    if ( uri.hasParam( u"sslTrustStore"_s ) )
      addItem( "sslTrustStore", uri.param( u"sslTrustStore"_s ) );
  }

  return connectionItems.join( ' '_L1 );
}

QString QgsHanaUtils::quotedIdentifier( const QString &str )
{
  QString result = str;
  result.replace( '"', "\"\""_L1 );
  return result.prepend( '\"' ).append( '\"' );
}

QString QgsHanaUtils::quotedString( const QString &str )
{
  QString result = str;
  result.replace( '\'', "''"_L1 );
  return result.prepend( '\'' ).append( '\'' );
}

QString QgsHanaUtils::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return u"NULL"_s;

  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();
    case QMetaType::Type::Bool:
      return value.toBool() ? u"TRUE"_s : u"FALSE"_s;
    case QMetaType::Type::QString:
    default:
      return quotedString( value.toString() );
  }
}

QString QgsHanaUtils::toConstant( const QVariant &value, QMetaType::Type type )
{
  if ( value.isNull() )
    return u"NULL"_s;

  switch ( type )
  {
    case QMetaType::Type::Bool:
      return value.toBool() ? u"TRUE"_s : u"FALSE"_s;
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
      return u"date'%1'"_s.arg( value.toDate().toString( u"yyyy-MM-dd"_s ) );
    case QMetaType::Type::QDateTime:
      return u"timestamp'%1'"_s.arg( value.toDateTime().toString( u"yyyy-MM-dd hh:mm:ss.zzz"_s ) );
    case QMetaType::Type::QTime:
      return u"time'%1'"_s.arg( value.toTime().toString( u"hh:mm:ss.zzz"_s ) );
    case QMetaType::Type::QByteArray:
      return u"x'%1'"_s.arg( QString( value.toByteArray().toHex() ) );
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
      return u"meter"_s;
    case Qgis::DistanceUnit::Kilometers:
      return u"kilometer"_s;
    case Qgis::DistanceUnit::Feet:
      return u"foot"_s;
    case Qgis::DistanceUnit::Yards:
      return u"yard"_s;
    case Qgis::DistanceUnit::Miles:
      return u"mile"_s;
    case Qgis::DistanceUnit::Degrees:
      return u"degree"_s;
    case Qgis::DistanceUnit::Centimeters:
      return u"centimeter"_s;
    case Qgis::DistanceUnit::Millimeters:
      return u"millimeter"_s;
    case Qgis::DistanceUnit::NauticalMiles:
      return u"nautical mile"_s;
    case Qgis::DistanceUnit::Inches:
      return u"inch"_s;
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
      return u"<unknown>"_s;
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

  if ( hanaType == "ST_POINT"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::Point, hasZValue, hasMValue );
  else if ( hanaType == "ST_MULTIPOINT"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::MultiPoint, hasZValue, hasMValue );
  else if ( hanaType == "ST_LINESTRING"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::LineString, hasZValue, hasMValue );
  else if ( hanaType == "ST_MULTILINESTRING"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::MultiLineString, hasZValue, hasMValue );
  else if ( hanaType == "ST_POLYGON"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::Polygon, hasZValue, hasMValue );
  else if ( hanaType == "ST_MULTIPOLYGON"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::MultiPolygon, hasZValue, hasMValue );
  else if ( hanaType == "ST_GEOMETRYCOLLECTION"_L1 )
    return QgsWkbTypes::zmType( Qgis::WkbType::GeometryCollection, hasZValue, hasMValue );
  else if ( hanaType == "ST_CIRCULARSTRING"_L1 )
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
  QString fieldType = u"NVARCHAR(5000)"_s; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QMetaType::Type::Bool:
      fieldType = u"BOOLEAN"_s;
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::Int:
      fieldType = u"INTEGER"_s;
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::UInt:
      fieldType = u"DECIMAL"_s;
      fieldSize = 10;
      fieldPrec = 0;
      break;
    case QMetaType::Type::LongLong:
      fieldType = u"BIGINT"_s;
      fieldSize = -1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::ULongLong:
      fieldType = u"DECIMAL"_s;
      fieldSize = 20;
      fieldPrec = 0;
      break;
    case QMetaType::Type::QDate:
      fieldType = u"DATE"_s;
      fieldPrec = -1;
      break;
    case QMetaType::Type::QTime:
      fieldType = u"TIME"_s;
      fieldPrec = -1;
      break;
    case QMetaType::Type::QDateTime:
      fieldType = u"TIMESTAMP"_s;
      fieldPrec = -1;
      break;
    case QMetaType::Type::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = u"DOUBLE"_s;
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = u"DECIMAL(%1,%2)"_s.arg( QString::number( fieldSize ), QString::number( fieldPrec ) );
      }
      break;
    case QMetaType::Type::QChar:
      fieldType = u"NCHAR(1)"_s;
      fieldSize = 1;
      fieldPrec = 0;
      break;
    case QMetaType::Type::QString:
      if ( field.typeName() == "REAL_VECTOR"_L1 )
      {
        if ( fieldSize > 0 )
          fieldType = u"REAL_VECTOR(%1)"_s.arg( QString::number( fieldSize ) );
        else
          fieldType = u"REAL_VECTOR"_s;
      }
      else if ( field.typeName() == "ST_GEOMETRY"_L1 )
      {
        QVariant srid = field.metadata( Qgis::FieldMetadataProperty::CustomProperty );
        if ( srid.isValid() && srid.toInt() >= 0 )
          fieldType = u"ST_GEOMETRY(%1)"_s.arg( QString::number( srid.toInt() ) );
        else
          fieldType = u"ST_GEOMETRY"_s;
      }
      else
      {
        if ( fieldSize > 0 )
        {
          if ( fieldSize <= 5000 )
            fieldType = u"NVARCHAR(%1)"_s.arg( QString::number( fieldSize ) );
          else
            fieldType = u"NCLOB"_s;
        }
        else
          fieldType = u"NVARCHAR(5000)"_s;
        fieldPrec = -1;
      }
      break;
    case QMetaType::Type::QByteArray:
      if ( fieldSize >= 1 && fieldSize <= 5000 )
        fieldType = u"VARBINARY(%1)"_s.arg( QString::number( fieldSize ) );
      else
        fieldType = u"BLOB"_s;
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
  const QString mark = u"[HDBODBC] "_s;
  const int pos = ret.indexOf( mark );
  if ( pos != -1 )
    ret = ret.remove( 0, pos + mark.length() );
  if ( withPrefix && ret.indexOf( "HANA"_L1 ) == -1 )
    return u"SAP HANA: "_s + ret;
  return ret;
}
