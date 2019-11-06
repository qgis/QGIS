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
#include <QTextCodec>
#include <QUuid>
#include <cpl_error.h>
#include <QJsonDocument>

// Starting with GDAL 2.2, there are 2 concepts: unset fields and null fields
// whereas previously there was only unset fields. For QGIS purposes, both
// states (unset/null) are equivalent.
#ifndef OGRNullMarker
#define OGR_F_IsFieldSetAndNotNull OGR_F_IsFieldSet
#endif



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
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,4,0)
        if ( OGR_Fld_GetSubType( fldDef ) == OFSTJSON )
          varType = QVariant::Map;
        else
          varType = QVariant::String;
        break;
#endif
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
        int year, month, day, hour, minute, second, tzf;

        OGR_F_GetFieldAsDateTime( ogrFet, attIndex, &year, &month, &day, &hour, &minute, &second, &tzf );
        if ( field.type() == QVariant::Date )
          value = QDate( year, month, day );
        else if ( field.type() == QVariant::Time )
          value = QTime( hour, minute, second );
        else
          value = QDateTime( QDate( year, month, day ), QTime( hour, minute, second ) );
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

      case QVariant::List:
      {
        if ( field.subType() == QVariant::String )
        {
          QStringList list;
          char **lst = OGR_F_GetFieldAsStringList( ogrFet, attIndex );
          const int count = CSLCount( lst );
          if ( count > 0 )
          {
            for ( int i = 0; i < count; i++ )
            {
              if ( encoding )
                list << encoding->toUnicode( lst[i] );
              else
                list << QString::fromUtf8( lst[i] );
            }
          }
          value = list;
        }
        else
        {
          Q_ASSERT_X( false, "QgsOgrUtils::getOgrFeatureAttribute", "unsupported field type" );
          if ( ok )
            *ok = false;
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
    value = QVariant( QString() );
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
  return qgis::make_unique< QgsPoint >( wkbType, x, y, z, m );
}

std::unique_ptr< QgsMultiPoint > ogrGeometryToQgsMultiPoint( OGRGeometryH geom )
{
  std::unique_ptr< QgsMultiPoint > mp = qgis::make_unique< QgsMultiPoint >();

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

  return qgis::make_unique< QgsLineString>( x, y, z, m, wkbType == QgsWkbTypes::LineString25D );
}

std::unique_ptr< QgsMultiLineString > ogrGeometryToQgsMultiLineString( OGRGeometryH geom )
{
  std::unique_ptr< QgsMultiLineString > mp = qgis::make_unique< QgsMultiLineString >();

  const int count = OGR_G_GetGeometryCount( geom );
  mp->reserve( count );
  for ( int i = 0; i < count; ++i )
  {
    mp->addGeometry( ogrGeometryToQgsLineString( OGR_G_GetGeometryRef( geom, i ) ).release() );
  }

  return mp;
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
      // optimised case for line -- avoid wkb conversion
      return QgsGeometry( ogrGeometryToQgsLineString( geom ) );
    }

    case QgsWkbTypes::MultiLineString:
    {
      // optimised case for line -- avoid wkb conversion
      return QgsGeometry( ogrGeometryToQgsMultiLineString( geom ) );
    }

    default:
      break;
  };

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

