/***************************************************************************
                             qgsogrutils.cpp
                             ---------------
    begin                : February 2016
    copyright            : (C) 2016 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgsfields.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgslinesymbollayer.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"
#include "qgsmapinfosymbolconverter.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsfontutils.h"
#include "qgsmessagelog.h"
#include "qgssymbol.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfielddomain.h"
#include "qgsfontmanager.h"
#include "qgsvariantutils.h"
#include "qgsogrproviderutils.h"

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
#include "qgsweakrelation.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#endif

#include <cmath>
#include <limits>
#include <QTextCodec>
#include <QUuid>
#include <cpl_error.h>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDataStream>
#include <QRegularExpression>

#include "ogr_srs_api.h"


void gdal::OGRDataSourceDeleter::operator()( OGRDataSourceH source ) const
{
  OGR_DS_Destroy( source );
}


void gdal::OGRGeometryDeleter::operator()( OGRGeometryH geometry ) const
{
  OGR_G_DestroyGeometry( geometry );
}

void gdal::OGRFldDeleter::operator()( OGRFieldDefnH definition ) const
{
  OGR_Fld_Destroy( definition );
}

void gdal::OGRFeatureDeleter::operator()( OGRFeatureH feature ) const
{
  OGR_F_Destroy( feature );
}

void gdal::GDALDatasetCloser::operator()( GDALDatasetH dataset ) const
{
  GDALClose( dataset );
}

void gdal::fast_delete_and_close( gdal::dataset_unique_ptr &dataset, GDALDriverH driver, const QString &path )
{
  // see https://github.com/qgis/QGIS/commit/d024910490a39e65e671f2055c5b6543e06c7042#commitcomment-25194282
  // faster if we close the handle AFTER delete, but doesn't work for windows
#ifdef Q_OS_WIN
  // close dataset handle
  dataset.reset();
#endif

  CPLPushErrorHandler( CPLQuietErrorHandler );
  GDALDeleteDataset( driver, path.toUtf8().constData() );
  CPLPopErrorHandler();

#ifndef Q_OS_WIN
  // close dataset handle
  dataset.reset();
#endif
}


void gdal::GDALWarpOptionsDeleter::operator()( GDALWarpOptions *options ) const
{
  GDALDestroyWarpOptions( options );
}

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
void gdal::GDALRelationshipDeleter::operator()( GDALRelationshipH relationship ) const
{
  GDALDestroyRelationship( relationship );
}
#endif

static void setQTTimeZoneFromOGRTZFlag( QDateTime &dt, int nTZFlag )
{
  // Take into account time zone
  if ( nTZFlag == 0 )
  {
    // unknown time zone
  }
  else if ( nTZFlag == 1 )
  {
    dt.setTimeSpec( Qt::LocalTime );
  }
  else if ( nTZFlag == 100 )
  {
    dt.setTimeSpec( Qt::UTC );
  }
  else
  {
    // TZFlag = 101 ==> UTC+00:15
    // TZFlag = 99 ==> UTC-00:15
    dt.setOffsetFromUtc( ( nTZFlag - 100 ) * 15 * 60 );
  }
}

QVariant QgsOgrUtils::OGRFieldtoVariant( const OGRField *value, OGRFieldType type )
{
  if ( !value || OGR_RawField_IsUnset( value ) || OGR_RawField_IsNull( value ) )
    return QVariant();

  switch ( type )
  {
    case OFTInteger:
      return value->Integer;

    case OFTInteger64:
      return value->Integer64;

    case OFTReal:
      return value->Real;

    case OFTString:
    case OFTWideString:
      return QString::fromUtf8( value->String );

    case OFTDate:
      return QDate( value->Date.Year, value->Date.Month, value->Date.Day );

    case OFTTime:
    {
      float secondsPart = 0;
      float millisecondPart = std::modf( value->Date.Second, &secondsPart );
      return QTime( value->Date.Hour, value->Date.Minute, static_cast< int >( secondsPart ), static_cast< int >( std::round( 1000 * millisecondPart ) ) );
    }

    case OFTDateTime:
    {
      float secondsPart = 0;
      float millisecondPart = std::modf( value->Date.Second, &secondsPart );
      QDateTime dt = QDateTime( QDate( value->Date.Year, value->Date.Month, value->Date.Day ),
                                QTime( value->Date.Hour, value->Date.Minute, static_cast< int >( secondsPart ), static_cast< int >( std::round( 1000 * millisecondPart ) ) ) );
      setQTTimeZoneFromOGRTZFlag( dt, value->Date.TZFlag );
      return dt;
    }

    case OFTBinary:
      // not supported!
      Q_ASSERT_X( false, "QgsOgrUtils::OGRFieldtoVariant", "OFTBinary type not supported" );
      return QVariant();

    case OFTIntegerList:
    {
      QVariantList res;
      res.reserve( value->IntegerList.nCount );
      for ( int i = 0; i < value->IntegerList.nCount; ++i )
        res << value->IntegerList.paList[ i ];
      return res;
    }

    case OFTInteger64List:
    {
      QVariantList res;
      res.reserve( value->Integer64List.nCount );
      for ( int i = 0; i < value->Integer64List.nCount; ++i )
        res << value->Integer64List.paList[ i ];
      return res;
    }

    case OFTRealList:
    {
      QVariantList res;
      res.reserve( value->RealList.nCount );
      for ( int i = 0; i < value->RealList.nCount; ++i )
        res << value->RealList.paList[ i ];
      return res;
    }

    case OFTStringList:
    case OFTWideStringList:
    {
      QVariantList res;
      res.reserve( value->StringList.nCount );
      for ( int i = 0; i < value->StringList.nCount; ++i )
        res << QString::fromUtf8( value->StringList.paList[ i ] );
      return res;
    }
  }
  return QVariant();
}

int QgsOgrUtils::OGRTZFlagFromQt( const QDateTime &datetime )
{
  if ( datetime.timeSpec() == Qt::LocalTime )
    return 1;
  return 100 + datetime.offsetFromUtc() / ( 60 * 15 );
}

std::unique_ptr< OGRField > QgsOgrUtils::variantToOGRField( const QVariant &value, OGRFieldType type )
{
  std::unique_ptr< OGRField > res = std::make_unique< OGRField >();

  switch ( value.userType() )
  {
    case QMetaType::Type::UnknownType:
      OGR_RawField_SetUnset( res.get() );
      break;
    case QMetaType::Type::Bool:
    {
      const int val = value.toBool() ? 1 : 0;
      if ( type == OFTInteger )
        res->Integer = val;
      else if ( type == OFTInteger64 )
        res->Integer64 = val;
      else if ( type == OFTReal )
        res->Real = val;
      else
      {
        QgsDebugError( "Unsupported output data type for Bool" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::Int:
    {
      const int val = value.toInt();
      if ( type == OFTInteger )
        res->Integer = val;
      else if ( type == OFTInteger64 )
        res->Integer64 = val;
      else if ( type == OFTReal )
        res->Real = val;
      else
      {
        QgsDebugError( "Unsupported output data type for Int" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::LongLong:
    {
      const qint64 val = value.toLongLong();
      if ( type == OFTInteger )
      {
        if ( val <= std::numeric_limits<int>::max() &&
             val >= std::numeric_limits<int>::min() )
        {
          res->Integer = static_cast<int>( val );
        }
        else
        {
          QgsDebugError( "Value does not fit on Integer" );
          return nullptr;
        }
      }
      else if ( type == OFTInteger64 )
        res->Integer64 = val;
      else if ( type == OFTReal )
      {
        res->Real = static_cast<double>( val );
      }
      else
      {
        QgsDebugError( "Unsupported output data type for LongLong" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::Double:
    {
      double val = value.toDouble();
      if ( type == OFTInteger )
      {
        if ( val <= std::numeric_limits<int>::max() &&
             val >= std::numeric_limits<int>::min() )
        {
          res->Integer = static_cast<int>( val );
        }
        else
        {
          QgsDebugError( "Value does not fit on Integer" );
          return nullptr;
        }
      }
      else if ( type == OFTInteger64 )
      {
        if ( val <= static_cast<double>( std::numeric_limits<qint64>::max() ) &&
             val >= static_cast<double>( std::numeric_limits<qint64>::min() ) )
        {
          res->Integer64 = static_cast<qint64>( val );
        }
        else
        {
          QgsDebugError( "Value does not fit on Integer64" );
          return nullptr;
        }
      }
      else if ( type == OFTReal )
      {
        res->Real = val;
      }
      else
      {
        QgsDebugError( "Unsupported output data type for LongLong" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::QChar:
    case QMetaType::Type::QString:
    {
      if ( type == OFTString )
        res->String = CPLStrdup( value.toString().toUtf8().constData() );
      else
      {
        QgsDebugError( "Unsupported output data type for String" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::QDate:
    {
      if ( type == OFTDate )
      {
        const QDate date = value.toDate();
        res->Date.Day = date.day();
        res->Date.Month = date.month();
        res->Date.Year = static_cast<GInt16>( date.year() );
        res->Date.TZFlag = 0;
      }
      else
      {
        QgsDebugError( "Unsupported output data type for Date" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::QTime:
    {
      if ( type == OFTTime )
      {
        const QTime time = value.toTime();
        res->Date.Hour = time.hour();
        res->Date.Minute = time.minute();
        res->Date.Second = static_cast<float>( time.second() + static_cast< double >( time.msec() ) / 1000 );
        res->Date.TZFlag = 0;
      }
      else
      {
        QgsDebugError( "Unsupported output data type for Time" );
        return nullptr;
      }
      break;
    }
    case QMetaType::Type::QDateTime:
    {
      if ( type == OFTDateTime )
      {
        const QDateTime dt = value.toDateTime();
        const QDate date = dt.date();
        res->Date.Day = date.day();
        res->Date.Month = date.month();
        res->Date.Year = static_cast<GInt16>( date.year() );
        const QTime time = dt.time();
        res->Date.Hour = time.hour();
        res->Date.Minute = time.minute();
        res->Date.Second = static_cast<float>( time.second() + static_cast< double >( time.msec() ) / 1000 );
        res->Date.TZFlag = OGRTZFlagFromQt( dt );
      }
      else
      {
        QgsDebugError( "Unsupported output data type for DateTime" );
        return nullptr;
      }
      break;
    }

    default:
      QgsDebugError( "Unhandled variant type in variantToOGRField" );
      OGR_RawField_SetUnset( res.get() );
      break;
  }

  return res;
}

QgsFeature QgsOgrUtils::readOgrFeature( OGRFeatureH ogrFet, const QgsFields &fields, QTextCodec *encoding )
{
  QgsFeature feature;
  if ( !ogrFet )
  {
    feature.setValid( false );
    return feature;
  }

  feature.setId( OGR_F_GetFID( ogrFet ) );
  feature.setValid( true );

  if ( !readOgrFeatureGeometry( ogrFet, feature ) )
  {
    feature.setValid( false );
  }

  if ( !readOgrFeatureAttributes( ogrFet, fields, feature, encoding ) )
  {
    feature.setValid( false );
  }

  return feature;
}

QgsFields QgsOgrUtils::readOgrFields( OGRFeatureH ogrFet, QTextCodec *encoding )
{
  QgsFields fields;

  if ( !ogrFet )
    return fields;

  int fieldCount = OGR_F_GetFieldCount( ogrFet );
  for ( int i = 0; i < fieldCount; ++i )
  {
    OGRFieldDefnH fldDef = OGR_F_GetFieldDefnRef( ogrFet, i );
    if ( !fldDef )
    {
      fields.append( QgsField() );
      continue;
    }

    QString name = encoding ? encoding->toUnicode( OGR_Fld_GetNameRef( fldDef ) ) : QString::fromUtf8( OGR_Fld_GetNameRef( fldDef ) );
    QMetaType::Type varType;
    switch ( OGR_Fld_GetType( fldDef ) )
    {
      case OFTInteger:
        if ( OGR_Fld_GetSubType( fldDef ) == OFSTBoolean )
          varType = QMetaType::Type::Bool;
        else
          varType = QMetaType::Type::Int;
        break;
      case OFTInteger64:
        varType = QMetaType::Type::LongLong;
        break;
      case OFTReal:
        varType = QMetaType::Type::Double;
        break;
      case OFTDate:
        varType = QMetaType::Type::QDate;
        break;
      case OFTTime:
        varType = QMetaType::Type::QTime;
        break;
      case OFTDateTime:
        varType = QMetaType::Type::QDateTime;
        break;
      case OFTString:
        if ( OGR_Fld_GetSubType( fldDef ) == OFSTJSON )
          varType = QMetaType::Type::QVariantMap;
        else
          varType = QMetaType::Type::QString;
        break;
      default:
        varType = QMetaType::Type::QString; // other unsupported, leave it as a string
    }
    fields.append( QgsField( name, varType ) );
  }
  return fields;
}


QVariant QgsOgrUtils::getOgrFeatureAttribute( OGRFeatureH ogrFet, const QgsFields &fields, int attIndex, QTextCodec *encoding, bool *ok )
{
  if ( attIndex < 0 || attIndex >= fields.count() )
  {
    if ( ok )
      *ok = false;
    return QVariant();
  }

  const QgsField field = fields.at( attIndex );
  return getOgrFeatureAttribute( ogrFet, field, attIndex, encoding, ok );
}

QVariant QgsOgrUtils::getOgrFeatureAttribute( OGRFeatureH ogrFet, const QgsField &field, int attIndex, QTextCodec *encoding, bool *ok )
{
  if ( !ogrFet || attIndex < 0 )
  {
    if ( ok )
      *ok = false;
    return QVariant();
  }

  OGRFieldDefnH fldDef = OGR_F_GetFieldDefnRef( ogrFet, attIndex );

  if ( ! fldDef )
  {
    if ( ok )
      *ok = false;

    QgsDebugError( QStringLiteral( "ogrFet->GetFieldDefnRef(attindex) returns NULL" ) );
    return QVariant();
  }

  QVariant value;

  if ( ok )
    *ok = true;

  if ( OGR_F_IsFieldSetAndNotNull( ogrFet, attIndex ) )
  {
    switch ( field.type() )
    {
      case QMetaType::Type::QString:
      {
        if ( encoding )
          value = QVariant( encoding->toUnicode( OGR_F_GetFieldAsString( ogrFet, attIndex ) ) );
        else
          value = QVariant( QString::fromUtf8( OGR_F_GetFieldAsString( ogrFet, attIndex ) ) );

#ifdef Q_OS_WIN
        // Fixes GH #41076 (empty strings shown as NULL), because we have checked before that it was NOT NULL
        // Note:  QVariant( QString( ) ).isNull( ) is still true on windows so we really need string literal :(
        if ( value.isNull() )
          value = QVariant( QStringLiteral( "" ) ); // skip-keyword-check
#endif

        break;
      }
      case QMetaType::Type::Int:
        value = QVariant( OGR_F_GetFieldAsInteger( ogrFet, attIndex ) );
        break;
      case QMetaType::Type::Bool:
        value = QVariant( bool( OGR_F_GetFieldAsInteger( ogrFet, attIndex ) ) );
        break;
      case QMetaType::Type::LongLong:
        value = QVariant( OGR_F_GetFieldAsInteger64( ogrFet, attIndex ) );
        break;
      case QMetaType::Type::Double:
        value = QVariant( OGR_F_GetFieldAsDouble( ogrFet, attIndex ) );
        break;
      case QMetaType::Type::QDate:
      case QMetaType::Type::QDateTime:
      case QMetaType::Type::QTime:
      {
        int year, month, day, hour, minute, tzf;
        float second;
        float secondsPart = 0;

        OGR_F_GetFieldAsDateTimeEx( ogrFet, attIndex, &year, &month, &day, &hour, &minute, &second, &tzf );
        float millisecondPart = std::modf( second, &secondsPart );

        if ( field.type() == QMetaType::Type::QDate )
          value = QDate( year, month, day );
        else if ( field.type() == QMetaType::Type::QTime )
          value = QTime( hour, minute, static_cast< int >( secondsPart ), static_cast< int >( std::round( 1000 * millisecondPart ) ) );
        else
        {
          QDateTime dt = QDateTime( QDate( year, month, day ),
                                    QTime( hour, minute, static_cast< int >( secondsPart ), static_cast< int >( std::round( 1000 * millisecondPart ) ) ) );
          setQTTimeZoneFromOGRTZFlag( dt, tzf );
          value = dt;
        }
      }
      break;

      case QMetaType::Type::QByteArray:
      {
        int size = 0;
        const GByte *b = OGR_F_GetFieldAsBinary( ogrFet, attIndex, &size );

        // QByteArray::fromRawData is funny. It doesn't take ownership of the data, so we have to explicitly call
        // detach on it to force a copy which owns the data
        QByteArray ba = QByteArray::fromRawData( reinterpret_cast<const char *>( b ), size );
        ba.detach();

        value = ba;
        break;
      }

      case QMetaType::Type::QStringList:
      {
        QStringList list;
        char **lst = OGR_F_GetFieldAsStringList( ogrFet, attIndex );
        const int count = CSLCount( lst );
        if ( count > 0 )
        {
          list.reserve( count );
          for ( int i = 0; i < count; i++ )
          {
            if ( encoding )
              list << encoding->toUnicode( lst[i] );
            else
              list << QString::fromUtf8( lst[i] );
          }
        }
        value = list;
        break;
      }

      case QMetaType::Type::QVariantList:
      {
        switch ( field.subType() )
        {
          case QMetaType::Type::QString:
          {
            QStringList list;
            char **lst = OGR_F_GetFieldAsStringList( ogrFet, attIndex );
            const int count = CSLCount( lst );
            if ( count > 0 )
            {
              list.reserve( count );
              for ( int i = 0; i < count; i++ )
              {
                if ( encoding )
                  list << encoding->toUnicode( lst[i] );
                else
                  list << QString::fromUtf8( lst[i] );
              }
            }
            value = list;
            break;
          }

          case QMetaType::Type::Int:
          {
            QVariantList list;
            int count = 0;
            const int *lst = OGR_F_GetFieldAsIntegerList( ogrFet, attIndex, &count );
            if ( count > 0 )
            {
              list.reserve( count );
              for ( int i = 0; i < count; i++ )
              {
                list << lst[i];
              }
            }
            value = list;
            break;
          }

          case QMetaType::Type::Double:
          {
            QVariantList list;
            int count = 0;
            const double *lst = OGR_F_GetFieldAsDoubleList( ogrFet, attIndex, &count );
            if ( count > 0 )
            {
              list.reserve( count );
              for ( int i = 0; i < count; i++ )
              {
                list << lst[i];
              }
            }
            value = list;
            break;
          }

          case QMetaType::Type::LongLong:
          {
            QVariantList list;
            int count = 0;
            const long long *lst = OGR_F_GetFieldAsInteger64List( ogrFet, attIndex, &count );
            if ( count > 0 )
            {
              list.reserve( count );
              for ( int i = 0; i < count; i++ )
              {
                list << lst[i];
              }
            }
            value = list;
            break;
          }

          default:
          {
            Q_ASSERT_X( false, "QgsOgrUtils::getOgrFeatureAttribute", "unsupported field type" );
            if ( ok )
              *ok = false;
            break;
          }
        }
        break;
      }

      case QMetaType::Type::QVariantMap:
      {
        //it has to be JSON
        //it's null if no json format
        if ( encoding )
          value = QJsonDocument::fromJson( encoding->toUnicode( OGR_F_GetFieldAsString( ogrFet, attIndex ) ).toUtf8() ).toVariant();
        else
          value = QJsonDocument::fromJson( QString::fromUtf8( OGR_F_GetFieldAsString( ogrFet, attIndex ) ).toUtf8() ).toVariant();
        break;
      }
      default:
        Q_ASSERT_X( false, "QgsOgrUtils::getOgrFeatureAttribute", "unsupported field type" );
        if ( ok )
          *ok = false;
    }
  }
  else
  {
    value = QgsVariantUtils::createNullVariant( field.type() );
  }

  return value;
}

bool QgsOgrUtils::readOgrFeatureAttributes( OGRFeatureH ogrFet, const QgsFields &fields, QgsFeature &feature, QTextCodec *encoding )
{
  // read all attributes
  feature.initAttributes( fields.count() );
  feature.setFields( fields );

  if ( !ogrFet )
    return false;

  bool ok = false;
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QVariant value = getOgrFeatureAttribute( ogrFet, fields, idx, encoding, &ok );
    if ( ok )
    {
      feature.setAttribute( idx, value );
    }
  }
  return true;
}

bool QgsOgrUtils::readOgrFeatureGeometry( OGRFeatureH ogrFet, QgsFeature &feature )
{
  if ( !ogrFet )
    return false;

  OGRGeometryH geom = OGR_F_GetGeometryRef( ogrFet );
  if ( !geom )
    feature.clearGeometry();
  else
    feature.setGeometry( ogrGeometryToQgsGeometry( geom ) );

  return true;
}

std::unique_ptr< QgsPoint > ogrGeometryToQgsPoint( OGRGeometryH geom )
{
  Qgis::WkbType wkbType = QgsOgrUtils::ogrGeometryTypeToQgsWkbType( OGR_G_GetGeometryType( geom ) );

  double x, y, z, m;
  OGR_G_GetPointZM( geom, 0, &x, &y, &z, &m );
  return std::make_unique< QgsPoint >( wkbType, x, y, z, m );
}

std::unique_ptr< QgsMultiPoint > ogrGeometryToQgsMultiPoint( OGRGeometryH geom )
{
  std::unique_ptr< QgsMultiPoint > mp = std::make_unique< QgsMultiPoint >();

  const int count = OGR_G_GetGeometryCount( geom );
  mp->reserve( count );
  for ( int i = 0; i < count; ++i )
  {
    mp->addGeometry( ogrGeometryToQgsPoint( OGR_G_GetGeometryRef( geom, i ) ).release() );
  }

  return mp;
}

std::unique_ptr< QgsLineString > ogrGeometryToQgsLineString( OGRGeometryH geom )
{
  Qgis::WkbType wkbType = QgsOgrUtils::ogrGeometryTypeToQgsWkbType( OGR_G_GetGeometryType( geom ) );

  int count = OGR_G_GetPointCount( geom );
  QVector< double > x( count );
  QVector< double > y( count );
  QVector< double > z;
  double *pz = nullptr;
  if ( QgsWkbTypes::hasZ( wkbType ) )
  {
    z.resize( count );
    pz = z.data();
  }
  double *pm = nullptr;
  QVector< double > m;
  if ( QgsWkbTypes::hasM( wkbType ) )
  {
    m.resize( count );
    pm = m.data();
  }
  OGR_G_GetPointsZM( geom, x.data(), sizeof( double ), y.data(), sizeof( double ), pz, sizeof( double ), pm, sizeof( double ) );

  return std::make_unique< QgsLineString>( x, y, z, m, wkbType == Qgis::WkbType::LineString25D );
}

std::unique_ptr< QgsMultiLineString > ogrGeometryToQgsMultiLineString( OGRGeometryH geom )
{
  std::unique_ptr< QgsMultiLineString > mp = std::make_unique< QgsMultiLineString >();

  const int count = OGR_G_GetGeometryCount( geom );
  mp->reserve( count );
  for ( int i = 0; i < count; ++i )
  {
    mp->addGeometry( ogrGeometryToQgsLineString( OGR_G_GetGeometryRef( geom, i ) ).release() );
  }

  return mp;
}

std::unique_ptr< QgsPolygon > ogrGeometryToQgsPolygon( OGRGeometryH geom )
{
  std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();

  const int count = OGR_G_GetGeometryCount( geom );
  if ( count >= 1 )
  {
    polygon->setExteriorRing( ogrGeometryToQgsLineString( OGR_G_GetGeometryRef( geom, 0 ) ).release() );
  }

  for ( int i = 1; i < count; ++i )
  {
    polygon->addInteriorRing( ogrGeometryToQgsLineString( OGR_G_GetGeometryRef( geom, i ) ).release() );
  }

  return polygon;
}

std::unique_ptr< QgsMultiPolygon > ogrGeometryToQgsMultiPolygon( OGRGeometryH geom )
{
  std::unique_ptr< QgsMultiPolygon > polygon = std::make_unique< QgsMultiPolygon >();

  const int count = OGR_G_GetGeometryCount( geom );
  polygon->reserve( count );
  for ( int i = 0; i < count; ++i )
  {
    polygon->addGeometry( ogrGeometryToQgsPolygon( OGR_G_GetGeometryRef( geom, i ) ).release() );
  }

  return polygon;
}

Qgis::WkbType QgsOgrUtils::ogrGeometryTypeToQgsWkbType( OGRwkbGeometryType ogrGeomType )
{
  switch ( ogrGeomType )
  {
    case wkbUnknown: return Qgis::WkbType::Unknown;
    case wkbPoint: return Qgis::WkbType::Point;
    case wkbLineString: return Qgis::WkbType::LineString;
    case wkbPolygon: return Qgis::WkbType::Polygon;
    case wkbMultiPoint: return Qgis::WkbType::MultiPoint;
    case wkbMultiLineString: return Qgis::WkbType::MultiLineString;
    case wkbMultiPolygon: return Qgis::WkbType::MultiPolygon;
    case wkbGeometryCollection: return Qgis::WkbType::GeometryCollection;
    case wkbCircularString: return Qgis::WkbType::CircularString;
    case wkbCompoundCurve: return Qgis::WkbType::CompoundCurve;
    case wkbCurvePolygon: return Qgis::WkbType::CurvePolygon;
    case wkbMultiCurve: return Qgis::WkbType::MultiCurve;
    case wkbMultiSurface: return Qgis::WkbType::MultiSurface;
    case wkbCurve: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbSurface: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbPolyhedralSurface: return Qgis::WkbType::PolyhedralSurface;
    case wkbTIN: return Qgis::WkbType::TIN;
    case wkbTriangle: return Qgis::WkbType::Triangle;

    case wkbNone: return Qgis::WkbType::NoGeometry;
    case wkbLinearRing: return Qgis::WkbType::LineString; // approximate match

    case wkbCircularStringZ: return Qgis::WkbType::CircularStringZ;
    case wkbCompoundCurveZ: return Qgis::WkbType::CompoundCurveZ;
    case wkbCurvePolygonZ: return Qgis::WkbType::CurvePolygonZ;
    case wkbMultiCurveZ: return Qgis::WkbType::MultiCurveZ;
    case wkbMultiSurfaceZ: return Qgis::WkbType::MultiSurfaceZ;
    case wkbCurveZ: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbSurfaceZ: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbPolyhedralSurfaceZ: return Qgis::WkbType::PolyhedralSurfaceZ;
    case wkbTINZ: return Qgis::WkbType::TINZ;
    case wkbTriangleZ: return Qgis::WkbType::TriangleZ;

    case wkbPointM: return Qgis::WkbType::PointM;
    case wkbLineStringM: return Qgis::WkbType::LineStringM;
    case wkbPolygonM: return Qgis::WkbType::PolygonM;
    case wkbMultiPointM: return Qgis::WkbType::MultiPointM;
    case wkbMultiLineStringM: return Qgis::WkbType::MultiLineStringM;
    case wkbMultiPolygonM: return Qgis::WkbType::MultiPolygonM;
    case wkbGeometryCollectionM: return Qgis::WkbType::GeometryCollectionM;
    case wkbCircularStringM: return Qgis::WkbType::CircularStringM;
    case wkbCompoundCurveM: return Qgis::WkbType::CompoundCurveM;
    case wkbCurvePolygonM: return Qgis::WkbType::CurvePolygonM;
    case wkbMultiCurveM: return Qgis::WkbType::MultiCurveM;
    case wkbMultiSurfaceM: return Qgis::WkbType::MultiSurfaceM;
    case wkbCurveM: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbSurfaceM: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbPolyhedralSurfaceM: return Qgis::WkbType::PolyhedralSurfaceM;
    case wkbTINM: return Qgis::WkbType::TINM;
    case wkbTriangleM: return Qgis::WkbType::TriangleM;

    case wkbPointZM: return Qgis::WkbType::PointZM;
    case wkbLineStringZM: return Qgis::WkbType::LineStringZM;
    case wkbPolygonZM: return Qgis::WkbType::PolygonZM;
    case wkbMultiPointZM: return Qgis::WkbType::MultiPointZM;
    case wkbMultiLineStringZM: return Qgis::WkbType::MultiLineStringZM;
    case wkbMultiPolygonZM: return Qgis::WkbType::MultiPolygonZM;
    case wkbGeometryCollectionZM: return Qgis::WkbType::GeometryCollectionZM;
    case wkbCircularStringZM: return Qgis::WkbType::CircularStringZM;
    case wkbCompoundCurveZM: return Qgis::WkbType::CompoundCurveZM;
    case wkbCurvePolygonZM: return Qgis::WkbType::CurvePolygonZM;
    case wkbMultiCurveZM: return Qgis::WkbType::MultiCurveZM;
    case wkbMultiSurfaceZM: return Qgis::WkbType::MultiSurfaceZM;
    case wkbCurveZM: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbSurfaceZM: return Qgis::WkbType::Unknown; // not an actual concrete type
    case wkbPolyhedralSurfaceZM: return Qgis::WkbType::PolyhedralSurfaceZM;
    case wkbTINZM: return Qgis::WkbType::TINZM;
    case wkbTriangleZM: return Qgis::WkbType::TriangleZM;

    case wkbPoint25D: return Qgis::WkbType::PointZ;
    case wkbLineString25D: return Qgis::WkbType::LineStringZ;
    case wkbPolygon25D: return Qgis::WkbType::PolygonZ;
    case wkbMultiPoint25D: return Qgis::WkbType::MultiPointZ;
    case wkbMultiLineString25D: return Qgis::WkbType::MultiLineStringZ;
    case wkbMultiPolygon25D: return Qgis::WkbType::MultiPolygonZ;
    case wkbGeometryCollection25D: return Qgis::WkbType::GeometryCollectionZ;
  }

  // should not reach that point normally
  return Qgis::WkbType::Unknown;
}

QgsGeometry QgsOgrUtils::ogrGeometryToQgsGeometry( OGRGeometryH geom )
{
  if ( !geom )
    return QgsGeometry();

  const auto ogrGeomType = OGR_G_GetGeometryType( geom );
  Qgis::WkbType wkbType = ogrGeometryTypeToQgsWkbType( ogrGeomType );

  // optimised case for some geometry classes, avoiding wkb conversion on OGR/QGIS sides
  // TODO - extend to other classes!
  switch ( QgsWkbTypes::flatType( wkbType ) )
  {
    case Qgis::WkbType::Point:
    {
      return QgsGeometry( ogrGeometryToQgsPoint( geom ) );
    }

    case Qgis::WkbType::MultiPoint:
    {
      return QgsGeometry( ogrGeometryToQgsMultiPoint( geom ) );
    }

    case Qgis::WkbType::LineString:
    {
      return QgsGeometry( ogrGeometryToQgsLineString( geom ) );
    }

    case Qgis::WkbType::MultiLineString:
    {
      return QgsGeometry( ogrGeometryToQgsMultiLineString( geom ) );
    }

    case Qgis::WkbType::Polygon:
    {
      return QgsGeometry( ogrGeometryToQgsPolygon( geom ) );
    }

    case Qgis::WkbType::MultiPolygon:
    {
      return QgsGeometry( ogrGeometryToQgsMultiPolygon( geom ) );
    }

    default:
      break;
  }

  // Fallback to inefficient WKB conversions

  if ( wkbFlatten( wkbType ) == wkbGeometryCollection )
  {
    // Shapefile MultiPatch can be reported as GeometryCollectionZ of TINZ
    if ( OGR_G_GetGeometryCount( geom ) >= 1 &&
         wkbFlatten( OGR_G_GetGeometryType( OGR_G_GetGeometryRef( geom, 0 ) ) ) == wkbTIN )
    {
      auto newGeom = OGR_G_ForceToMultiPolygon( OGR_G_Clone( geom ) );
      auto ret = ogrGeometryToQgsGeometry( newGeom );
      OGR_G_DestroyGeometry( newGeom );
      return ret;
    }
  }

  // get the wkb representation
  int memorySize = OGR_G_WkbSize( geom );
  unsigned char *wkb = new unsigned char[memorySize];
  OGR_G_ExportToWkb( geom, static_cast<OGRwkbByteOrder>( QgsApplication::endian() ), wkb );

  QgsGeometry g;
  g.fromWkb( wkb, memorySize );
  return g;
}

QgsFeatureList QgsOgrUtils::stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding )
{
  QgsFeatureList features;
  if ( string.isEmpty() )
    return features;

  QString randomFileName = QStringLiteral( "/vsimem/%1" ).arg( QUuid::createUuid().toString() );

  // create memory file system object from string buffer
  QByteArray ba = string.toUtf8();
  VSIFCloseL( VSIFileFromMemBuffer( randomFileName.toUtf8().constData(), reinterpret_cast< GByte * >( ba.data() ),
                                    static_cast< vsi_l_offset >( ba.size() ), FALSE ) );

  gdal::ogr_datasource_unique_ptr hDS( OGROpen( randomFileName.toUtf8().constData(), false, nullptr ) );
  if ( !hDS )
  {
    VSIUnlink( randomFileName.toUtf8().constData() );
    return features;
  }

  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS.get(), 0 );
  if ( !ogrLayer )
  {
    hDS.reset();
    VSIUnlink( randomFileName.toUtf8().constData() );
    return features;
  }

  gdal::ogr_feature_unique_ptr oFeat;
  while ( oFeat.reset( OGR_L_GetNextFeature( ogrLayer ) ), oFeat )
  {
    QgsFeature feat = readOgrFeature( oFeat.get(), fields, encoding );
    if ( feat.isValid() )
      features << feat;
  }

  hDS.reset();
  VSIUnlink( randomFileName.toUtf8().constData() );

  return features;
}

QgsFields QgsOgrUtils::stringToFields( const QString &string, QTextCodec *encoding )
{
  QgsFields fields;
  if ( string.isEmpty() )
    return fields;

  QString randomFileName = QStringLiteral( "/vsimem/%1" ).arg( QUuid::createUuid().toString() );

  // create memory file system object from buffer
  QByteArray ba = string.toUtf8();
  VSIFCloseL( VSIFileFromMemBuffer( randomFileName.toUtf8().constData(), reinterpret_cast< GByte * >( ba.data() ),
                                    static_cast< vsi_l_offset >( ba.size() ), FALSE ) );

  gdal::ogr_datasource_unique_ptr hDS( OGROpen( randomFileName.toUtf8().constData(), false, nullptr ) );
  if ( !hDS )
  {
    VSIUnlink( randomFileName.toUtf8().constData() );
    return fields;
  }

  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS.get(), 0 );
  if ( !ogrLayer )
  {
    hDS.reset();
    VSIUnlink( randomFileName.toUtf8().constData() );
    return fields;
  }

  gdal::ogr_feature_unique_ptr oFeat;
  //read in the first feature only
  if ( oFeat.reset( OGR_L_GetNextFeature( ogrLayer ) ), oFeat )
  {
    fields = readOgrFields( oFeat.get(), encoding );
  }

  hDS.reset();
  VSIUnlink( randomFileName.toUtf8().constData() );
  return fields;
}

QStringList QgsOgrUtils::cStringListToQStringList( char **stringList )
{
  if ( !stringList )
    return {};

  QStringList strings;
  // presume null terminated string list
  for ( qgssize i = 0; stringList[i]; ++i )
  {
    strings.append( QString::fromUtf8( stringList[i] ) );
  }

  return strings;
}

QString QgsOgrUtils::OGRSpatialReferenceToWkt( OGRSpatialReferenceH srs )
{
  if ( !srs )
    return QString();

  char *pszWkt = nullptr;
  const QByteArray multiLineOption = QStringLiteral( "MULTILINE=NO" ).toLocal8Bit();
  const QByteArray formatOption = QStringLiteral( "FORMAT=WKT2" ).toLocal8Bit();
  const char *const options[] = {multiLineOption.constData(), formatOption.constData(), nullptr};
  OSRExportToWktEx( srs, &pszWkt, options );

  const QString res( pszWkt );
  CPLFree( pszWkt );
  return res;
}

QgsCoordinateReferenceSystem QgsOgrUtils::OGRSpatialReferenceToCrs( OGRSpatialReferenceH srs )
{
  const QString wkt = OGRSpatialReferenceToWkt( srs );
  if ( wkt.isEmpty() )
    return QgsCoordinateReferenceSystem();

  const char *authorityName = OSRGetAuthorityName( srs, nullptr );
  const char *authorityCode = OSRGetAuthorityCode( srs, nullptr );
  QgsCoordinateReferenceSystem res;
  if ( authorityName && authorityCode )
  {
    QString authId = QString( authorityName ) + ':' + QString( authorityCode );
    OGRSpatialReferenceH ogrSrsTmp = OSRNewSpatialReference( nullptr );
    // Check that the CRS build from authId and the input one are the "same".
    if ( OSRSetFromUserInput( ogrSrsTmp, authId.toUtf8().constData() ) != OGRERR_NONE &&
         OSRIsSame( srs, ogrSrsTmp ) )
    {
      res = QgsCoordinateReferenceSystem();
      res.createFromUserInput( authId );
    }
    OSRDestroySpatialReference( ogrSrsTmp );
  }
  if ( !res.isValid() )
    res = QgsCoordinateReferenceSystem::fromWkt( wkt );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  const double coordinateEpoch = OSRGetCoordinateEpoch( srs );
  if ( coordinateEpoch > 0 )
    res.setCoordinateEpoch( coordinateEpoch );
#endif
  return res;
}

OGRSpatialReferenceH QgsOgrUtils::crsToOGRSpatialReference( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
  {
    OGRSpatialReferenceH ogrSrs = nullptr;

    // First try instantiating the CRS from its authId. This will give a
    // more complete representation of the CRS for GDAL. In particular it might
    // help a few drivers to get the datum code, that would be missing in WKT-2.
    // See https://github.com/OSGeo/gdal/pull/5218
    const QString authId = crs.authid();
    const QString srsWkt = crs.toWkt( Qgis::CrsWktVariant::PreferredGdal );
    if ( !authId.isEmpty() )
    {
      ogrSrs = OSRNewSpatialReference( nullptr );
      if ( OSRSetFromUserInput( ogrSrs, authId.toUtf8().constData() ) == OGRERR_NONE )
      {
        // Check that the CRS build from WKT and authId are the "same".
        OGRSpatialReferenceH ogrSrsFromWkt = OSRNewSpatialReference( srsWkt.toUtf8().constData() );
        if ( ogrSrsFromWkt )
        {
          if ( !OSRIsSame( ogrSrs, ogrSrsFromWkt ) )
          {
            OSRDestroySpatialReference( ogrSrs );
            ogrSrs = ogrSrsFromWkt;
          }
          else
          {
            OSRDestroySpatialReference( ogrSrsFromWkt );
          }
        }
      }
      else
      {
        OSRDestroySpatialReference( ogrSrs );
        ogrSrs = nullptr;
      }
    }
    if ( !ogrSrs )
    {
      ogrSrs = OSRNewSpatialReference( srsWkt.toUtf8().constData() );
    }
    if ( ogrSrs )
    {
      OSRSetAxisMappingStrategy( ogrSrs, OAMS_TRADITIONAL_GIS_ORDER );
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
      if ( !std::isnan( crs.coordinateEpoch() ) )
      {
        OSRSetCoordinateEpoch( ogrSrs, crs.coordinateEpoch() );
      }
#endif
      return ogrSrs;
    }
  }

  return nullptr;
}

QString QgsOgrUtils::readShapefileEncoding( const QString &path )
{
  const QString cpgEncoding = readShapefileEncodingFromCpg( path );
  if ( !cpgEncoding.isEmpty() )
    return cpgEncoding;

  return readShapefileEncodingFromLdid( path );
}

QString QgsOgrUtils::readShapefileEncodingFromCpg( const QString &path )
{
  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( path, false, QStringList(), 0, errCause, false );
  return layer ? layer->GetMetadataItem( QStringLiteral( "ENCODING_FROM_CPG" ), QStringLiteral( "SHAPEFILE" ) ) : QString();
}

QString QgsOgrUtils::readShapefileEncodingFromLdid( const QString &path )
{
  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( path, false, QStringList(), 0, errCause, false );
  return layer ? layer->GetMetadataItem( QStringLiteral( "ENCODING_FROM_LDID" ), QStringLiteral( "SHAPEFILE" ) ) : QString();
}

QVariantMap QgsOgrUtils::parseStyleString( const QString &string )
{
  QVariantMap styles;

  char **papszStyleString = CSLTokenizeString2( string.toUtf8().constData(), ";",
                            CSLT_HONOURSTRINGS
                            | CSLT_PRESERVEQUOTES
                            | CSLT_PRESERVEESCAPES );
  for ( int i = 0; papszStyleString[i] != nullptr; ++i )
  {
    // style string format is:
    // <tool_name>([<tool_param>[,<tool_param>[,...]]])

    // first extract tool name
    const thread_local QRegularExpression sToolPartRx( QStringLiteral( "^(.*?)\\((.*)\\)$" ) );
    const QString stylePart( papszStyleString[i] );
    const QRegularExpressionMatch match = sToolPartRx.match( stylePart );
    if ( !match.hasMatch() )
      continue;

    const QString tool = match.captured( 1 );
    const QString params = match.captured( 2 );

    char **papszTokens = CSLTokenizeString2( params.toUtf8().constData(), ",", CSLT_HONOURSTRINGS
                         | CSLT_PRESERVEESCAPES );

    QVariantMap toolParts;
    const thread_local QRegularExpression sToolParamRx( QStringLiteral( "^(.*?):(.*)$" ) );
    for ( int j = 0; papszTokens[j] != nullptr; ++j )
    {
      const QString toolPart( papszTokens[j] );
      const QRegularExpressionMatch toolMatch = sToolParamRx.match( toolPart );
      if ( !match.hasMatch() )
        continue;

      // note we always convert the keys to lowercase, just to be safe...
      toolParts.insert( toolMatch.captured( 1 ).toLower(), toolMatch.captured( 2 ) );
    }
    CSLDestroy( papszTokens );

    // note we always convert the keys to lowercase, just to be safe...
    styles.insert( tool.toLower(), toolParts );
  }
  CSLDestroy( papszStyleString );
  return styles;
}

std::unique_ptr<QgsSymbol> QgsOgrUtils::symbolFromStyleString( const QString &string, Qgis::SymbolType type )
{
  const QVariantMap styles = parseStyleString( string );

  auto convertSize = []( const QString & size, double & value, Qgis::RenderUnit & unit )->bool
  {
    const thread_local QRegularExpression sUnitRx = QRegularExpression( QStringLiteral( "^([\\d\\.]+)(g|px|pt|mm|cm|in)$" ) );
    const QRegularExpressionMatch match = sUnitRx.match( size );
    if ( match.hasMatch() )
    {
      value = match.captured( 1 ).toDouble();
      const QString unitString = match.captured( 2 );
      if ( unitString.compare( QLatin1String( "px" ), Qt::CaseInsensitive ) == 0 )
      {
        // pixels are a poor unit choice for QGIS -- they render badly in hidpi layouts. Convert to points instead, using
        // a 96 dpi conversion
        static constexpr double PT_TO_INCHES_FACTOR = 1 / 72.0;
        static constexpr double PX_TO_PT_FACTOR = 1 / ( 96.0 * PT_TO_INCHES_FACTOR );
        unit = Qgis::RenderUnit::Points;
        value *= PX_TO_PT_FACTOR;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "pt" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = Qgis::RenderUnit::Points;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "mm" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = Qgis::RenderUnit::Millimeters;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "cm" ), Qt::CaseInsensitive ) == 0 )
      {
        value *= 10;
        unit = Qgis::RenderUnit::Millimeters;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "in" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = Qgis::RenderUnit::Inches;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "g" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = Qgis::RenderUnit::MapUnits;
        return true;
      }
      QgsDebugError( QStringLiteral( "Unknown unit %1" ).arg( unitString ) );
    }
    else
    {
      QgsDebugError( QStringLiteral( "Could not parse style size %1" ).arg( size ) );
    }
    return false;
  };

  auto convertColor = []( const QString & string ) -> QColor
  {
    if ( string.isEmpty() )
      return QColor();

    const thread_local QRegularExpression sColorWithAlphaRx = QRegularExpression( QStringLiteral( "^#([0-9a-fA-F]{6})([0-9a-fA-F]{2})$" ) );
    const QRegularExpressionMatch match = sColorWithAlphaRx.match( string );
    if ( match.hasMatch() )
    {
      // need to convert #RRGGBBAA to #AARRGGBB for QColor
      return QColor( QStringLiteral( "#%1%2" ).arg( match.captured( 2 ), match.captured( 1 ) ) );
    }
    else
    {
      return QColor( string );
    }
  };

  auto convertPen = [&convertColor, &convertSize, string]( const QVariantMap & lineStyle ) -> std::unique_ptr< QgsSymbol >
  {
    QColor color = convertColor( lineStyle.value( QStringLiteral( "c" ), QStringLiteral( "#000000" ) ).toString() );

    double lineWidth = DEFAULT_SIMPLELINE_WIDTH;
    Qgis::RenderUnit lineWidthUnit = Qgis::RenderUnit::Millimeters;
    convertSize( lineStyle.value( QStringLiteral( "w" ) ).toString(), lineWidth, lineWidthUnit );

    // if the pen is a mapinfo pen, use dedicated converter for more accurate results
    const thread_local QRegularExpression sMapInfoId = QRegularExpression( QStringLiteral( "mapinfo-pen-(\\d+)" ) );
    const QRegularExpressionMatch match = sMapInfoId.match( string );
    if ( match.hasMatch() )
    {
      const int penId = match.captured( 1 ).toInt();
      QgsMapInfoSymbolConversionContext context;
      std::unique_ptr<QgsSymbol> res( QgsMapInfoSymbolConverter::convertLineSymbol( penId, context, color, lineWidth, lineWidthUnit ) );
      if ( res )
        return res;
    }

    std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine = std::make_unique< QgsSimpleLineSymbolLayer >( color, lineWidth );
    simpleLine->setWidthUnit( lineWidthUnit );

    // pattern
    const QString pattern = lineStyle.value( QStringLiteral( "p" ) ).toString();
    if ( !pattern.isEmpty() )
    {
      const thread_local QRegularExpression sPatternUnitRx = QRegularExpression( QStringLiteral( "^([\\d\\.\\s]+)(g|px|pt|mm|cm|in)$" ) );
      const QRegularExpressionMatch match = sPatternUnitRx.match( pattern );
      if ( match.hasMatch() )
      {
        const QStringList patternValues = match.captured( 1 ).split( ' ' );
        QVector< qreal > dashPattern;
        Qgis::RenderUnit patternUnits = Qgis::RenderUnit::Millimeters;
        for ( const QString &val : patternValues )
        {
          double length;
          convertSize( val + match.captured( 2 ), length, patternUnits );
          dashPattern.push_back( length * lineWidth * 2 );
        }

        simpleLine->setCustomDashVector( dashPattern );
        simpleLine->setCustomDashPatternUnit( patternUnits );
        simpleLine->setUseCustomDashPattern( true );
      }
    }

    Qt::PenCapStyle capStyle = Qt::FlatCap;
    Qt::PenJoinStyle joinStyle = Qt::MiterJoin;
    // workaround https://github.com/OSGeo/gdal/pull/3509 in older GDAL versions
    const QString id = lineStyle.value( QStringLiteral( "id" ) ).toString();
    if ( id.contains( QLatin1String( "mapinfo-pen" ), Qt::CaseInsensitive ) )
    {
      // MapInfo renders all lines using a round pen cap and round pen join
      // which are not the default values for OGR pen cap/join styles. So we need to explicitly
      // override the OGR default values here on older GDAL versions
      capStyle = Qt::RoundCap;
      joinStyle = Qt::RoundJoin;
    }

    // pen cap
    const QString penCap = lineStyle.value( QStringLiteral( "cap" ) ).toString();
    if ( penCap.compare( QLatin1String( "b" ), Qt::CaseInsensitive ) == 0 )
    {
      capStyle = Qt::FlatCap;
    }
    else if ( penCap.compare( QLatin1String( "r" ), Qt::CaseInsensitive ) == 0 )
    {
      capStyle = Qt::RoundCap;
    }
    else if ( penCap.compare( QLatin1String( "p" ), Qt::CaseInsensitive ) == 0 )
    {
      capStyle = Qt::SquareCap;
    }
    simpleLine->setPenCapStyle( capStyle );

    // pen join
    const QString penJoin = lineStyle.value( QStringLiteral( "j" ) ).toString();
    if ( penJoin.compare( QLatin1String( "m" ), Qt::CaseInsensitive ) == 0 )
    {
      joinStyle = Qt::MiterJoin;
    }
    else if ( penJoin.compare( QLatin1String( "r" ), Qt::CaseInsensitive ) == 0 )
    {
      joinStyle = Qt::RoundJoin;
    }
    else if ( penJoin.compare( QLatin1String( "b" ), Qt::CaseInsensitive ) == 0 )
    {
      joinStyle = Qt::BevelJoin;
    }
    simpleLine->setPenJoinStyle( joinStyle );

    const QString priority = lineStyle.value( QStringLiteral( "l" ) ).toString();
    if ( !priority.isEmpty() )
    {
      simpleLine->setRenderingPass( priority.toInt() );
    }
    return std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << simpleLine.release() );
  };

  auto convertBrush = [&convertColor]( const QVariantMap & brushStyle ) -> std::unique_ptr< QgsSymbol >
  {
    const QColor foreColor = convertColor( brushStyle.value( QStringLiteral( "fc" ), QStringLiteral( "#000000" ) ).toString() );
    const QColor backColor = convertColor( brushStyle.value( QStringLiteral( "bc" ), QString() ).toString() );

    const QString id = brushStyle.value( QStringLiteral( "id" ) ).toString();

    // if the pen is a mapinfo brush, use dedicated converter for more accurate results
    const thread_local QRegularExpression sMapInfoId = QRegularExpression( QStringLiteral( "mapinfo-brush-(\\d+)" ) );
    const QRegularExpressionMatch match = sMapInfoId.match( id );
    if ( match.hasMatch() )
    {
      const int brushId = match.captured( 1 ).toInt();
      QgsMapInfoSymbolConversionContext context;
      std::unique_ptr<QgsSymbol> res( QgsMapInfoSymbolConverter::convertFillSymbol( brushId, context, foreColor, backColor ) );
      if ( res )
        return res;
    }

    const thread_local QRegularExpression sOgrId = QRegularExpression( QStringLiteral( "ogr-brush-(\\d+)" ) );
    const QRegularExpressionMatch ogrMatch = sOgrId.match( id );

    Qt::BrushStyle style = Qt::SolidPattern;
    if ( ogrMatch.hasMatch() )
    {
      const int brushId = ogrMatch.captured( 1 ).toInt();
      switch ( brushId )
      {
        case 0:
          style = Qt::SolidPattern;
          break;

        case 1:
          style = Qt::NoBrush;
          break;

        case 2:
          style = Qt::HorPattern;
          break;

        case 3:
          style = Qt::VerPattern;
          break;

        case 4:
          style = Qt::FDiagPattern;
          break;

        case 5:
          style = Qt::BDiagPattern;
          break;

        case 6:
          style = Qt::CrossPattern;
          break;

        case 7:
          style = Qt::DiagCrossPattern;
          break;
      }
    }

    QgsSymbolLayerList layers;
    if ( backColor.isValid() && style != Qt::SolidPattern && style != Qt::NoBrush )
    {
      std::unique_ptr< QgsSimpleFillSymbolLayer > backgroundFill = std::make_unique< QgsSimpleFillSymbolLayer >( backColor );
      backgroundFill->setLocked( true );
      backgroundFill->setStrokeStyle( Qt::NoPen );
      layers << backgroundFill.release();
    }

    std::unique_ptr< QgsSimpleFillSymbolLayer > foregroundFill = std::make_unique< QgsSimpleFillSymbolLayer >( foreColor );
    foregroundFill->setBrushStyle( style );
    foregroundFill->setStrokeStyle( Qt::NoPen );

    const QString priority = brushStyle.value( QStringLiteral( "l" ) ).toString();
    if ( !priority.isEmpty() )
    {
      foregroundFill->setRenderingPass( priority.toInt() );
    }
    layers << foregroundFill.release();
    return std::make_unique< QgsFillSymbol >( layers );
  };

  auto convertSymbol = [&convertColor, &convertSize, string]( const QVariantMap & symbolStyle ) -> std::unique_ptr< QgsSymbol >
  {
    const QColor color = convertColor( symbolStyle.value( QStringLiteral( "c" ), QStringLiteral( "#000000" ) ).toString() );

    double symbolSize = DEFAULT_SIMPLEMARKER_SIZE;
    Qgis::RenderUnit symbolSizeUnit = Qgis::RenderUnit::Millimeters;
    convertSize( symbolStyle.value( QStringLiteral( "s" ) ).toString(), symbolSize, symbolSizeUnit );

    const double angle = symbolStyle.value( QStringLiteral( "a" ), QStringLiteral( "0" ) ).toDouble();

    const QString id = symbolStyle.value( QStringLiteral( "id" ) ).toString();

    // if the symbol is a mapinfo symbol, use dedicated converter for more accurate results
    const thread_local QRegularExpression sMapInfoId = QRegularExpression( QStringLiteral( "mapinfo-sym-(\\d+)" ) );
    const QRegularExpressionMatch match = sMapInfoId.match( id );
    if ( match.hasMatch() )
    {
      const int symbolId = match.captured( 1 ).toInt();
      QgsMapInfoSymbolConversionContext context;

      // ogr interpretations of mapinfo symbol sizes are too large -- scale these down
      symbolSize *= 0.61;

      std::unique_ptr<QgsSymbol> res( QgsMapInfoSymbolConverter::convertMarkerSymbol( symbolId, context, color, symbolSize, symbolSizeUnit ) );
      if ( res )
        return res;
    }

    std::unique_ptr< QgsMarkerSymbolLayer > markerLayer;

    const thread_local QRegularExpression sFontId = QRegularExpression( QStringLiteral( "font-sym-(\\d+)" ) );
    const QRegularExpressionMatch fontMatch = sFontId.match( id );
    if ( fontMatch.hasMatch() )
    {
      const int symId = fontMatch.captured( 1 ).toInt();
      const QStringList families = symbolStyle.value( QStringLiteral( "f" ), QString() ).toString().split( ',' );

      bool familyFound = false;
      QString fontFamily;
      QString matched;
      for ( const QString &family : std::as_const( families ) )
      {
        const QString processedFamily = QgsApplication::fontManager()->processFontFamilyName( family );

        if ( QgsFontUtils::fontFamilyMatchOnSystem( processedFamily ) ||
             QgsApplication::fontManager()->tryToDownloadFontFamily( processedFamily, matched ) )
        {
          familyFound = true;
          fontFamily = processedFamily;
          break;
        }
      }

      if ( familyFound )
      {
        std::unique_ptr< QgsFontMarkerSymbolLayer > fontMarker = std::make_unique< QgsFontMarkerSymbolLayer >( fontFamily, QChar( symId ), symbolSize );
        fontMarker->setSizeUnit( symbolSizeUnit );
        fontMarker->setAngle( -angle );

        fontMarker->setColor( color );

        const QColor strokeColor = convertColor( symbolStyle.value( QStringLiteral( "o" ), QString() ).toString() );
        if ( strokeColor.isValid() )
        {
          fontMarker->setStrokeColor( strokeColor );
          fontMarker->setStrokeWidth( 1 );
          fontMarker->setStrokeWidthUnit( Qgis::RenderUnit::Points );
        }
        else
        {
          fontMarker->setStrokeWidth( 0 );
        }

        markerLayer = std::move( fontMarker );
      }
      else if ( !families.empty() )
      {
        // couldn't even find a matching font in the backup list
        QgsMessageLog::logMessage( QObject::tr( "Font %1 not found on system" ).arg( families.at( 0 ) ) );
      }
    }

    if ( !markerLayer )
    {
      const thread_local QRegularExpression sOgrId = QRegularExpression( QStringLiteral( "ogr-sym-(\\d+)" ) );
      const QRegularExpressionMatch ogrMatch = sOgrId.match( id );

      Qgis::MarkerShape shape;
      bool isFilled = true;
      if ( ogrMatch.hasMatch() )
      {
        const int symId = ogrMatch.captured( 1 ).toInt();
        switch ( symId )
        {
          case 0:
            shape = Qgis::MarkerShape::Cross;
            break;

          case 1:
            shape = Qgis::MarkerShape::Cross2;
            break;

          case 2:
            isFilled = false;
            shape = Qgis::MarkerShape::Circle;
            break;

          case 3:
            shape = Qgis::MarkerShape::Circle;
            break;

          case 4:
            isFilled = false;
            shape = Qgis::MarkerShape::Square;
            break;

          case 5:
            shape = Qgis::MarkerShape::Square;
            break;

          case 6:
            isFilled = false;
            shape = Qgis::MarkerShape::Triangle;
            break;

          case 7:
            shape = Qgis::MarkerShape::Triangle;
            break;

          case 8:
            isFilled = false;
            shape = Qgis::MarkerShape::Star;
            break;

          case 9:
            shape = Qgis::MarkerShape::Star;
            break;

          case 10:
            shape = Qgis::MarkerShape::Line;
            break;

          default:
            isFilled = false;
            shape = Qgis::MarkerShape::Square; // to initialize the variable
            break;
        }
      }
      else
      {
        isFilled = false;
        shape = Qgis::MarkerShape::Square; // to initialize the variable
      }

      std::unique_ptr< QgsSimpleMarkerSymbolLayer > simpleMarker = std::make_unique< QgsSimpleMarkerSymbolLayer >( shape, symbolSize, -angle );
      simpleMarker->setSizeUnit( symbolSizeUnit );
      simpleMarker->setStrokeWidth( 1.0 );
      simpleMarker->setStrokeWidthUnit( Qgis::RenderUnit::Points );

      if ( isFilled && QgsSimpleMarkerSymbolLayer::shapeIsFilled( shape ) )
      {
        simpleMarker->setColor( color );
        simpleMarker->setStrokeStyle( Qt::NoPen );
      }
      else
      {
        simpleMarker->setFillColor( QColor( 0, 0, 0, 0 ) );
        simpleMarker->setStrokeColor( color );
      }

      const QColor strokeColor = convertColor( symbolStyle.value( QStringLiteral( "o" ), QString() ).toString() );
      if ( strokeColor.isValid() )
      {
        simpleMarker->setStrokeColor( strokeColor );
        simpleMarker->setStrokeStyle( Qt::SolidLine );
      }

      markerLayer = std::move( simpleMarker );
    }

    return std::make_unique< QgsMarkerSymbol >( QgsSymbolLayerList() << markerLayer.release() );
  };

  switch ( type )
  {
    case Qgis::SymbolType::Marker:
      if ( styles.contains( QStringLiteral( "symbol" ) ) )
      {
        const QVariantMap symbolStyle = styles.value( QStringLiteral( "symbol" ) ).toMap();
        return convertSymbol( symbolStyle );
      }
      else
      {
        return nullptr;
      }

    case Qgis::SymbolType::Line:
      if ( styles.contains( QStringLiteral( "pen" ) ) )
      {
        // line symbol type
        const QVariantMap lineStyle = styles.value( QStringLiteral( "pen" ) ).toMap();
        return convertPen( lineStyle );
      }
      else
      {
        return nullptr;
      }

    case Qgis::SymbolType::Fill:
    {
      std::unique_ptr< QgsSymbol > fillSymbol = std::make_unique< QgsFillSymbol >();
      if ( styles.contains( QStringLiteral( "brush" ) ) )
      {
        const QVariantMap brushStyle = styles.value( QStringLiteral( "brush" ) ).toMap();
        fillSymbol = convertBrush( brushStyle );
      }
      else
      {
        std::unique_ptr< QgsSimpleFillSymbolLayer > emptyFill = std::make_unique< QgsSimpleFillSymbolLayer >();
        emptyFill->setBrushStyle( Qt::NoBrush );
        fillSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList() << emptyFill.release() );
      }

      std::unique_ptr< QgsSymbol > penSymbol;
      if ( styles.contains( QStringLiteral( "pen" ) ) )
      {
        const QVariantMap lineStyle = styles.value( QStringLiteral( "pen" ) ).toMap();
        penSymbol = convertPen( lineStyle );
      }

      if ( penSymbol )
      {
        const int count = penSymbol->symbolLayerCount();

        if ( count == 1 )
        {
          // if only one pen symbol layer, let's try and combine it with the topmost brush layer, so that the resultant QGIS symbol is simpler
          if ( QgsSymbolLayerUtils::condenseFillAndOutline( dynamic_cast< QgsFillSymbolLayer * >( fillSymbol->symbolLayer( fillSymbol->symbolLayerCount() - 1 ) ),
               dynamic_cast< QgsLineSymbolLayer * >( penSymbol->symbolLayer( 0 ) ) ) )
            return fillSymbol;
        }

        for ( int i = 0; i < count; ++i )
        {
          std::unique_ptr< QgsSymbolLayer > layer( penSymbol->takeSymbolLayer( 0 ) );
          layer->setLocked( true );
          fillSymbol->appendSymbolLayer( layer.release() );
        }
      }

      return fillSymbol;
    }

    case Qgis::SymbolType::Hybrid:
      break;
  }

  return nullptr;
}

void QgsOgrUtils::ogrFieldTypeToQVariantType( OGRFieldType ogrType, OGRFieldSubType ogrSubType, QMetaType::Type &variantType, QMetaType::Type &variantSubType )
{
  variantType = QMetaType::Type::UnknownType;
  variantSubType = QMetaType::Type::UnknownType;

  switch ( ogrType )
  {
    case OFTInteger:
      if ( ogrSubType == OFSTBoolean )
      {
        variantType = QMetaType::Type::Bool;
        ogrSubType = OFSTBoolean;
      }
      else
        variantType = QMetaType::Type::Int;
      break;
    case OFTInteger64:
      variantType = QMetaType::Type::LongLong;
      break;
    case OFTReal:
      variantType = QMetaType::Type::Double;
      break;
    case OFTDate:
      variantType = QMetaType::Type::QDate;
      break;
    case OFTTime:
      variantType = QMetaType::Type::QTime;
      break;
    case OFTDateTime:
      variantType = QMetaType::Type::QDateTime;
      break;

    case OFTBinary:
      variantType = QMetaType::Type::QByteArray;
      break;

    case OFTString:
    case OFTWideString:
      if ( ogrSubType == OFSTJSON )
      {
        ogrSubType = OFSTJSON;
        variantType = QMetaType::Type::QVariantMap;
        variantSubType = QMetaType::Type::QString;
      }
      else
      {
        variantType = QMetaType::Type::QString;
      }
      break;

    case OFTStringList:
    case OFTWideStringList:
      variantType = QMetaType::Type::QStringList;
      variantSubType = QMetaType::Type::QString;
      break;

    case OFTIntegerList:
      variantType = QMetaType::Type::QVariantList;
      variantSubType = QMetaType::Type::Int;
      break;

    case OFTRealList:
      variantType = QMetaType::Type::QVariantList;
      variantSubType = QMetaType::Type::Double;
      break;

    case OFTInteger64List:
      variantType = QMetaType::Type::QVariantList;
      variantSubType = QMetaType::Type::LongLong;
      break;
  }
}

void QgsOgrUtils::variantTypeToOgrFieldType( QMetaType::Type variantType, OGRFieldType &ogrType, OGRFieldSubType &ogrSubType )
{
  ogrSubType = OFSTNone;
  switch ( variantType )
  {
    case QMetaType::Type::Bool:
      ogrType = OFTInteger;
      ogrSubType = OFSTBoolean;
      break;

    case QMetaType::Type::Int:
      ogrType = OFTInteger;
      break;

    case QMetaType::Type::LongLong:
      ogrType = OFTInteger64;
      break;

    case QMetaType::Type::Double:
      ogrType = OFTReal;
      break;

    case QMetaType::Type::QChar:
    case QMetaType::Type::QString:
      ogrType = OFTString;
      break;

    case QMetaType::Type::QStringList:
      ogrType = OFTStringList;
      break;

    case QMetaType::Type::QByteArray:
      ogrType = OFTBinary;
      break;

    case QMetaType::Type::QDate:
      ogrType = OFTDate;
      break;

    case QMetaType::Type::QTime:
      ogrType = OFTTime;
      break;
    case QMetaType::Type::QDateTime:
      ogrType = OFTDateTime;
      break;

    default:
      ogrType = OFTString;
      break;
  }
}

QVariant QgsOgrUtils::stringToVariant( OGRFieldType type, OGRFieldSubType, const QString &string )
{
  if ( string.isEmpty() )
    return QVariant();

  bool ok = false;
  QVariant res;
  switch ( type )
  {
    case OFTInteger:
      res = string.toInt( &ok );
      break;

    case OFTInteger64:
      res = string.toLongLong( &ok );
      break;

    case OFTReal:
      res = string.toDouble( &ok );
      break;

    case OFTString:
    case OFTWideString:
      res = string;
      ok = true;
      break;

    case OFTDate:
      res = QDate::fromString( string, Qt::ISODate );
      ok = res.isValid();
      break;

    case OFTTime:
      res = QTime::fromString( string, Qt::ISODate );
      ok = res.isValid();
      break;

    case OFTDateTime:
      res = QDateTime::fromString( string, Qt::ISODate );
      ok = res.isValid();
      break;

    default:
      res = string;
      ok = true;
      break;
  }

  return ok ? res : QVariant();
}

QList<QgsVectorDataProvider::NativeType> QgsOgrUtils::nativeFieldTypesForDriver( GDALDriverH driver )
{
  if ( !driver )
    return {};

  const QString driverName = QString::fromUtf8( GDALGetDriverShortName( driver ) );

  int nMaxIntLen = 11;
  int nMaxInt64Len = 21;
  int nMaxDoubleLen = 20;
  int nMaxDoublePrec = 15;
  int nDateLen = 8;
  if ( driverName == QLatin1String( "GPKG" ) )
  {
    // GPKG only supports field length for text (and binary)
    nMaxIntLen = 0;
    nMaxInt64Len = 0;
    nMaxDoubleLen = 0;
    nMaxDoublePrec = 0;
    nDateLen = 0;
  }

  QList<QgsVectorDataProvider::NativeType> nativeTypes;
  nativeTypes
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), QStringLiteral( "integer" ), QMetaType::Type::Int, 0, nMaxIntLen )
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::LongLong ), QStringLiteral( "integer64" ), QMetaType::Type::LongLong, 0, nMaxInt64Len )
      << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal number (real)" ), QStringLiteral( "double" ), QMetaType::Type::Double, 0, nMaxDoubleLen, 0, nMaxDoublePrec )
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ), QStringLiteral( "string" ), QMetaType::Type::QString, 0, 65535 );

  if ( driverName == QLatin1String( "GPKG" ) )
    nativeTypes << QgsVectorDataProvider::NativeType( QObject::tr( "JSON (string)" ), QStringLiteral( "JSON" ), QMetaType::Type::QVariantMap, 0, 0, 0, 0, QMetaType::Type::QString );

  bool supportsDate = true;
  bool supportsTime = true;
  bool supportsDateTime = true;
  bool supportsBinary = false;
  bool supportIntegerList = false;
  bool supportInteger64List = false;
  bool supportRealList = false;
  bool supportsStringList = false;

  // For drivers that advertise their data type, use that instead of the
  // above hardcoded defaults.
  if ( const char *pszDataTypes = GDALGetMetadataItem( driver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszDataTypes, " ", 0 );
    supportsDate = CSLFindString( papszTokens, "Date" ) >= 0;
    supportsTime = CSLFindString( papszTokens, "Time" ) >= 0;
    supportsDateTime = CSLFindString( papszTokens, "DateTime" ) >= 0;
    supportsBinary = CSLFindString( papszTokens, "Binary" ) >= 0;
    supportIntegerList = CSLFindString( papszTokens, "IntegerList" ) >= 0;
    supportInteger64List = CSLFindString( papszTokens, "Integer64List" ) >= 0;
    supportRealList = CSLFindString( papszTokens, "RealList" ) >= 0;
    supportsStringList = CSLFindString( papszTokens, "StringList" ) >= 0;
    CSLDestroy( papszTokens );
  }

  // Older versions of GDAL incorrectly report that shapefiles support
  // DateTime.
#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,2,0)
  if ( driverName == QLatin1String( "ESRI Shapefile" ) )
  {
    supportsDateTime = false;
  }
#endif

  if ( supportsDate )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), QStringLiteral( "date" ), QMetaType::Type::QDate, nDateLen, nDateLen );
  }
  if ( supportsTime )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), QStringLiteral( "time" ), QMetaType::Type::QTime );
  }
  if ( supportsDateTime )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), QStringLiteral( "datetime" ), QMetaType::Type::QDateTime );
  }
  if ( supportsBinary )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), QStringLiteral( "binary" ), QMetaType::Type::QByteArray );
  }
  if ( supportIntegerList )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Int ), QStringLiteral( "integerlist" ), QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::Int );
  }
  if ( supportInteger64List )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::LongLong ), QStringLiteral( "integer64list" ), QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::LongLong );
  }
  if ( supportRealList )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Double ), QStringLiteral( "doublelist" ), QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::Double );
  }
  if ( supportsStringList )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QStringList ), QStringLiteral( "stringlist" ), QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::QString );
  }

  const char *pszDataSubTypes = GDALGetMetadataItem( driver, GDAL_DMD_CREATIONFIELDDATASUBTYPES, nullptr );
  if ( pszDataSubTypes && strstr( pszDataSubTypes, "Boolean" ) )
  {
    // boolean data type
    nativeTypes
        << QgsVectorDataProvider::NativeType( QObject::tr( "Boolean" ), QStringLiteral( "bool" ), QMetaType::Type::Bool );
  }

  return nativeTypes;
}


#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
std::unique_ptr< QgsFieldDomain > QgsOgrUtils::convertFieldDomain( OGRFieldDomainH domain )
{
  if ( !domain )
    return nullptr;

  const QString name{ OGR_FldDomain_GetName( domain ) };
  const QString description{ OGR_FldDomain_GetDescription( domain ) };

  QMetaType::Type fieldType = QMetaType::Type::UnknownType;
  QMetaType::Type fieldSubType = QMetaType::Type::UnknownType;
  const OGRFieldType domainFieldType = OGR_FldDomain_GetFieldType( domain );
  const OGRFieldSubType domainFieldSubType = OGR_FldDomain_GetFieldSubType( domain );
  ogrFieldTypeToQVariantType( domainFieldType, domainFieldSubType, fieldType, fieldSubType );

  std::unique_ptr< QgsFieldDomain > res;
  switch ( OGR_FldDomain_GetDomainType( domain ) )
  {
    case OFDT_CODED:
    {
      QList< QgsCodedValue > values;
      const OGRCodedValue *codedValue = OGR_CodedFldDomain_GetEnumeration( domain );
      while ( codedValue && codedValue->pszCode )
      {
        const QString code( codedValue->pszCode );

        // if pszValue is null then it indicates we are working with a set of acceptable values which aren't
        // coded. In this case we copy the code as the value so that QGIS exposes the domain as a choice of
        // the valid code values.
        const QString value( codedValue->pszValue ? codedValue->pszValue : codedValue->pszCode );
        values.append( QgsCodedValue( stringToVariant( domainFieldType, domainFieldSubType, code ), value ) );

        codedValue++;
      }

      res = std::make_unique< QgsCodedFieldDomain >( name, description, fieldType, values );
      break;
    }

    case OFDT_RANGE:
    {
      QVariant minValue;
      bool minIsInclusive = false;
      if ( const OGRField *min = OGR_RangeFldDomain_GetMin( domain, &minIsInclusive ) )
      {
        minValue = QgsOgrUtils::OGRFieldtoVariant( min, domainFieldType );
      }
      QVariant maxValue;
      bool maxIsInclusive = false;
      if ( const OGRField *max = OGR_RangeFldDomain_GetMax( domain, &maxIsInclusive ) )
      {
        maxValue = QgsOgrUtils::OGRFieldtoVariant( max, domainFieldType );
      }

      res = std::make_unique< QgsRangeFieldDomain >( name, description, fieldType,
            minValue, minIsInclusive,
            maxValue, maxIsInclusive );
      break;
    }

    case OFDT_GLOB:
      res = std::make_unique< QgsGlobFieldDomain >( name, description, fieldType,
            QString( OGR_GlobFldDomain_GetGlob( domain ) ) );
      break;
  }

  switch ( OGR_FldDomain_GetMergePolicy( domain ) )
  {
    case OFDMP_DEFAULT_VALUE:
      res->setMergePolicy( Qgis::FieldDomainMergePolicy::DefaultValue );
      break;
    case OFDMP_SUM:
      res->setMergePolicy( Qgis::FieldDomainMergePolicy::Sum );
      break;
    case OFDMP_GEOMETRY_WEIGHTED:
      res->setMergePolicy( Qgis::FieldDomainMergePolicy::GeometryWeighted );
      break;
  }

  switch ( OGR_FldDomain_GetSplitPolicy( domain ) )
  {
    case OFDSP_DEFAULT_VALUE:
      res->setSplitPolicy( Qgis::FieldDomainSplitPolicy::DefaultValue );
      break;
    case OFDSP_DUPLICATE:
      res->setSplitPolicy( Qgis::FieldDomainSplitPolicy::Duplicate );
      break;
    case OFDSP_GEOMETRY_RATIO:
      res->setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
      break;
  }
  return res;
}

OGRFieldDomainH QgsOgrUtils::convertFieldDomain( const QgsFieldDomain *domain )
{
  if ( !domain )
    return nullptr;

  OGRFieldType domainFieldType = OFTInteger;
  OGRFieldSubType domainFieldSubType = OFSTNone;
  variantTypeToOgrFieldType( domain->fieldType(), domainFieldType, domainFieldSubType );

  OGRFieldDomainH res = nullptr;
  switch ( domain->type() )
  {
    case Qgis::FieldDomainType::Coded:
    {
      std::vector< OGRCodedValue > enumeration;
      const QList< QgsCodedValue> values = qgis::down_cast< const QgsCodedFieldDomain * >( domain )->values();
      enumeration.reserve( values.size() );
      for ( const QgsCodedValue &value : values )
      {
        OGRCodedValue codedValue;
        codedValue.pszCode = CPLStrdup( value.code().toString().toUtf8().constData() );
        codedValue.pszValue = CPLStrdup( value.value().toUtf8().constData() );
        enumeration.push_back( codedValue );
      }
      OGRCodedValue last;
      last.pszCode = nullptr;
      last.pszValue = nullptr;
      enumeration.push_back( last );
      res = OGR_CodedFldDomain_Create(
              domain->name().toUtf8().constData(),
              domain->description().toUtf8().constData(),
              domainFieldType,
              domainFieldSubType,
              enumeration.data()
            );

      for ( const OGRCodedValue &value : std::as_const( enumeration ) )
      {
        CPLFree( value.pszCode );
        CPLFree( value.pszValue );
      }
      break;
    }

    case Qgis::FieldDomainType::Range:
    {
      std::unique_ptr< OGRField > min = variantToOGRField( qgis::down_cast< const QgsRangeFieldDomain * >( domain )->minimum(), domainFieldType );
      std::unique_ptr< OGRField > max = variantToOGRField( qgis::down_cast< const QgsRangeFieldDomain * >( domain )->maximum(), domainFieldType );
      if ( !min || !max )
        return nullptr;
      res = OGR_RangeFldDomain_Create(
              domain->name().toUtf8().constData(),
              domain->description().toUtf8().constData(),
              domainFieldType,
              domainFieldSubType,
              min.get(),
              qgis::down_cast< const QgsRangeFieldDomain * >( domain )->minimumIsInclusive(),
              max.get(),
              qgis::down_cast< const QgsRangeFieldDomain * >( domain )->maximumIsInclusive()
            );
      break;
    }

    case Qgis::FieldDomainType::Glob:
    {
      res = OGR_GlobFldDomain_Create(
              domain->name().toUtf8().constData(),
              domain->description().toUtf8().constData(),
              domainFieldType,
              domainFieldSubType,
              qgis::down_cast< const QgsGlobFieldDomain * >( domain )->glob().toUtf8().constData()
            );
      break;
    }
  }

  switch ( domain->mergePolicy() )
  {
    case Qgis::FieldDomainMergePolicy::DefaultValue:
      OGR_FldDomain_SetMergePolicy( res, OFDMP_DEFAULT_VALUE );
      break;
    case Qgis::FieldDomainMergePolicy::GeometryWeighted:
      OGR_FldDomain_SetMergePolicy( res, OFDMP_GEOMETRY_WEIGHTED );
      break;
    case Qgis::FieldDomainMergePolicy::Sum:
      OGR_FldDomain_SetMergePolicy( res, OFDMP_SUM );
      break;
  }

  switch ( domain->splitPolicy() )
  {
    case Qgis::FieldDomainSplitPolicy::DefaultValue:
      OGR_FldDomain_SetSplitPolicy( res, OFDSP_DEFAULT_VALUE );
      break;
    case Qgis::FieldDomainSplitPolicy::GeometryRatio:
      OGR_FldDomain_SetSplitPolicy( res, OFDSP_GEOMETRY_RATIO );
      break;
    case Qgis::FieldDomainSplitPolicy::Duplicate:
      OGR_FldDomain_SetSplitPolicy( res, OFDSP_DUPLICATE );
      break;

    case Qgis::FieldDomainSplitPolicy::UnsetField:
      // not supported
      break;
  }

  return res;
}

#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
QgsWeakRelation QgsOgrUtils::convertRelationship( GDALRelationshipH relationship, const QString &datasetUri )
{
  QgsProviderMetadata *ogrProviderMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) );
  const QVariantMap datasetUriParts = ogrProviderMetadata->decodeUri( datasetUri );

  const QString leftTableName( GDALRelationshipGetLeftTableName( relationship ) );

  QVariantMap leftTableUriParts = datasetUriParts;
  leftTableUriParts.insert( QStringLiteral( "layerName" ), leftTableName );
  const QString leftTableSource = ogrProviderMetadata->encodeUri( leftTableUriParts );

  const QString rightTableName( GDALRelationshipGetRightTableName( relationship ) );
  QVariantMap rightTableUriParts = datasetUriParts;
  rightTableUriParts.insert( QStringLiteral( "layerName" ), rightTableName );
  const QString rightTableSource = ogrProviderMetadata->encodeUri( rightTableUriParts );

  const QString mappingTableName( GDALRelationshipGetMappingTableName( relationship ) );
  QString mappingTableSource;
  if ( !mappingTableName.isEmpty() )
  {
    QVariantMap mappingTableUriParts = datasetUriParts;
    mappingTableUriParts.insert( QStringLiteral( "layerName" ), mappingTableName );
    mappingTableSource = ogrProviderMetadata->encodeUri( mappingTableUriParts );
  }

  const QString relationshipName( GDALRelationshipGetName( relationship ) );

  char **cslLeftTableFieldNames = GDALRelationshipGetLeftTableFields( relationship );
  const QStringList leftTableFieldNames = QgsOgrUtils::cStringListToQStringList( cslLeftTableFieldNames );
  CSLDestroy( cslLeftTableFieldNames );

  char **cslRightTableFieldNames = GDALRelationshipGetRightTableFields( relationship );
  const QStringList rightTableFieldNames = QgsOgrUtils::cStringListToQStringList( cslRightTableFieldNames );
  CSLDestroy( cslRightTableFieldNames );

  char **cslLeftMappingTableFieldNames = GDALRelationshipGetLeftMappingTableFields( relationship );
  const QStringList leftMappingTableFieldNames = QgsOgrUtils::cStringListToQStringList( cslLeftMappingTableFieldNames );
  CSLDestroy( cslLeftMappingTableFieldNames );

  char **cslRightMappingTableFieldNames = GDALRelationshipGetRightMappingTableFields( relationship );
  const QStringList rightMappingTableFieldNames = QgsOgrUtils::cStringListToQStringList( cslRightMappingTableFieldNames );
  CSLDestroy( cslRightMappingTableFieldNames );

  const QString forwardPathLabel( GDALRelationshipGetForwardPathLabel( relationship ) );
  const QString backwardPathLabel( GDALRelationshipGetBackwardPathLabel( relationship ) );
  const QString relatedTableType( GDALRelationshipGetRelatedTableType( relationship ) );

  const GDALRelationshipType relationshipType = GDALRelationshipGetType( relationship );
  Qgis::RelationshipStrength strength = Qgis::RelationshipStrength::Association;
  switch ( relationshipType )
  {
    case GRT_COMPOSITE:
      strength = Qgis::RelationshipStrength::Composition;
      break;

    case GRT_ASSOCIATION:
      strength = Qgis::RelationshipStrength::Association;
      break;

    case GRT_AGGREGATION:
      QgsLogger::warning( "Aggregation relationships are not supported, treating as association instead" );
      break;
  }

  const GDALRelationshipCardinality eCardinality = GDALRelationshipGetCardinality( relationship );
  Qgis::RelationshipCardinality cardinality = Qgis::RelationshipCardinality::OneToOne;
  switch ( eCardinality )
  {
    case GRC_ONE_TO_ONE:
      cardinality = Qgis::RelationshipCardinality::OneToOne;
      break;
    case GRC_ONE_TO_MANY:
      cardinality = Qgis::RelationshipCardinality::OneToMany;
      break;
    case GRC_MANY_TO_ONE:
      cardinality = Qgis::RelationshipCardinality::ManyToOne;
      break;
    case GRC_MANY_TO_MANY:
      cardinality = Qgis::RelationshipCardinality::ManyToMany;
      break;
  }

  switch ( cardinality )
  {
    case Qgis::RelationshipCardinality::OneToOne:
    case Qgis::RelationshipCardinality::OneToMany:
    case Qgis::RelationshipCardinality::ManyToOne:
    {
      QgsWeakRelation rel( relationshipName,
                           relationshipName,
                           strength,
                           QString(), QString(), rightTableSource, QStringLiteral( "ogr" ),
                           QString(), QString(), leftTableSource, QStringLiteral( "ogr" ) );
      rel.setCardinality( cardinality );
      rel.setForwardPathLabel( forwardPathLabel );
      rel.setBackwardPathLabel( backwardPathLabel );
      rel.setRelatedTableType( relatedTableType );
      rel.setReferencedLayerFields( leftTableFieldNames );
      rel.setReferencingLayerFields( rightTableFieldNames );
      return rel;
    }

    case Qgis::RelationshipCardinality::ManyToMany:
    {
      QgsWeakRelation rel( relationshipName,
                           relationshipName,
                           strength,
                           QString(), QString(), rightTableSource, QStringLiteral( "ogr" ),
                           QString(), QString(), leftTableSource, QStringLiteral( "ogr" ) );
      rel.setCardinality( cardinality );
      rel.setForwardPathLabel( forwardPathLabel );
      rel.setBackwardPathLabel( backwardPathLabel );
      rel.setRelatedTableType( relatedTableType );
      rel.setMappingTable( QgsVectorLayerRef( QString(), QString(), mappingTableSource, QStringLiteral( "ogr" ) ) );
      rel.setReferencedLayerFields( leftTableFieldNames );
      rel.setMappingReferencedLayerFields( leftMappingTableFieldNames );
      rel.setReferencingLayerFields( rightTableFieldNames );
      rel.setMappingReferencingLayerFields( rightMappingTableFieldNames );
      return rel;
    }
  }
  return QgsWeakRelation();
}

gdal::relationship_unique_ptr QgsOgrUtils::convertRelationship( const QgsWeakRelation &relationship, QString &error )
{
  GDALRelationshipCardinality gCardinality = GDALRelationshipCardinality::GRC_ONE_TO_MANY;
  switch ( relationship.cardinality() )
  {
    case Qgis::RelationshipCardinality::OneToOne:
      gCardinality = GDALRelationshipCardinality::GRC_ONE_TO_ONE;
      break;
    case Qgis::RelationshipCardinality::OneToMany:
      gCardinality = GDALRelationshipCardinality::GRC_ONE_TO_MANY;
      break;
    case Qgis::RelationshipCardinality::ManyToOne:
      gCardinality = GDALRelationshipCardinality::GRC_MANY_TO_ONE;
      break;
    case Qgis::RelationshipCardinality::ManyToMany:
      gCardinality = GDALRelationshipCardinality::GRC_MANY_TO_MANY;
      break;
  }

  QgsProviderMetadata *ogrProviderMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) );

  const QVariantMap leftParts = ogrProviderMetadata->decodeUri( relationship.referencedLayerSource() );
  const QString leftTableName = leftParts.value( QStringLiteral( "layerName" ) ).toString();
  if ( leftTableName.isEmpty() )
  {
    error = QObject::tr( "Parent table name was not set" );
    return nullptr;
  }

  const QVariantMap rightParts = ogrProviderMetadata->decodeUri( relationship.referencingLayerSource() );
  const QString rightTableName = rightParts.value( QStringLiteral( "layerName" ) ).toString();
  if ( rightTableName.isEmpty() )
  {
    error = QObject::tr( "Child table name was not set" );
    return nullptr;
  }

  if ( leftParts.value( QStringLiteral( "path" ) ).toString() != rightParts.value( QStringLiteral( "path" ) ).toString() )
  {
    error = QObject::tr( "Parent and child table must be from the same dataset" );
    return nullptr;
  }

  QString mappingTableName;
  if ( !relationship.mappingTableSource().isEmpty() )
  {
    const QVariantMap mappingParts = ogrProviderMetadata->decodeUri( relationship.mappingTableSource() );
    mappingTableName = mappingParts.value( QStringLiteral( "layerName" ) ).toString();
    if ( leftParts.value( QStringLiteral( "path" ) ).toString() != mappingParts.value( QStringLiteral( "path" ) ).toString() )
    {
      error = QObject::tr( "Parent and mapping table must be from the same dataset" );
      return nullptr;
    }
  }

  gdal::relationship_unique_ptr relationH( GDALRelationshipCreate( relationship.name().toLocal8Bit().constData(),
      leftTableName.toLocal8Bit().constData(),
      rightTableName.toLocal8Bit().constData(),
      gCardinality ) );

  // set left table fields
  const QStringList leftFieldNames = relationship.referencedLayerFields();
  int count = leftFieldNames.count();
  char **lst = new char *[count + 1];
  if ( count > 0 )
  {
    int pos = 0;
    for ( const QString &string : leftFieldNames )
    {
      lst[pos] = CPLStrdup( string.toLocal8Bit().constData() );
      pos++;
    }
  }
  lst[count] = nullptr;
  GDALRelationshipSetLeftTableFields( relationH.get(), lst );
  CSLDestroy( lst );

  // set right table fields
  const QStringList rightFieldNames = relationship.referencingLayerFields();
  count = rightFieldNames.count();
  lst = new char *[count + 1];
  if ( count > 0 )
  {
    int pos = 0;
    for ( const QString &string : rightFieldNames )
    {
      lst[pos] = CPLStrdup( string.toLocal8Bit().constData() );
      pos++;
    }
  }
  lst[count] = nullptr;
  GDALRelationshipSetRightTableFields( relationH.get(), lst );
  CSLDestroy( lst );

  if ( !mappingTableName.isEmpty() )
  {
    GDALRelationshipSetMappingTableName( relationH.get(), mappingTableName.toLocal8Bit().constData() );

    // set left mapping table fields
    const QStringList leftFieldNames = relationship.mappingReferencedLayerFields();
    int count = leftFieldNames.count();
    char **lst = new char *[count + 1];
    if ( count > 0 )
    {
      int pos = 0;
      for ( const QString &string : leftFieldNames )
      {
        lst[pos] = CPLStrdup( string.toLocal8Bit().constData() );
        pos++;
      }
    }
    lst[count] = nullptr;
    GDALRelationshipSetLeftMappingTableFields( relationH.get(), lst );
    CSLDestroy( lst );

    // set right table fields
    const QStringList rightFieldNames = relationship.mappingReferencingLayerFields();
    count = rightFieldNames.count();
    lst = new char *[count + 1];
    if ( count > 0 )
    {
      int pos = 0;
      for ( const QString &string : rightFieldNames )
      {
        lst[pos] = CPLStrdup( string.toLocal8Bit().constData() );
        pos++;
      }
    }
    lst[count] = nullptr;
    GDALRelationshipSetRightMappingTableFields( relationH.get(), lst );
    CSLDestroy( lst );
  }

  // set type
  switch ( relationship.strength() )
  {
    case Qgis::RelationshipStrength::Association:
      GDALRelationshipSetType( relationH.get(), GDALRelationshipType::GRT_ASSOCIATION );
      break;

    case Qgis::RelationshipStrength::Composition:
      GDALRelationshipSetType( relationH.get(), GDALRelationshipType::GRT_COMPOSITE );
      break;
  }

  // set labels
  if ( !relationship.forwardPathLabel().isEmpty() )
    GDALRelationshipSetForwardPathLabel( relationH.get(), relationship.forwardPathLabel().toLocal8Bit().constData() );
  if ( !relationship.backwardPathLabel().isEmpty() )
    GDALRelationshipSetBackwardPathLabel( relationH.get(), relationship.backwardPathLabel().toLocal8Bit().constData() );

  // set table type
  if ( !relationship.relatedTableType().isEmpty() )
    GDALRelationshipSetRelatedTableType( relationH.get(), relationship.relatedTableType().toLocal8Bit().constData() );

  return relationH;
}
#endif

int QgsOgrUtils::listStyles( GDALDatasetH hDS, const QString &layerName, const QString &geomColumn, QStringList &ids, QStringList &names, QStringList &descriptions, QString &errCause )
{
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
  {
    QgsDebugMsgLevel( QStringLiteral( "No styles available on DB" ), 2 );
    errCause = QObject::tr( "No styles available on DB" );
    return 0;
  }

  if ( OGR_L_GetFeatureCount( hLayer, TRUE ) == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "No styles available on DB" ), 2 );
    errCause = QObject::tr( "No styles available on DB" );
    return 0;
  }

  OGRFeatureDefnH hLayerDefn = OGR_L_GetLayerDefn( hLayer );

  OGR_L_ResetReading( hLayer );

  QList<qlonglong> listTimestamp;
  QMap<int, QString> mapIdToStyleName;
  QMap<int, QString> mapIdToDescription;
  QMap<qlonglong, QList<int> > mapTimestampToId;
  int numberOfRelatedStyles = 0;

  while ( true )
  {
    gdal::ogr_feature_unique_ptr hFeature( OGR_L_GetNextFeature( hLayer ) );
    if ( !hFeature )
      break;

    QString tableName( QString::fromUtf8(
                         OGR_F_GetFieldAsString( hFeature.get(),
                             OGR_FD_GetFieldIndex( hLayerDefn, "f_table_name" ) ) ) );
    QString geometryColumn( QString::fromUtf8(
                              OGR_F_GetFieldAsString( hFeature.get(),
                                  OGR_FD_GetFieldIndex( hLayerDefn, "f_geometry_column" ) ) ) );
    QString styleName( QString::fromUtf8(
                         OGR_F_GetFieldAsString( hFeature.get(),
                             OGR_FD_GetFieldIndex( hLayerDefn, "styleName" ) ) ) );
    QString description( QString::fromUtf8(
                           OGR_F_GetFieldAsString( hFeature.get(),
                               OGR_FD_GetFieldIndex( hLayerDefn, "description" ) ) ) );
    int fid = static_cast<int>( OGR_F_GetFID( hFeature.get() ) );
    if ( tableName == layerName &&
         geometryColumn == geomColumn )
    {
      // Append first all related styles
      QString id( QString::number( fid ) );
      ids.append( id );
      names.append( styleName );
      descriptions.append( description );
      ++ numberOfRelatedStyles;
    }
    else
    {
      int  year, month, day, hour, minute, second, TZ;
      OGR_F_GetFieldAsDateTime( hFeature.get(), OGR_FD_GetFieldIndex( hLayerDefn, "update_time" ),
                                &year, &month, &day, &hour, &minute, &second, &TZ );
      const qlonglong ts = second + minute * 60 + hour * 3600 + day * 24 * 3600 +
                           static_cast<qlonglong>( month ) * 31 * 24 * 3600 + static_cast<qlonglong>( year ) * 12 * 31 * 24 * 3600;

      listTimestamp.append( ts );
      mapIdToStyleName[fid] = styleName;
      mapIdToDescription[fid] = description;
      mapTimestampToId[ts].append( fid );
    }
  }

  std::sort( listTimestamp.begin(), listTimestamp.end() );
  // Sort from most recent to least recent
  for ( int i = listTimestamp.size() - 1; i >= 0; i-- )
  {
    const QList<int> &listId = mapTimestampToId[listTimestamp[i]];
    for ( int j = 0; j < listId.size(); j++ )
    {
      int fid = listId[j];
      QString id( QString::number( fid ) );
      ids.append( id );
      names.append( mapIdToStyleName[fid] );
      descriptions.append( mapIdToDescription[fid] );
    }
  }

  return numberOfRelatedStyles;
}

bool QgsOgrUtils::styleExists( GDALDatasetH hDS, const QString &layerName, const QString &geomColumn, const QString &styleId, QString &errorCause )
{
  errorCause.clear();

  // check if layer_styles table exists
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
    return false;

  const QString realStyleId = styleId.isEmpty() ? layerName : styleId;

  const QString checkQuery = QStringLiteral( "f_table_schema=''"
                             " AND f_table_name=%1"
                             " AND f_geometry_column=%2"
                             " AND styleName=%3" )
                             .arg( QgsOgrProviderUtils::quotedValue( layerName ),
                                   QgsOgrProviderUtils::quotedValue( geomColumn ),
                                   QgsOgrProviderUtils::quotedValue( realStyleId ) );
  OGR_L_SetAttributeFilter( hLayer, checkQuery.toUtf8().constData() );
  OGR_L_ResetReading( hLayer );
  gdal::ogr_feature_unique_ptr hFeature( OGR_L_GetNextFeature( hLayer ) );
  OGR_L_ResetReading( hLayer );

  if ( hFeature )
    return true;

  return false;
}

QString QgsOgrUtils::getStyleById( GDALDatasetH hDS, const QString &styleId, QString &errCause )
{
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
  {
    QgsDebugMsgLevel( QStringLiteral( "No styles available on DB" ), 2 );
    errCause = QObject::tr( "No styles available on DB" );
    return QString();
  }

  bool ok;
  int id = styleId.toInt( &ok );
  if ( !ok )
  {
    errCause = QObject::tr( "Invalid style identifier" );
    return QString();
  }

  gdal::ogr_feature_unique_ptr hFeature( OGR_L_GetFeature( hLayer, id ) );
  if ( !hFeature )
  {
    errCause = QObject::tr( "No style corresponding to style identifier" );
    return QString();
  }

  OGRFeatureDefnH hLayerDefn = OGR_L_GetLayerDefn( hLayer );
  QString styleQML( QString::fromUtf8(
                      OGR_F_GetFieldAsString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "styleQML" ) ) ) );
  OGR_L_ResetReading( hLayer );

  return styleQML;
}

bool QgsOgrUtils::deleteStyleById( GDALDatasetH hDS, const QString &styleId, QString &errCause )
{
  bool deleted;

  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );

  // check if layer_styles table already exist
  if ( !hLayer )
  {
    errCause = QObject::tr( "Connection to database failed" );
    deleted = false;
  }
  else
  {
    if ( OGR_L_DeleteFeature( hLayer, styleId.toInt() ) != OGRERR_NONE )
    {
      errCause = QObject::tr( "Error executing the delete query." );
      deleted = false;
    }
    else
    {
      deleted = true;
    }
  }
  return deleted;
}

QString QgsOgrUtils::loadStoredStyle( GDALDatasetH hDS, const QString &layerName, const QString &geomColumn, QString &styleName, QString &errCause )
{
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
  {
    QgsDebugMsgLevel( QStringLiteral( "No styles available on DB" ), 2 );
    errCause = QObject::tr( "No styles available on DB" );
    return QString();
  }

  QString selectQmlQuery = QStringLiteral( "f_table_schema=''"
                           " AND f_table_name=%1"
                           " AND f_geometry_column=%2"
                           " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                           ",update_time DESC" )
                           .arg( QgsOgrProviderUtils::quotedValue( layerName ),
                                 QgsOgrProviderUtils::quotedValue( geomColumn ) );
  OGR_L_SetAttributeFilter( hLayer, selectQmlQuery.toUtf8().constData() );
  OGR_L_ResetReading( hLayer );
  OGRFeatureDefnH hLayerDefn = OGR_L_GetLayerDefn( hLayer );
  QString styleQML;
  qlonglong moreRecentTimestamp = 0;
  while ( true )
  {
    gdal::ogr_feature_unique_ptr hFeat( OGR_L_GetNextFeature( hLayer ) );
    if ( !hFeat )
      break;
    if ( OGR_F_GetFieldAsInteger( hFeat.get(), OGR_FD_GetFieldIndex( hLayerDefn, "useAsDefault" ) ) )
    {
      styleQML = QString::fromUtf8(
                   OGR_F_GetFieldAsString( hFeat.get(), OGR_FD_GetFieldIndex( hLayerDefn, "styleQML" ) ) );
      styleName = QString::fromUtf8(
                    OGR_F_GetFieldAsString( hFeat.get(), OGR_FD_GetFieldIndex( hLayerDefn, "styleName" ) ) );
      break;
    }

    int  year, month, day, hour, minute, second, TZ;
    OGR_F_GetFieldAsDateTime( hFeat.get(), OGR_FD_GetFieldIndex( hLayerDefn, "update_time" ),
                              &year, &month, &day, &hour, &minute, &second, &TZ );
    qlonglong ts = second + minute * 60 + hour * 3600 + day * 24 * 3600 +
                   static_cast<qlonglong>( month ) * 31 * 24 * 3600 + static_cast<qlonglong>( year ) * 12 * 31 * 24 * 3600;
    if ( ts > moreRecentTimestamp )
    {
      moreRecentTimestamp = ts;
      styleQML = QString::fromUtf8(
                   OGR_F_GetFieldAsString( hFeat.get(), OGR_FD_GetFieldIndex( hLayerDefn, "styleQML" ) ) );
      styleName = QString::fromUtf8(
                    OGR_F_GetFieldAsString( hFeat.get(), OGR_FD_GetFieldIndex( hLayerDefn, "styleName" ) ) );
    }
  }
  OGR_L_ResetReading( hLayer );

  return styleQML;
}

bool QgsOgrUtils::saveStyle(
  GDALDatasetH hDS, const QString &layerName, const QString &geomColumn, const QString &qmlStyle, const QString &sldStyle,
  const QString &styleName, const QString &styleDescription,
  const QString &uiFileContent, bool useAsDefault, QString &errCause
)
{
  // check if layer_styles table already exist
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
  {
    // if not create it
    // Note: we use the same schema as in the SpatiaLite and postgres providers
    //for cross interoperability

    char **options = nullptr;
    // TODO: might need change if other drivers than GPKG / SQLite
    options = CSLSetNameValue( options, "FID", "id" );
    hLayer = GDALDatasetCreateLayer( hDS, "layer_styles", nullptr, wkbNone, options );
    QgsOgrProviderUtils::invalidateCachedDatasets( QString::fromUtf8( GDALGetDescription( hDS ) ) );
    CSLDestroy( options );
    if ( !hLayer )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database." );
      return false;
    }
    bool ok = true;
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "f_table_catalog", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 256 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "f_table_schema", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 256 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "f_table_name", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 256 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "f_geometry_column", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 256 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "styleName", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 30 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "styleQML", OFTString ) );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "styleSLD", OFTString ) );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "useAsDefault", OFTInteger ) );
      OGR_Fld_SetSubType( fld.get(), OFSTBoolean );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "description", OFTString ) );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "owner", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 30 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "ui", OFTString ) );
      OGR_Fld_SetWidth( fld.get(), 30 );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( "update_time", OFTDateTime ) );
      OGR_Fld_SetDefault( fld.get(), "CURRENT_TIMESTAMP" );
      ok &= OGR_L_CreateField( hLayer, fld.get(), true ) == OGRERR_NONE;
    }
    if ( !ok )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database." );
      return false;
    }
  }

  QString realStyleName =
    styleName.isEmpty() ? layerName : styleName;

  OGRFeatureDefnH hLayerDefn = OGR_L_GetLayerDefn( hLayer );

  if ( useAsDefault )
  {
    QString oldDefaultQuery = QStringLiteral( "useAsDefault = 1 AND f_table_schema=''"
                              " AND f_table_name=%1"
                              " AND f_geometry_column=%2" )
                              .arg( QgsOgrProviderUtils::quotedValue( layerName ) )
                              .arg( QgsOgrProviderUtils::quotedValue( geomColumn ) );
    OGR_L_SetAttributeFilter( hLayer, oldDefaultQuery.toUtf8().constData() );
    gdal::ogr_feature_unique_ptr hFeature( OGR_L_GetNextFeature( hLayer ) );
    if ( hFeature )
    {
      OGR_F_SetFieldInteger( hFeature.get(),
                             OGR_FD_GetFieldIndex( hLayerDefn, "useAsDefault" ),
                             0 );
      bool ok = OGR_L_SetFeature( hLayer, hFeature.get() ) == 0;
      if ( !ok )
      {
        QgsDebugError( QStringLiteral( "Could not unset previous useAsDefault style" ) );
      }
    }
  }

  QString checkQuery = QStringLiteral( "f_table_schema=''"
                                       " AND f_table_name=%1"
                                       " AND f_geometry_column=%2"
                                       " AND styleName=%3" )
                       .arg( QgsOgrProviderUtils::quotedValue( layerName ) )
                       .arg( QgsOgrProviderUtils::quotedValue( geomColumn ) )
                       .arg( QgsOgrProviderUtils::quotedValue( realStyleName ) );
  OGR_L_SetAttributeFilter( hLayer, checkQuery.toUtf8().constData() );
  OGR_L_ResetReading( hLayer );
  gdal::ogr_feature_unique_ptr hFeature( OGR_L_GetNextFeature( hLayer ) );
  OGR_L_ResetReading( hLayer );
  bool bNew = true;

  if ( hFeature )
  {
    bNew = false;
  }
  else
  {
    hFeature.reset( OGR_F_Create( hLayerDefn ) );
    OGR_F_SetFieldString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "f_table_catalog" ),
                          "" );
    OGR_F_SetFieldString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "f_table_schema" ),
                          "" );
    OGR_F_SetFieldString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "f_table_name" ),
                          layerName.toUtf8().constData() );
    OGR_F_SetFieldString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "f_geometry_column" ),
                          geomColumn.toUtf8().constData() );
    OGR_F_SetFieldString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "styleName" ),
                          realStyleName.toUtf8().constData() );
    if ( !uiFileContent.isEmpty() )
    {
      OGR_F_SetFieldString( hFeature.get(),
                            OGR_FD_GetFieldIndex( hLayerDefn, "ui" ),
                            uiFileContent.toUtf8().constData() );
    }
  }
  OGR_F_SetFieldString( hFeature.get(),
                        OGR_FD_GetFieldIndex( hLayerDefn, "styleQML" ),
                        qmlStyle.toUtf8().constData() );
  OGR_F_SetFieldString( hFeature.get(),
                        OGR_FD_GetFieldIndex( hLayerDefn, "styleSLD" ),
                        sldStyle.toUtf8().constData() );
  OGR_F_SetFieldInteger( hFeature.get(),
                         OGR_FD_GetFieldIndex( hLayerDefn, "useAsDefault" ),
                         useAsDefault ? 1 : 0 );
  OGR_F_SetFieldString( hFeature.get(),
                        OGR_FD_GetFieldIndex( hLayerDefn, "description" ),
                        ( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ).toUtf8().constData() );
  OGR_F_SetFieldString( hFeature.get(),
                        OGR_FD_GetFieldIndex( hLayerDefn, "owner" ),
                        "" );

  bool bFeatureOK;
  if ( bNew )
    bFeatureOK = OGR_L_CreateFeature( hLayer, hFeature.get() ) == OGRERR_NONE;
  else
    bFeatureOK = OGR_L_SetFeature( hLayer, hFeature.get() ) == OGRERR_NONE;

  if ( !bFeatureOK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error updating style" ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  return true;
}
