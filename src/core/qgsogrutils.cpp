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
#include "qgsogrprovider.h"
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


void gdal::OGRDataSourceDeleter::operator()( OGRDataSourceH source )
{
  OGR_DS_Destroy( source );
}


void gdal::OGRGeometryDeleter::operator()( OGRGeometryH geometry )
{
  OGR_G_DestroyGeometry( geometry );
}

void gdal::OGRFldDeleter::operator()( OGRFieldDefnH definition )
{
  OGR_Fld_Destroy( definition );
}

void gdal::OGRFeatureDeleter::operator()( OGRFeatureH feature )
{
  OGR_F_Destroy( feature );
}

void gdal::GDALDatasetCloser::operator()( GDALDatasetH dataset )
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


void gdal::GDALWarpOptionsDeleter::operator()( GDALWarpOptions *options )
{
  GDALDestroyWarpOptions( options );
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
      return QTime( value->Date.Hour, value->Date.Minute, static_cast< int >( secondsPart ), static_cast< int >( 1000 * millisecondPart ) );
    }

    case OFTDateTime:
    {
      float secondsPart = 0;
      float millisecondPart = std::modf( value->Date.Second, &secondsPart );
      return QDateTime( QDate( value->Date.Year, value->Date.Month, value->Date.Day ),
                        QTime( value->Date.Hour, value->Date.Minute, static_cast< int >( secondsPart ), static_cast< int >( 1000 * millisecondPart ) ) );
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

std::unique_ptr< OGRField > QgsOgrUtils::variantToOGRField( const QVariant &value )
{
  std::unique_ptr< OGRField > res = std::make_unique< OGRField >();

  switch ( value.type() )
  {
    case QVariant::Invalid:
      OGR_RawField_SetUnset( res.get() );
      break;
    case QVariant::Bool:
      res->Integer = value.toBool() ? 1 : 0;
      break;
    case QVariant::Int:
      res->Integer = value.toInt();
      break;
    case QVariant::LongLong:
      res->Integer64 = value.toLongLong();
      break;
    case QVariant::Double:
      res->Real = value.toDouble();
      break;
    case QVariant::Char:
    case QVariant::String:
      res->String = CPLStrdup( value.toString().toUtf8().constData() );
      break;
    case QVariant::Date:
    {
      const QDate date = value.toDate();
      res->Date.Day = date.day();
      res->Date.Month = date.month();
      res->Date.Year = date.year();
      res->Date.TZFlag = 0;
      break;
    }
    case QVariant::Time:
    {
      const QTime time = value.toTime();
      res->Date.Hour = time.hour();
      res->Date.Minute = time.minute();
      res->Date.Second = time.second() + static_cast< double >( time.msec() ) / 1000;
      res->Date.TZFlag = 0;
      break;
    }
    case QVariant::DateTime:
    {
      const QDateTime dateTime = value.toDateTime();
      res->Date.Day = dateTime.date().day();
      res->Date.Month = dateTime.date().month();
      res->Date.Year = dateTime.date().year();
      res->Date.Hour = dateTime.time().hour();
      res->Date.Minute = dateTime.time().minute();
      res->Date.Second = dateTime.time().second() + static_cast< double >( dateTime.time().msec() ) / 1000;
      res->Date.TZFlag = 0;
      break;
    }

    default:
      QgsDebugMsg( "Unhandled variant type in variantToOGRField" );
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
    QVariant::Type varType;
    switch ( OGR_Fld_GetType( fldDef ) )
    {
      case OFTInteger:
        if ( OGR_Fld_GetSubType( fldDef ) == OFSTBoolean )
          varType = QVariant::Bool;
        else
          varType = QVariant::Int;
        break;
      case OFTInteger64:
        varType = QVariant::LongLong;
        break;
      case OFTReal:
        varType = QVariant::Double;
        break;
      case OFTDate:
        varType = QVariant::Date;
        break;
      case OFTTime:
        varType = QVariant::Time;
        break;
      case OFTDateTime:
        varType = QVariant::DateTime;
        break;
      case OFTString:
        if ( OGR_Fld_GetSubType( fldDef ) == OFSTJSON )
          varType = QVariant::Map;
        else
          varType = QVariant::String;
        break;
      default:
        varType = QVariant::String; // other unsupported, leave it as a string
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

    QgsDebugMsg( QStringLiteral( "ogrFet->GetFieldDefnRef(attindex) returns NULL" ) );
    return QVariant();
  }

  QVariant value;

  if ( ok )
    *ok = true;

  if ( OGR_F_IsFieldSetAndNotNull( ogrFet, attIndex ) )
  {
    switch ( field.type() )
    {
      case QVariant::String:
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
      case QVariant::Int:
        value = QVariant( OGR_F_GetFieldAsInteger( ogrFet, attIndex ) );
        break;
      case QVariant::Bool:
        value = QVariant( bool( OGR_F_GetFieldAsInteger( ogrFet, attIndex ) ) );
        break;
      case QVariant::LongLong:
        value = QVariant( OGR_F_GetFieldAsInteger64( ogrFet, attIndex ) );
        break;
      case QVariant::Double:
        value = QVariant( OGR_F_GetFieldAsDouble( ogrFet, attIndex ) );
        break;
      case QVariant::Date:
      case QVariant::DateTime:
      case QVariant::Time:
      {
        int year, month, day, hour, minute, tzf;
        float second;
        float secondsPart = 0;

        OGR_F_GetFieldAsDateTimeEx( ogrFet, attIndex, &year, &month, &day, &hour, &minute, &second, &tzf );
        float millisecondPart = std::modf( second, &secondsPart );

        if ( field.type() == QVariant::Date )
          value = QDate( year, month, day );
        else if ( field.type() == QVariant::Time )
          value = QTime( hour, minute, static_cast< int >( secondsPart ), static_cast< int >( 1000 * millisecondPart ) );
        else
          value = QDateTime( QDate( year, month, day ),
                             QTime( hour, minute, static_cast< int >( secondsPart ), static_cast< int >( 1000 * millisecondPart ) ) );
      }
      break;

      case QVariant::ByteArray:
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

      case QVariant::StringList:
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

      case QVariant::List:
      {
        switch ( field.subType() )
        {
          case QVariant::String:
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

          case QVariant::Int:
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

          case QVariant::Double:
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

          case QVariant::LongLong:
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

      case QVariant::Map:
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
    value = QVariant( field.type() );
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
  QgsWkbTypes::Type wkbType = static_cast<QgsWkbTypes::Type>( OGR_G_GetGeometryType( geom ) );

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
  QgsWkbTypes::Type wkbType = static_cast<QgsWkbTypes::Type>( OGR_G_GetGeometryType( geom ) );

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

  return std::make_unique< QgsLineString>( x, y, z, m, wkbType == QgsWkbTypes::LineString25D );
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

QgsWkbTypes::Type QgsOgrUtils::ogrGeometryTypeToQgsWkbType( OGRwkbGeometryType ogrGeomType )
{
  switch ( ogrGeomType )
  {
    case wkbUnknown: return QgsWkbTypes::Type::Unknown;
    case wkbPoint: return QgsWkbTypes::Type::Point;
    case wkbLineString: return QgsWkbTypes::Type::LineString;
    case wkbPolygon: return QgsWkbTypes::Type::Polygon;
    case wkbMultiPoint: return QgsWkbTypes::Type::MultiPoint;
    case wkbMultiLineString: return QgsWkbTypes::Type::MultiLineString;
    case wkbMultiPolygon: return QgsWkbTypes::Type::MultiPolygon;
    case wkbGeometryCollection: return QgsWkbTypes::Type::GeometryCollection;
    case wkbCircularString: return QgsWkbTypes::Type::CircularString;
    case wkbCompoundCurve: return QgsWkbTypes::Type::CompoundCurve;
    case wkbCurvePolygon: return QgsWkbTypes::Type::CurvePolygon;
    case wkbMultiCurve: return QgsWkbTypes::Type::MultiCurve;
    case wkbMultiSurface: return QgsWkbTypes::Type::MultiSurface;
    case wkbCurve: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbSurface: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbPolyhedralSurface: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTIN: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTriangle: return QgsWkbTypes::Type::Triangle;

    case wkbNone: return QgsWkbTypes::Type::NoGeometry;
    case wkbLinearRing: return QgsWkbTypes::Type::LineString; // approximate match

    case wkbCircularStringZ: return QgsWkbTypes::Type::CircularStringZ;
    case wkbCompoundCurveZ: return QgsWkbTypes::Type::CompoundCurveZ;
    case wkbCurvePolygonZ: return QgsWkbTypes::Type::CurvePolygonZ;
    case wkbMultiCurveZ: return QgsWkbTypes::Type::MultiCurveZ;
    case wkbMultiSurfaceZ: return QgsWkbTypes::Type::MultiSurfaceZ;
    case wkbCurveZ: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbSurfaceZ: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbPolyhedralSurfaceZ: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTINZ: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTriangleZ: return QgsWkbTypes::Type::TriangleZ;

    case wkbPointM: return QgsWkbTypes::Type::PointM;
    case wkbLineStringM: return QgsWkbTypes::Type::LineStringM;
    case wkbPolygonM: return QgsWkbTypes::Type::PolygonM;
    case wkbMultiPointM: return QgsWkbTypes::Type::MultiPointM;
    case wkbMultiLineStringM: return QgsWkbTypes::Type::MultiLineStringM;
    case wkbMultiPolygonM: return QgsWkbTypes::Type::MultiPolygonM;
    case wkbGeometryCollectionM: return QgsWkbTypes::Type::GeometryCollectionM;
    case wkbCircularStringM: return QgsWkbTypes::Type::CircularStringM;
    case wkbCompoundCurveM: return QgsWkbTypes::Type::CompoundCurveM;
    case wkbCurvePolygonM: return QgsWkbTypes::Type::CurvePolygonM;
    case wkbMultiCurveM: return QgsWkbTypes::Type::MultiCurveM;
    case wkbMultiSurfaceM: return QgsWkbTypes::Type::MultiSurfaceM;
    case wkbCurveM: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbSurfaceM: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbPolyhedralSurfaceM: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTINM: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTriangleM: return QgsWkbTypes::Type::TriangleM;

    case wkbPointZM: return QgsWkbTypes::Type::PointZM;
    case wkbLineStringZM: return QgsWkbTypes::Type::LineStringZM;
    case wkbPolygonZM: return QgsWkbTypes::Type::PolygonZM;
    case wkbMultiPointZM: return QgsWkbTypes::Type::MultiPointZM;
    case wkbMultiLineStringZM: return QgsWkbTypes::Type::MultiLineStringZM;
    case wkbMultiPolygonZM: return QgsWkbTypes::Type::MultiPolygonZM;
    case wkbGeometryCollectionZM: return QgsWkbTypes::Type::GeometryCollectionZM;
    case wkbCircularStringZM: return QgsWkbTypes::Type::CircularStringZM;
    case wkbCompoundCurveZM: return QgsWkbTypes::Type::CompoundCurveZM;
    case wkbCurvePolygonZM: return QgsWkbTypes::Type::CurvePolygonZM;
    case wkbMultiCurveZM: return QgsWkbTypes::Type::MultiCurveZM;
    case wkbMultiSurfaceZM: return QgsWkbTypes::Type::MultiSurfaceZM;
    case wkbCurveZM: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbSurfaceZM: return QgsWkbTypes::Type::Unknown; // not an actual concrete type
    case wkbPolyhedralSurfaceZM: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTINZM: return QgsWkbTypes::Type::Unknown; // no actual matching
    case wkbTriangleZM: return QgsWkbTypes::Type::TriangleZM;

    case wkbPoint25D: return QgsWkbTypes::Type::PointZ;
    case wkbLineString25D: return QgsWkbTypes::Type::LineStringZ;
    case wkbPolygon25D: return QgsWkbTypes::Type::PolygonZ;
    case wkbMultiPoint25D: return QgsWkbTypes::Type::MultiPointZ;
    case wkbMultiLineString25D: return QgsWkbTypes::Type::MultiLineStringZ;
    case wkbMultiPolygon25D: return QgsWkbTypes::Type::MultiPolygonZ;
    case wkbGeometryCollection25D: return QgsWkbTypes::Type::GeometryCollectionZ;
  }

  // should not reach that point normally
  return QgsWkbTypes::Type::Unknown;
}

QgsGeometry QgsOgrUtils::ogrGeometryToQgsGeometry( OGRGeometryH geom )
{
  if ( !geom )
    return QgsGeometry();

  const auto ogrGeomType = OGR_G_GetGeometryType( geom );
  QgsWkbTypes::Type wkbType = ogrGeometryTypeToQgsWkbType( ogrGeomType );

  // optimised case for some geometry classes, avoiding wkb conversion on OGR/QGIS sides
  // TODO - extend to other classes!
  switch ( QgsWkbTypes::flatType( wkbType ) )
  {
    case QgsWkbTypes::Point:
    {
      return QgsGeometry( ogrGeometryToQgsPoint( geom ) );
    }

    case QgsWkbTypes::MultiPoint:
    {
      return QgsGeometry( ogrGeometryToQgsMultiPoint( geom ) );
    }

    case QgsWkbTypes::LineString:
    {
      return QgsGeometry( ogrGeometryToQgsLineString( geom ) );
    }

    case QgsWkbTypes::MultiLineString:
    {
      return QgsGeometry( ogrGeometryToQgsMultiLineString( geom ) );
    }

    case QgsWkbTypes::Polygon:
    {
      return QgsGeometry( ogrGeometryToQgsPolygon( geom ) );
    }

    case QgsWkbTypes::MultiPolygon:
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

  // Read original geometry type
  uint32_t origGeomType;
  memcpy( &origGeomType, wkb + 1, sizeof( uint32_t ) );
  bool hasZ = ( origGeomType >= 1000 && origGeomType < 2000 ) || ( origGeomType >= 3000 && origGeomType < 4000 );
  bool hasM = ( origGeomType >= 2000 && origGeomType < 3000 ) || ( origGeomType >= 3000 && origGeomType < 4000 );

  // PolyhedralSurface and TINs are not supported, map them to multipolygons...
  if ( origGeomType % 1000 == 16 ) // is TIN, TINZ, TINM or TINZM
  {
    // TIN has the same wkb layout as a multipolygon, just need to overwrite the geom types...
    int nDims = 2 + hasZ + hasM;
    uint32_t newMultiType = static_cast<uint32_t>( QgsWkbTypes::zmType( QgsWkbTypes::MultiPolygon, hasZ, hasM ) );
    uint32_t newSingleType = static_cast<uint32_t>( QgsWkbTypes::zmType( QgsWkbTypes::Polygon, hasZ, hasM ) );
    unsigned char *wkbptr = wkb;

    // Endianness
    wkbptr += 1;

    // Overwrite geom type
    memcpy( wkbptr, &newMultiType, sizeof( uint32_t ) );
    wkbptr += 4;

    // Geom count
    uint32_t numGeoms;
    memcpy( &numGeoms, wkb + 5, sizeof( uint32_t ) );
    wkbptr += 4;

    // For each part, overwrite the geometry type to polygon (Z|M)
    for ( uint32_t i = 0; i < numGeoms; ++i )
    {
      // Endianness
      wkbptr += 1;

      // Overwrite geom type
      memcpy( wkbptr, &newSingleType, sizeof( uint32_t ) );
      wkbptr += sizeof( uint32_t );

      // skip coordinates
      uint32_t nRings;
      memcpy( &nRings, wkbptr, sizeof( uint32_t ) );
      wkbptr += sizeof( uint32_t );

      for ( uint32_t j = 0; j < nRings; ++j )
      {
        uint32_t nPoints;
        memcpy( &nPoints, wkbptr, sizeof( uint32_t ) );
        wkbptr += sizeof( uint32_t ) + sizeof( double ) * nDims * nPoints;
      }
    }
  }
  else if ( origGeomType % 1000 == 15 ) // PolyhedralSurface, PolyhedralSurfaceZ, PolyhedralSurfaceM or PolyhedralSurfaceZM
  {
    // PolyhedralSurface has the same wkb layout as a MultiPolygon, just need to overwrite the geom type...
    uint32_t newType = static_cast<uint32_t>( QgsWkbTypes::zmType( QgsWkbTypes::MultiPolygon, hasZ, hasM ) );
    // Overwrite geom type
    memcpy( wkb + 1, &newType, sizeof( uint32_t ) );
  }

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
    const QString srsWkt = crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL );
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
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( path, false, QStringList(), 0, errCause, false );
  return layer ? layer->GetMetadataItem( QStringLiteral( "ENCODING_FROM_CPG" ), QStringLiteral( "SHAPEFILE" ) ) : QString();
#else
  if ( !QFileInfo::exists( path ) )
    return QString();

  // first try to read cpg file, if present
  const QFileInfo fi( path );
  const QString baseName = fi.completeBaseName();
  const QString cpgPath = fi.dir().filePath( QStringLiteral( "%1.%2" ).arg( baseName, fi.suffix() == QLatin1String( "SHP" ) ? QStringLiteral( "CPG" ) : QStringLiteral( "cpg" ) ) );
  if ( QFile::exists( cpgPath ) )
  {
    QFile cpgFile( cpgPath );
    if ( cpgFile.open( QIODevice::ReadOnly ) )
    {
      QTextStream cpgStream( &cpgFile );
      const QString cpgString = cpgStream.readLine();
      cpgFile.close();

      if ( !cpgString.isEmpty() )
      {
        // from OGRShapeLayer::ConvertCodePage
        // https://github.com/OSGeo/gdal/blob/master/gdal/ogr/ogrsf_frmts/shape/ogrshapelayer.cpp#L342
        bool ok = false;
        int cpgCodePage = cpgString.toInt( &ok );
        if ( ok && ( ( cpgCodePage >= 437 && cpgCodePage <= 950 )
                     || ( cpgCodePage >= 1250 && cpgCodePage <= 1258 ) ) )
        {
          return QStringLiteral( "CP%1" ).arg( cpgCodePage );
        }
        else if ( cpgString.startsWith( QLatin1String( "8859" ) ) )
        {
          if ( cpgString.length() > 4 && cpgString.at( 4 ) == '-' )
            return QStringLiteral( "ISO-8859-%1" ).arg( cpgString.mid( 5 ) );
          else
            return QStringLiteral( "ISO-8859-%1" ).arg( cpgString.mid( 4 ) );
        }
        else if ( cpgString.startsWith( QLatin1String( "UTF-8" ), Qt::CaseInsensitive ) ||
                  cpgString.startsWith( QLatin1String( "UTF8" ), Qt::CaseInsensitive ) )
          return QStringLiteral( "UTF-8" );
        else if ( cpgString.startsWith( QLatin1String( "ANSI 1251" ), Qt::CaseInsensitive ) )
          return QStringLiteral( "CP1251" );

        return cpgString;
      }
    }
  }

  return QString();
#endif
}

QString QgsOgrUtils::readShapefileEncodingFromLdid( const QString &path )
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( path, false, QStringList(), 0, errCause, false );
  return layer ? layer->GetMetadataItem( QStringLiteral( "ENCODING_FROM_LDID" ), QStringLiteral( "SHAPEFILE" ) ) : QString();
#else
  // from OGRShapeLayer::ConvertCodePage
  // https://github.com/OSGeo/gdal/blob/master/gdal/ogr/ogrsf_frmts/shape/ogrshapelayer.cpp#L342

  if ( !QFileInfo::exists( path ) )
    return QString();

  // first try to read cpg file, if present
  const QFileInfo fi( path );
  const QString baseName = fi.completeBaseName();

  // fallback to LDID value, read from DBF file
  const QString dbfPath = fi.dir().filePath( QStringLiteral( "%1.%2" ).arg( baseName, fi.suffix() == QLatin1String( "SHP" ) ? QStringLiteral( "DBF" ) : QStringLiteral( "dbf" ) ) );
  if ( QFile::exists( dbfPath ) )
  {
    QFile dbfFile( dbfPath );
    if ( dbfFile.open( QIODevice::ReadOnly ) )
    {
      dbfFile.read( 29 );
      QDataStream dbfIn( &dbfFile );
      dbfIn.setByteOrder( QDataStream::LittleEndian );
      quint8 ldid;
      dbfIn >> ldid;
      dbfFile.close();

      int nCP = -1;  // Windows code page.

      // http://www.autopark.ru/ASBProgrammerGuide/DBFSTRUC.HTM
      switch ( ldid )
      {
        case 1: nCP = 437;      break;
        case 2: nCP = 850;      break;
        case 3: nCP = 1252;     break;
        case 4: nCP = 10000;    break;
        case 8: nCP = 865;      break;
        case 10: nCP = 850;     break;
        case 11: nCP = 437;     break;
        case 13: nCP = 437;     break;
        case 14: nCP = 850;     break;
        case 15: nCP = 437;     break;
        case 16: nCP = 850;     break;
        case 17: nCP = 437;     break;
        case 18: nCP = 850;     break;
        case 19: nCP = 932;     break;
        case 20: nCP = 850;     break;
        case 21: nCP = 437;     break;
        case 22: nCP = 850;     break;
        case 23: nCP = 865;     break;
        case 24: nCP = 437;     break;
        case 25: nCP = 437;     break;
        case 26: nCP = 850;     break;
        case 27: nCP = 437;     break;
        case 28: nCP = 863;     break;
        case 29: nCP = 850;     break;
        case 31: nCP = 852;     break;
        case 34: nCP = 852;     break;
        case 35: nCP = 852;     break;
        case 36: nCP = 860;     break;
        case 37: nCP = 850;     break;
        case 38: nCP = 866;     break;
        case 55: nCP = 850;     break;
        case 64: nCP = 852;     break;
        case 77: nCP = 936;     break;
        case 78: nCP = 949;     break;
        case 79: nCP = 950;     break;
        case 80: nCP = 874;     break;
        case 87: return QStringLiteral( "ISO-8859-1" );
        case 88: nCP = 1252;     break;
        case 89: nCP = 1252;     break;
        case 100: nCP = 852;     break;
        case 101: nCP = 866;     break;
        case 102: nCP = 865;     break;
        case 103: nCP = 861;     break;
        case 104: nCP = 895;     break;
        case 105: nCP = 620;     break;
        case 106: nCP = 737;     break;
        case 107: nCP = 857;     break;
        case 108: nCP = 863;     break;
        case 120: nCP = 950;     break;
        case 121: nCP = 949;     break;
        case 122: nCP = 936;     break;
        case 123: nCP = 932;     break;
        case 124: nCP = 874;     break;
        case 134: nCP = 737;     break;
        case 135: nCP = 852;     break;
        case 136: nCP = 857;     break;
        case 150: nCP = 10007;   break;
        case 151: nCP = 10029;   break;
        case 200: nCP = 1250;    break;
        case 201: nCP = 1251;    break;
        case 202: nCP = 1254;    break;
        case 203: nCP = 1253;    break;
        case 204: nCP = 1257;    break;
        default: break;
      }

      if ( nCP != -1 )
      {
        return QStringLiteral( "CP%1" ).arg( nCP );
      }
    }
  }
  return QString();
#endif
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

  auto convertSize = []( const QString & size, double & value, QgsUnitTypes::RenderUnit & unit )->bool
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
        unit = QgsUnitTypes::RenderPoints;
        value *= PX_TO_PT_FACTOR;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "pt" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = QgsUnitTypes::RenderPoints;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "mm" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = QgsUnitTypes::RenderMillimeters;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "cm" ), Qt::CaseInsensitive ) == 0 )
      {
        value *= 10;
        unit = QgsUnitTypes::RenderMillimeters;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "in" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = QgsUnitTypes::RenderInches;
        return true;
      }
      else if ( unitString.compare( QLatin1String( "g" ), Qt::CaseInsensitive ) == 0 )
      {
        unit = QgsUnitTypes::RenderMapUnits;
        return true;
      }
      QgsDebugMsg( QStringLiteral( "Unknown unit %1" ).arg( unitString ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Could not parse style size %1" ).arg( size ) );
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
    QgsUnitTypes::RenderUnit lineWidthUnit = QgsUnitTypes::RenderMillimeters;
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
        QgsUnitTypes::RenderUnit patternUnits = QgsUnitTypes::RenderMillimeters;
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
    QgsUnitTypes::RenderUnit symbolSizeUnit = QgsUnitTypes::RenderMillimeters;
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
      for ( const QString &family : std::as_const( families ) )
      {
        if ( QgsFontUtils::fontFamilyMatchOnSystem( family ) )
        {
          familyFound = true;
          fontFamily = family;
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
          fontMarker->setStrokeWidthUnit( QgsUnitTypes::RenderPoints );
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

void QgsOgrUtils::ogrFieldTypeToQVariantType( OGRFieldType ogrType, OGRFieldSubType ogrSubType, QVariant::Type &variantType, QVariant::Type &variantSubType )
{
  variantType = QVariant::Type::Invalid;
  variantSubType = QVariant::Type::Invalid;

  switch ( ogrType )
  {
    case OFTInteger:
      if ( ogrSubType == OFSTBoolean )
      {
        variantType = QVariant::Bool;
        ogrSubType = OFSTBoolean;
      }
      else
        variantType = QVariant::Int;
      break;
    case OFTInteger64:
      variantType = QVariant::LongLong;
      break;
    case OFTReal:
      variantType = QVariant::Double;
      break;
    case OFTDate:
      variantType = QVariant::Date;
      break;
    case OFTTime:
      variantType = QVariant::Time;
      break;
    case OFTDateTime:
      variantType = QVariant::DateTime;
      break;

    case OFTBinary:
      variantType = QVariant::ByteArray;
      break;

    case OFTString:
    case OFTWideString:
      if ( ogrSubType == OFSTJSON )
      {
        ogrSubType = OFSTJSON;
        variantType = QVariant::Map;
        variantSubType = QVariant::String;
      }
      else
      {
        variantType = QVariant::String;
      }
      break;

    case OFTStringList:
    case OFTWideStringList:
      variantType = QVariant::StringList;
      variantSubType = QVariant::String;
      break;

    case OFTIntegerList:
      variantType = QVariant::List;
      variantSubType = QVariant::Int;
      break;

    case OFTRealList:
      variantType = QVariant::List;
      variantSubType = QVariant::Double;
      break;

    case OFTInteger64List:
      variantType = QVariant::List;
      variantSubType = QVariant::LongLong;
      break;
  }
}

void QgsOgrUtils::variantTypeToOgrFieldType( QVariant::Type variantType, OGRFieldType &ogrType, OGRFieldSubType &ogrSubType )
{
  ogrSubType = OFSTNone;
  switch ( variantType )
  {
    case QVariant::Bool:
      ogrType = OFTInteger;
      ogrSubType = OFSTBoolean;
      break;

    case QVariant::Int:
      ogrType = OFTInteger;
      break;

    case QVariant::LongLong:
      ogrType = OFTInteger64;
      break;

    case QVariant::Double:
      ogrType = OFTReal;
      break;

    case QVariant::Char:
      ogrType = OFTString;
      break;

    case QVariant::String:
      ogrType = OFTString;
      break;

    case QVariant::StringList:
      ogrType = OFTStringList;
      break;

    case QVariant::ByteArray:
      ogrType = OFTBinary;
      break;

    case QVariant::Date:
      ogrType = OFTDate;
      break;

    case QVariant::Time:
      ogrType = OFTTime;
      break;
    case QVariant::DateTime:
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


#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
std::unique_ptr< QgsFieldDomain > QgsOgrUtils::convertFieldDomain( OGRFieldDomainH domain )
{
  if ( !domain )
    return nullptr;

  const QString name{ OGR_FldDomain_GetName( domain ) };
  const QString description{ OGR_FldDomain_GetDescription( domain ) };

  QVariant::Type fieldType = QVariant::Type::Invalid;
  QVariant::Type fieldSubType = QVariant::Type::Invalid;
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
      std::unique_ptr< OGRField > min = variantToOGRField( qgis::down_cast< const QgsRangeFieldDomain * >( domain )->minimum() );
      std::unique_ptr< OGRField > max = variantToOGRField( qgis::down_cast< const QgsRangeFieldDomain * >( domain )->maximum() );
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
  }

  return res;
}

#endif
