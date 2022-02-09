/***************************************************************************
                          qgsvectorfilewriter.cpp
                          generic vector file writer
                             -------------------
    begin                : Sat Jun 16 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include "qgsrenderer.h"
#include "qgssymbollayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslocalec.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsgeometryengine.h"
#include "qgsproviderregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsreadwritelocker.h"
#include "qgssymbol.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#include <QSet>
#include <QMetaType>
#include <QMutex>
#include <QRegularExpression>
#include <QJsonDocument>

#include <cassert>
#include <cstdlib> // size_t
#include <limits> // std::numeric_limits

#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal.h>

QgsField QgsVectorFileWriter::FieldValueConverter::fieldDefinition( const QgsField &field )
{
  return field;
}

QVariant QgsVectorFileWriter::FieldValueConverter::convert( int /*fieldIdxInLayer*/, const QVariant &value )
{
  return value;
}

QgsVectorFileWriter::FieldValueConverter *QgsVectorFileWriter::FieldValueConverter::clone() const
{
  return new FieldValueConverter( *this );
}

QgsVectorFileWriter::QgsVectorFileWriter(
  const QString &vectorFileName,
  const QString &fileEncoding,
  const QgsFields &fields,
  QgsWkbTypes::Type geometryType,
  const QgsCoordinateReferenceSystem &srs,
  const QString &driverName,
  const QStringList &datasourceOptions,
  const QStringList &layerOptions,
  QString *newFilename,
  SymbologyExport symbologyExport,
  QgsFeatureSink::SinkFlags sinkFlags,
  QString *newLayer,
  const QgsCoordinateTransformContext &transformContext,
  FieldNameSource fieldNameSource
)
  : mError( NoError )
  , mWkbType( geometryType )
  , mSymbologyExport( symbologyExport )
  , mSymbologyScale( 1.0 )
{
  init( vectorFileName, fileEncoding, fields,  geometryType,
        srs, driverName, datasourceOptions, layerOptions, newFilename, nullptr,
        QString(), CreateOrOverwriteFile, newLayer, sinkFlags, transformContext, fieldNameSource );
}

QgsVectorFileWriter::QgsVectorFileWriter(
  const QString &vectorFileName,
  const QString &fileEncoding,
  const QgsFields &fields,
  QgsWkbTypes::Type geometryType,
  const QgsCoordinateReferenceSystem &srs,
  const QString &driverName,
  const QStringList &datasourceOptions,
  const QStringList &layerOptions,
  QString *newFilename,
  QgsVectorFileWriter::SymbologyExport symbologyExport,
  FieldValueConverter *fieldValueConverter,
  const QString &layerName,
  ActionOnExistingFile action,
  QString *newLayer,
  const QgsCoordinateTransformContext &transformContext,
  QgsFeatureSink::SinkFlags sinkFlags,
  FieldNameSource fieldNameSource
)
  : mError( NoError )
  , mWkbType( geometryType )
  , mSymbologyExport( symbologyExport )
  , mSymbologyScale( 1.0 )
{
  init( vectorFileName, fileEncoding, fields, geometryType, srs, driverName,
        datasourceOptions, layerOptions, newFilename, fieldValueConverter,
        layerName, action, newLayer, sinkFlags, transformContext, fieldNameSource );
}

QgsVectorFileWriter *QgsVectorFileWriter::create(
  const QString &fileName,
  const QgsFields &fields,
  QgsWkbTypes::Type geometryType,
  const QgsCoordinateReferenceSystem &srs,
  const QgsCoordinateTransformContext &transformContext,
  const QgsVectorFileWriter::SaveVectorOptions &options,
  QgsFeatureSink::SinkFlags sinkFlags,
  QString *newFilename,
  QString *newLayer
)
{
  Q_NOWARN_DEPRECATED_PUSH
  return new QgsVectorFileWriter( fileName, options.fileEncoding, fields, geometryType, srs,
                                  options.driverName, options.datasourceOptions, options.layerOptions,
                                  newFilename, options.symbologyExport, options.fieldValueConverter, options.layerName,
                                  options.actionOnExistingFile, newLayer, transformContext, sinkFlags, options.fieldNameSource );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsVectorFileWriter::supportsFeatureStyles( const QString &driverName )
{
  if ( driverName == QLatin1String( "MapInfo MIF" ) )
  {
    return true;
  }
  GDALDriverH gdalDriver = GDALGetDriverByName( driverName.toLocal8Bit().constData() );
  if ( !gdalDriver )
    return false;

  char **driverMetadata = GDALGetMetadata( gdalDriver, nullptr );
  if ( !driverMetadata )
    return false;

  return CSLFetchBoolean( driverMetadata, GDAL_DCAP_FEATURE_STYLES, false );
}

void QgsVectorFileWriter::init( QString vectorFileName,
                                QString fileEncoding,
                                const QgsFields &fields,
                                QgsWkbTypes::Type geometryType,
                                QgsCoordinateReferenceSystem srs,
                                const QString &driverName,
                                QStringList datasourceOptions,
                                QStringList layerOptions,
                                QString *newFilename,
                                FieldValueConverter *fieldValueConverter,
                                const QString &layerNameIn,
                                ActionOnExistingFile action,
                                QString *newLayer, SinkFlags sinkFlags,
                                const QgsCoordinateTransformContext &transformContext, FieldNameSource fieldNameSource )
{
  mRenderContext.setRendererScale( mSymbologyScale );

  if ( vectorFileName.isEmpty() )
  {
    mErrorMessage = QObject::tr( "Empty filename given" );
    mError = ErrCreateDataSource;
    return;
  }

  if ( driverName == QLatin1String( "MapInfo MIF" ) )
  {
    mOgrDriverName = QStringLiteral( "MapInfo File" );
  }
  else if ( driverName == QLatin1String( "SpatiaLite" ) )
  {
    mOgrDriverName = QStringLiteral( "SQLite" );
    if ( !datasourceOptions.contains( QStringLiteral( "SPATIALITE=YES" ) ) )
    {
      datasourceOptions.append( QStringLiteral( "SPATIALITE=YES" ) );
    }
  }
  else if ( driverName == QLatin1String( "DBF file" ) )
  {
    mOgrDriverName = QStringLiteral( "ESRI Shapefile" );
    if ( !layerOptions.contains( QStringLiteral( "SHPT=NULL" ) ) )
    {
      layerOptions.append( QStringLiteral( "SHPT=NULL" ) );
    }
    srs = QgsCoordinateReferenceSystem();
  }
  else
  {
    mOgrDriverName = driverName;
  }

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,3,1)
  QString fidFieldName;
  if ( mOgrDriverName == QLatin1String( "GPKG" ) )
  {
    for ( const QString &layerOption : layerOptions )
    {
      if ( layerOption.startsWith( QLatin1String( "FID=" ) ) )
      {
        fidFieldName = layerOption.mid( 4 );
        break;
      }
    }
    if ( fidFieldName.isEmpty() )
      fidFieldName = QStringLiteral( "fid" );
  }
#endif

  // find driver in OGR
  OGRSFDriverH poDriver;
  QgsApplication::registerOgrDrivers();

  poDriver = OGRGetDriverByName( mOgrDriverName.toLocal8Bit().constData() );

  if ( !poDriver )
  {
    mErrorMessage = QObject::tr( "OGR driver for '%1' not found (OGR error: %2)" )
                    .arg( driverName,
                          QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrDriverNotFound;
    return;
  }

  MetaData metadata;
  bool metadataFound = driverMetadata( driverName, metadata );

  if ( mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    if ( layerOptions.join( QString() ).toUpper().indexOf( QLatin1String( "ENCODING=" ) ) == -1 )
    {
      layerOptions.append( "ENCODING=" + convertCodecNameForEncodingOption( fileEncoding ) );
    }

    if ( driverName == QLatin1String( "ESRI Shapefile" ) && !vectorFileName.endsWith( QLatin1String( ".shp" ), Qt::CaseInsensitive ) )
    {
      vectorFileName += QLatin1String( ".shp" );
    }
    else if ( driverName == QLatin1String( "DBF file" ) && !vectorFileName.endsWith( QLatin1String( ".dbf" ), Qt::CaseInsensitive ) )
    {
      vectorFileName += QLatin1String( ".dbf" );
    }

    if ( action == CreateOrOverwriteFile || action == CreateOrOverwriteLayer )
      deleteShapeFile( vectorFileName );
  }
  else
  {
    if ( metadataFound )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      QStringList allExts = metadata.ext.split( ' ', QString::SkipEmptyParts );
#else
      QStringList allExts = metadata.ext.split( ' ', Qt::SkipEmptyParts );
#endif
      bool found = false;
      const auto constAllExts = allExts;
      for ( const QString &ext : constAllExts )
      {
        if ( vectorFileName.endsWith( '.' + ext, Qt::CaseInsensitive ) )
        {
          found = true;
          break;
        }
      }

      if ( !found )
      {
        vectorFileName += '.' + allExts[0];
      }
    }

    if ( action == CreateOrOverwriteFile )
    {
      if ( vectorFileName.endsWith( QLatin1String( ".gdb" ), Qt::CaseInsensitive ) )
      {
        QDir dir( vectorFileName );
        if ( dir.exists() )
        {
          QFileInfoList fileList = dir.entryInfoList(
                                     QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst );
          const auto constFileList = fileList;
          for ( const QFileInfo &info : constFileList )
          {
            QFile::remove( info.absoluteFilePath() );
          }
        }
        QDir().rmdir( vectorFileName );
      }
      else
      {
        QFile::remove( vectorFileName );
      }
    }
  }

  if ( metadataFound && !metadata.compulsoryEncoding.isEmpty() )
  {
    if ( fileEncoding.compare( metadata.compulsoryEncoding, Qt::CaseInsensitive ) != 0 )
    {
      QgsDebugMsgLevel( QStringLiteral( "forced %1 encoding for %2" ).arg( metadata.compulsoryEncoding, driverName ), 2 );
      fileEncoding = metadata.compulsoryEncoding;
    }

  }

  char **options = nullptr;
  if ( !datasourceOptions.isEmpty() )
  {
    options = new char *[ datasourceOptions.size() + 1 ];
    for ( int i = 0; i < datasourceOptions.size(); i++ )
    {
      QgsDebugMsgLevel( QStringLiteral( "-dsco=%1" ).arg( datasourceOptions[i] ), 2 );
      options[i] = CPLStrdup( datasourceOptions[i].toLocal8Bit().constData() );
    }
    options[ datasourceOptions.size()] = nullptr;
  }
  mAttrIdxToOgrIdx.remove( 0 );

  // create the data source
  if ( action == CreateOrOverwriteFile )
    mDS.reset( OGR_Dr_CreateDataSource( poDriver, vectorFileName.toUtf8().constData(), options ) );
  else
    mDS.reset( OGROpen( vectorFileName.toUtf8().constData(), TRUE, nullptr ) );

  if ( options )
  {
    for ( int i = 0; i < datasourceOptions.size(); i++ )
      CPLFree( options[i] );
    delete [] options;
    options = nullptr;
  }

  if ( !mDS )
  {
    mError = ErrCreateDataSource;
    if ( action == CreateOrOverwriteFile )
      mErrorMessage = QObject::tr( "Creation of data source failed (OGR error: %1)" )
                      .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    else
      mErrorMessage = QObject::tr( "Opening of data source in update mode failed (OGR error: %1)" )
                      .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return;
  }

  QString layerName( layerNameIn );
  if ( layerName.isEmpty() )
    layerName = QFileInfo( vectorFileName ).baseName();

  if ( action == CreateOrOverwriteLayer )
  {
    const int layer_count = OGR_DS_GetLayerCount( mDS.get() );
    for ( int i = 0; i < layer_count; i++ )
    {
      OGRLayerH hLayer = OGR_DS_GetLayer( mDS.get(), i );
      if ( EQUAL( OGR_L_GetName( hLayer ), layerName.toUtf8().constData() ) )
      {
        if ( OGR_DS_DeleteLayer( mDS.get(), i ) != OGRERR_NONE )
        {
          mError = ErrCreateLayer;
          mErrorMessage = QObject::tr( "Overwriting of existing layer failed (OGR error: %1)" )
                          .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
          return;
        }
        break;
      }
    }
  }

  if ( action == CreateOrOverwriteFile )
  {
    QgsDebugMsgLevel( QStringLiteral( "Created data source" ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Opened data source in update mode" ), 2 );
  }

  // use appropriate codec
  mCodec = QTextCodec::codecForName( fileEncoding.toLocal8Bit().constData() );
  if ( !mCodec )
  {
    QgsDebugMsg( "error finding QTextCodec for " + fileEncoding );

    QgsSettings settings;
    QString enc = settings.value( QStringLiteral( "UI/encoding" ), "System" ).toString();
    mCodec = QTextCodec::codecForName( enc.toLocal8Bit().constData() );
    if ( !mCodec )
    {
      QgsDebugMsg( "error finding QTextCodec for " + enc );
      mCodec = QTextCodec::codecForLocale();
      Q_ASSERT( mCodec );
    }
  }

  // consider spatial reference system of the layer
  if ( driverName == QLatin1String( "KML" ) || driverName == QLatin1String( "LIBKML" ) || driverName == QLatin1String( "GPX" ) )
  {
    if ( srs.authid() != QLatin1String( "EPSG:4326" ) )
    {
      // Those drivers outputs WGS84 geometries, let's align our output CRS to have QGIS take charge of geometry transformation
      QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
      mCoordinateTransform.reset( new QgsCoordinateTransform( srs, wgs84, transformContext ) );
      srs = wgs84;
    }
  }

  mOgrRef = QgsOgrUtils::crsToOGRSpatialReference( srs );

  // datasource created, now create the output layer
  OGRwkbGeometryType wkbType = ogrTypeFromWkbType( geometryType );

  // Remove FEATURE_DATASET layer option (used for ESRI File GDB driver) if its value is not set
  int optIndex = layerOptions.indexOf( QLatin1String( "FEATURE_DATASET=" ) );
  if ( optIndex != -1 )
  {
    layerOptions.removeAt( optIndex );
  }

  if ( !layerOptions.isEmpty() )
  {
    options = new char *[ layerOptions.size() + 1 ];
    for ( int i = 0; i < layerOptions.size(); i++ )
    {
      QgsDebugMsgLevel( QStringLiteral( "-lco=%1" ).arg( layerOptions[i] ), 2 );
      options[i] = CPLStrdup( layerOptions[i].toLocal8Bit().constData() );
    }
    options[ layerOptions.size()] = nullptr;
  }

  // disable encoding conversion of OGR Shapefile layer
  CPLSetConfigOption( "SHAPE_ENCODING", "" );

  if ( action == CreateOrOverwriteFile || action == CreateOrOverwriteLayer )
  {
    mLayer = OGR_DS_CreateLayer( mDS.get(), layerName.toUtf8().constData(), mOgrRef, wkbType, options );
    if ( newLayer && mLayer )
    {
      *newLayer = OGR_L_GetName( mLayer );
      if ( driverName == QLatin1String( "GPX" ) )
      {
        // See logic in GDAL ogr/ogrsf_frmts/gpx/ogrgpxdatasource.cpp ICreateLayer()
        switch ( QgsWkbTypes::flatType( geometryType ) )
        {
          case QgsWkbTypes::Point:
          {
            if ( !EQUAL( layerName.toUtf8().constData(), "track_points" ) &&
                 !EQUAL( layerName.toUtf8().constData(), "route_points" ) )
            {
              *newLayer = QStringLiteral( "waypoints" );
            }
          }
          break;

          case QgsWkbTypes::LineString:
          {
            const char *pszForceGPXTrack
              = CSLFetchNameValue( options, "FORCE_GPX_TRACK" );
            if ( pszForceGPXTrack && CPLTestBool( pszForceGPXTrack ) )
              *newLayer = QStringLiteral( "tracks" );
            else
              *newLayer = QStringLiteral( "routes" );

          }
          break;

          case QgsWkbTypes::MultiLineString:
          {
            const char *pszForceGPXRoute
              = CSLFetchNameValue( options, "FORCE_GPX_ROUTE" );
            if ( pszForceGPXRoute && CPLTestBool( pszForceGPXRoute ) )
              *newLayer = QStringLiteral( "routes" );
            else
              *newLayer = QStringLiteral( "tracks" );
          }
          break;

          default:
            break;
        }
      }
    }
  }
  else if ( driverName == QLatin1String( "DGN" ) )
  {
    mLayer = OGR_DS_GetLayerByName( mDS.get(), "elements" );
  }
  else
  {
    mLayer = OGR_DS_GetLayerByName( mDS.get(), layerName.toUtf8().constData() );
  }

  if ( options )
  {
    for ( int i = 0; i < layerOptions.size(); i++ )
      CPLFree( options[i] );
    delete [] options;
    options = nullptr;
  }

  if ( srs.isValid() )
  {
    if ( mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
    {
      QString layerName = vectorFileName.left( vectorFileName.indexOf( QLatin1String( ".shp" ), Qt::CaseInsensitive ) );
      QFile prjFile( layerName + ".qpj" );
      if ( prjFile.exists() )
        prjFile.remove();
    }
  }

  if ( !mLayer )
  {
    if ( action == CreateOrOverwriteFile || action == CreateOrOverwriteLayer )
      mErrorMessage = QObject::tr( "Creation of layer failed (OGR error: %1)" )
                      .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    else
      mErrorMessage = QObject::tr( "Opening of layer failed (OGR error: %1)" )
                      .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrCreateLayer;
    return;
  }

  OGRFeatureDefnH defn = OGR_L_GetLayerDefn( mLayer );

  QgsDebugMsgLevel( QStringLiteral( "created layer" ), 2 );

  // create the fields
  QgsDebugMsgLevel( "creating " + QString::number( fields.size() ) + " fields", 2 );

  mFields = fields;
  mAttrIdxToOgrIdx.clear();
  QSet<int> existingIdxs;

  mFieldValueConverter = fieldValueConverter;

  switch ( action )
  {
    case CreateOrOverwriteFile:
    case CreateOrOverwriteLayer:
    case AppendToLayerAddFields:
    {
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        QgsField attrField = fields.at( fldIdx );

        if ( fieldValueConverter )
        {
          attrField = fieldValueConverter->fieldDefinition( fields.at( fldIdx ) );
        }

        if ( action == AppendToLayerAddFields )
        {
          int ogrIdx = OGR_FD_GetFieldIndex( defn, mCodec->fromUnicode( attrField.name() ) );
          if ( ogrIdx >= 0 )
          {
            mAttrIdxToOgrIdx.insert( fldIdx, ogrIdx );
            continue;
          }
        }

        QString name;
        switch ( fieldNameSource )
        {
          case Original:
            name = attrField.name();
            break;

          case PreferAlias:
            name = !attrField.alias().isEmpty() ? attrField.alias() : attrField.name();
            break;
        }

        OGRFieldType ogrType = OFTString; //default to string
        OGRFieldSubType ogrSubType = OFSTNone;
        int ogrWidth = attrField.length();
        int ogrPrecision = attrField.precision();
        if ( ogrPrecision > 0 )
          ++ogrWidth;

        switch ( attrField.type() )
        {
          case QVariant::LongLong:
          {
            const char *pszDataTypes = GDALGetMetadataItem( poDriver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
            if ( pszDataTypes && strstr( pszDataTypes, "Integer64" ) )
              ogrType = OFTInteger64;
            else
              ogrType = OFTReal;
            ogrWidth = ogrWidth > 0 && ogrWidth <= 20 ? ogrWidth : 20;
            ogrPrecision = 0;
            break;
          }
          case QVariant::String:
            ogrType = OFTString;
            if ( ( ogrWidth <= 0 || ogrWidth > 255 ) && mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
              ogrWidth = 255;
            break;

          case QVariant::Int:
            ogrType = OFTInteger;
            ogrWidth = ogrWidth > 0 && ogrWidth <= 10 ? ogrWidth : 10;
            ogrPrecision = 0;
            break;

          case QVariant::Bool:
            ogrType = OFTInteger;
            ogrSubType = OFSTBoolean;
            ogrWidth = 1;
            ogrPrecision = 0;
            break;

          case QVariant::Double:
#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,3,1)
            if ( mOgrDriverName == QLatin1String( "GPKG" ) && attrField.precision() == 0 && attrField.name().compare( fidFieldName, Qt::CaseInsensitive ) == 0 )
            {
              // Convert field to match required FID type
              ogrType = OFTInteger64;
              break;
            }
#endif
            ogrType = OFTReal;
            break;

          case QVariant::Date:
            ogrType = OFTDate;
            break;

          case QVariant::Time:
            if ( mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
            {
              ogrType = OFTString;
              ogrWidth = 12; // %02d:%02d:%06.3f
            }
            else
            {
              ogrType = OFTTime;
            }
            break;

          case QVariant::DateTime:
            if ( mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
            {
              ogrType = OFTString;
              ogrWidth = 24; // "%04d/%02d/%02d %02d:%02d:%06.3f"
            }
            else
            {
              ogrType = OFTDateTime;
            }
            break;

          case QVariant::ByteArray:
            ogrType = OFTBinary;
            break;

          case QVariant::StringList:
          {
            // handle GPKG conversion to JSON
            if ( mOgrDriverName == QLatin1String( "GPKG" ) )
            {
              ogrType = OFTString;
              ogrSubType = OFSTJSON;
              break;
            }

            const char *pszDataTypes = GDALGetMetadataItem( poDriver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
            if ( pszDataTypes && strstr( pszDataTypes, "StringList" ) )
            {
              ogrType = OFTStringList;
              mSupportedListSubTypes.insert( QVariant::String );
            }
            else
            {
              ogrType = OFTString;
              ogrWidth = 255;
            }
            break;
          }

          case QVariant::List:
            // handle GPKG conversion to JSON
            if ( mOgrDriverName == QLatin1String( "GPKG" ) )
            {
              ogrType = OFTString;
              ogrSubType = OFSTJSON;
              break;
            }

            // fall through to default for other unsupported types
            if ( attrField.subType() == QVariant::String )
            {
              const char *pszDataTypes = GDALGetMetadataItem( poDriver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
              if ( pszDataTypes && strstr( pszDataTypes, "StringList" ) )
              {
                ogrType = OFTStringList;
                mSupportedListSubTypes.insert( QVariant::String );
              }
              else
              {
                ogrType = OFTString;
                ogrWidth = 255;
              }
              break;
            }
            else if ( attrField.subType() == QVariant::Int )
            {
              const char *pszDataTypes = GDALGetMetadataItem( poDriver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
              if ( pszDataTypes && strstr( pszDataTypes, "IntegerList" ) )
              {
                ogrType = OFTIntegerList;
                mSupportedListSubTypes.insert( QVariant::Int );
              }
              else
              {
                ogrType = OFTString;
                ogrWidth = 255;
              }
              break;
            }
            else if ( attrField.subType() == QVariant::Double )
            {
              const char *pszDataTypes = GDALGetMetadataItem( poDriver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
              if ( pszDataTypes && strstr( pszDataTypes, "RealList" ) )
              {
                ogrType = OFTRealList;
                mSupportedListSubTypes.insert( QVariant::Double );
              }
              else
              {
                ogrType = OFTString;
                ogrWidth = 255;
              }
              break;
            }
            else if ( attrField.subType() == QVariant::LongLong )
            {
              const char *pszDataTypes = GDALGetMetadataItem( poDriver, GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
              if ( pszDataTypes && strstr( pszDataTypes, "Integer64List" ) )
              {
                ogrType = OFTInteger64List;
                mSupportedListSubTypes.insert( QVariant::LongLong );
              }
              else
              {
                ogrType = OFTString;
                ogrWidth = 255;
              }
              break;
            }
            //intentional fall-through
            FALLTHROUGH

          default:
            //assert(0 && "invalid variant type!");
            mErrorMessage = QObject::tr( "Unsupported type for field %1" )
                            .arg( attrField.name() );
            mError = ErrAttributeTypeUnsupported;
            return;
        }

        if ( mOgrDriverName == QLatin1String( "SQLite" ) && name.compare( QLatin1String( "ogc_fid" ), Qt::CaseInsensitive ) == 0 )
        {
          int i;
          for ( i = 0; i < 10; i++ )
          {
            name = QStringLiteral( "ogc_fid%1" ).arg( i );

            int j;
            for ( j = 0; j < fields.size() && name.compare( fields.at( j ).name(), Qt::CaseInsensitive ) != 0; j++ )
              ;

            if ( j == fields.size() )
              break;
          }

          if ( i == 10 )
          {
            mErrorMessage = QObject::tr( "No available replacement for internal fieldname ogc_fid found" ).arg( attrField.name() );
            mError = ErrAttributeCreationFailed;
            return;
          }

          QgsMessageLog::logMessage( QObject::tr( "Reserved attribute name ogc_fid replaced with %1" ).arg( name ), QObject::tr( "OGR" ) );
        }

        // create field definition
        gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( mCodec->fromUnicode( name ), ogrType ) );
        if ( ogrWidth > 0 )
        {
          OGR_Fld_SetWidth( fld.get(), ogrWidth );
        }

        if ( ogrPrecision >= 0 )
        {
          OGR_Fld_SetPrecision( fld.get(), ogrPrecision );
        }

        if ( ogrSubType != OFSTNone )
          OGR_Fld_SetSubType( fld.get(), ogrSubType );

        // create the field
        QgsDebugMsgLevel( "creating field " + attrField.name() +
                          " type " + QString( QVariant::typeToName( attrField.type() ) ) +
                          " width " + QString::number( ogrWidth ) +
                          " precision " + QString::number( ogrPrecision ), 2 );
        if ( OGR_L_CreateField( mLayer, fld.get(), true ) != OGRERR_NONE )
        {
          QgsDebugMsg( "error creating field " + attrField.name() );
          mErrorMessage = QObject::tr( "Creation of field %1 failed (OGR error: %2)" )
                          .arg( attrField.name(),
                                QString::fromUtf8( CPLGetLastErrorMsg() ) );
          mError = ErrAttributeCreationFailed;
          return;
        }

        int ogrIdx = OGR_FD_GetFieldIndex( defn, mCodec->fromUnicode( name ) );
        QgsDebugMsgLevel( QStringLiteral( "returned field index for %1: %2" ).arg( name ).arg( ogrIdx ), 2 );
        if ( ogrIdx < 0 || existingIdxs.contains( ogrIdx ) )
        {
          // GDAL 1.7 not just truncates, but launders more aggressivly.
          ogrIdx = OGR_FD_GetFieldCount( defn ) - 1;

          if ( ogrIdx < 0 )
          {
            QgsDebugMsg( "error creating field " + attrField.name() );
            mErrorMessage = QObject::tr( "Created field %1 not found (OGR error: %2)" )
                            .arg( attrField.name(),
                                  QString::fromUtf8( CPLGetLastErrorMsg() ) );
            mError = ErrAttributeCreationFailed;
            return;
          }
        }

        existingIdxs.insert( ogrIdx );
        mAttrIdxToOgrIdx.insert( fldIdx, ogrIdx );
      }
    }
    break;

    case AppendToLayerNoNewFields:
    {
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        QgsField attrField = fields.at( fldIdx );
        QString name( attrField.name() );
        int ogrIdx = OGR_FD_GetFieldIndex( defn, mCodec->fromUnicode( name ) );
        if ( ogrIdx >= 0 )
          mAttrIdxToOgrIdx.insert( fldIdx, ogrIdx );
      }
    }
    break;
  }

  // Geopackages require a unique feature id. If the input feature stream cannot guarantee
  // the uniqueness of the FID column, we drop it and let OGR generate new ones
  if ( sinkFlags.testFlag( QgsFeatureSink::RegeneratePrimaryKey ) && driverName == QLatin1String( "GPKG" ) )
  {
    int fidIdx = fields.lookupField( QStringLiteral( "FID" ) );

    if ( fidIdx >= 0 )
      mAttrIdxToOgrIdx.remove( fidIdx );
  }

  QgsDebugMsgLevel( QStringLiteral( "Done creating fields" ), 2 );

  mWkbType = geometryType;

  if ( newFilename )
    *newFilename = vectorFileName;

  // enabling transaction on databases that support it
  mUsingTransaction = true;
  if ( OGRERR_NONE != OGR_L_StartTransaction( mLayer ) )
  {
    mUsingTransaction = false;
  }
}

OGRGeometryH QgsVectorFileWriter::createEmptyGeometry( QgsWkbTypes::Type wkbType )
{
  return OGR_G_CreateGeometry( ogrTypeFromWkbType( wkbType ) );
}

///@cond PRIVATE
class QgsVectorFileWriterMetadataContainer
{
  public:

    QgsVectorFileWriterMetadataContainer()
    {
      QMap<QString, QgsVectorFileWriter::Option *> datasetOptions;
      QMap<QString, QgsVectorFileWriter::Option *> layerOptions;

      // Arc/Info ASCII Coverage
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "AVCE00" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Arc/Info ASCII Coverage" ),
                               QObject::tr( "Arc/Info ASCII Coverage" ),
                               QStringLiteral( "*.e00" ),
                               QStringLiteral( "e00" ),
                               datasetOptions,
                               layerOptions
                             )
                           );


#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,3,0)
      // Support for Atlas BNA was removed in GDAL 3.3

      // Atlas BNA
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "LINEFORMAT" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "New BNA files are created by the "
                                            "systems default line termination conventions. "
                                            "This may be overridden here." ),
                               QStringList()
                               << QStringLiteral( "CRLF" )
                               << QStringLiteral( "LF" ),
                               QString(), // Default value
                               true // Allow None
                             ) );

      datasetOptions.insert( QStringLiteral( "MULTILINE" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "By default, BNA files are created in multi-line format. "
                                            "For each record, the first line contains the identifiers and the "
                                            "type/number of coordinates to follow. Each following line contains "
                                            "a pair of coordinates." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "NB_IDS" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "BNA records may contain from 2 to 4 identifiers per record. "
                                            "Some software packages only support a precise number of identifiers. "
                                            "You can override the default value (2) by a precise value." ),
                               QStringList()
                               << QStringLiteral( "2" )
                               << QStringLiteral( "3" )
                               << QStringLiteral( "4" )
                               << QStringLiteral( "NB_SOURCE_FIELDS" ),
                               QStringLiteral( "2" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "ELLIPSES_AS_ELLIPSES" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "The BNA writer will try to recognize ellipses and circles when writing a polygon. "
                                            "This will only work if the feature has previously been read from a BNA file. "
                                            "As some software packages do not support ellipses/circles in BNA data file, "
                                            "it may be useful to tell the writer by specifying ELLIPSES_AS_ELLIPSES=NO not "
                                            "to export them as such, but keep them as polygons." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "NB_PAIRS_PER_LINE" ), new QgsVectorFileWriter::IntOption(
                               QObject::tr( "Limit the number of coordinate pairs per line in multiline format." ),
                               2 // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "COORDINATE_PRECISION" ), new QgsVectorFileWriter::IntOption(
                               QObject::tr( "Set the number of decimal for coordinates. Default value is 10." ),
                               10 // Default value
                             ) );

      driverMetadata.insert( QStringLiteral( "BNA" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Atlas BNA" ),
                               QObject::tr( "Atlas BNA" ),
                               QStringLiteral( "*.bna" ),
                               QStringLiteral( "bna" ),
                               datasetOptions,
                               layerOptions
                             )
                           );
#endif

      // Comma Separated Value
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "LINEFORMAT" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "By default when creating new .csv files they "
                                          "are created with the line termination conventions "
                                          "of the local platform (CR/LF on Win32 or LF on all other systems). "
                                          "This may be overridden through the use of the LINEFORMAT option." ),
                             QStringList()
                             << QStringLiteral( "CRLF" )
                             << QStringLiteral( "LF" ),
                             QString(), // Default value
                             true // Allow None
                           ) );

      layerOptions.insert( QStringLiteral( "GEOMETRY" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "By default, the geometry of a feature written to a .csv file is discarded. "
                                          "It is possible to export the geometry in its WKT representation by "
                                          "specifying GEOMETRY=AS_WKT. It is also possible to export point geometries "
                                          "into their X,Y,Z components by specifying GEOMETRY=AS_XYZ, GEOMETRY=AS_XY "
                                          "or GEOMETRY=AS_YX." ),
                             QStringList()
                             << QStringLiteral( "AS_WKT" )
                             << QStringLiteral( "AS_XYZ" )
                             << QStringLiteral( "AS_XY" )
                             << QStringLiteral( "AS_YX" ),
                             QString(), // Default value
                             true // Allow None
                           ) );

      layerOptions.insert( QStringLiteral( "CREATE_CSVT" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Create the associated .csvt file to describe the type of each "
                                          "column of the layer and its optional width and precision." ),
                             false  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "SEPARATOR" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Field separator character." ),
                             QStringList()
                             << QStringLiteral( "COMMA" )
                             << QStringLiteral( "SEMICOLON" )
                             << QStringLiteral( "TAB" ),
                             QStringLiteral( "COMMA" ) // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "STRING_QUOTING" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Double-quote strings. IF_AMBIGUOUS means that string values that look like numbers will be quoted." ),
                             QStringList()
                             << QStringLiteral( "IF_NEEDED" )
                             << QStringLiteral( "IF_AMBIGUOUS" )
                             << QStringLiteral( "ALWAYS" ),
                             QStringLiteral( "IF_AMBIGUOUS" ) // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "WRITE_BOM" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Write a UTF-8 Byte Order Mark (BOM) at the start of the file." ),
                             false  // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "CSV" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Comma Separated Value [CSV]" ),
                               QObject::tr( "Comma Separated Value [CSV]" ),
                               QStringLiteral( "*.csv" ),
                               QStringLiteral( "csv" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
      // FlatGeobuf
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "FlatGeobuf" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "FlatGeobuf" ),
                               QObject::tr( "FlatGeobuf" ),
                               QStringLiteral( "*.fgb" ),
                               QStringLiteral( "fgb" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );
#endif

      // ESRI Shapefile
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "SHPT" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Override the type of shapefile created. "
                                          "Can be one of NULL for a simple .dbf file with no .shp file, POINT, "
                                          "ARC, POLYGON or MULTIPOINT for 2D, or POINTZ, ARCZ, POLYGONZ or "
                                          "MULTIPOINTZ for 3D;" ) +
                             QObject::tr( " POINTM, ARCM, POLYGONM or MULTIPOINTM for measured geometries"
                                          " and POINTZM, ARCZM, POLYGONZM or MULTIPOINTZM for 3D measured"
                                          " geometries." ) +
                             QObject::tr( " MULTIPATCH files are supported since GDAL 2.2." ) +
                             ""
                             , QStringList()
                             << QStringLiteral( "NULL" )
                             << QStringLiteral( "POINT" )
                             << QStringLiteral( "ARC" )
                             << QStringLiteral( "POLYGON" )
                             << QStringLiteral( "MULTIPOINT" )
                             << QStringLiteral( "POINTZ" )
                             << QStringLiteral( "ARCZ" )
                             << QStringLiteral( "POLYGONZ" )
                             << QStringLiteral( "MULTIPOINTZ" )
                             << QStringLiteral( "POINTM" )
                             << QStringLiteral( "ARCM" )
                             << QStringLiteral( "POLYGONM" )
                             << QStringLiteral( "MULTIPOINTM" )
                             << QStringLiteral( "POINTZM" )
                             << QStringLiteral( "ARCZM" )
                             << QStringLiteral( "POLYGONZM" )
                             << QStringLiteral( "MULTIPOINTZM" )
                             << QStringLiteral( "MULTIPATCH" )
                             << QString(),
                             QString(), // Default value
                             true  // Allow None
                           ) );

      // there does not seem to be a reason to provide this option to the user again
      // as we set encoding for shapefiles based on "fileEncoding" parameter passed to the writer
#if 0
      layerOptions.insert( "ENCODING", new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Set the encoding value in the DBF file. "
                                          "The default value is LDID/87. It is not clear "
                                          "what other values may be appropriate." ),
                             QStringList()
                             << "LDID/87",
                             "LDID/87" // Default value
                           ) );
#endif

      layerOptions.insert( QStringLiteral( "RESIZE" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Set to YES to resize fields to their optimal size." ),
                             false  // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "ESRI" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "ESRI Shapefile" ),
                               QObject::tr( "ESRI Shapefile" ),
                               QStringLiteral( "*.shp" ),
                               QStringLiteral( "shp" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // DBF File
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "DBF File" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "DBF File" ),
                               QObject::tr( "DBF File" ),
                               QStringLiteral( "*.dbf" ),
                               QStringLiteral( "dbf" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // FMEObjects Gateway
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "FMEObjects Gateway" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "FMEObjects Gateway" ),
                               QObject::tr( "FMEObjects Gateway" ),
                               QStringLiteral( "*.fdd" ),
                               QStringLiteral( "fdd" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // GeoJSON
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "WRITE_BBOX" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Set to YES to write a bbox property with the bounding box "
                                          "of the geometries at the feature and feature collection level." ),
                             false  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "COORDINATE_PRECISION" ), new QgsVectorFileWriter::IntOption(
                             QObject::tr( "Maximum number of figures after decimal separator to write in coordinates. "
                                          "Defaults to 15. Truncation will occur to remove trailing zeros." ),
                             15 // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "RFC7946" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Whether to use RFC 7946 standard. "
                                          "If disabled GeoJSON 2008 initial version will be used. "
                                          "Default is NO (thus GeoJSON 2008). See also Documentation (via Help button)" ),
                             false // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "GeoJSON" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "GeoJSON" ),
                               QObject::tr( "GeoJSON" ),
                               QStringLiteral( "*.geojson" ),
                               QStringLiteral( "geojson" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // GeoJSONSeq
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "COORDINATE_PRECISION" ), new QgsVectorFileWriter::IntOption(
                             QObject::tr( "Maximum number of figures after decimal separator to write in coordinates. "
                                          "Defaults to 15. Truncation will occur to remove trailing zeros." ),
                             15 // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "RS" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Whether to start records with the RS=0x1E character (RFC 8142 standard). "
                                          "Defaults to NO: Newline Delimited JSON (geojsonl). \n"
                                          "If set to YES:  RFC 8142 standard: GeoJSON Text Sequences  (geojsons)." ),
                             false  // Default value = NO
                           ) );

      driverMetadata.insert( QStringLiteral( "GeoJSONSeq" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "GeoJSON - Newline Delimited" ),
                               QObject::tr( "GeoJSON - Newline Delimited" ),
                               QStringLiteral( "*.geojsonl *.geojsons *.json" ),
                               QStringLiteral( "json" ),  // add json for now
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // GeoRSS
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "FORMAT" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "whether the document must be in RSS 2.0 or Atom 1.0 format. "
                                            "Default value : RSS" ),
                               QStringList()
                               << QStringLiteral( "RSS" )
                               << QStringLiteral( "ATOM" ),
                               QStringLiteral( "RSS" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "GEOM_DIALECT" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "The encoding of location information. Default value : SIMPLE. "
                                            "W3C_GEO only supports point geometries. "
                                            "SIMPLE or W3C_GEO only support geometries in geographic WGS84 coordinates." ),
                               QStringList()
                               << QStringLiteral( "SIMPLE" )
                               << QStringLiteral( "GML" )
                               << QStringLiteral( "W3C_GEO" ),
                               QStringLiteral( "SIMPLE" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "USE_EXTENSIONS" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "If defined to YES, extension fields will be written. "
                                            "If the field name not found in the base schema matches "
                                            "the foo_bar pattern, foo will be considered as the namespace "
                                            "of the element, and a <foo:bar> element will be written. "
                                            "Otherwise, elements will be written in the <ogr:> namespace." ),
                               false // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "WRITE_HEADER_AND_FOOTER" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "If defined to NO, only <entry> or <item> elements will be written. "
                                            "The user will have to provide the appropriate header and footer of the document." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "HEADER" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "XML content that will be put between the <channel> element and the "
                                            "first <item> element for a RSS document, or between the xml tag and "
                                            "the first <entry> element for an Atom document." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "TITLE" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Value put inside the <title> element in the header. "
                                            "If not provided, a dummy value will be used as that element is compulsory." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "DESCRIPTION" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Value put inside the <description> element in the header. "
                                            "If not provided, a dummy value will be used as that element is compulsory." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "LINK" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Value put inside the <link> element in the header. "
                                            "If not provided, a dummy value will be used as that element is compulsory." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "UPDATED" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Value put inside the <updated> element in the header. "
                                            "Should be formatted as a XML datetime. "
                                            "If not provided, a dummy value will be used as that element is compulsory." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "AUTHOR_NAME" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Value put inside the <author><name> element in the header. "
                                            "If not provided, a dummy value will be used as that element is compulsory." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "ID" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Value put inside the <id> element in the header. "
                                            "If not provided, a dummy value will be used as that element is compulsory." ),
                               QString()  // Default value
                             ) );

      driverMetadata.insert( QStringLiteral( "GeoRSS" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "GeoRSS" ),
                               QObject::tr( "GeoRSS" ),
                               QStringLiteral( "*.xml" ),
                               QStringLiteral( "xml" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // Geography Markup Language [GML]
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "XSISCHEMAURI" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "If provided, this URI will be inserted as the schema location. "
                                            "Note that the schema file isn't actually accessed by OGR, so it "
                                            "is up to the user to ensure it will match the schema of the OGR "
                                            "produced GML data file." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "XSISCHEMA" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "This writes a GML application schema file to a corresponding "
                                            ".xsd file (with the same basename). If INTERNAL is used the "
                                            "schema is written within the GML file, but this is experimental "
                                            "and almost certainly not valid XML. "
                                            "OFF disables schema generation (and is implicit if XSISCHEMAURI is used)." ),
                               QStringList()
                               << QStringLiteral( "EXTERNAL" )
                               << QStringLiteral( "INTERNAL" )
                               << QStringLiteral( "OFF" ),
                               QStringLiteral( "EXTERNAL" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "PREFIX" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "This is the prefix for the application target namespace." ),
                               QStringLiteral( "ogr" )  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "STRIP_PREFIX" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Can be set to TRUE to avoid writing the prefix of the "
                                            "application target namespace in the GML file." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "TARGET_NAMESPACE" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Defaults to 'http://ogr.maptools.org/'. "
                                            "This is the application target namespace." ),
                               QStringLiteral( "http://ogr.maptools.org/" )  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "FORMAT" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "If not specified, GML2 will be used." ),
                               QStringList()
                               << QStringLiteral( "GML3" )
                               << QStringLiteral( "GML3Deegree" )
                               << QStringLiteral( "GML3.2" ),
                               QString(), // Default value
                               true // Allow None
                             ) );

      datasetOptions.insert( QStringLiteral( "GML3_LONGSRS" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Only valid when FORMAT=GML3/GML3Degree/GML3.2. Default to YES. " //needs review here
                                            "If YES, SRS with EPSG authority will be written with the "
                                            "'urn:ogc:def:crs:EPSG::' prefix. In the case the SRS is a "
                                            "geographic SRS without explicit AXIS order, but that the same "
                                            "SRS authority code imported with ImportFromEPSGA() should be "
                                            "treated as lat/long, then the function will take care of coordinate "
                                            "order swapping. If set to NO, SRS with EPSG authority will be "
                                            "written with the 'EPSG:' prefix, even if they are in lat/long order." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "WRITE_FEATURE_BOUNDED_BY" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "only valid when FORMAT=GML3/GML3Degree/GML3.2) Default to YES. "
                                            "If set to NO, the <gml:boundedBy> element will not be written for "
                                            "each feature." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "SPACE_INDENTATION" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Default to YES. If YES, the output will be indented with spaces "
                                            "for more readability, but at the expense of file size." ),
                               true  // Default value
                             ) );


      driverMetadata.insert( QStringLiteral( "GML" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Geography Markup Language [GML]" ),
                               QObject::tr( "Geography Markup Language [GML]" ),
                               QStringLiteral( "*.gml" ),
                               QStringLiteral( "gml" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // GeoPackage
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "IDENTIFIER" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Human-readable identifier (e.g. short name) for the layer content" ),
                             QString()  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "DESCRIPTION" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Human-readable description for the layer content" ),
                             QString()  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "FID" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Name for the feature identifier column" ),
                             QStringLiteral( "fid" )  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "GEOMETRY_NAME" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Name for the geometry column" ),
                             QStringLiteral( "geom" )  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "SPATIAL_INDEX" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "If a spatial index must be created." ),
                             true  // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "GPKG" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "GeoPackage" ),
                               QObject::tr( "GeoPackage" ),
                               QStringLiteral( "*.gpkg" ),
                               QStringLiteral( "gpkg" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // Generic Mapping Tools [GMT]
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "GMT" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Generic Mapping Tools [GMT]" ),
                               QObject::tr( "Generic Mapping Tools [GMT]" ),
                               QStringLiteral( "*.gmt" ),
                               QStringLiteral( "gmt" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // GPS eXchange Format [GPX]
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "FORCE_GPX_TRACK" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "By default when writing a layer whose features are of "
                                          "type wkbLineString, the GPX driver chooses to write "
                                          "them as routes. If FORCE_GPX_TRACK=YES is specified, "
                                          "they will be written as tracks." ),
                             false  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "FORCE_GPX_ROUTE" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "By default when writing a layer whose features are of "
                                          "type wkbMultiLineString, the GPX driver chooses to write "
                                          "them as tracks. If FORCE_GPX_ROUTE=YES is specified, "
                                          "they will be written as routes, provided that the multilines "
                                          "are composed of only one single line." ),
                             false  // Default value
                           ) );

      datasetOptions.insert( QStringLiteral( "GPX_USE_EXTENSIONS" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "If GPX_USE_EXTENSIONS=YES is specified, "
                                            "extra fields will be written inside the <extensions> tag." ),
                               false // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "GPX_EXTENSIONS_NS" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Only used if GPX_USE_EXTENSIONS=YES and GPX_EXTENSIONS_NS_URL "
                                            "is set. The namespace value used for extension tags. By default, 'ogr'." ),
                               QStringLiteral( "ogr" )  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "GPX_EXTENSIONS_NS_URL" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Only used if GPX_USE_EXTENSIONS=YES and GPX_EXTENSIONS_NS "
                                            "is set. The namespace URI. By default, 'http://osgeo.org/gdal'." ),
                               QStringLiteral( "http://osgeo.org/gdal" )  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "LINEFORMAT" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "By default files are created with the line termination "
                                            "conventions of the local platform (CR/LF on win32 or LF "
                                            "on all other systems). This may be overridden through use "
                                            "of the LINEFORMAT layer creation option which may have a value "
                                            "of CRLF (DOS format) or LF (Unix format)." ),
                               QStringList()
                               << QStringLiteral( "CRLF" )
                               << QStringLiteral( "LF" ),
                               QString(), // Default value
                               true // Allow None
                             ) );

      driverMetadata.insert( QStringLiteral( "GPX" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "GPS eXchange Format [GPX]" ),
                               QObject::tr( "GPS eXchange Format [GPX]" ),
                               QStringLiteral( "*.gpx" ),
                               QStringLiteral( "gpx" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // INTERLIS 1
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "Interlis 1" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "INTERLIS 1" ),
                               QObject::tr( "INTERLIS 1" ),
                               QStringLiteral( "*.itf *.xml *.ili" ),
                               QStringLiteral( "ili" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // INTERLIS 2
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "Interlis 2" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "INTERLIS 2" ),
                               QObject::tr( "INTERLIS 2" ),
                               QStringLiteral( "*.xtf *.xml *.ili" ),
                               QStringLiteral( "ili" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // Keyhole Markup Language [KML]
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "NameField" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Allows you to specify the field to use for the KML <name> element." ),
                               QStringLiteral( "Name" )  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "DescriptionField" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Allows you to specify the field to use for the KML <description> element." ),
                               QStringLiteral( "Description" )  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "AltitudeMode" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "Allows you to specify the AltitudeMode to use for KML geometries. "
                                            "This will only affect 3D geometries and must be one of the valid KML options." ),
                               QStringList()
                               << QStringLiteral( "clampToGround" )
                               << QStringLiteral( "relativeToGround" )
                               << QStringLiteral( "absolute" ),
                               QStringLiteral( "relativeToGround" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "DOCUMENT_ID" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "The DOCUMENT_ID datasource creation option can be used to specified "
                                            "the id of the root <Document> node. The default value is root_doc." ),
                               QStringLiteral( "root_doc" )  // Default value
                             ) );

      driverMetadata.insert( QStringLiteral( "KML" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Keyhole Markup Language [KML]" ),
                               QObject::tr( "Keyhole Markup Language [KML]" ),
                               QStringLiteral( "*.kml" ),
                               QStringLiteral( "kml" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // Mapinfo
      datasetOptions.clear();
      layerOptions.clear();

      auto insertMapInfoOptions = []( QMap<QString, QgsVectorFileWriter::Option *> &datasetOptions, QMap<QString, QgsVectorFileWriter::Option *> &layerOptions )
      {
        datasetOptions.insert( QStringLiteral( "SPATIAL_INDEX_MODE" ), new QgsVectorFileWriter::SetOption(
                                 QObject::tr( "Use this to turn on 'quick spatial index mode'. "
                                              "In this mode writing files can be about 5 times faster, "
                                              "but spatial queries can be up to 30 times slower." ),
                                 QStringList()
                                 << QStringLiteral( "QUICK" )
                                 << QStringLiteral( "OPTIMIZED" ),
                                 QStringLiteral( "QUICK" ), // Default value
                                 true // Allow None
                               ) );

        datasetOptions.insert( QStringLiteral( "BLOCK_SIZE" ), new QgsVectorFileWriter::IntOption(
                                 QObject::tr( "(multiples of 512): Block size for .map files. Defaults "
                                              "to 512. MapInfo 15.2 and above creates .tab files with a "
                                              "blocksize of 16384 bytes. Any MapInfo version should be "
                                              "able to handle block sizes from 512 to 32256." ),
                                 512
                               ) );
        layerOptions.insert( QStringLiteral( "BOUNDS" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "xmin,ymin,xmax,ymax: Define custom layer bounds to increase the "
                                            "accuracy of the coordinates. Note: the geometry of written "
                                            "features must be within the defined box." ),
                               QString() // Default value
                             ) );
      };
      insertMapInfoOptions( datasetOptions, layerOptions );

      driverMetadata.insert( QStringLiteral( "MapInfo File" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Mapinfo" ),
                               QObject::tr( "Mapinfo TAB" ),
                               QStringLiteral( "*.tab" ),
                               QStringLiteral( "tab" ),
                               datasetOptions,
                               layerOptions
                             )
                           );
      datasetOptions.clear();
      layerOptions.clear();
      insertMapInfoOptions( datasetOptions, layerOptions );

      // QGIS internal alias for MIF files
      driverMetadata.insert( QStringLiteral( "MapInfo MIF" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Mapinfo" ),
                               QObject::tr( "Mapinfo MIF" ),
                               QStringLiteral( "*.mif" ),
                               QStringLiteral( "mif" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // Microstation DGN
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "3D" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Determine whether 2D (seed_2d.dgn) or 3D (seed_3d.dgn) "
                                            "seed file should be used. This option is ignored if the SEED option is provided." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "SEED" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Override the seed file to use." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "COPY_WHOLE_SEED_FILE" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Indicate whether the whole seed file should be copied. "
                                            "If not, only the first three elements will be copied." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "COPY_SEED_FILE_COLOR_TABLE" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Indicates whether the color table should be copied from the seed file." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "MASTER_UNIT_NAME" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Override the master unit name from the seed file with "
                                            "the provided one or two character unit name." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "SUB_UNIT_NAME" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Override the sub unit name from the seed file with the provided "
                                            "one or two character unit name." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "SUB_UNITS_PER_MASTER_UNIT" ), new QgsVectorFileWriter::IntOption(
                               QObject::tr( "Override the number of subunits per master unit. "
                                            "By default the seed file value is used." ),
                               0 // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "UOR_PER_SUB_UNIT" ), new QgsVectorFileWriter::IntOption(
                               QObject::tr( "Override the number of UORs (Units of Resolution) "
                                            "per sub unit. By default the seed file value is used." ),
                               0 // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "ORIGIN" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "ORIGIN=x,y,z: Override the origin of the design plane. "
                                            "By default the origin from the seed file is used." ),
                               QString()  // Default value
                             ) );

      driverMetadata.insert( QStringLiteral( "DGN" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Microstation DGN" ),
                               QObject::tr( "Microstation DGN" ),
                               QStringLiteral( "*.dgn" ),
                               QStringLiteral( "dgn" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // S-57 Base file
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "UPDATES" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "Should update files be incorporated into the base data on the fly." ),
                               QStringList()
                               << QStringLiteral( "APPLY" )
                               << QStringLiteral( "IGNORE" ),
                               QStringLiteral( "APPLY" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "SPLIT_MULTIPOINT" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Should multipoint soundings be split into many single point sounding features. "
                                            "Multipoint geometries are not well handled by many formats, "
                                            "so it can be convenient to split single sounding features with many points "
                                            "into many single point features." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "ADD_SOUNDG_DEPTH" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Should a DEPTH attribute be added on SOUNDG features and assign the depth "
                                            "of the sounding. This should only be enabled when SPLIT_MULTIPOINT is "
                                            "also enabled." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "RETURN_PRIMITIVES" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Should all the low level geometry primitives be returned as special "
                                            "IsolatedNode, ConnectedNode, Edge and Face layers." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "PRESERVE_EMPTY_NUMBERS" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "If enabled, numeric attributes assigned an empty string as a value will "
                                            "be preserved as a special numeric value. This option should not generally "
                                            "be needed, but may be useful when translated S-57 to S-57 losslessly." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "LNAM_REFS" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Should LNAM and LNAM_REFS fields be attached to features capturing "
                                            "the feature to feature relationships in the FFPT group of the S-57 file." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "RETURN_LINKAGES" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Should additional attributes relating features to their underlying "
                                            "geometric primitives be attached. These are the values of the FSPT group, "
                                            "and are primarily needed when doing S-57 to S-57 translations." ),
                               false  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "RECODE_BY_DSSI" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Should attribute values be recoded to UTF-8 from the character encoding "
                                            "specified in the S57 DSSI record." ),
                               false  // Default value
                             ) );

      // set OGR_S57_OPTIONS = "RETURN_PRIMITIVES=ON,RETURN_LINKAGES=ON,LNAM_REFS=ON"

      driverMetadata.insert( QStringLiteral( "S57" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "S-57 Base file" ),
                               QObject::tr( "S-57 Base file" ),
                               QStringLiteral( "*.000" ),
                               QStringLiteral( "000" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // Spatial Data Transfer Standard [SDTS]
      datasetOptions.clear();
      layerOptions.clear();

      driverMetadata.insert( QStringLiteral( "SDTS" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Spatial Data Transfer Standard [SDTS]" ),
                               QObject::tr( "Spatial Data Transfer Standard [SDTS]" ),
                               QStringLiteral( "*catd.ddf" ),
                               QStringLiteral( "ddf" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // SQLite
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "METADATA" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Can be used to avoid creating the geometry_columns and spatial_ref_sys "
                                            "tables in a new database. By default these metadata tables are created "
                                            "when a new database is created." ),
                               true  // Default value
                             ) );

      // Will handle the SpatiaLite alias
      datasetOptions.insert( QStringLiteral( "SPATIALITE" ), new QgsVectorFileWriter::HiddenOption(
                               QStringLiteral( "NO" )
                             ) );


      datasetOptions.insert( QStringLiteral( "INIT_WITH_EPSG" ), new QgsVectorFileWriter::HiddenOption(
                               QStringLiteral( "NO" )
                             ) );

      layerOptions.insert( QStringLiteral( "FORMAT" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Controls the format used for the geometry column. Defaults to WKB. "
                                          "This is generally more space and processing efficient, but harder "
                                          "to inspect or use in simple applications than WKT (Well Known Text)." ),
                             QStringList()
                             << QStringLiteral( "WKB" )
                             << QStringLiteral( "WKT" ),
                             QStringLiteral( "WKB" ) // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "LAUNDER" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Controls whether layer and field names will be laundered for easier use "
                                          "in SQLite. Laundered names will be converted to lower case and some special "
                                          "characters(' - #) will be changed to underscores." ),
                             true  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "SPATIAL_INDEX" ), new QgsVectorFileWriter::HiddenOption(
                             QStringLiteral( "NO" )
                           ) );

      layerOptions.insert( QStringLiteral( "COMPRESS_GEOM" ), new QgsVectorFileWriter::HiddenOption(
                             QStringLiteral( "NO" )
                           ) );

      layerOptions.insert( QStringLiteral( "SRID" ), new QgsVectorFileWriter::HiddenOption(
                             QString()
                           ) );

      layerOptions.insert( QStringLiteral( "COMPRESS_COLUMNS" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "column_name1[,column_name2, …] A list of (String) columns that "
                                          "must be compressed with ZLib DEFLATE algorithm. This might be beneficial "
                                          "for databases that have big string blobs. However, use with care, since "
                                          "the value of such columns will be seen as compressed binary content with "
                                          "other SQLite utilities (or previous OGR versions). With OGR, when inserting, "
                                          "modifying or querying compressed columns, compression/decompression is "
                                          "done transparently. However, such columns cannot be (easily) queried with "
                                          "an attribute filter or WHERE clause. Note: in table definition, such columns "
                                          "have the 'VARCHAR_deflate' declaration type." ),
                             QString()  // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "SQLite" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "SQLite" ),
                               QObject::tr( "SQLite" ),
                               QStringLiteral( "*.sqlite" ),
                               QStringLiteral( "sqlite" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // SpatiaLite
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "METADATA" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Can be used to avoid creating the geometry_columns and spatial_ref_sys "
                                            "tables in a new database. By default these metadata tables are created "
                                            "when a new database is created." ),
                               true  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "SPATIALITE" ), new QgsVectorFileWriter::HiddenOption(
                               QStringLiteral( "YES" )
                             ) );

      datasetOptions.insert( QStringLiteral( "INIT_WITH_EPSG" ), new QgsVectorFileWriter::BoolOption(
                               QObject::tr( "Insert the content of the EPSG CSV files into the spatial_ref_sys table. "
                                            "Set to NO for regular SQLite databases." ),
                               true  // Default value
                             ) );

      layerOptions.insert( QStringLiteral( "FORMAT" ), new QgsVectorFileWriter::HiddenOption(
                             QStringLiteral( "SPATIALITE" )
                           ) );

      layerOptions.insert( QStringLiteral( "LAUNDER" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Controls whether layer and field names will be laundered for easier use "
                                          "in SQLite. Laundered names will be converted to lower case and some special "
                                          "characters(' - #) will be changed to underscores." ),
                             true  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "SPATIAL_INDEX" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "If the database is of the SpatiaLite flavor, and if OGR is linked "
                                          "against libspatialite, this option can be used to control if a spatial "
                                          "index must be created." ),
                             true  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "COMPRESS_GEOM" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "If the format of the geometry BLOB is of the SpatiaLite flavor, "
                                          "this option can be used to control if the compressed format for "
                                          "geometries (LINESTRINGs, POLYGONs) must be used." ),
                             false  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "SRID" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Used to force the SRID number of the SRS associated with the layer. "
                                          "When this option isn't specified and that a SRS is associated with the "
                                          "layer, a search is made in the spatial_ref_sys to find a match for the "
                                          "SRS, and, if there is no match, a new entry is inserted for the SRS in "
                                          "the spatial_ref_sys table. When the SRID option is specified, this "
                                          "search (and the eventual insertion of a new entry) will not be done: "
                                          "the specified SRID is used as such." ),
                             QString()  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "COMPRESS_COLUMNS" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "column_name1[,column_name2, …] A list of (String) columns that "
                                          "must be compressed with ZLib DEFLATE algorithm. This might be beneficial "
                                          "for databases that have big string blobs. However, use with care, since "
                                          "the value of such columns will be seen as compressed binary content with "
                                          "other SQLite utilities (or previous OGR versions). With OGR, when inserting, "
                                          "modifying or queryings compressed columns, compression/decompression is "
                                          "done transparently. However, such columns cannot be (easily) queried with "
                                          "an attribute filter or WHERE clause. Note: in table definition, such columns "
                                          "have the 'VARCHAR_deflate' declaration type." ),
                             QString()  // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "SpatiaLite" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "SpatiaLite" ),
                               QObject::tr( "SpatiaLite" ),
                               QStringLiteral( "*.sqlite" ),
                               QStringLiteral( "sqlite" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );
      // AutoCAD DXF
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "HEADER" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Override the header file used - in place of header.dxf." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "TRAILER" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Override the trailer file used - in place of trailer.dxf." ),
                               QString()  // Default value
                             ) );

      driverMetadata.insert( QStringLiteral( "DXF" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "AutoCAD DXF" ),
                               QObject::tr( "AutoCAD DXF" ),
                               QStringLiteral( "*.dxf" ),
                               QStringLiteral( "dxf" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // Geoconcept
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "EXTENSION" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "Indicates the GeoConcept export file extension. "
                                            "TXT was used by earlier releases of GeoConcept. GXT is currently used." ),
                               QStringList()
                               << QStringLiteral( "GXT" )
                               << QStringLiteral( "TXT" ),
                               QStringLiteral( "GXT" ) // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "CONFIG" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Path to the GCT: the GCT file describes the GeoConcept types definitions: "
                                            "In this file, every line must start with //# followed by a keyword. "
                                            "Lines starting with // are comments." ),
                               QString()  // Default value
                             ) );

      datasetOptions.insert( QStringLiteral( "FEATURETYPE" ), new QgsVectorFileWriter::StringOption(
                               QObject::tr( "Defines the feature to be created. The TYPE corresponds to one of the Name "
                                            "found in the GCT file for a type section. The SUBTYPE corresponds to one of "
                                            "the Name found in the GCT file for a sub-type section within the previous "
                                            "type section." ),
                               QString()  // Default value
                             ) );

      driverMetadata.insert( QStringLiteral( "Geoconcept" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Geoconcept" ),
                               QObject::tr( "Geoconcept" ),
                               QStringLiteral( "*.gxt *.txt" ),
                               QStringLiteral( "gxt" ),
                               datasetOptions,
                               layerOptions
                             )
                           );

      // ESRI FileGDB
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "FEATURE_DATASET" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "When this option is set, the new layer will be created inside the named "
                                          "FeatureDataset folder. If the folder does not already exist, it will be created." ),
                             QString()  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "GEOMETRY_NAME" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Set name of geometry column in new layer. Defaults to 'SHAPE'." ),
                             QStringLiteral( "SHAPE" )  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "FID" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Name of the OID column to create. Defaults to 'OBJECTID'." ),
                             QStringLiteral( "OBJECTID" )  // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "FileGDB" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "ESRI FileGDB" ),
                               QObject::tr( "ESRI FileGDB" ),
                               QStringLiteral( "*.gdb" ),
                               QStringLiteral( "gdb" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // XLSX
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "OGR_XLSX_FIELD_TYPES" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "By default, the driver will try to detect the data type of fields. If set "
                                          "to STRING, all fields will be of String type." ),
                             QStringList()
                             << QStringLiteral( "AUTO" )
                             << QStringLiteral( "STRING" ),
                             QStringLiteral( "AUTO" ), // Default value
                             false // Allow None
                           ) );

      layerOptions.insert( QStringLiteral( "OGR_XLSX_HEADERS" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "By default, the driver will read the first lines of each sheet to detect "
                                          "if the first line might be the name of columns. If set to FORCE, the driver "
                                          "will consider the first line as the header line. If set to "
                                          "DISABLE, it will be considered as the first feature. Otherwise "
                                          "auto-detection will occur." ),
                             QStringList()
                             << QStringLiteral( "FORCE" )
                             << QStringLiteral( "DISABLE" )
                             << QStringLiteral( "AUTO" ),
                             QStringLiteral( "AUTO" ), // Default value
                             false // Allow None
                           ) );

      driverMetadata.insert( QStringLiteral( "XLSX" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "MS Office Open XML spreadsheet" ),
                               QObject::tr( "MS Office Open XML spreadsheet [XLSX]" ),
                               QStringLiteral( "*.xlsx" ),
                               QStringLiteral( "xlsx" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // ODS
      datasetOptions.clear();
      layerOptions.clear();

      layerOptions.insert( QStringLiteral( "OGR_ODS_FIELD_TYPES" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "By default, the driver will try to detect the data type of fields. If set "
                                          "to STRING, all fields will be of String type." ),
                             QStringList()
                             << QStringLiteral( "AUTO" )
                             << QStringLiteral( "STRING" ),
                             QStringLiteral( "AUTO" ), // Default value
                             false // Allow None
                           ) );

      layerOptions.insert( QStringLiteral( "OGR_ODS_HEADERS" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "By default, the driver will read the first lines of each sheet to detect "
                                          "if the first line might be the name of columns. If set to FORCE, the driver "
                                          "will consider the first line as the header line. If set to "
                                          "DISABLE, it will be considered as the first feature. Otherwise "
                                          "auto-detection will occur." ),
                             QStringList()
                             << QStringLiteral( "FORCE" )
                             << QStringLiteral( "DISABLE" )
                             << QStringLiteral( "AUTO" ),
                             QStringLiteral( "AUTO" ), // Default value
                             false // Allow None
                           ) );

      driverMetadata.insert( QStringLiteral( "ODS" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "Open Document Spreadsheet" ),
                               QObject::tr( "Open Document Spreadsheet [ODS]" ),
                               QStringLiteral( "*.ods" ),
                               QStringLiteral( "ods" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

      // PGDump
      datasetOptions.clear();
      layerOptions.clear();

      datasetOptions.insert( QStringLiteral( "LINEFORMAT" ), new QgsVectorFileWriter::SetOption(
                               QObject::tr( "Line termination character sequence." ),
                               QStringList()
                               << QStringLiteral( "CRLF" )
                               << QStringLiteral( "LF" ),
                               QStringLiteral( "LF" ), // Default value
                               false // Allow None
                             ) );


      layerOptions.insert( QStringLiteral( "GEOM_TYPE" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Format of geometry columns." ),
                             QStringList()
                             << QStringLiteral( "geometry" )
                             << QStringLiteral( "geography" ),
                             QStringLiteral( "geometry" ), // Default value
                             false // Allow None
                           ) );

      layerOptions.insert( QStringLiteral( "LAUNDER" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Controls whether layer and field names will be laundered for easier use. "
                                          "Laundered names will be converted to lower case and some special "
                                          "characters(' - #) will be changed to underscores." ),
                             true  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "GEOMETRY_NAME" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Name for the geometry column. Defaults to wkb_geometry "
                                          "for GEOM_TYPE=geometry or the_geog for GEOM_TYPE=geography" ) ) );

      layerOptions.insert( QStringLiteral( "SCHEMA" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Name of schema into which to create the new table" ) ) );

      layerOptions.insert( QStringLiteral( "CREATE_SCHEMA" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Whether to explicitly emit the CREATE SCHEMA statement to create the specified schema." ),
                             true  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "CREATE_TABLE" ), new QgsVectorFileWriter::BoolOption(
                             QObject::tr( "Whether to explicitly recreate the table if necessary." ),
                             true  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "DROP_TABLE" ), new QgsVectorFileWriter::SetOption(
                             QObject::tr( "Whether to explicitly destroy tables before recreating them." ),
                             QStringList()
                             << QStringLiteral( "YES" )
                             << QStringLiteral( "NO" )
                             << QStringLiteral( "IF_EXISTS" ),
                             QStringLiteral( "YES" ), // Default value
                             false // Allow None
                           ) );

      layerOptions.insert( QStringLiteral( "SRID" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Used to force the SRID number of the SRS associated with the layer. "
                                          "When this option isn't specified and that a SRS is associated with the "
                                          "layer, a search is made in the spatial_ref_sys to find a match for the "
                                          "SRS, and, if there is no match, a new entry is inserted for the SRS in "
                                          "the spatial_ref_sys table. When the SRID option is specified, this "
                                          "search (and the eventual insertion of a new entry) will not be done: "
                                          "the specified SRID is used as such." ),
                             QString()  // Default value
                           ) );

      layerOptions.insert( QStringLiteral( "POSTGIS_VERSION" ), new QgsVectorFileWriter::StringOption(
                             QObject::tr( "Can be set to 2.0 or 2.2 for PostGIS 2.0/2.2 compatibility. "
                                          "Important to set it correctly if using non-linear geometry types" ),
                             QStringLiteral( "2.2" ) // Default value
                           ) );

      driverMetadata.insert( QStringLiteral( "PGDUMP" ),
                             QgsVectorFileWriter::MetaData(
                               QStringLiteral( "PostgreSQL SQL dump" ),
                               QObject::tr( "PostgreSQL SQL dump" ),
                               QStringLiteral( "*.sql" ),
                               QStringLiteral( "sql" ),
                               datasetOptions,
                               layerOptions,
                               QStringLiteral( "UTF-8" )
                             )
                           );

    }

    QgsVectorFileWriterMetadataContainer( const QgsVectorFileWriterMetadataContainer &other ) = delete;
    QgsVectorFileWriterMetadataContainer &operator=( const QgsVectorFileWriterMetadataContainer &other ) = delete;
    ~QgsVectorFileWriterMetadataContainer()
    {
      for ( auto it = driverMetadata.constBegin(); it != driverMetadata.constEnd(); ++it )
      {
        for ( auto optionIt = it.value().driverOptions.constBegin(); optionIt != it.value().driverOptions.constEnd(); ++optionIt )
          delete optionIt.value();
        for ( auto optionIt = it.value().layerOptions.constBegin(); optionIt != it.value().layerOptions.constEnd(); ++optionIt )
          delete optionIt.value();
      }
    }

    QMap<QString,  QgsVectorFileWriter::MetaData> driverMetadata;

};
///@endcond

bool QgsVectorFileWriter::driverMetadata( const QString &driverName, QgsVectorFileWriter::MetaData &driverMetadata )
{
  static QgsVectorFileWriterMetadataContainer sDriverMetadata;
  QMap<QString, MetaData>::ConstIterator it = sDriverMetadata.driverMetadata.constBegin();

  for ( ; it != sDriverMetadata.driverMetadata.constEnd(); ++it )
  {
    if ( it.key() == QLatin1String( "PGDUMP" ) &&
         driverName != QLatin1String( "PGDUMP" ) &&
         driverName != QLatin1String( "PostgreSQL SQL dump" ) )
    {
      // We do not want the 'PG' driver to be wrongly identified with PGDUMP
      continue;
    }
    if ( it.key().startsWith( driverName ) || it.value().longName.startsWith( driverName ) )
    {
      driverMetadata = it.value();
      return true;
    }
  }

  return false;
}

QStringList QgsVectorFileWriter::defaultDatasetOptions( const QString &driverName )
{
  MetaData metadata;
  bool ok = driverMetadata( driverName, metadata );
  if ( !ok )
    return QStringList();
  return concatenateOptions( metadata.driverOptions );
}

QStringList QgsVectorFileWriter::defaultLayerOptions( const QString &driverName )
{
  MetaData metadata;
  bool ok = driverMetadata( driverName, metadata );
  if ( !ok )
    return QStringList();
  return concatenateOptions( metadata.layerOptions );
}

OGRwkbGeometryType QgsVectorFileWriter::ogrTypeFromWkbType( QgsWkbTypes::Type type )
{

  OGRwkbGeometryType ogrType = static_cast<OGRwkbGeometryType>( type );

  if ( type >= QgsWkbTypes::PointZ && type <= QgsWkbTypes::GeometryCollectionZ )
  {
    ogrType = static_cast<OGRwkbGeometryType>( QgsWkbTypes::to25D( type ) );
  }
  return ogrType;
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::hasError()
{
  return mError;
}

QString QgsVectorFileWriter::errorMessage()
{
  return mErrorMessage;
}

bool QgsVectorFileWriter::addFeature( QgsFeature &feature, QgsFeatureSink::Flags )
{
  return addFeatureWithStyle( feature, nullptr, QgsUnitTypes::DistanceMeters );
}

bool QgsVectorFileWriter::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags )
{
  QgsFeatureList::iterator fIt = features.begin();
  bool result = true;
  for ( ; fIt != features.end(); ++fIt )
  {
    result = result && addFeatureWithStyle( *fIt, nullptr, QgsUnitTypes::DistanceMeters );
  }
  return result;
}

QString QgsVectorFileWriter::lastError() const
{
  return mErrorMessage;
}

bool QgsVectorFileWriter::addFeatureWithStyle( QgsFeature &feature, QgsFeatureRenderer *renderer, QgsUnitTypes::DistanceUnit outputUnit )
{
  // create the feature
  gdal::ogr_feature_unique_ptr poFeature = createFeature( feature );
  if ( !poFeature )
    return false;

  //add OGR feature style type
  if ( mSymbologyExport != NoSymbology && renderer )
  {
    mRenderContext.expressionContext().setFeature( feature );
    //SymbolLayerSymbology: concatenate ogr styles of all symbollayers
    QgsSymbolList symbols = renderer->symbolsForFeature( feature, mRenderContext );
    QString styleString;
    QString currentStyle;

    QgsSymbolList::const_iterator symbolIt = symbols.constBegin();
    for ( ; symbolIt != symbols.constEnd(); ++symbolIt )
    {
      int nSymbolLayers = ( *symbolIt )->symbolLayerCount();
      for ( int i = 0; i < nSymbolLayers; ++i )
      {
#if 0
        QMap< QgsSymbolLayer *, QString >::const_iterator it = mSymbolLayerTable.find( ( *symbolIt )->symbolLayer( i ) );
        if ( it == mSymbolLayerTable.constEnd() )
        {
          continue;
        }
#endif
        double mmsf = mmScaleFactor( mSymbologyScale, ( *symbolIt )->outputUnit(), outputUnit );
        double musf = mapUnitScaleFactor( mSymbologyScale, ( *symbolIt )->outputUnit(), outputUnit );

        currentStyle = ( *symbolIt )->symbolLayer( i )->ogrFeatureStyle( mmsf, musf );//"@" + it.value();

        if ( mSymbologyExport == FeatureSymbology )
        {
          if ( symbolIt != symbols.constBegin() || i != 0 )
          {
            styleString.append( ';' );
          }
          styleString.append( currentStyle );
        }
        else if ( mSymbologyExport == SymbolLayerSymbology )
        {
          OGR_F_SetStyleString( poFeature.get(), currentStyle.toLocal8Bit().constData() );
          if ( !writeFeature( mLayer, poFeature.get() ) )
          {
            return false;
          }
        }
      }
    }
    OGR_F_SetStyleString( poFeature.get(), styleString.toLocal8Bit().constData() );
  }

  if ( mSymbologyExport == NoSymbology || mSymbologyExport == FeatureSymbology )
  {
    if ( !writeFeature( mLayer, poFeature.get() ) )
    {
      return false;
    }
  }

  return true;
}

gdal::ogr_feature_unique_ptr QgsVectorFileWriter::createFeature( const QgsFeature &feature )
{
  QgsLocaleNumC l; // Make sure the decimal delimiter is a dot
  Q_UNUSED( l )

  gdal::ogr_feature_unique_ptr poFeature( OGR_F_Create( OGR_L_GetLayerDefn( mLayer ) ) );

  // attribute handling
  for ( QMap<int, int>::const_iterator it = mAttrIdxToOgrIdx.constBegin(); it != mAttrIdxToOgrIdx.constEnd(); ++it )
  {
    int fldIdx = it.key();
    int ogrField = it.value();

    QVariant attrValue = feature.attribute( fldIdx );
    QgsField field = mFields.at( fldIdx );

    if ( !attrValue.isValid() || attrValue.isNull() )
    {
// Starting with GDAL 2.2, there are 2 concepts: unset fields and null fields
// whereas previously there was only unset fields. For a GeoJSON output,
// leaving a field unset will cause it to not appear at all in the output
// feature.
// When all features of a layer have a field unset, this would cause the
// field to not be present at all in the output, and thus on reading to
// have disappeared. #16812
#ifdef OGRNullMarker
      OGR_F_SetFieldNull( poFeature.get(), ogrField );
#endif
      continue;
    }

    if ( mFieldValueConverter )
    {
      field = mFieldValueConverter->fieldDefinition( field );
      attrValue = mFieldValueConverter->convert( fldIdx, attrValue );
    }

    // Check type compatibility before passing attribute value to OGR
    QString errorMessage;
    if ( ! field.convertCompatible( attrValue, &errorMessage ) )
    {
      mErrorMessage = QObject::tr( "Error converting value (%1) for attribute field %2: %3" )
                      .arg( feature.attribute( fldIdx ).toString(),
                            mFields.at( fldIdx ).name(), errorMessage );
      QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
      mError = ErrFeatureWriteFailed;
      return nullptr;
    }

    switch ( field.type() )
    {
      case QVariant::Int:
        OGR_F_SetFieldInteger( poFeature.get(), ogrField, attrValue.toInt() );
        break;
      case QVariant::LongLong:
        OGR_F_SetFieldInteger64( poFeature.get(), ogrField, attrValue.toLongLong() );
        break;
      case QVariant::Bool:
        OGR_F_SetFieldInteger( poFeature.get(), ogrField, attrValue.toInt() );
        break;
      case QVariant::String:
        OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( attrValue.toString() ).constData() );
        break;
      case QVariant::Double:
        OGR_F_SetFieldDouble( poFeature.get(), ogrField, attrValue.toDouble() );
        break;
      case QVariant::Date:
        OGR_F_SetFieldDateTime( poFeature.get(), ogrField,
                                attrValue.toDate().year(),
                                attrValue.toDate().month(),
                                attrValue.toDate().day(),
                                0, 0, 0, 0 );
        break;
      case QVariant::DateTime:
        if ( mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
        {
          OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( attrValue.toDateTime().toString( QStringLiteral( "yyyy/MM/dd hh:mm:ss.zzz" ) ) ).constData() );
        }
        else
        {
          OGR_F_SetFieldDateTime( poFeature.get(), ogrField,
                                  attrValue.toDateTime().date().year(),
                                  attrValue.toDateTime().date().month(),
                                  attrValue.toDateTime().date().day(),
                                  attrValue.toDateTime().time().hour(),
                                  attrValue.toDateTime().time().minute(),
                                  attrValue.toDateTime().time().second(),
                                  0 );
        }
        break;
      case QVariant::Time:
        if ( mOgrDriverName == QLatin1String( "ESRI Shapefile" ) )
        {
          OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( attrValue.toString() ).constData() );
        }
        else
        {
          OGR_F_SetFieldDateTime( poFeature.get(), ogrField,
                                  0, 0, 0,
                                  attrValue.toTime().hour(),
                                  attrValue.toTime().minute(),
                                  attrValue.toTime().second(),
                                  0 );
        }
        break;

      case QVariant::ByteArray:
      {
        const QByteArray ba = attrValue.toByteArray();
        OGR_F_SetFieldBinary( poFeature.get(), ogrField, ba.size(), const_cast< GByte * >( reinterpret_cast< const GByte * >( ba.data() ) ) );
        break;
      }

      case QVariant::Invalid:
        break;

      case QVariant::StringList:
      {
        // handle GPKG conversion to JSON
        if ( mOgrDriverName == QLatin1String( "GPKG" ) )
        {
          const QJsonDocument doc = QJsonDocument::fromVariant( attrValue );
          QString jsonString;
          if ( !doc.isNull() )
          {
            jsonString = QString::fromUtf8( doc.toJson( QJsonDocument::Compact ).constData() );
          }
          OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( jsonString.constData() ) );
          break;
        }

        QStringList list = attrValue.toStringList();
        if ( mSupportedListSubTypes.contains( QVariant::String ) )
        {
          int count = list.count();
          char **lst = new char *[count + 1];
          if ( count > 0 )
          {
            int pos = 0;
            for ( const QString &string : list )
            {
              lst[pos] = CPLStrdup( mCodec->fromUnicode( string ).data() );
              pos++;
            }
          }
          lst[count] = nullptr;
          OGR_F_SetFieldStringList( poFeature.get(), ogrField, lst );
          CSLDestroy( lst );
        }
        else
        {
          OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( list.join( ',' ) ).constData() );
        }
        break;
      }

      case QVariant::List:
        // handle GPKG conversion to JSON
        if ( mOgrDriverName == QLatin1String( "GPKG" ) )
        {
          const QJsonDocument doc = QJsonDocument::fromVariant( attrValue );
          QString jsonString;
          if ( !doc.isNull() )
          {
            jsonString = QString::fromUtf8( doc.toJson( QJsonDocument::Compact ).data() );
          }
          OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( jsonString.constData() ) );
          break;
        }

        // fall through to default for unsupported types
        if ( field.subType() == QVariant::String )
        {
          QStringList list = attrValue.toStringList();
          if ( mSupportedListSubTypes.contains( QVariant::String ) )
          {
            int count = list.count();
            char **lst = new char *[count + 1];
            if ( count > 0 )
            {
              int pos = 0;
              for ( const QString &string : list )
              {
                lst[pos] = CPLStrdup( mCodec->fromUnicode( string ).data() );
                pos++;
              }
            }
            lst[count] = nullptr;
            OGR_F_SetFieldStringList( poFeature.get(), ogrField, lst );
            CSLDestroy( lst );
          }
          else
          {
            OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( list.join( ',' ) ).constData() );
          }
          break;
        }
        else if ( field.subType() == QVariant::Int )
        {
          const QVariantList list = attrValue.toList();
          if ( mSupportedListSubTypes.contains( QVariant::Int ) )
          {
            const int count = list.count();
            int *lst = new int[count];
            if ( count > 0 )
            {
              int pos = 0;
              for ( const QVariant &value : list )
              {
                lst[pos] = value.toInt();
                pos++;
              }
            }
            OGR_F_SetFieldIntegerList( poFeature.get(), ogrField, count, lst );
            delete [] lst;
          }
          else
          {
            QStringList strings;
            strings.reserve( list.size() );
            for ( const QVariant &value : list )
            {
              strings << QString::number( value.toInt() );
            }
            OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( strings.join( ',' ) ).constData() );
          }
          break;
        }
        else if ( field.subType() == QVariant::Double )
        {
          const QVariantList list = attrValue.toList();
          if ( mSupportedListSubTypes.contains( QVariant::Double ) )
          {
            const int count = list.count();
            double *lst = new double[count];
            if ( count > 0 )
            {
              int pos = 0;
              for ( const QVariant &value : list )
              {
                lst[pos] = value.toDouble();
                pos++;
              }
            }
            OGR_F_SetFieldDoubleList( poFeature.get(), ogrField, count, lst );
            delete [] lst;
          }
          else
          {
            QStringList strings;
            strings.reserve( list.size() );
            for ( const QVariant &value : list )
            {
              strings << QString::number( value.toDouble() );
            }
            OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( strings.join( ',' ) ).constData() );
          }
          break;
        }
        else if ( field.subType() == QVariant::LongLong )
        {
          const QVariantList list = attrValue.toList();
          if ( mSupportedListSubTypes.contains( QVariant::LongLong ) )
          {
            const int count = list.count();
            long long *lst = new long long[count];
            if ( count > 0 )
            {
              int pos = 0;
              for ( const QVariant &value : list )
              {
                lst[pos] = value.toLongLong();
                pos++;
              }
            }
            OGR_F_SetFieldInteger64List( poFeature.get(), ogrField, count, lst );
            delete [] lst;
          }
          else
          {
            QStringList strings;
            strings.reserve( list.size() );
            for ( const QVariant &value : list )
            {
              strings << QString::number( value.toLongLong() );
            }
            OGR_F_SetFieldString( poFeature.get(), ogrField, mCodec->fromUnicode( strings.join( ',' ) ).constData() );
          }
          break;
        }
        //intentional fall-through
        FALLTHROUGH

      default:
        mErrorMessage = QObject::tr( "Invalid variant type for field %1[%2]: received %3 with type %4" )
                        .arg( mFields.at( fldIdx ).name() )
                        .arg( ogrField )
                        .arg( attrValue.typeName(),
                              attrValue.toString() );
        QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
        mError = ErrFeatureWriteFailed;
        return nullptr;
    }
  }

  if ( mWkbType != QgsWkbTypes::NoGeometry )
  {
    if ( feature.hasGeometry() )
    {
      // build geometry from WKB
      QgsGeometry geom = feature.geometry();
      if ( mCoordinateTransform )
      {
        // output dataset requires coordinate transform
        try
        {
          geom.transform( *mCoordinateTransform );
        }
        catch ( QgsCsException & )
        {
          QgsLogger::warning( QObject::tr( "Feature geometry failed to transform" ) );
          return nullptr;
        }
      }

      // turn single geometry to multi geometry if needed
      if ( QgsWkbTypes::flatType( geom.wkbType() ) != QgsWkbTypes::flatType( mWkbType ) &&
           QgsWkbTypes::flatType( geom.wkbType() ) == QgsWkbTypes::flatType( QgsWkbTypes::singleType( mWkbType ) ) )
      {
        geom.convertToMultiType();
      }

      if ( geom.wkbType() != mWkbType )
      {
        OGRGeometryH mGeom2 = nullptr;

        // If requested WKB type is 25D and geometry WKB type is 3D,
        // we must force the use of 25D.
        if ( mWkbType >= QgsWkbTypes::Point25D && mWkbType <= QgsWkbTypes::MultiPolygon25D )
        {
          //ND: I suspect there's a bug here, in that this is NOT converting the geometry's WKB type,
          //so the exported WKB has a different type to what the OGRGeometry is expecting.
          //possibly this is handled already in OGR, but it should be fixed regardless by actually converting
          //geom to the correct WKB type
          QgsWkbTypes::Type wkbType = geom.wkbType();
          if ( wkbType >= QgsWkbTypes::PointZ && wkbType <= QgsWkbTypes::MultiPolygonZ )
          {
            QgsWkbTypes::Type wkbType25d = static_cast<QgsWkbTypes::Type>( geom.wkbType() - QgsWkbTypes::PointZ + QgsWkbTypes::Point25D );
            mGeom2 = createEmptyGeometry( wkbType25d );
          }
        }

        // drop m/z value if not present in output wkb type
        if ( !QgsWkbTypes::hasZ( mWkbType ) && QgsWkbTypes::hasZ( geom.wkbType() ) )
          geom.get()->dropZValue();
        if ( !QgsWkbTypes::hasM( mWkbType ) && QgsWkbTypes::hasM( geom.wkbType() ) )
          geom.get()->dropMValue();

        // add m/z values if not present in the input wkb type -- this is needed for formats which determine
        // geometry type based on features, e.g. geojson
        if ( QgsWkbTypes::hasZ( mWkbType ) && !QgsWkbTypes::hasZ( geom.wkbType() ) )
          geom.get()->addZValue( 0 );
        if ( QgsWkbTypes::hasM( mWkbType ) && !QgsWkbTypes::hasM( geom.wkbType() ) )
          geom.get()->addMValue( 0 );

        if ( !mGeom2 )
        {
          // there's a problem when layer type is set as wkbtype Polygon
          // although there are also features of type MultiPolygon
          // (at least in OGR provider)
          // If the feature's wkbtype is different from the layer's wkbtype,
          // try to export it too.
          //
          // Btw. OGRGeometry must be exactly of the type of the geometry which it will receive
          // i.e. Polygons can't be imported to OGRMultiPolygon
          mGeom2 = createEmptyGeometry( geom.wkbType() );
        }

        if ( !mGeom2 )
        {
          mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                          .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
          mError = ErrFeatureWriteFailed;
          QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
          return nullptr;
        }

        QByteArray wkb( geom.asWkb() );
        OGRErr err = OGR_G_ImportFromWkb( mGeom2, reinterpret_cast<unsigned char *>( const_cast<char *>( wkb.constData() ) ), wkb.length() );
        if ( err != OGRERR_NONE )
        {
          mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                          .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
          mError = ErrFeatureWriteFailed;
          QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
          return nullptr;
        }

        // pass ownership to geometry
        OGR_F_SetGeometryDirectly( poFeature.get(), mGeom2 );
      }
      else // wkb type matches
      {
        QByteArray wkb( geom.asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons ) );
        OGRGeometryH ogrGeom = createEmptyGeometry( mWkbType );
        OGRErr err = OGR_G_ImportFromWkb( ogrGeom, reinterpret_cast<unsigned char *>( const_cast<char *>( wkb.constData() ) ), wkb.length() );
        if ( err != OGRERR_NONE )
        {
          mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                          .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
          mError = ErrFeatureWriteFailed;
          QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
          return nullptr;
        }

        // set geometry (ownership is passed to OGR)
        OGR_F_SetGeometryDirectly( poFeature.get(), ogrGeom );
      }
    }
    else
    {
      OGR_F_SetGeometryDirectly( poFeature.get(), createEmptyGeometry( mWkbType ) );
    }
  }
  return poFeature;
}

void QgsVectorFileWriter::resetMap( const QgsAttributeList &attributes )
{
  QMap<int, int> omap( mAttrIdxToOgrIdx );
  mAttrIdxToOgrIdx.clear();
  for ( int i = 0; i < attributes.size(); i++ )
  {
    if ( omap.find( i ) != omap.end() )
      mAttrIdxToOgrIdx.insert( attributes[i], omap[i] );
  }
}

bool QgsVectorFileWriter::writeFeature( OGRLayerH layer, OGRFeatureH feature )
{
  if ( OGR_L_CreateFeature( layer, feature ) != OGRERR_NONE )
  {
    mErrorMessage = QObject::tr( "Feature creation error (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrFeatureWriteFailed;
    QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
    return false;
  }
  return true;
}

QgsVectorFileWriter::~QgsVectorFileWriter()
{
  if ( mUsingTransaction )
  {
    if ( OGRERR_NONE != OGR_L_CommitTransaction( mLayer ) )
    {
      QgsDebugMsg( QStringLiteral( "Error while committing transaction on OGRLayer." ) );
    }
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0) && GDAL_VERSION_NUM <= GDAL_COMPUTE_VERSION(3,1,3)
  if ( mDS )
  {
    // Workaround bug in GDAL 3.1.0 to 3.1.3 that creates XLSX and ODS files incompatible with LibreOffice due to use of ZIP64
    QString drvName = GDALGetDriverShortName( GDALGetDatasetDriver( mDS.get() ) );
    if ( drvName == QLatin1String( "XLSX" ) ||
         drvName == QLatin1String( "ODS" ) )
    {
      CPLSetThreadLocalConfigOption( "CPL_CREATE_ZIP64", "NO" );
      mDS.reset();
      CPLSetThreadLocalConfigOption( "CPL_CREATE_ZIP64", nullptr );
    }
  }
#endif

  mDS.reset();

  if ( mOgrRef )
  {
    OSRRelease( mOgrRef );
  }
}

QgsVectorFileWriter::WriterError
QgsVectorFileWriter::writeAsVectorFormat( QgsVectorLayer *layer,
    const QString &fileName,
    const QString &fileEncoding,
    const QgsCoordinateReferenceSystem &destCRS,
    const QString &driverName,
    bool onlySelected,
    QString *errorMessage,
    const QStringList &datasourceOptions,
    const QStringList &layerOptions,
    bool skipAttributeCreation,
    QString *newFilename,
    SymbologyExport symbologyExport,
    double symbologyScale,
    const QgsRectangle *filterExtent,
    QgsWkbTypes::Type overrideGeometryType,
    bool forceMulti,
    bool includeZ,
    const QgsAttributeList &attributes,
    FieldValueConverter *fieldValueConverter,
    QString *newLayer )
{
  QgsCoordinateTransform ct;
  if ( destCRS.isValid() && layer )
  {
    ct = QgsCoordinateTransform( layer->crs(), destCRS, layer->transformContext() );
  }

  SaveVectorOptions options;
  options.fileEncoding = fileEncoding;
  options.ct = ct;
  options.driverName = driverName;
  options.onlySelectedFeatures = onlySelected;
  options.datasourceOptions = datasourceOptions;
  options.layerOptions = layerOptions;
  options.skipAttributeCreation = skipAttributeCreation;
  options.symbologyExport = symbologyExport;
  options.symbologyScale = symbologyScale;
  if ( filterExtent )
    options.filterExtent = *filterExtent;
  options.overrideGeometryType = overrideGeometryType;
  options.forceMulti = forceMulti;
  options.includeZ = includeZ;
  options.attributes = attributes;
  options.fieldValueConverter = fieldValueConverter;
  return writeAsVectorFormatV3( layer, fileName, layer->transformContext(), options, errorMessage, newFilename, newLayer );
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::writeAsVectorFormat( QgsVectorLayer *layer,
    const QString &fileName,
    const QString &fileEncoding,
    const QgsCoordinateTransform &ct,
    const QString &driverName,
    bool onlySelected,
    QString *errorMessage,
    const QStringList &datasourceOptions,
    const QStringList &layerOptions,
    bool skipAttributeCreation,
    QString *newFilename,
    SymbologyExport symbologyExport,
    double symbologyScale,
    const QgsRectangle *filterExtent,
    QgsWkbTypes::Type overrideGeometryType,
    bool forceMulti,
    bool includeZ,
    const QgsAttributeList &attributes,
    FieldValueConverter *fieldValueConverter,
    QString *newLayer )
{
  SaveVectorOptions options;
  options.fileEncoding = fileEncoding;
  options.ct = ct;
  options.driverName = driverName;
  options.onlySelectedFeatures = onlySelected;
  options.datasourceOptions = datasourceOptions;
  options.layerOptions = layerOptions;
  options.skipAttributeCreation = skipAttributeCreation;
  options.symbologyExport = symbologyExport;
  options.symbologyScale = symbologyScale;
  if ( filterExtent )
    options.filterExtent = *filterExtent;
  options.overrideGeometryType = overrideGeometryType;
  options.forceMulti = forceMulti;
  options.includeZ = includeZ;
  options.attributes = attributes;
  options.fieldValueConverter = fieldValueConverter;
  return writeAsVectorFormatV3( layer, fileName, layer->transformContext(), options, errorMessage, newFilename, newLayer );
}


QgsVectorFileWriter::SaveVectorOptions::SaveVectorOptions()
  : driverName( QStringLiteral( "GPKG" ) )
{
}



QgsVectorFileWriter::WriterError QgsVectorFileWriter::prepareWriteAsVectorFormat( QgsVectorLayer *layer, const QgsVectorFileWriter::SaveVectorOptions &options, QgsVectorFileWriter::PreparedWriterDetails &details )
{
  if ( !layer || !layer->isValid() )
  {
    return ErrInvalidLayer;
  }

  if ( layer->renderer() )
    details.renderer.reset( layer->renderer()->clone() );
  details.sourceCrs = layer->crs();
  details.sourceWkbType = layer->wkbType();
  details.sourceFields = layer->fields();
  details.providerType = layer->providerType();
  details.featureCount = options.onlySelectedFeatures ? layer->selectedFeatureCount() : layer->featureCount();
  if ( layer->dataProvider() )
    details.dataSourceUri = layer->dataProvider()->dataSourceUri();
  details.storageType = layer->storageType();
  details.selectedFeatureIds = layer->selectedFeatureIds();
  details.providerUriParams = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );

  if ( details.storageType == QLatin1String( "ESRI Shapefile" ) )
  {
    QgsFeatureRequest req;
    if ( options.onlySelectedFeatures )
    {
      req.setFilterFids( details.selectedFeatureIds );
    }
    req.setNoAttributes();
    details.geometryTypeScanIterator = layer->getFeatures( req );
  }

  details.expressionContext = QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  details.renderContext.setExpressionContext( details.expressionContext );
  details.renderContext.setRendererScale( options.symbologyScale );

  details.shallTransform = false;
  if ( options.ct.isValid() )
  {
    // This means we should transform
    details.outputCrs = options.ct.destinationCrs();
    details.shallTransform = true;
  }
  else
  {
    // This means we shouldn't transform, use source CRS as output (if defined)
    details.outputCrs = details.sourceCrs;
  }

  details.destWkbType = details.sourceWkbType;
  if ( options.overrideGeometryType != QgsWkbTypes::Unknown )
  {
    details.destWkbType = QgsWkbTypes::flatType( options.overrideGeometryType );
    if ( QgsWkbTypes::hasZ( options.overrideGeometryType ) || options.includeZ )
      details.destWkbType = QgsWkbTypes::addZ( details.destWkbType );
  }
  if ( options.forceMulti )
  {
    details.destWkbType = QgsWkbTypes::multiType( details.destWkbType );
  }

  details.attributes = options.attributes;
  if ( options.skipAttributeCreation )
    details.attributes.clear();
  else if ( details.attributes.isEmpty() )
  {
    const QgsAttributeList allAttributes = details.sourceFields.allAttributesList();
    for ( int idx : allAttributes )
    {
      QgsField fld = details.sourceFields.at( idx );
      if ( details.providerType == QLatin1String( "oracle" ) && fld.typeName().contains( QLatin1String( "SDO_GEOMETRY" ) ) )
        continue;
      details.attributes.append( idx );
    }
  }

  if ( !details.attributes.isEmpty() )
  {
    for ( int attrIdx : std::as_const( details.attributes ) )
    {
      details.outputFields.append( details.sourceFields.at( attrIdx ) );
    }
  }

  // not ideal - would be nice to avoid this happening in the preparation step if possible,
  // but currently requires access to the layer's minimumValue/maximumValue methods
  if ( details.providerType == QLatin1String( "spatialite" ) )
  {
    for ( int i = 0; i < details.outputFields.size(); i++ )
    {
      if ( details.outputFields.at( i ).type() == QVariant::LongLong )
      {
        QVariant min;
        QVariant max;
        layer->minimumAndMaximumValue( i, min, max );
        if ( std::max( std::llabs( min.toLongLong() ), std::llabs( max.toLongLong() ) ) < std::numeric_limits<int>::max() )
        {
          details.outputFields[i].setType( QVariant::Int );
        }
      }
    }
  }


  //add possible attributes needed by renderer
  addRendererAttributes( details.renderer.get(), details.renderContext, details.sourceFields, details.attributes );

  QgsFeatureRequest req;
  req.setSubsetOfAttributes( details.attributes );
  if ( options.onlySelectedFeatures )
    req.setFilterFids( details.selectedFeatureIds );

  if ( !options.filterExtent.isNull() )
  {
    QgsRectangle filterRect = options.filterExtent;
    bool useFilterRect = true;
    if ( details.shallTransform )
    {
      try
      {
        // map filter rect back from destination CRS to layer CRS
        QgsCoordinateTransform extentTransform = options.ct;
        extentTransform.setBallparkTransformsAreAppropriate( true );
        filterRect = extentTransform.transformBoundingBox( filterRect, Qgis::TransformDirection::Reverse );
      }
      catch ( QgsCsException & )
      {
        useFilterRect = false;
      }
    }
    if ( useFilterRect )
    {
      req.setFilterRect( filterRect );
    }
    details.filterRectGeometry = QgsGeometry::fromRect( options.filterExtent );
    details.filterRectEngine.reset( QgsGeometry::createGeometryEngine( details.filterRectGeometry.constGet() ) );
    details.filterRectEngine->prepareGeometry();
  }
  details.sourceFeatureIterator = layer->getFeatures( req );

  return NoError;
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::writeAsVectorFormat( PreparedWriterDetails &details, const QString &fileName, const QgsVectorFileWriter::SaveVectorOptions &options, QString *newFilename, QString *errorMessage, QString *newLayer )
{
  return writeAsVectorFormatV2( details, fileName, QgsCoordinateTransformContext(), options, newFilename, newLayer, errorMessage );
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::writeAsVectorFormatV2( PreparedWriterDetails &details, const QString &fileName, const QgsCoordinateTransformContext &transformContext, const QgsVectorFileWriter::SaveVectorOptions &options, QString *newFilename, QString *newLayer, QString *errorMessage )
{
  QgsWkbTypes::Type destWkbType = details.destWkbType;

  int lastProgressReport = 0;
  long long total = details.featureCount;

  // Special rules for OGR layers
  if ( details.providerType == QLatin1String( "ogr" ) && !details.dataSourceUri.isEmpty() )
  {
    QString srcFileName( details.providerUriParams.value( QStringLiteral( "path" ) ).toString() );
    if ( QFile::exists( srcFileName ) && QFileInfo( fileName ).canonicalFilePath() == QFileInfo( srcFileName ).canonicalFilePath() )
    {
      // Check the layer name too if it's a GPKG/SpatiaLite/SQLite OGR driver (pay attention: camel case in layerName)
      QgsDataSourceUri uri( details.dataSourceUri );
      if ( !( ( options.driverName == QLatin1String( "GPKG" ) ||
                options.driverName == QLatin1String( "SpatiaLite" ) ||
                options.driverName == QLatin1String( "SQLite" ) ) &&
              options.layerName != details.providerUriParams.value( QStringLiteral( "layerName" ) ) ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Cannot overwrite a OGR layer in place" );
        return ErrCreateDataSource;
      }
    }

    // Shapefiles might contain multi types although wkbType() only reports singles
    if ( details.storageType == QLatin1String( "ESRI Shapefile" ) && !QgsWkbTypes::isMultiType( destWkbType ) )
    {
      QgsFeatureIterator fit = details.geometryTypeScanIterator;
      QgsFeature fet;
      long scanned = 0;
      while ( fit.nextFeature( fet ) )
      {
        if ( options.feedback && options.feedback->isCanceled() )
        {
          return Canceled;
        }
        if ( options.feedback )
        {
          //dedicate first 5% of progress bar to this scan
          int newProgress = static_cast<int>( ( 5.0 * scanned ) / total );
          if ( newProgress != lastProgressReport )
          {
            lastProgressReport = newProgress;
            options.feedback->setProgress( lastProgressReport );
          }
        }

        if ( fet.hasGeometry() && QgsWkbTypes::isMultiType( fet.geometry().wkbType() ) )
        {
          destWkbType = QgsWkbTypes::multiType( destWkbType );
          break;
        }
        scanned++;
      }
    }
  }

  QString tempNewFilename;
  QString tempNewLayer;

  std::unique_ptr< QgsVectorFileWriter > writer( create( fileName, details.outputFields, destWkbType, details.outputCrs, transformContext, options, QgsFeatureSink::SinkFlags(), &tempNewFilename, &tempNewLayer ) );
  writer->setSymbologyScale( options.symbologyScale );

  if ( newFilename )
    *newFilename = tempNewFilename;

  if ( newLayer )
    *newLayer = tempNewLayer;

  if ( newFilename )
  {
    QgsDebugMsgLevel( "newFilename = " + *newFilename, 2 );
  }

  // check whether file creation was successful
  WriterError err = writer->hasError();
  if ( err != NoError )
  {
    if ( errorMessage )
      *errorMessage = writer->errorMessage();
    return err;
  }

  if ( errorMessage )
  {
    errorMessage->clear();
  }

  QgsFeature fet;

  //create symbol table if needed
  if ( writer->symbologyExport() != NoSymbology )
  {
    //writer->createSymbolLayerTable( layer,  writer->mDS );
  }

  if ( writer->symbologyExport() == SymbolLayerSymbology )
  {
    QgsFeatureRenderer *r = details.renderer.get();
    if ( r && r->capabilities() & QgsFeatureRenderer::SymbolLevels
         && r->usingSymbolLevels() )
    {
      QgsVectorFileWriter::WriterError error = writer->exportFeaturesSymbolLevels( details, details.sourceFeatureIterator, options.ct, errorMessage );
      return ( error == NoError ) ? NoError : ErrFeatureWriteFailed;
    }
  }

  int n = 0, errors = 0;

  //unit type
  QgsUnitTypes::DistanceUnit mapUnits = details.sourceCrs.mapUnits();
  if ( options.ct.isValid() )
  {
    mapUnits = options.ct.destinationCrs().mapUnits();
  }

  writer->startRender( details.renderer.get(), details.sourceFields );

  writer->resetMap( details.attributes );
  // Reset mFields to layer fields, and not just exported fields
  writer->mFields = details.sourceFields;

  // write all features
  long saved = 0;
  int initialProgress = lastProgressReport;
  while ( details.sourceFeatureIterator.nextFeature( fet ) )
  {
    if ( options.feedback && options.feedback->isCanceled() )
    {
      return Canceled;
    }

    saved++;
    if ( options.feedback )
    {
      //avoid spamming progress reports
      int newProgress = static_cast<int>( initialProgress + ( ( 100.0 - initialProgress ) * saved ) / total );
      if ( newProgress < 100 && newProgress != lastProgressReport )
      {
        lastProgressReport = newProgress;
        options.feedback->setProgress( lastProgressReport );
      }
    }

    if ( details.shallTransform )
    {
      try
      {
        if ( fet.hasGeometry() )
        {
          QgsGeometry g = fet.geometry();
          g.transform( options.ct );
          fet.setGeometry( g );
        }
      }
      catch ( QgsCsException &e )
      {
        QString msg = QObject::tr( "Failed to transform a point while drawing a feature with ID '%1'. Writing stopped. (Exception: %2)" )
                      .arg( fet.id() ).arg( e.what() );
        QgsLogger::warning( msg );
        if ( errorMessage )
          *errorMessage = msg;

        return ErrProjection;
      }
    }

    if ( fet.hasGeometry() && details.filterRectEngine && !details.filterRectEngine->intersects( fet.geometry().constGet() ) )
      continue;

    if ( details.attributes.empty() && options.skipAttributeCreation )
    {
      fet.initAttributes( 0 );
    }

    if ( !writer->addFeatureWithStyle( fet, writer->mRenderer.get(), mapUnits ) )
    {
      WriterError err = writer->hasError();
      if ( err != NoError && errorMessage )
      {
        if ( errorMessage->isEmpty() )
        {
          *errorMessage = QObject::tr( "Feature write errors:" );
        }
        *errorMessage += '\n' + writer->errorMessage();
      }
      errors++;

      if ( errors > 1000 )
      {
        if ( errorMessage )
        {
          *errorMessage += QObject::tr( "Stopping after %n error(s)", nullptr, errors );
        }

        n = -1;
        break;
      }
    }
    n++;
  }

  writer->stopRender();

  if ( errors > 0 && errorMessage && n > 0 )
  {
    *errorMessage += QObject::tr( "\nOnly %1 of %2 features written." ).arg( n - errors ).arg( n );
  }

  writer.reset();

  bool metadataFailure = false;
  if ( options.saveMetadata )
  {
    QString uri = QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), QVariantMap
    {
      {QStringLiteral( "path" ), tempNewFilename },
      {QStringLiteral( "layerName" ), tempNewLayer }
    } );

    try
    {
      QString error;
      if ( !QgsProviderRegistry::instance()->saveLayerMetadata( QStringLiteral( "ogr" ), uri, options.layerMetadata, error ) )
      {
        if ( errorMessage )
        {
          if ( !errorMessage->isEmpty() )
            *errorMessage += '\n';
          *errorMessage += error;
        }
        metadataFailure = true;
      }
    }
    catch ( QgsNotSupportedException &e )
    {
      if ( errorMessage )
      {
        if ( !errorMessage->isEmpty() )
          *errorMessage += '\n';
        *errorMessage += e.what();
      }
      metadataFailure = true;
    }
  }

  return errors == 0 ? ( !metadataFailure ? NoError : ErrSavingMetadata ) : ErrFeatureWriteFailed;
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::writeAsVectorFormat( QgsVectorLayer *layer,
    const QString &fileName,
    const SaveVectorOptions &options,
    QString *newFilename,
    QString *errorMessage,
    QString *newLayer )
{
  QgsVectorFileWriter::PreparedWriterDetails details;
  WriterError err = prepareWriteAsVectorFormat( layer, options, details );
  if ( err != NoError )
    return err;

  return writeAsVectorFormatV2( details, fileName, layer->transformContext(), options, newFilename, newLayer, errorMessage );
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::writeAsVectorFormatV2( QgsVectorLayer *layer,
    const QString &fileName,
    const QgsCoordinateTransformContext &transformContext,
    const SaveVectorOptions &options,
    QString *newFilename,
    QString *newLayer,
    QString *errorMessage )
{
  QgsVectorFileWriter::PreparedWriterDetails details;
  WriterError err = prepareWriteAsVectorFormat( layer, options, details );
  if ( err != NoError )
    return err;

  return writeAsVectorFormatV2( details, fileName, transformContext, options, errorMessage, newFilename, newLayer );
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::writeAsVectorFormatV3( QgsVectorLayer *layer, const QString &fileName, const QgsCoordinateTransformContext &transformContext, const QgsVectorFileWriter::SaveVectorOptions &options, QString *errorMessage, QString *newFilename, QString *newLayer )
{
  QgsVectorFileWriter::PreparedWriterDetails details;
  WriterError err = prepareWriteAsVectorFormat( layer, options, details );
  if ( err != NoError )
    return err;

  return writeAsVectorFormatV2( details, fileName, transformContext, options, newFilename, newLayer, errorMessage );
}

bool QgsVectorFileWriter::deleteShapeFile( const QString &fileName )
{
  QFileInfo fi( fileName );
  QDir dir = fi.dir();

  QStringList filter;
  for ( const char *suffix : { ".shp", ".shx", ".dbf", ".prj", ".qix", ".qpj", ".cpg", ".sbn", ".sbx", ".idm", ".ind" } )
  {
    filter << fi.completeBaseName() + suffix;
  }

  bool ok = true;
  const auto constEntryList = dir.entryList( filter );
  for ( const QString &file : constEntryList )
  {
    QFile f( dir.canonicalPath() + '/' + file );
    if ( !f.remove() )
    {
      QgsDebugMsg( QStringLiteral( "Removing file %1 failed: %2" ).arg( file, f.errorString() ) );
      ok = false;
    }
  }

  return ok;
}

void QgsVectorFileWriter::setSymbologyScale( double d )
{
  mSymbologyScale = d;
  mRenderContext.setRendererScale( mSymbologyScale );
}

QList< QgsVectorFileWriter::FilterFormatDetails > QgsVectorFileWriter::supportedFiltersAndFormats( const VectorFormatOptions options )
{
  static QReadWriteLock sFilterLock;
  static QMap< VectorFormatOptions, QList< QgsVectorFileWriter::FilterFormatDetails > > sFilters;

  QgsReadWriteLocker locker( sFilterLock, QgsReadWriteLocker::Read );

  const auto it = sFilters.constFind( options );
  if ( it != sFilters.constEnd() )
    return it.value();

  locker.changeMode( QgsReadWriteLocker::Write );
  QList< QgsVectorFileWriter::FilterFormatDetails > results;

  QgsApplication::registerOgrDrivers();
  int const drvCount = OGRGetDriverCount();

  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drv = OGRGetDriver( i );
    if ( drv )
    {
      QString drvName = OGR_Dr_GetName( drv );

      GDALDriverH gdalDriver = GDALGetDriverByName( drvName.toLocal8Bit().constData() );
      char **metadata = nullptr;
      if ( gdalDriver )
      {
        metadata = GDALGetMetadata( gdalDriver, nullptr );
      }

      bool nonSpatialFormat = CSLFetchBoolean( metadata, GDAL_DCAP_NONSPATIAL, false );

      if ( OGR_Dr_TestCapability( drv, "CreateDataSource" ) != 0 )
      {
        if ( options & SkipNonSpatialFormats )
        {
          // skip non-spatial formats
          if ( nonSpatialFormat )
            continue;
        }

        QString filterString = filterForDriver( drvName );
        if ( filterString.isEmpty() )
          continue;

        MetaData metadata;
        QStringList globs;
        if ( driverMetadata( drvName, metadata ) && !metadata.glob.isEmpty() )
        {
          globs = metadata.glob.toLower().split( ' ' );
        }

        FilterFormatDetails details;
        details.driverName = drvName;
        details.filterString = filterString;
        details.globs = globs;

        results << details;
      }
    }
  }

  std::sort( results.begin(), results.end(), [options]( const FilterFormatDetails & a, const FilterFormatDetails & b ) -> bool
  {
    if ( options & SortRecommended )
    {
      if ( a.driverName == QLatin1String( "GPKG" ) )
        return true; // Make https://twitter.com/shapefiIe a sad little fellow
      else if ( b.driverName == QLatin1String( "GPKG" ) )
        return false;
      else if ( a.driverName == QLatin1String( "ESRI Shapefile" ) )
        return true;
      else if ( b.driverName == QLatin1String( "ESRI Shapefile" ) )
        return false;
    }

    return a.filterString.toLower().localeAwareCompare( b.filterString.toLower() ) < 0;
  } );

  sFilters.insert( options, results );
  return results;
}

QStringList QgsVectorFileWriter::supportedFormatExtensions( const VectorFormatOptions options )
{
  const auto formats = supportedFiltersAndFormats( options );
  QSet< QString > extensions;

  const QRegularExpression rx( QStringLiteral( "\\*\\.(.*)$" ) );

  for ( const FilterFormatDetails &format : formats )
  {
    for ( const QString &glob : format.globs )
    {
      const QRegularExpressionMatch match = rx.match( glob );
      if ( !match.hasMatch() )
        continue;

      const QString matched = match.captured( 1 );
      extensions.insert( matched );
    }
  }

  QStringList extensionList = qgis::setToList( extensions );

  std::sort( extensionList.begin(), extensionList.end(), [options]( const QString & a, const QString & b ) -> bool
  {
    if ( options & SortRecommended )
    {
      if ( a == QLatin1String( "gpkg" ) )
        return true; // Make https://twitter.com/shapefiIe a sad little fellow
      else if ( b == QLatin1String( "gpkg" ) )
        return false;
      else if ( a == QLatin1String( "shp" ) )
        return true;
      else if ( b == QLatin1String( "shp" ) )
        return false;
    }

    return a.toLower().localeAwareCompare( b.toLower() ) < 0;
  } );

  return extensionList;
}

QList< QgsVectorFileWriter::DriverDetails > QgsVectorFileWriter::ogrDriverList( const VectorFormatOptions options )
{
  QList< QgsVectorFileWriter::DriverDetails > results;

  QgsApplication::registerOgrDrivers();
  const int drvCount = OGRGetDriverCount();

  QStringList writableDrivers;
  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drv = OGRGetDriver( i );
    if ( drv )
    {
      QString drvName = OGR_Dr_GetName( drv );

      if ( options & SkipNonSpatialFormats )
      {
        // skip non-spatial formats
        // TODO - use GDAL metadata to determine this, when support exists in GDAL
        if ( drvName == QLatin1String( "ODS" ) || drvName == QLatin1String( "XLSX" ) || drvName == QLatin1String( "XLS" ) )
          continue;
      }

      if ( drvName == QLatin1String( "ESRI Shapefile" ) )
      {
        writableDrivers << QStringLiteral( "DBF file" );
      }
      if ( OGR_Dr_TestCapability( drv, "CreateDataSource" ) != 0 )
      {
        // Add separate format for Mapinfo MIF (MITAB is OGR default)
        if ( drvName == QLatin1String( "MapInfo File" ) )
        {
          writableDrivers << QStringLiteral( "MapInfo MIF" );
        }
        else if ( drvName == QLatin1String( "SQLite" ) )
        {
          // Unfortunately it seems that there is no simple way to detect if
          // OGR SQLite driver is compiled with SpatiaLite support.
          // We have HAVE_SPATIALITE in QGIS, but that may differ from OGR
          // http://lists.osgeo.org/pipermail/gdal-dev/2012-November/034580.html
          // -> test if creation fails
          QString option = QStringLiteral( "SPATIALITE=YES" );
          char *options[2] = { CPLStrdup( option.toLocal8Bit().constData() ), nullptr };
          OGRSFDriverH poDriver;
          QgsApplication::registerOgrDrivers();
          poDriver = OGRGetDriverByName( drvName.toLocal8Bit().constData() );
          if ( poDriver )
          {
            gdal::ogr_datasource_unique_ptr ds( OGR_Dr_CreateDataSource( poDriver, QStringLiteral( "/vsimem/spatialitetest.sqlite" ).toUtf8().constData(), options ) );
            if ( ds )
            {
              writableDrivers << QStringLiteral( "SpatiaLite" );
              OGR_Dr_DeleteDataSource( poDriver, QStringLiteral( "/vsimem/spatialitetest.sqlite" ).toUtf8().constData() );
            }
          }
          CPLFree( options[0] );
        }
        writableDrivers << drvName;
      }
    }
  }

  results.reserve( writableDrivers.count() );
  for ( const QString &drvName : std::as_const( writableDrivers ) )
  {
    MetaData metadata;
    if ( driverMetadata( drvName, metadata ) && !metadata.trLongName.isEmpty() )
    {
      DriverDetails details;
      details.driverName = drvName;
      details.longName = metadata.trLongName;
      results << details;
    }
  }

  std::sort( results.begin(), results.end(), [options]( const DriverDetails & a, const DriverDetails & b ) -> bool
  {
    if ( options & SortRecommended )
    {
      if ( a.driverName == QLatin1String( "GPKG" ) )
        return true; // Make https://twitter.com/shapefiIe a sad little fellow
      else if ( b.driverName == QLatin1String( "GPKG" ) )
        return false;
      else if ( a.driverName == QLatin1String( "ESRI Shapefile" ) )
        return true;
      else if ( b.driverName == QLatin1String( "ESRI Shapefile" ) )
        return false;
    }

    return a.longName.toLower().localeAwareCompare( b.longName.toLower() ) < 0;
  } );
  return results;
}

QString QgsVectorFileWriter::driverForExtension( const QString &extension )
{
  QString ext = extension.trimmed();
  if ( ext.isEmpty() )
    return QString();

  if ( ext.startsWith( '.' ) )
    ext.remove( 0, 1 );

  GDALAllRegister();
  int const drvCount = GDALGetDriverCount();

  for ( int i = 0; i < drvCount; ++i )
  {
    GDALDriverH drv = GDALGetDriver( i );
    if ( drv )
    {
      char **driverMetadata = GDALGetMetadata( drv, nullptr );
      if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) && CSLFetchBoolean( driverMetadata, GDAL_DCAP_VECTOR, false ) )
      {
        QString drvName = GDALGetDriverShortName( drv );
        QStringList driverExtensions = QString( GDALGetMetadataItem( drv, GDAL_DMD_EXTENSIONS, nullptr ) ).split( ' ' );

        const auto constDriverExtensions = driverExtensions;
        for ( const QString &driver : constDriverExtensions )
        {
          if ( driver.compare( ext, Qt::CaseInsensitive ) == 0 )
            return drvName;
        }
      }
    }
  }
  return QString();
}

QString QgsVectorFileWriter::fileFilterString( const VectorFormatOptions options )
{
  QString filterString;
  const auto driverFormats = supportedFiltersAndFormats( options );
  for ( const FilterFormatDetails &details : driverFormats )
  {
    if ( !filterString.isEmpty() )
      filterString += QLatin1String( ";;" );

    filterString += details.filterString;
  }
  return filterString;
}

QString QgsVectorFileWriter::filterForDriver( const QString &driverName )
{
  MetaData metadata;
  if ( !driverMetadata( driverName, metadata ) || metadata.trLongName.isEmpty() || metadata.glob.isEmpty() )
    return QString();

  return QStringLiteral( "%1 (%2 %3)" ).arg( metadata.trLongName,
         metadata.glob.toLower(),
         metadata.glob.toUpper() );
}

QString QgsVectorFileWriter::convertCodecNameForEncodingOption( const QString &codecName )
{
  if ( codecName == QLatin1String( "System" ) )
    return QStringLiteral( "LDID/0" );

  const QRegularExpression re( QRegularExpression::anchoredPattern( QString( "(CP|windows-|ISO[ -])(.+)" ) ), QRegularExpression::CaseInsensitiveOption );
  const QRegularExpressionMatch match = re.match( codecName );
  if ( match.hasMatch() )
  {
    QString c = match.captured( 2 ).remove( '-' );
    bool isNumber;
    ( void ) c.toInt( &isNumber );
    if ( isNumber )
      return c;
  }
  return codecName;
}

void QgsVectorFileWriter::createSymbolLayerTable( QgsVectorLayer *vl,  const QgsCoordinateTransform &ct, OGRDataSourceH ds )
{
  if ( !vl || !ds )
  {
    return;
  }

  QgsFeatureRenderer *renderer = vl->renderer();
  if ( !renderer )
  {
    return;
  }

  //unit type
  QgsUnitTypes::DistanceUnit mapUnits = vl->crs().mapUnits();
  if ( ct.isValid() )
  {
    mapUnits = ct.destinationCrs().mapUnits();
  }

  mSymbolLayerTable.clear();
  OGRStyleTableH ogrStyleTable = OGR_STBL_Create();
  OGRStyleMgrH styleManager = OGR_SM_Create( ogrStyleTable );

  //get symbols
  int nTotalLevels = 0;
  QgsSymbolList symbolList = renderer->symbols( mRenderContext );
  QgsSymbolList::iterator symbolIt = symbolList.begin();
  for ( ; symbolIt != symbolList.end(); ++symbolIt )
  {
    double mmsf = mmScaleFactor( mSymbologyScale, ( *symbolIt )->outputUnit(), mapUnits );
    double musf = mapUnitScaleFactor( mSymbologyScale, ( *symbolIt )->outputUnit(), mapUnits );

    int nLevels = ( *symbolIt )->symbolLayerCount();
    for ( int i = 0; i < nLevels; ++i )
    {
      mSymbolLayerTable.insert( ( *symbolIt )->symbolLayer( i ), QString::number( nTotalLevels ) );
      OGR_SM_AddStyle( styleManager, QString::number( nTotalLevels ).toLocal8Bit(),
                       ( *symbolIt )->symbolLayer( i )->ogrFeatureStyle( mmsf, musf ).toLocal8Bit() );
      ++nTotalLevels;
    }
  }
  OGR_DS_SetStyleTableDirectly( ds, ogrStyleTable );
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::exportFeaturesSymbolLevels( const PreparedWriterDetails &details, QgsFeatureIterator &fit,
    const QgsCoordinateTransform &ct, QString *errorMessage )
{
  if ( !details.renderer )
    return ErrInvalidLayer;

  mRenderContext.expressionContext() = details.expressionContext;

  QHash< QgsSymbol *, QList<QgsFeature> > features;

  //unit type
  QgsUnitTypes::DistanceUnit mapUnits = details.sourceCrs.mapUnits();
  if ( ct.isValid() )
  {
    mapUnits = ct.destinationCrs().mapUnits();
  }

  startRender( details.renderer.get(), details.sourceFields );

  //fetch features
  QgsFeature fet;
  QgsSymbol *featureSymbol = nullptr;
  while ( fit.nextFeature( fet ) )
  {
    if ( ct.isValid() )
    {
      try
      {
        if ( fet.hasGeometry() )
        {
          QgsGeometry g = fet.geometry();
          g.transform( ct );
          fet.setGeometry( g );
        }
      }
      catch ( QgsCsException &e )
      {
        QString msg = QObject::tr( "Failed to transform, writing stopped. (Exception: %1)" )
                      .arg( e.what() );
        QgsLogger::warning( msg );
        if ( errorMessage )
          *errorMessage = msg;

        return ErrProjection;
      }
    }
    mRenderContext.expressionContext().setFeature( fet );

    featureSymbol = mRenderer->symbolForFeature( fet, mRenderContext );
    if ( !featureSymbol )
    {
      continue;
    }

    QHash< QgsSymbol *, QList<QgsFeature> >::iterator it = features.find( featureSymbol );
    if ( it == features.end() )
    {
      it = features.insert( featureSymbol, QList<QgsFeature>() );
    }
    it.value().append( fet );
  }

  //find out order
  QgsSymbolLevelOrder levels;
  QgsSymbolList symbols = mRenderer->symbols( mRenderContext );
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbol *sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolLevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolLevel() );
      levels[level].append( item );
    }
  }

  int nErrors = 0;
  int nTotalFeatures = 0;

  //export symbol layers and symbology
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolLevel &level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolLevelItem &item = level[i];
      QHash< QgsSymbol *, QList<QgsFeature> >::iterator levelIt = features.find( item.symbol() );
      if ( levelIt == features.end() )
      {
        ++nErrors;
        continue;
      }

      double mmsf = mmScaleFactor( mSymbologyScale, levelIt.key()->outputUnit(), mapUnits );
      double musf = mapUnitScaleFactor( mSymbologyScale, levelIt.key()->outputUnit(), mapUnits );

      int llayer = item.layer();
      QList<QgsFeature> &featureList = levelIt.value();
      QList<QgsFeature>::iterator featureIt = featureList.begin();
      for ( ; featureIt != featureList.end(); ++featureIt )
      {
        ++nTotalFeatures;
        gdal::ogr_feature_unique_ptr ogrFeature = createFeature( *featureIt );
        if ( !ogrFeature )
        {
          ++nErrors;
          continue;
        }

        QString styleString = levelIt.key()->symbolLayer( llayer )->ogrFeatureStyle( mmsf, musf );
        if ( !styleString.isEmpty() )
        {
          OGR_F_SetStyleString( ogrFeature.get(), styleString.toLocal8Bit().constData() );
          if ( !writeFeature( mLayer, ogrFeature.get() ) )
          {
            ++nErrors;
          }
        }
      }
    }
  }

  stopRender();

  if ( nErrors > 0 && errorMessage )
  {
    *errorMessage += QObject::tr( "\nOnly %1 of %2 features written." ).arg( nTotalFeatures - nErrors ).arg( nTotalFeatures );
  }

  return ( nErrors > 0 ) ? QgsVectorFileWriter::ErrFeatureWriteFailed : QgsVectorFileWriter::NoError;
}

double QgsVectorFileWriter::mmScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits )
{
  if ( symbolUnits == QgsUnitTypes::RenderMillimeters )
  {
    return 1.0;
  }
  else
  {
    //conversion factor map units -> mm
    if ( mapUnits == QgsUnitTypes::DistanceMeters )
    {
      return 1000 / scale;
    }

  }
  return 1.0; //todo: map units
}

double QgsVectorFileWriter::mapUnitScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits )
{
  if ( symbolUnits == QgsUnitTypes::RenderMapUnits )
  {
    return 1.0;
  }
  else
  {
    if ( symbolUnits == QgsUnitTypes::RenderMillimeters && mapUnits == QgsUnitTypes::DistanceMeters )
    {
      return scale / 1000;
    }
  }
  return 1.0;
}

void QgsVectorFileWriter::startRender( QgsFeatureRenderer *sourceRenderer, const QgsFields &fields )
{
  mRenderer = createSymbologyRenderer( sourceRenderer );
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->startRender( mRenderContext,  fields );
}

void QgsVectorFileWriter::stopRender()
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->stopRender( mRenderContext );
}

std::unique_ptr<QgsFeatureRenderer> QgsVectorFileWriter::createSymbologyRenderer( QgsFeatureRenderer *sourceRenderer ) const
{
  if ( mSymbologyExport == NoSymbology )
  {
    return nullptr;
  }
  if ( !sourceRenderer )
  {
    return nullptr;
  }

  return std::unique_ptr< QgsFeatureRenderer >( sourceRenderer->clone() );
}

void QgsVectorFileWriter::addRendererAttributes( QgsFeatureRenderer *renderer, QgsRenderContext &context, const QgsFields &fields, QgsAttributeList &attList )
{
  if ( renderer )
  {
    const QSet<QString> rendererAttributes = renderer->usedAttributes( context );
    for ( const QString &attr : rendererAttributes )
    {
      int index = fields.lookupField( attr );
      if ( index != -1 )
      {
        attList.append( index );
      }
    }
  }
}

QStringList QgsVectorFileWriter::concatenateOptions( const QMap<QString, QgsVectorFileWriter::Option *> &options )
{
  QStringList list;
  QMap<QString, QgsVectorFileWriter::Option *>::ConstIterator it;

  for ( it = options.constBegin(); it != options.constEnd(); ++it )
  {
    QgsVectorFileWriter::Option *option = it.value();
    switch ( option->type )
    {
      case QgsVectorFileWriter::Int:
      {
        QgsVectorFileWriter::IntOption *opt = dynamic_cast<QgsVectorFileWriter::IntOption *>( option );
        if ( opt )
        {
          list.append( QStringLiteral( "%1=%2" ).arg( it.key() ).arg( opt->defaultValue ) );
        }
        break;
      }

      case QgsVectorFileWriter::Set:
      {
        QgsVectorFileWriter::SetOption *opt = dynamic_cast<QgsVectorFileWriter::SetOption *>( option );
        if ( opt && !opt->defaultValue.isEmpty() )
        {
          list.append( QStringLiteral( "%1=%2" ).arg( it.key(), opt->defaultValue ) );
        }
        break;
      }

      case QgsVectorFileWriter::String:
      {
        QgsVectorFileWriter::StringOption *opt = dynamic_cast<QgsVectorFileWriter::StringOption *>( option );
        if ( opt && !opt->defaultValue.isNull() )
        {
          list.append( QStringLiteral( "%1=%2" ).arg( it.key(), opt->defaultValue ) );
        }
        break;
      }

      case QgsVectorFileWriter::Hidden:
        QgsVectorFileWriter::HiddenOption *opt = dynamic_cast<QgsVectorFileWriter::HiddenOption *>( option );
        if ( opt )
        {
          list.append( QStringLiteral( "%1=%2" ).arg( it.key(), opt->mValue ) );
        }
        break;
    }
  }

  return list;
}

QgsVectorFileWriter::EditionCapabilities QgsVectorFileWriter::editionCapabilities( const QString &datasetName )
{
  OGRSFDriverH hDriver = nullptr;
  gdal::ogr_datasource_unique_ptr hDS( OGROpen( datasetName.toUtf8().constData(), TRUE, &hDriver ) );
  if ( !hDS )
    return QgsVectorFileWriter::EditionCapabilities();
  QString drvName = OGR_Dr_GetName( hDriver );
  QgsVectorFileWriter::EditionCapabilities caps = QgsVectorFileWriter::EditionCapabilities();
  if ( OGR_DS_TestCapability( hDS.get(), ODsCCreateLayer ) )
  {
    // Shapefile driver returns True for a "foo.shp" dataset name,
    // creating "bar.shp" new layer, but this would be a bit confusing
    // for the user, so pretent that it does not support that
    if ( !( drvName == QLatin1String( "ESRI Shapefile" ) && QFile::exists( datasetName ) ) )
      caps |= CanAddNewLayer;
  }
  if ( OGR_DS_TestCapability( hDS.get(), ODsCDeleteLayer ) )
  {
    caps |= CanDeleteLayer;
  }
  int layer_count = OGR_DS_GetLayerCount( hDS.get() );
  if ( layer_count )
  {
    OGRLayerH hLayer = OGR_DS_GetLayer( hDS.get(), 0 );
    if ( hLayer )
    {
      if ( OGR_L_TestCapability( hLayer, OLCSequentialWrite ) )
      {
        caps |= CanAppendToExistingLayer;
        if ( OGR_L_TestCapability( hLayer, OLCCreateField ) )
        {
          caps |= CanAddNewFieldsToExistingLayer;
        }
      }
    }
  }
  return caps;
}

bool QgsVectorFileWriter::targetLayerExists( const QString &datasetName,
    const QString &layerNameIn )
{
  OGRSFDriverH hDriver = nullptr;
  gdal::ogr_datasource_unique_ptr hDS( OGROpen( datasetName.toUtf8().constData(), TRUE, &hDriver ) );
  if ( !hDS )
    return false;

  QString layerName( layerNameIn );
  if ( layerName.isEmpty() )
    layerName = QFileInfo( datasetName ).baseName();

  return OGR_DS_GetLayerByName( hDS.get(), layerName.toUtf8().constData() );
}


bool QgsVectorFileWriter::areThereNewFieldsToCreate( const QString &datasetName,
    const QString &layerName,
    QgsVectorLayer *layer,
    const QgsAttributeList &attributes )
{
  OGRSFDriverH hDriver = nullptr;
  gdal::ogr_datasource_unique_ptr hDS( OGROpen( datasetName.toUtf8().constData(), TRUE, &hDriver ) );
  if ( !hDS )
    return false;
  OGRLayerH hLayer = OGR_DS_GetLayerByName( hDS.get(), layerName.toUtf8().constData() );
  if ( !hLayer )
  {
    return false;
  }
  bool ret = false;
  OGRFeatureDefnH defn = OGR_L_GetLayerDefn( hLayer );
  const auto constAttributes = attributes;
  for ( int idx : constAttributes )
  {
    QgsField fld = layer->fields().at( idx );
    if ( OGR_FD_GetFieldIndex( defn, fld.name().toUtf8().constData() ) < 0 )
    {
      ret = true;
      break;
    }
  }
  return ret;
}
