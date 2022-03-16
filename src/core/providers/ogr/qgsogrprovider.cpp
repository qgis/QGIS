/***************************************************************************
           qgsogrprovider.cpp Data provider for OGR supported formats
                    Formerly known as qgsshapefileprovider.cpp
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrprovider.h"
///@cond PRIVATE

#include "qgscplerrorhandler_p.h"
#include "qgslogger.h"
#include "qgsvectorfilewriter.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsogrconnpool.h"
#include "qgsogrtransaction.h"
#include "qgsogrfeatureiterator.h"
#include "qgsgdalutils.h"
#include "qgsfeedback.h"
#include "qgscplhttpfetchoverrider.h"
#include "qgsmetadatautils.h"
#include "qgslocalec.h"
#include "qgssymbol.h"
#include "qgsembeddedsymbolrenderer.h"
#include "qgszipitem.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsvariantutils.h"

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include <gdal.h>         // to collect version information
#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_string.h>

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3, 2, 1)
// Temporary solution for gdal < 3.2.1 without GDAL Unique support
#include "qgssqliteutils.h"
#include <sqlite3.h>
// end temporary
#endif

#include <limits>
#include <memory>

#include <QTextCodec>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#define TEXT_PROVIDER_KEY QStringLiteral( "ogr" )
#define TEXT_PROVIDER_DESCRIPTION QStringLiteral( "OGR data provider" )

bool QgsOgrProvider::convertField( QgsField &field, const QTextCodec &encoding )
{
  OGRFieldType ogrType = OFTString; //default to string
  OGRFieldSubType ogrSubType = OFSTNone;
  int ogrWidth = field.length();
  int ogrPrecision = field.precision();
  if ( ogrPrecision > 0 )
    ogrWidth += 1;
  switch ( field.type() )
  {
    case QVariant::LongLong:
      ogrType = OFTInteger64;
      ogrPrecision = 0;
      ogrWidth = ogrWidth > 0 && ogrWidth <= 21 ? ogrWidth : 21;
      break;

    case QVariant::String:
      ogrType = OFTString;
      if ( ogrWidth < 0 || ogrWidth > 255 )
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
      ogrType = OFTReal;
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

    case QVariant::StringList:
    {
      ogrType = OFTStringList;
      break;
    }

    case QVariant::List:
      if ( field.subType() == QVariant::String )
      {
        ogrType = OFTStringList;
      }
      else if ( field.subType() == QVariant::Int )
      {
        ogrType = OFTIntegerList;
      }
      else if ( field.subType() == QVariant::LongLong )
      {
        ogrType = OFTInteger64List;
      }
      else if ( field.subType() == QVariant::Double )
      {
        ogrType = OFTRealList;
      }
      else
      {
        // other lists are supported at this moment
        return false;
      }
      break;

    case QVariant::Map:
      ogrType = OFTString;
      ogrSubType = OFSTJSON;
      break;

    default:
      return false;
  }

  if ( ogrSubType != OFSTNone )
    field.setTypeName( encoding.toUnicode( OGR_GetFieldSubTypeName( ogrSubType ) ) );
  else
    field.setTypeName( encoding.toUnicode( OGR_GetFieldTypeName( ogrType ) ) );

  field.setLength( ogrWidth );
  field.setPrecision( ogrPrecision );
  return true;
}

void QgsOgrProvider::repack()
{
  if ( !mValid || mGDALDriverName != QLatin1String( "ESRI Shapefile" ) || !mOgrOrigLayer )
    return;

  // run REPACK on shape files
  QByteArray sql = QByteArray( "REPACK " ) + mOgrOrigLayer->name();   // don't quote the layer name as it works with spaces in the name and won't work if the name is quoted
  QgsDebugMsgLevel( QStringLiteral( "SQL: %1" ).arg( QString::fromUtf8( sql ) ), 2 );
  CPLErrorReset();
  mOgrOrigLayer->ExecuteSQLNoReturn( sql );
  if ( CPLGetLastErrorType() != CE_None )
  {
    pushError( tr( "OGR[%1] error %2: %3" ).arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );
  }

  if ( mFilePath.endsWith( QLatin1String( ".shp" ), Qt::CaseInsensitive ) || mFilePath.endsWith( QLatin1String( ".dbf" ), Qt::CaseInsensitive ) )
  {
    QString packedDbf( mFilePath.left( mFilePath.size() - 4 ) + "_packed.dbf" );
    if ( QFile::exists( packedDbf ) )
    {
      QgsMessageLog::logMessage( tr( "Possible corruption after REPACK detected. %1 still exists. This may point to a permission or locking problem of the original DBF." ).arg( packedDbf ), tr( "OGR" ), Qgis::MessageLevel::Critical );

      mOgrSqlLayer.reset();
      mOgrOrigLayer.reset();

      QString errCause;
      if ( mLayerName.isNull() )
      {
        mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, true, mOpenOptions, mLayerIndex, errCause, true );
      }
      else
      {
        mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, true, mOpenOptions, mLayerName, errCause, true );
      }

      if ( !mOgrOrigLayer )
      {
        QgsMessageLog::logMessage( tr( "Original layer could not be reopened." ) + " " + errCause, tr( "OGR" ), Qgis::MessageLevel::Critical );
        mValid = false;
      }

      mOgrLayer = mOgrOrigLayer.get();
    }

  }

  if ( mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::Uncounted ) &&
       mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::UnknownCount ) )
  {
    long long oldcount = mFeaturesCounted;
    recalculateFeatureCount();
    if ( oldcount != mFeaturesCounted )
      emit dataChanged();
  }
}

Qgis::VectorExportResult QgsOgrProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  QString encoding;
  QString driverName = QStringLiteral( "GPKG" );
  QStringList dsOptions, layerOptions;
  QString layerName;

  if ( options )
  {
    if ( options->contains( QStringLiteral( "fileEncoding" ) ) )
      encoding = options->value( QStringLiteral( "fileEncoding" ) ).toString();

    if ( options->contains( QStringLiteral( "driverName" ) ) )
      driverName = options->value( QStringLiteral( "driverName" ) ).toString();

    if ( options->contains( QStringLiteral( "datasourceOptions" ) ) )
      dsOptions << options->value( QStringLiteral( "datasourceOptions" ) ).toStringList();

    if ( options->contains( QStringLiteral( "layerOptions" ) ) )
      layerOptions << options->value( QStringLiteral( "layerOptions" ) ).toStringList();

    if ( options->contains( QStringLiteral( "layerName" ) ) )
      layerName = options->value( QStringLiteral( "layerName" ) ).toString();
  }

  oldToNewAttrIdxMap->clear();
  if ( errorMessage )
    errorMessage->clear();

  QgsVectorFileWriter::ActionOnExistingFile action( QgsVectorFileWriter::CreateOrOverwriteFile );

  bool update = false;
  if ( options && options->contains( QStringLiteral( "update" ) ) )
  {
    update = options->value( QStringLiteral( "update" ) ).toBool();
    if ( update )
    {
      if ( !overwrite && !layerName.isEmpty() )
      {
        gdal::dataset_unique_ptr hDS( GDALOpenEx( uri.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
        if ( hDS )
        {
          if ( GDALDatasetGetLayerByName( hDS.get(), layerName.toUtf8().constData() ) )
          {
            if ( errorMessage )
              *errorMessage += QObject::tr( "Layer %2 of %1 exists and overwrite flag is false." )
                               .arg( uri, layerName );
            return Qgis::VectorExportResult::ErrorCreatingDataSource;
          }
        }
      }
      if ( QFileInfo::exists( uri ) )
        action = QgsVectorFileWriter::CreateOrOverwriteLayer;
    }
  }

  if ( !overwrite && !update )
  {
    if ( QFileInfo::exists( uri ) )
    {
      if ( errorMessage )
        *errorMessage += QObject::tr( "Unable to create the datasource. %1 exists and overwrite flag is false." )
                         .arg( uri );
      return Qgis::VectorExportResult::ErrorCreatingDataSource;
    }
  }

  QString newLayerName( layerName );

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.layerName = layerName;
  saveOptions.fileEncoding = encoding;
  saveOptions.driverName = driverName;
  saveOptions.datasourceOptions = dsOptions;
  saveOptions.layerOptions = layerOptions;
  saveOptions.actionOnExistingFile = action;
  saveOptions.symbologyExport = QgsVectorFileWriter::NoSymbology;
  std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( uri, fields, wkbType, srs, QgsCoordinateTransformContext(), saveOptions, QgsFeatureSink::SinkFlags(), nullptr, &newLayerName ) );
  layerName = newLayerName;

  QgsVectorFileWriter::WriterError error = writer->hasError();
  if ( error )
  {
    if ( errorMessage )
      *errorMessage += writer->errorMessage();

    return static_cast<Qgis::VectorExportResult>( error );
  }

  QMap<int, int> attrIdxMap = writer->attrIdxToOgrIdx();
  writer.reset();

  {
    bool firstFieldIsFid = false;
    bool fidColumnIsField = false;
    if ( !layerName.isEmpty() )
    {
      gdal::dataset_unique_ptr hDS( GDALOpenEx( uri.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
      if ( hDS )
      {
        OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS.get(), layerName.toUtf8().constData() );
        if ( hLayer )
        {
          // Expose the OGR FID if it comes from a "real" column (typically GPKG)
          // and make sure that this FID column is not exposed as a regular OGR field (shouldn't happen normally)
          const QString ogrFidColumnName { OGR_L_GetFIDColumn( hLayer ) };
          firstFieldIsFid = !( EQUAL( OGR_L_GetFIDColumn( hLayer ), "" ) ) &&
                            OGR_FD_GetFieldIndex( OGR_L_GetLayerDefn( hLayer ), ogrFidColumnName.toUtf8() ) < 0 &&
                            fields.indexFromName( ogrFidColumnName.toUtf8() ) < 0;
          // At this point we must check if there is a real FID field in the the fields argument,
          // because in that case we don't want to shift all fields (see issue GH #34333)
          // Check for unique values should be performed in client code.
          for ( const auto &f : std::as_const( fields ) )
          {
            if ( f.name().compare( ogrFidColumnName, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
            {
              fidColumnIsField = true;
              break;
            }
          }
        }
      }
    }

    const bool shiftColumnsByOne { firstFieldIsFid &&( ! fidColumnIsField ) };

    for ( QMap<int, int>::const_iterator attrIt = attrIdxMap.constBegin(); attrIt != attrIdxMap.constEnd(); ++attrIt )
    {
      oldToNewAttrIdxMap->insert( attrIt.key(), *attrIt + ( shiftColumnsByOne ? 1 : 0 ) );
    }
  }

  QgsOgrProviderUtils::invalidateCachedLastModifiedDate( uri );

  return Qgis::VectorExportResult::Success;
}

QgsOgrProvider::QgsOgrProvider( QString const &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  QgsApplication::registerOgrDrivers();

  QgsSettings settings;
  // we always disable GDAL side shapefile encoding handling, and do it on the QGIS side.
  // why? it's not the ideal choice, but...
  // - if we DON'T disable GDAL side encoding support, then there's NO way to change the encoding used when reading
  //   shapefiles. And unfortunately the embedded encoding (which is read by GDAL) is sometimes wrong, so we need
  //   to expose support for users to be able to change and correct this
  // - we can't change this setting on-the-fly. If we don't set it upfront, we can't reverse this decision later when
  //   a user does want/need to manually specify the encoding
  CPLSetConfigOption( "SHAPE_ENCODING", "" );

#ifndef QT_NO_NETWORKPROXY
  QgsGdalUtils::setupProxy();
#endif

  // make connection to the data source

  QgsDebugMsgLevel( "Data source uri is [" + uri + ']', 2 );

  mFilePath = QgsOgrProviderUtils::analyzeURI( uri,
              mIsSubLayer,
              mLayerIndex,
              mLayerName,
              mSubsetString,
              mOgrGeometryTypeFilter,
              mOpenOptions );

  // to be called only after mFilePath has been set
  invalidateNetworkCache();

  if ( uri.contains( QLatin1String( "authcfg" ) ) )
  {
    QRegularExpression authcfgRe( QStringLiteral( " authcfg='([^']+)'" ) );
    QRegularExpressionMatch match;
    if ( uri.contains( authcfgRe, &match ) )
    {
      mAuthCfg = match.captured( 1 );
      // momentarily re-add authcfg since it was stripped off in analyzeURI
      mFilePath = QgsOgrProviderUtils::expandAuthConfig( QStringLiteral( "%1 authcfg='%2'" ).arg( mFilePath, mAuthCfg ) );
    }
  }
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  open( OpenModeInitial );

  int nMaxIntLen = 11;
  int nMaxInt64Len = 21;
  int nMaxDoubleLen = 20;
  int nMaxDoublePrec = 15;
  int nDateLen = 8;
  if ( mGDALDriverName == QLatin1String( "GPKG" ) )
  {
    // GPKG only supports field length for text (and binary)
    nMaxIntLen = 0;
    nMaxInt64Len = 0;
    nMaxDoubleLen = 0;
    nMaxDoublePrec = 0;
    nDateLen = 0;
  }

  QList<NativeType> nativeTypes;
  nativeTypes
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Int ), QStringLiteral( "integer" ), QVariant::Int, 0, nMaxIntLen )
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::LongLong ), QStringLiteral( "integer64" ), QVariant::LongLong, 0, nMaxInt64Len )
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "double" ), QVariant::Double, 0, nMaxDoubleLen, 0, nMaxDoublePrec )
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::String ), QStringLiteral( "string" ), QVariant::String, 0, 65535 );

  if ( mGDALDriverName == QLatin1String( "GPKG" ) )
    nativeTypes << QgsVectorDataProvider::NativeType( tr( "JSON (string)" ), QStringLiteral( "JSON" ), QVariant::Map, 0, 0, 0, 0, QVariant::String );

  bool supportsDate = true;
  bool supportsTime = mGDALDriverName != QLatin1String( "ESRI Shapefile" ) && mGDALDriverName != QLatin1String( "GPKG" );
  bool supportsDateTime = mGDALDriverName != QLatin1String( "ESRI Shapefile" );
  bool supportsBinary = false;
  bool supportIntegerList = false;
  bool supportInteger64List = false;
  bool supportRealList = false;
  bool supportsStringList = false;
  const char *pszDataTypes = nullptr;
  if ( mOgrOrigLayer )
  {
    pszDataTypes = GDALGetMetadataItem( mOgrOrigLayer->driver(), GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
  }
  // For drivers that advertise their data type, use that instead of the
  // above hardcoded defaults.
  if ( pszDataTypes )
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
  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    supportsDateTime = false;
  }
#endif

  if ( supportsDate )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Date ), QStringLiteral( "date" ), QVariant::Date, nDateLen, nDateLen );
  }
  if ( supportsTime )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Time ), QStringLiteral( "time" ), QVariant::Time );
  }
  if ( supportsDateTime )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), QStringLiteral( "datetime" ), QVariant::DateTime );
  }
  if ( supportsBinary )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::ByteArray ), QStringLiteral( "binary" ), QVariant::ByteArray );
  }
  if ( supportIntegerList )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Int ), QStringLiteral( "integerlist" ), QVariant::List, 0, 0, 0, 0, QVariant::Int );
  }
  if ( supportInteger64List )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::LongLong ), QStringLiteral( "integer64list" ), QVariant::List, 0, 0, 0, 0, QVariant::LongLong );
  }
  if ( supportRealList )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Double ), QStringLiteral( "doublelist" ), QVariant::List, 0, 0, 0, 0, QVariant::Double );
  }
  if ( supportsStringList )
  {
    nativeTypes
        << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::StringList ), QStringLiteral( "stringlist" ), QVariant::List, 0, 0, 0, 0, QVariant::String );
  }

  bool supportsBoolean = false;

  // layer metadata
  loadMetadata();

  if ( mOgrOrigLayer )
  {
    const char *pszDataSubTypes = GDALGetMetadataItem( mOgrOrigLayer->driver(), GDAL_DMD_CREATIONFIELDDATASUBTYPES, nullptr );
    if ( pszDataSubTypes && strstr( pszDataSubTypes, "Boolean" ) )
      supportsBoolean = true;
  }

  if ( supportsBoolean )
  {
    // boolean data type
    nativeTypes
        << QgsVectorDataProvider::NativeType( tr( "Boolean" ), QStringLiteral( "bool" ), QVariant::Bool );
  }

  setNativeTypes( nativeTypes );

  QgsOgrConnPool::instance()->ref( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
}

QgsOgrProvider::~QgsOgrProvider()
{
  QgsOgrConnPool::instance()->unref( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  // We must also make sure to flush unusef cached connections so that
  // the file can be removed (#15137)
  QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );

  // Do that as last step for final cleanup that might be prevented by
  // still opened datasets.
  close();
}

QString QgsOgrProvider::dataSourceUri( bool expandAuthConfig ) const
{
  if ( expandAuthConfig && QgsDataProvider::dataSourceUri( ).contains( QLatin1String( "authcfg" ) ) )
  {
    return QgsOgrProviderUtils::expandAuthConfig( QgsDataProvider::dataSourceUri( ) );
  }
  else
  {
    return QgsDataProvider::dataSourceUri( );
  }
}

QgsTransaction *QgsOgrProvider::transaction() const
{
  return static_cast<QgsTransaction *>( mTransaction );
}

void QgsOgrProvider::setTransaction( QgsTransaction *transaction )
{
  QgsDebugMsgLevel( QStringLiteral( "set transaction %1" ).arg( transaction != nullptr ), 2 );
  // static_cast since layers cannot be added to a transaction of a non-matching provider
  mTransaction = static_cast<QgsOgrTransaction *>( transaction );
}

QgsAbstractFeatureSource *QgsOgrProvider::featureSource() const
{
  return new QgsOgrFeatureSource( this );
}

bool QgsOgrProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  return _setSubsetString( theSQL, updateFeatureCount, true );
}

QString QgsOgrProvider::subsetString() const
{
  return mSubsetString;
}

uint QgsOgrProvider::subLayerCount() const
{
  uint count = layerCount();

  QString errCause;
  QgsOgrLayerUniquePtr layerStyles = QgsOgrProviderUtils::getLayer( mFilePath, QStringLiteral( "layer_styles" ), errCause );
  if ( layerStyles )
  {
    count--;
  }
  return count;
}

QStringList subLayerDetailsToStringList( const QList< QgsProviderSublayerDetails > &layers )
{
  QStringList res;
  res.reserve( layers.size() );

  for ( const QgsProviderSublayerDetails &layer : layers )
  {
    const OGRwkbGeometryType ogrGeomType = QgsOgrProviderUtils::ogrTypeFromQgisType( layer.wkbType() );

    const QStringList parts { QString::number( layer.layerNumber() ),
                              layer.name(),
                              QString::number( layer.featureCount() ),
                              QgsOgrProviderUtils::ogrWkbGeometryTypeName( ogrGeomType ),
                              layer.geometryColumnName(),
                              layer.description() };
    res << parts.join( QgsDataProvider::sublayerSeparator() );
  }
  return res;
}

QStringList QgsOgrProvider::subLayers() const
{
  const bool withFeatureCount = ( mReadFlags & QgsDataProvider::SkipFeatureCount ) == 0;

  Qgis::SublayerQueryFlags flags = withFeatureCount
                                   ? ( Qgis::SublayerQueryFlag::CountFeatures | Qgis::SublayerQueryFlag::ResolveGeometryType )
                                   : Qgis::SublayerQueryFlag::ResolveGeometryType;
  if ( mIsSubLayer )
    flags |= Qgis::SublayerQueryFlag::IncludeSystemTables;

  return subLayerDetailsToStringList( _subLayers( flags ) );
}

QgsLayerMetadata QgsOgrProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QList<QgsProviderSublayerDetails> QgsOgrProvider::_subLayers( Qgis::SublayerQueryFlags flags ) const
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !mValid )
  {
    return {};
  }

  if ( !mSubLayerList.isEmpty() )
    return mSubLayerList;

  const size_t totalLayerCount = layerCount();
  if ( mOgrLayer && ( mIsSubLayer || totalLayerCount == 1 ) )
  {
    mSubLayerList << QgsOgrProviderUtils::querySubLayerList( mLayerIndex, mOgrLayer, nullptr, mGDALDriverName, flags, dataSourceUri(), totalLayerCount == 1 );
  }
  else
  {
    // In case there is no free opened dataset in the cache, keep the first
    // layer alive while we iterate over the other layers, so that we can
    // reuse the same dataset. Can help in a particular with a FileGDB with
    // the FileGDB driver
    QgsOgrLayerUniquePtr firstLayer;
    for ( size_t i = 0; i < totalLayerCount ; i++ )
    {
      QString errCause;
      QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( mOgrOrigLayer->datasetName(),
                                   mOgrOrigLayer->updateMode(),
                                   mOgrOrigLayer->options(),
                                   i,
                                   errCause,
                                   // do not check timestamp beyond the first
                                   // layer
                                   firstLayer == nullptr );
      if ( !layer )
        continue;

      mSubLayerList << QgsOgrProviderUtils::querySubLayerList( i, layer.get(), nullptr, mGDALDriverName, flags, dataSourceUri(), totalLayerCount == 1 );
      if ( firstLayer == nullptr )
      {
        firstLayer = std::move( layer );
      }
    }
  }

  return mSubLayerList;
}

void QgsOgrProvider::setEncoding( const QString &e )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  QgsSettings settings;

  // if the layer has the OLCStringsAsUTF8 capability, we CANNOT override the
  // encoding on the QGIS side!
  if ( mOgrLayer && !mOgrLayer->TestCapability( OLCStringsAsUTF8 ) )
  {
    QgsVectorDataProvider::setEncoding( e );
  }
  else
  {
    QgsVectorDataProvider::setEncoding( QStringLiteral( "UTF-8" ) );
  }
  loadFields();
}

// This is reused by dataItem
OGRwkbGeometryType QgsOgrProvider::getOgrGeomType( const QString &driverName, OGRLayerH ogrLayer )
{
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  OGRwkbGeometryType geomType = wkbUnknown;
  if ( fdef )
  {
    geomType = OGR_FD_GetGeomType( fdef );

    // Handle wkbUnknown and its Z/M variants. QGIS has no unknown Z/M variants,
    // so just use flat wkbUnknown
    if ( wkbFlatten( geomType ) == wkbUnknown )
      geomType = wkbUnknown;

    // Some ogr drivers (e.g. GML) are not able to determine the geometry type of a layer like this.
    // In such cases, we use virtual sublayers for each geometry if the layer contains
    // multiple geometries (see subLayers) otherwise we guess geometry type from the first
    // feature that has a geometry (limit us to a few features, not the whole layer)
    //
    // For ESRI formats with a GeometryCollection25D type we also query features for the geometry type,
    // as they may be ESRI MultiPatch files which we want to report as MultiPolygon25D types
    if ( geomType == wkbUnknown
         || ( geomType == wkbGeometryCollection25D && ( driverName == QLatin1String( "ESRI Shapefile" ) || driverName == QLatin1String( "OpenFileGDB" ) || driverName == QLatin1String( "FileGDB" ) ) ) )
    {
      geomType = wkbNone;
      OGR_L_ResetReading( ogrLayer );
      for ( int i = 0; i < 10; i++ )
      {
        gdal::ogr_feature_unique_ptr nextFeature( OGR_L_GetNextFeature( ogrLayer ) );
        if ( !nextFeature )
          break;

        geomType = QgsOgrProviderUtils::resolveGeometryTypeForFeature( nextFeature.get(), driverName );
        if ( geomType != wkbNone )
          break;
      }
      OGR_L_ResetReading( ogrLayer );
    }
  }
  return geomType;
}

void QgsOgrProvider::loadFields()
{
  QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  //the attribute fields need to be read again when the encoding changes
  mAttributeFields.clear();
  mDefaultValues.clear();
  mPrimaryKeyAttrs.clear();
  if ( !mOgrLayer )
    return;

  if ( mOgrGeometryTypeFilter != wkbUnknown )
  {
    mOGRGeomType = mOgrGeometryTypeFilter;
  }
  else
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mutex = nullptr;
#else
    QRecursiveMutex *mutex = nullptr;
#endif
    OGRLayerH ogrLayer = mOgrLayer->getHandleAndMutex( mutex );
    QMutexLocker locker( mutex );
    mOGRGeomType = getOgrGeomType( mGDALDriverName, ogrLayer );
  }
  QgsOgrFeatureDefn &fdef = mOgrLayer->GetLayerDefn();

  // Expose the OGR FID if it comes from a "real" column (typically GPKG)
  // and make sure that this FID column is not exposed as a regular OGR field (shouldn't happen normally)
  QByteArray fidColumn( mOgrLayer->GetFIDColumn() );
  mFirstFieldIsFid = !fidColumn.isEmpty() &&
                     fdef.GetFieldIndex( fidColumn ) < 0;

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3, 2, 1)
  // This is a temporary solution until GDAL Unique support is available
  QSet<QString> uniqueFieldNames;


  if ( mGDALDriverName == QLatin1String( "GPKG" ) )
  {
    sqlite3_database_unique_ptr dsPtr;
    if ( dsPtr.open_v2( mFilePath, SQLITE_OPEN_READONLY, nullptr ) == SQLITE_OK )
    {
      QString errMsg;
      uniqueFieldNames = QgsSqliteUtils::uniqueFields( dsPtr.get(), mOgrLayer->name(), errMsg );
      if ( ! errMsg.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "GPKG error searching for unique constraints on fields for table %1. (%2)" ).arg( QString( mOgrLayer->name() ), errMsg ), tr( "OGR" ) );
      }
    }
  }
#endif

  int createdFields = 0;
  if ( mFirstFieldIsFid )
  {
    QgsField fidField(
      fidColumn,
      QVariant::LongLong,
      QStringLiteral( "Integer64" )
    );
    // Set constraints for feature id
    QgsFieldConstraints constraints = fidField.constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    fidField.setConstraints( constraints );
    mAttributeFields.append(
      fidField
    );
    mDefaultValues.insert( 0, tr( "Autogenerate" ) );
    createdFields++;
    mPrimaryKeyAttrs << 0;
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  // needed for field domain retrieval on GDAL 3.3+
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *datasetMutex = nullptr;
#else
  QRecursiveMutex *datasetMutex = nullptr;
#endif
  GDALDatasetH ds = mOgrLayer->getDatasetHandleAndMutex( datasetMutex );
  QMutexLocker locker( datasetMutex );
#endif

  for ( int i = 0; i < fdef.GetFieldCount(); ++i )
  {
    OGRFieldDefnH fldDef = fdef.GetFieldDefn( i );
    const OGRFieldType ogrType = OGR_Fld_GetType( fldDef );
    const OGRFieldSubType ogrSubType = OGR_Fld_GetSubType( fldDef );

    QVariant::Type varType = QVariant::Invalid;
    QVariant::Type varSubType = QVariant::Invalid;
    QgsOgrUtils::ogrFieldTypeToQVariantType( ogrType, ogrSubType, varType, varSubType );

    //TODO: fix this hack
#ifdef ANDROID
    QString name = OGR_Fld_GetNameRef( fldDef );
#else
    QString name = textEncoding()->toUnicode( OGR_Fld_GetNameRef( fldDef ) );
#endif

    if ( mAttributeFields.indexFromName( name ) != -1 )
    {

      QString tmpname = name + "_%1";
      int fix = 0;

      while ( mAttributeFields.indexFromName( name ) != -1 )
      {
        name = tmpname.arg( ++fix );
      }
    }

    int width = OGR_Fld_GetWidth( fldDef );
    int prec = OGR_Fld_GetPrecision( fldDef );
    if ( prec > 0 )
      width -= 1;

    QString typeName = OGR_GetFieldTypeName( ogrType );
    if ( ogrSubType != OFSTNone )
      typeName = OGR_GetFieldSubTypeName( ogrSubType );

    QgsField newField = QgsField(
                          name,
                          varType,
#ifdef ANDROID
                          typeName,
#else
                          textEncoding()->toUnicode( typeName.toStdString().c_str() ),
#endif
                          width, prec, QString(), varSubType
                        );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,2,0)
    const QString alias = textEncoding()->toUnicode( OGR_Fld_GetAlternativeNameRef( fldDef ) );
    if ( !alias.isEmpty() )
    {
      newField.setAlias( alias );
    }
#endif

    // check if field is nullable
    bool nullable = OGR_Fld_IsNullable( fldDef );
    if ( !nullable )
    {
      QgsFieldConstraints constraints;
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      newField.setConstraints( constraints );
    }

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3, 2, 1)
    if ( uniqueFieldNames.contains( OGR_Fld_GetNameRef( fldDef ) ) )
#else
    if ( OGR_Fld_IsUnique( fldDef ) )
#endif
    {
      QgsFieldConstraints constraints = newField.constraints();
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      newField.setConstraints( constraints );
    }

    // check if field has default value
    QString defaultValue = textEncoding()->toUnicode( OGR_Fld_GetDefault( fldDef ) );
    if ( !defaultValue.isEmpty() && !OGR_Fld_IsDefaultDriverSpecific( fldDef ) )
    {
      if ( defaultValue.startsWith( '\'' ) )
      {
        defaultValue = defaultValue.remove( 0, 1 );
        defaultValue.chop( 1 );
        defaultValue.replace( QLatin1String( "''" ), QLatin1String( "'" ) );
      }
      mDefaultValues.insert( createdFields, defaultValue );
    }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
    if ( const char *domainName = OGR_Fld_GetDomainName( fldDef ) )
    {
      QgsFieldConstraints constraints = newField.constraints();
      constraints.setDomainName( domainName );
      newField.setConstraints( constraints );

      // dataset retains ownership of domain!
      if ( OGRFieldDomainH domain = GDALDatasetGetFieldDomain( ds, domainName ) )
      {
        switch ( OGR_FldDomain_GetDomainType( domain ) )
        {
          case OFDT_CODED:
          {
            QVariantList valueConfig;
            const OGRCodedValue *codedValue = OGR_CodedFldDomain_GetEnumeration( domain );
            while ( codedValue && codedValue->pszCode )
            {
              const QString code( codedValue->pszCode );
              // if pszValue is null then it indicates we are working with a set of acceptable values which aren't
              // coded. In this case we copy the code as the value so that QGIS exposes the domain as a choice of
              // the valid code values.
              const QString value( codedValue->pszValue ? codedValue->pszValue : codedValue->pszCode );

              QVariantMap config;
              config[ value ] = code;
              valueConfig.append( config );

              codedValue++;
            }

            QVariantMap editorConfig;
            editorConfig.insert( QStringLiteral( "map" ), valueConfig );
            newField.setEditorWidgetSetup( QgsEditorWidgetSetup( QStringLiteral( "ValueMap" ), editorConfig ) );
            break;
          }

          case OFDT_RANGE:
            if ( newField.isNumeric() )
            {
              // QGIS doesn't support the inclusive option yet!
              bool isInclusive = false;

              QVariantMap editorConfig;
              editorConfig.insert( QStringLiteral( "Step" ), 1 );
              editorConfig.insert( QStringLiteral( "Style" ), QStringLiteral( "SpinBox" ) );
              editorConfig.insert( QStringLiteral( "AllowNull" ), nullable );
              editorConfig.insert( QStringLiteral( "Precision" ), newField.precision() );

              OGRFieldType domainFieldType = OGR_FldDomain_GetFieldType( domain );
              bool hasMinOrMax = false;
              if ( const OGRField *min = OGR_RangeFldDomain_GetMin( domain, &isInclusive ) )
              {
                const QVariant minValue = QgsOgrUtils::OGRFieldtoVariant( min, domainFieldType );
                if ( minValue.isValid() )
                {
                  editorConfig.insert( QStringLiteral( "Min" ),  minValue );
                  hasMinOrMax = true;
                }
              }
              if ( const OGRField *max = OGR_RangeFldDomain_GetMax( domain, &isInclusive ) )
              {
                const QVariant maxValue = QgsOgrUtils::OGRFieldtoVariant( max, domainFieldType );
                if ( maxValue.isValid() )
                {
                  editorConfig.insert( QStringLiteral( "Max" ),  maxValue );
                  hasMinOrMax = true;
                }
              }

              if ( hasMinOrMax )
                newField.setEditorWidgetSetup( QgsEditorWidgetSetup( QStringLiteral( "Range" ), editorConfig ) );
            }
            // GDAL also supports range domains for fields types like date/datetimes, but the QGIS corresponding field
            // config doesn't support this yet!
            break;

          case OFDT_GLOB:
            // not supported by QGIS yet
            break;
        }
      }
    }
#endif

    mAttributeFields.append( newField );
    createdFields++;
  }
}

void QgsOgrProvider::loadMetadata()
{
  if ( mOgrOrigLayer )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mutex = nullptr;
#else
    QRecursiveMutex *mutex = nullptr;
#endif
    OGRLayerH layer = mOgrOrigLayer->getHandleAndMutex( mutex );
    QMutexLocker locker( mutex );

    const QString identifier = GDALGetMetadataItem( layer, "IDENTIFIER", "" );
    if ( !identifier.isEmpty() )
      mLayerMetadata.setTitle( identifier ); // see geopackage specs -- "'identifier' is analogous to 'title'"
    const QString abstract = GDALGetMetadataItem( layer, "DESCRIPTION", "" );
    if ( !abstract.isEmpty() )
      mLayerMetadata.setAbstract( abstract );

    if ( mGDALDriverName == QLatin1String( "GPKG" ) )
    {
      // first check if metadata tables/extension exists
      QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE name='gpkg_metadata' AND type='table'" );
      bool metadataTableExists = false;
      if ( QgsOgrLayerUniquePtr l = mOgrOrigLayer->ExecuteSQL( sql.toLocal8Bit().constData() ) )
      {
        gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
        if ( f )
        {
          metadataTableExists = true;
        }
      }

      if ( metadataTableExists )
      {
        // read Geopackage layer metadata - scan gpkg_metadata table for QGIS metadata
        sql = QStringLiteral( "SELECT metadata from gpkg_metadata LEFT JOIN gpkg_metadata_reference ON "
                              "(gpkg_metadata_reference.table_name = %1 AND gpkg_metadata.id = gpkg_metadata_reference.md_file_id) "
                              "WHERE md_standard_uri = %2 and reference_scope = %3" ).arg(
                QgsSqliteUtils::quotedString( mOgrOrigLayer->name() ),
                QgsSqliteUtils::quotedString( QStringLiteral( "http://mrcc.com/qgis.dtd" ) ),
                QgsSqliteUtils::quotedString( QStringLiteral( "table" ) ) );

        if ( QgsOgrLayerUniquePtr l = mOgrOrigLayer->ExecuteSQL( sql.toLocal8Bit().constData() ) )
        {
          gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
          if ( f )
          {
            bool ok = false;
            QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), QgsField( QString(), QVariant::String ), 0, nullptr, &ok );
            if ( ok )
            {
              QDomDocument doc;
              doc.setContent( res.toString() );
              mLayerMetadata.readMetadataXml( doc.documentElement() );
            }
          }
        }
      }
    }
    else if ( mGDALDriverName == QLatin1String( "FileGDB" )
              || mGDALDriverName == QLatin1String( "OpenFileGDB" )
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
              || mGDALDriverName == QLatin1String( "PGeo" ) // supported on GDAL 3.4+ only
#endif
            )
    {
      // read ESRI FileGeodatabase/Personal Geodatabase layer metadata

      // important -- this ONLY works if the layer name is NOT quoted!!
      QByteArray sql = "GetLayerMetadata " + mOgrOrigLayer->name();
      if ( QgsOgrLayerUniquePtr l = mOgrOrigLayer->ExecuteSQL( sql ) )
      {
        gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
        if ( f )
        {
          bool ok = false;
          QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), QgsField( QString(), QVariant::String ), 0, textEncoding(), &ok );
          if ( ok )
          {
            QDomDocument metadataDoc;
            metadataDoc.setContent( res.toString() );
            mLayerMetadata = QgsMetadataUtils::convertFromEsri( metadataDoc );
          }
        }
      }
    }
    else if ( ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) ) )
    {
      // look for .shp.xml sidecar file
      const QString sidecarPath = mFilePath + ".xml";
      if ( QFileInfo::exists( sidecarPath ) )
      {
        QFile file( sidecarPath );
        if ( file.open( QFile::ReadOnly ) )
        {
          QDomDocument doc;
          int line, column;
          QString errorMessage;
          if ( doc.setContent( &file, &errorMessage, &line, &column ) )
          {
            mLayerMetadata = QgsMetadataUtils::convertFromEsri( doc );
          }
          else
          {
            QgsDebugMsg( QStringLiteral( "Error reading %1: %2 at line %3 column %4" ).arg( sidecarPath, errorMessage ).arg( line ).arg( column ) );
          }
          file.close();
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "Error reading %1 - could not open file for read" ).arg( sidecarPath ) );
        }
      }
    }
  }
  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
}

QString QgsOgrProvider::storageType() const
{
  // Delegate to the driver loaded in by OGR
  return mGDALDriverName;
}


void QgsOgrProvider::setRelevantFields( bool fetchGeometry, const QgsAttributeList &fetchAttributes ) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex = nullptr;
#else
  QRecursiveMutex *mutex = nullptr;
#endif
  OGRLayerH ogrLayer = mOgrLayer->getHandleAndMutex( mutex );
  QMutexLocker locker( mutex );
  QgsOgrProviderUtils::setRelevantFields( ogrLayer, mAttributeFields.count(), fetchGeometry, fetchAttributes, mFirstFieldIsFid, mSubsetString );
}

QgsFeatureIterator QgsOgrProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsOgrFeatureIterator( static_cast<QgsOgrFeatureSource *>( featureSource() ), true, request, mTransaction ) );
}

unsigned char *QgsOgrProvider::getGeometryPointer( OGRFeatureH fet )
{
  OGRGeometryH geom = OGR_F_GetGeometryRef( fet );
  unsigned char *gPtr = nullptr;

  if ( !geom )
    return nullptr;

  // get the wkb representation
  gPtr = new unsigned char[OGR_G_WkbSize( geom )];

  OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), gPtr );
  return gPtr;
}


QgsRectangle QgsOgrProvider::extent() const
{
  if ( !mExtent )
  {
    QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
    QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

    mExtent.reset( new OGREnvelope() );

    // get the extent_ (envelope) of the layer
    QgsDebugMsgLevel( QStringLiteral( "Starting get extent" ), 3 );

    if ( mForceRecomputeExtent && mValid && mGDALDriverName == QLatin1String( "GPKG" ) && mOgrOrigLayer )
    {
      // works with unquoted layerName
      QByteArray sql = QByteArray( "RECOMPUTE EXTENT ON " ) + mOgrOrigLayer->name();
      QgsDebugMsgLevel( QStringLiteral( "SQL: %1" ).arg( QString::fromUtf8( sql ) ), 2 );
      mOgrOrigLayer->ExecuteSQLNoReturn( sql );
    }

    mExtent->MinX = std::numeric_limits<double>::max();
    mExtent->MinY = std::numeric_limits<double>::max();
    mExtent->MaxX = -std::numeric_limits<double>::max();
    mExtent->MaxY = -std::numeric_limits<double>::max();

    // TODO: This can be expensive, do we really need it!
    if ( mOgrLayer == mOgrOrigLayer.get() && mSubsetString.isEmpty() )
    {
      if ( ( mGDALDriverName == QLatin1String( "OAPIF" ) || mGDALDriverName == QLatin1String( "WFS3" ) ) &&
           !mOgrLayer->TestCapability( OLCFastGetExtent ) )
      {
        // When the extent is not in the metadata, retrieving it would be
        // super slow
        mExtent->MinX = -180;
        mExtent->MinY = -90;
        mExtent->MaxX = 180;
        mExtent->MaxY = 90;
      }
      else
      {
        mOgrLayer->GetExtent( mExtent.get(), true );
      }
    }
    else
    {
      gdal::ogr_feature_unique_ptr f;

      mOgrLayer->ResetReading();
      while ( f.reset( mOgrLayer->GetNextFeature() ), f )
      {
        OGRGeometryH g = OGR_F_GetGeometryRef( f.get() );
        if ( g && !OGR_G_IsEmpty( g ) )
        {
          OGREnvelope env;
          OGR_G_GetEnvelope( g, &env );

          mExtent->MinX = std::min( mExtent->MinX, env.MinX );
          mExtent->MinY = std::min( mExtent->MinY, env.MinY );
          mExtent->MaxX = std::max( mExtent->MaxX, env.MaxX );
          mExtent->MaxY = std::max( mExtent->MaxY, env.MaxY );
        }
      }
      mOgrLayer->ResetReading();
    }

    QgsDebugMsgLevel( QStringLiteral( "Finished get extent" ), 4 );
  }

  mExtentRect.set( mExtent->MinX, mExtent->MinY, mExtent->MaxX, mExtent->MaxY );
  return mExtentRect;
}

QVariant QgsOgrProvider::defaultValue( int fieldId ) const
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( fieldId < 0 || fieldId >= mAttributeFields.count() )
    return QVariant();

  QString defaultVal = mDefaultValues.value( fieldId, QString() );
  if ( defaultVal.isEmpty() )
    return QVariant();

  QVariant resultVar = defaultVal;
  if ( defaultVal == QLatin1String( "CURRENT_TIMESTAMP" ) )
    resultVar = QDateTime::currentDateTime();
  else if ( defaultVal == QLatin1String( "CURRENT_DATE" ) )
    resultVar = QDate::currentDate();
  else if ( defaultVal == QLatin1String( "CURRENT_TIME" ) )
    resultVar = QTime::currentTime();

  // Get next sequence value for sqlite in case we are inside a transaction
  if ( mOgrOrigLayer &&
       mTransaction &&
       mDefaultValues.value( fieldId, QString() ) == tr( "Autogenerate" ) &&
       providerProperty( EvaluateDefaultValues, false ).toBool() &&
       ( mGDALDriverName == QLatin1String( "GPKG" ) ||
         mGDALDriverName == QLatin1String( "SQLite" ) ) &&
       mFirstFieldIsFid &&
       fieldId == 0 )
  {
    QgsOgrLayerUniquePtr resultLayer = mOgrOrigLayer->ExecuteSQL( QByteArray( "SELECT seq FROM sqlite_sequence WHERE name = " ) +  QgsSqliteUtils::quotedValue( mOgrOrigLayer->name() ).toUtf8() );
    if ( resultLayer )
    {
      gdal::ogr_feature_unique_ptr f;
      if ( f.reset( resultLayer->GetNextFeature() ), f )
      {
        bool ok { true };
        const QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(),
                             fields().at( 0 ),
                             0, textEncoding(), &ok );
        if ( ok )
        {
          long long nextVal { res.toLongLong( &ok ) };
          if ( ok )
          {
            // Increment
            resultVar = ++nextVal;
            mOgrOrigLayer->ExecuteSQLNoReturn( QByteArray( "UPDATE sqlite_sequence SET seq = seq + 1 WHERE name = " ) +  QgsSqliteUtils::quotedValue( mOgrOrigLayer->name() ).toUtf8() );
          }
        }

        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Error retrieving next sequence value for %1" ).arg( QString::fromUtf8( mOgrOrigLayer->name() ) ), tr( "OGR" ) );
        }
      }
      else  // no sequence!
      {
        resultVar = 1;
        mOgrOrigLayer->ExecuteSQLNoReturn( QByteArray( "INSERT INTO sqlite_sequence (name, seq) VALUES( " +
                                           QgsSqliteUtils::quotedValue( mOgrOrigLayer->name() ).toUtf8() ) + ", 1)" );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Error retrieving default value for %1" ).arg( mLayerName ), tr( "OGR" ) );
    }
  }

  ( void )mAttributeFields.at( fieldId ).convertCompatible( resultVar );
  return resultVar;
}

QString QgsOgrProvider::defaultValueClause( int fieldIndex ) const
{
  // Return empty clause to force defaultValue calls for sqlite in case we are inside a transaction
  if ( mTransaction &&
       mDefaultValues.value( fieldIndex, QString() ) == tr( "Autogenerate" ) &&
       providerProperty( EvaluateDefaultValues, false ).toBool() &&
       ( mGDALDriverName == QLatin1String( "GPKG" ) ||
         mGDALDriverName == QLatin1String( "SQLite" ) ) &&
       mFirstFieldIsFid &&
       fieldIndex == 0 )
    return QString();
  else
    return mDefaultValues.value( fieldIndex, QString() );
}

bool QgsOgrProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value ) const
{
  Q_UNUSED( constraint )
  if ( providerProperty( EvaluateDefaultValues, false ).toBool() )
  {
    return ! mDefaultValues.value( fieldIndex ).isEmpty();
  }
  else
  {
    // stricter check
    return mDefaultValues.contains( fieldIndex ) && mDefaultValues.value( fieldIndex ) == value.toString() && !value.isNull();
  }
}

void QgsOgrProvider::updateExtents()
{
  invalidateCachedExtent( true );
}

void QgsOgrProvider::invalidateCachedExtent( bool bForceRecomputeExtent )
{
  mForceRecomputeExtent = bForceRecomputeExtent;
  mExtent.reset();
}

size_t QgsOgrProvider::layerCount() const
{
  if ( !mValid )
    return 0;
  return mOgrLayer->GetLayerCount();
}

/**
 * Returns the feature type
 */
QgsWkbTypes::Type QgsOgrProvider::wkbType() const
{
  QgsWkbTypes::Type wkb = QgsOgrUtils::ogrGeometryTypeToQgsWkbType( mOGRGeomType );
  const QgsWkbTypes::Type wkbFlat = QgsWkbTypes::flatType( wkb );
  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) && ( wkbFlat == QgsWkbTypes::LineString || wkbFlat == QgsWkbTypes::Polygon ) )
  {
    wkb = QgsWkbTypes::multiType( wkb );
  }
  if ( mOGRGeomType % 1000 == wkbPolyhedralSurface ) // is PolyhedralSurface, PolyhedralSurfaceZ, PolyhedralSurfaceM or PolyhedralSurfaceZM => map to MultiPolygon
  {
    wkb = static_cast<QgsWkbTypes::Type>( mOGRGeomType - ( wkbPolyhedralSurface - wkbMultiPolygon ) );
  }
  else if ( mOGRGeomType % 1000 == wkbTIN ) // is TIN, TINZ, TINM or TINZM => map to MultiPolygon
  {
    wkb = static_cast<QgsWkbTypes::Type>( mOGRGeomType - ( wkbTIN - wkbMultiPolygon ) );
  }
  return wkb;
}

/**
 * Returns the feature count
 */
long long QgsOgrProvider::featureCount() const
{
  if ( ( mReadFlags & QgsDataProvider::SkipFeatureCount ) != 0 )
  {
    return static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
  }
  if ( mRefreshFeatureCount )
  {
    mRefreshFeatureCount = false;
    recalculateFeatureCount();
  }
  return mFeaturesCounted;
}


QgsFields QgsOgrProvider::fields() const
{
  return mAttributeFields;
}


//TODO - add sanity check for shape file layers, to include checking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsOgrProvider::isValid() const
{
  return mValid;
}

// Drivers may be more tolerant than we really wish (e.g. GeoPackage driver
// may accept any geometry type)
OGRGeometryH QgsOgrProvider::ConvertGeometryIfNecessary( OGRGeometryH hGeom )
{
  if ( !hGeom )
    return hGeom;
  OGRwkbGeometryType layerGeomType = mOgrLayer->GetLayerDefn().GetGeomType();
  OGRwkbGeometryType flattenLayerGeomType = wkbFlatten( layerGeomType );
  OGRwkbGeometryType geomType = OGR_G_GetGeometryType( hGeom );
  OGRwkbGeometryType flattenGeomType = wkbFlatten( geomType );

  if ( flattenLayerGeomType == wkbUnknown || flattenLayerGeomType == flattenGeomType )
  {
    return hGeom;
  }
  if ( flattenLayerGeomType == wkbMultiPolygon && flattenGeomType == wkbPolygon )
  {
    return OGR_G_ForceToMultiPolygon( hGeom );
  }
  if ( flattenLayerGeomType == wkbMultiLineString && flattenGeomType == wkbLineString )
  {
    return OGR_G_ForceToMultiLineString( hGeom );
  }

  return OGR_G_ForceTo( hGeom, layerGeomType, nullptr );
}

QString QgsOgrProvider::jsonStringValue( const QVariant &value ) const
{
  QString stringValue = QString::fromUtf8( QJsonDocument::fromVariant( value ).toJson().constData() );
  if ( stringValue.isEmpty() )
  {
    //store as string, because it's no valid QJson value
    stringValue = value.toString();
  }
  return stringValue;
}

bool QgsOgrProvider::addFeaturePrivate( QgsFeature &f, Flags flags, QgsFeatureId incrementalFeatureId )
{
  bool returnValue = true;
  QgsOgrFeatureDefn &featureDefinition = mOgrLayer->GetLayerDefn();
  gdal::ogr_feature_unique_ptr feature( featureDefinition.CreateFeature() );

  if ( f.hasGeometry() )
  {
    QByteArray wkb( f.geometry().asWkb() );
    OGRGeometryH geom = nullptr;

    if ( !wkb.isEmpty() )
    {
      if ( OGR_G_CreateFromWkb( reinterpret_cast<unsigned char *>( const_cast<char *>( wkb.constData() ) ), nullptr, &geom, wkb.length() ) != OGRERR_NONE )
      {
        pushError( tr( "OGR error creating wkb for feature %1: %2" ).arg( f.id() ).arg( CPLGetLastErrorMsg() ) );
        return false;
      }

      geom = ConvertGeometryIfNecessary( geom );

      OGR_F_SetGeometryDirectly( feature.get(), geom );
    }
  }

  QgsAttributes attributes = f.attributes();
  const QgsFields qgisFields { f.fields() };

  QgsLocaleNumC l;

  int qgisAttributeId = ( mFirstFieldIsFid ) ? 1 : 0;
  // If the first attribute is the FID and the user has set it, then use it
  if ( mFirstFieldIsFid && attributes.count() > 0 )
  {
    QVariant attrFid = attributes.at( 0 );
    if ( !attrFid.isNull() )
    {
      bool ok = false;
      qlonglong id = attrFid.toLongLong( &ok );
      if ( ok )
      {
        OGR_F_SetFID( feature.get(), static_cast<GIntBig>( id ) );
      }
    }
  }

  //add possible attribute information
  for ( int ogrAttributeId = 0; qgisAttributeId < attributes.count(); ++qgisAttributeId, ++ogrAttributeId )
  {
    // Skip fields that have no provider origin
    if ( qgisFields.exists( qgisAttributeId ) && qgisFields.fieldOrigin( qgisAttributeId ) != QgsFields::FieldOrigin::OriginProvider )
    {
      qgisAttributeId++;
      continue;
    }

    // don't try to set field from attribute map if it's not present in layer
    if ( ogrAttributeId >= featureDefinition.GetFieldCount() )
    {
      pushError( tr( "Feature has too many attributes (expecting %1, received %2)" ).arg( featureDefinition.GetFieldCount() ).arg( f.attributes().count() ) );
      continue;
    }

    //if(!s.isEmpty())
    // continue;
    //
    OGRFieldDefnH fldDef = featureDefinition.GetFieldDefn( ogrAttributeId );
    OGRFieldType type = OGR_Fld_GetType( fldDef );

    QVariant attrVal = attributes.at( qgisAttributeId );
    // The field value is equal to the default (that might be a provider-side expression)
    if ( mDefaultValues.contains( qgisAttributeId ) && attrVal.toString() == mDefaultValues.value( qgisAttributeId ) )
    {
      OGR_F_UnsetField( feature.get(), ogrAttributeId );
    }
    else if ( attrVal.isNull() || ( type != OFTString && ( ( attrVal.type() != QVariant::List && attrVal.toString().isEmpty() && attrVal.type() != QVariant::StringList && attrVal.toStringList().isEmpty() ) || ( attrVal.type() == QVariant::List && attrVal.toList().empty() ) ) ) )
    {
// Starting with GDAL 2.2, there are 2 concepts: unset fields and null fields
// whereas previously there was only unset fields. For a GeoJSON output,
// leaving a field unset will cause it to not appear at all in the output
// feature.
// When all features of a layer have a field unset, this would cause the
// field to not be present at all in the output, and thus on reading to
// have disappeared. #16812
#ifdef OGRNullMarker
      OGR_F_SetFieldNull( feature.get(), ogrAttributeId );
#else
      OGR_F_UnsetField( feature.get(), ogrAttId );
#endif
    }
    else
    {
      switch ( type )
      {
        case OFTInteger:
          OGR_F_SetFieldInteger( feature.get(), ogrAttributeId, attrVal.toInt() );
          break;


        case OFTInteger64:
          OGR_F_SetFieldInteger64( feature.get(), ogrAttributeId, attrVal.toLongLong() );
          break;

        case OFTReal:
          OGR_F_SetFieldDouble( feature.get(), ogrAttributeId, attrVal.toDouble() );
          break;

        case OFTDate:
          OGR_F_SetFieldDateTime( feature.get(), ogrAttributeId,
                                  attrVal.toDate().year(),
                                  attrVal.toDate().month(),
                                  attrVal.toDate().day(),
                                  0, 0, 0,
                                  0 );
          break;

        case OFTTime:
          OGR_F_SetFieldDateTime( feature.get(), ogrAttributeId,
                                  0, 0, 0,
                                  attrVal.toTime().hour(),
                                  attrVal.toTime().minute(),
                                  attrVal.toTime().second(),
                                  0 );
          break;

        case OFTDateTime:
          OGR_F_SetFieldDateTime( feature.get(), ogrAttributeId,
                                  attrVal.toDateTime().date().year(),
                                  attrVal.toDateTime().date().month(),
                                  attrVal.toDateTime().date().day(),
                                  attrVal.toDateTime().time().hour(),
                                  attrVal.toDateTime().time().minute(),
                                  attrVal.toDateTime().time().second(),
                                  0 );
          break;

        case OFTString:
        {
          QString stringValue;

          if ( OGR_Fld_GetSubType( fldDef ) == OFSTJSON )
            stringValue = jsonStringValue( attrVal );
          else
          {
            stringValue = attrVal.toString();
          }
          QgsDebugMsgLevel( QStringLiteral( "Writing string attribute %1 with %2, encoding %3" )
                            .arg( qgisAttributeId )
                            .arg( attrVal.toString(),
                                  textEncoding()->name().data() ), 3 );
          OGR_F_SetFieldString( feature.get(), ogrAttributeId, textEncoding()->fromUnicode( stringValue ).constData() );
          break;
        }
        case OFTBinary:
        {
          const QByteArray ba = attrVal.toByteArray();
          OGR_F_SetFieldBinary( feature.get(), ogrAttributeId, ba.size(), const_cast< GByte * >( reinterpret_cast< const GByte * >( ba.data() ) ) );
          break;
        }

        case OFTStringList:
        {
          QStringList list = attrVal.toStringList();
          int count = list.count();
          char **lst = new char *[count + 1];
          if ( count > 0 )
          {
            int pos = 0;
            for ( const QString &string : list )
            {
              lst[pos] = CPLStrdup( textEncoding()->fromUnicode( string ).data() );
              pos++;
            }
          }
          lst[count] = nullptr;
          OGR_F_SetFieldStringList( feature.get(), ogrAttributeId, lst );
          CSLDestroy( lst );
          break;
        }

        case OFTIntegerList:
        {
          const QVariantList list = attrVal.toList();
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
          OGR_F_SetFieldIntegerList( feature.get(), ogrAttributeId, count, lst );
          delete [] lst;
          break;
        }

        case OFTRealList:
        {
          const QVariantList list = attrVal.toList();
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
          OGR_F_SetFieldDoubleList( feature.get(), ogrAttributeId, count, lst );
          delete [] lst;
          break;
        }

        case OFTInteger64List:
        {
          const QVariantList list = attrVal.toList();
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
          OGR_F_SetFieldInteger64List( feature.get(), ogrAttributeId, count, lst );
          delete [] lst;
          break;
        }

        default:
          QgsMessageLog::logMessage( tr( "type %1 for attribute %2 not found" ).arg( type ).arg( qgisAttributeId ), tr( "OGR" ) );
          break;
      }
    }
  }

  if ( mOgrLayer->CreateFeature( feature.get() ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error creating feature %1: %2" ).arg( f.id() ).arg( CPLGetLastErrorMsg() ) );
    returnValue = false;
  }
  else
  {
    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      QgsFeatureId id = static_cast<QgsFeatureId>( OGR_F_GetFID( feature.get() ) );
      if ( id >= 0 )
      {
        f.setId( id );

        if ( mFirstFieldIsFid && attributes.count() > 0 )
        {
          f.setAttribute( 0, id );
        }
      }
      else if ( incrementalFeatureId >= 0 )
      {
        f.setId( incrementalFeatureId );
      }
    }
  }

  return returnValue;
}


bool QgsOgrProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  setRelevantFields( true, attributeIndexes() );

  const bool inTransaction = startTransaction();

  QgsFeatureId incrementalFeatureId = -1;
  if ( !( flags & QgsFeatureSink::FastInsert ) &&
       ( mGDALDriverName == QLatin1String( "CSV" ) || mGDALDriverName == QLatin1String( "XLSX" ) || mGDALDriverName == QLatin1String( "ODS" ) ) )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mutex = nullptr;
#else
    QRecursiveMutex *mutex = nullptr;
#endif
    OGRLayerH layer = mOgrOrigLayer->getHandleAndMutex( mutex );
    {
      QMutexLocker locker( mutex );

      if ( !mSubsetString.isEmpty() )
        OGR_L_SetAttributeFilter( layer, nullptr );

      incrementalFeatureId = static_cast< QgsFeatureId >( OGR_L_GetFeatureCount( layer, false ) ) + 1;

      if ( !mSubsetString.isEmpty() )
        OGR_L_SetAttributeFilter( layer, textEncoding()->fromUnicode( mSubsetString ).constData() );
    }
  }

  bool returnvalue = true;
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    if ( !addFeaturePrivate( *it, flags, incrementalFeatureId ) )
    {
      returnvalue = false;
    }
    if ( incrementalFeatureId >= 0 )
      incrementalFeatureId++;
  }

  if ( inTransaction )
  {
    if ( returnvalue )
      returnvalue = commitTransaction();
    else
      rollbackTransaction();
  }

  if ( !syncToDisc() )
  {
    returnvalue = false;
  }

  if ( mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::Uncounted ) &&
       mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::UnknownCount ) )
  {
    if ( returnvalue )
      mFeaturesCounted += flist.size();
    else
      recalculateFeatureCount();
  }

  if ( returnvalue )
    clearMinMaxCache();

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return returnvalue;
}

bool QgsOgrProvider::addAttributeOGRLevel( const QgsField &field, bool &ignoreErrorOut )
{
  ignoreErrorOut = false;

  OGRFieldType type;

  switch ( field.type() )
  {
    case QVariant::Int:
    case QVariant::Bool:
      type = OFTInteger;
      break;
    case QVariant::LongLong:
    {
      const char *pszDataTypes = GDALGetMetadataItem( mOgrLayer->driver(), GDAL_DMD_CREATIONFIELDDATATYPES, nullptr );
      if ( pszDataTypes && strstr( pszDataTypes, "Integer64" ) )
        type = OFTInteger64;
      else
      {
        type = OFTReal;
      }
      break;
    }
    case QVariant::Double:
      type = OFTReal;
      break;
    case QVariant::Date:
      type = OFTDate;
      break;
    case QVariant::Time:
      type = OFTTime;
      break;
    case QVariant::DateTime:
      type = OFTDateTime;
      break;
    case QVariant::String:
      type = OFTString;
      break;
    case QVariant::ByteArray:
      type = OFTBinary;
      break;
    case QVariant::Map:
      type = OFTString;
      break;
    case QVariant::StringList:
      type = OFTStringList;
      break;
    case QVariant::List:
      if ( field.subType() == QVariant::String )
      {
        type = OFTStringList;
        break;
      }
      else if ( field.subType() == QVariant::Int )
      {
        type = OFTIntegerList;
        break;
      }
      else if ( field.subType() == QVariant::LongLong )
      {
        type = OFTInteger64List;
        break;
      }
      else if ( field.subType() == QVariant::Double )
      {
        type = OFTRealList;
        break;
      }
      // other lists are supported at this moment, fall through to default for other types

      //intentional fall-through
      FALLTHROUGH

    default:
      pushError( tr( "type %1 for field %2 not found" ).arg( field.typeName(), field.name() ) );
      ignoreErrorOut = true;
      return false;
  }

  gdal::ogr_field_def_unique_ptr fielddefn( OGR_Fld_Create( textEncoding()->fromUnicode( field.name() ).constData(), type ) );
  int width = field.length();
// Increase width by 1 for OFTReal to make room for the decimal point
  if ( type == OFTReal && field.precision() )
    width += 1;
  OGR_Fld_SetWidth( fielddefn.get(), width );
  OGR_Fld_SetPrecision( fielddefn.get(), field.precision() );

  switch ( field.type() )
  {
    case QVariant::Bool:
      OGR_Fld_SetSubType( fielddefn.get(), OFSTBoolean );
      break;
    case QVariant::Map:
      OGR_Fld_SetSubType( fielddefn.get(), OFSTJSON );
      break;
    default:
      break;
  }

  if ( mOgrLayer->CreateField( fielddefn.get(), true ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error creating field %1: %2" ).arg( field.name(), CPLGetLastErrorMsg() ) );
    return false;
  }
  return true;
}

bool QgsOgrProvider::addAttributes( const QList<QgsField> &attributes )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  if ( mGDALDriverName == QLatin1String( "MapInfo File" ) )
  {
    // adding attributes in mapinfo requires to be able to delete the .dat file
    // so drop any cached connections.
    QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  }

  bool returnvalue = true;

  QMap< QString, QgsField > mapFieldNameToOriginalField;

  for ( const auto &field : attributes )
  {
    mapFieldNameToOriginalField[ field.name()] = field;

    bool ignoreErrorOut = false;
    if ( !addAttributeOGRLevel( field, ignoreErrorOut ) )
    {
      returnvalue = false;
      if ( !ignoreErrorOut )
      {
        break;
      }
    }
  }

  // Backup existing fields. We need them to 'restore' field type, length, precision
  QgsFields oldFields = mAttributeFields;

  loadFields();

  // The check in QgsVectorLayerEditBuffer::commitChanges() is questionable with
  // real-world drivers that might only be able to satisfy request only partially.
  // So to avoid erroring out, patch field type, width and precision to match
  // what was requested.
  // For example in case of Integer64->Real mapping so that QVariant::LongLong is
  // still returned to the caller
  // Or if a field width was specified but not strictly enforced by the driver (#15614)
  for ( QMap< QString, QgsField >::const_iterator it = mapFieldNameToOriginalField.constBegin();
        it != mapFieldNameToOriginalField.constEnd(); ++it )
  {
    int idx = mAttributeFields.lookupField( it.key() );
    if ( idx >= 0 )
    {
      mAttributeFields[ idx ].setType( it->type() );
      mAttributeFields[ idx ].setLength( it->length() );
      mAttributeFields[ idx ].setPrecision( it->precision() );
      mAttributeFields[ idx ].setEditorWidgetSetup( it->editorWidgetSetup() );
    }
  }

  // Restore field type, length, precision of existing fields as well
  // We need that in scenarios where the user adds a int field with length != 0
  // in a editing session, and repeat that again in another editing session
  // Without the below hack, the length of the first added field would have
  // been reset to zero, and QgsVectorLayerEditBuffer::commitChanges() would
  // error out because of this.
  // See https://github.com/qgis/QGIS/issues/26840
  for ( const QgsField &field : oldFields )
  {
    int idx = mAttributeFields.lookupField( field.name() );
    if ( idx >= 0 )
    {
      mAttributeFields[ idx ].setType( field.type() );
      mAttributeFields[ idx ].setLength( field.length() );
      mAttributeFields[ idx ].setPrecision( field.precision() );
    }
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return returnvalue;
}

bool QgsOgrProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  if ( !doInitialActionsForEdition() )
    return false;

  bool res = true;
  QList<int> attrsLst = qgis::setToList( attributes );
  // sort in descending order
  std::sort( attrsLst.begin(), attrsLst.end(), std::greater<int>() );
  const auto constAttrsLst = attrsLst;
  for ( int attr : constAttrsLst )
  {
    if ( mFirstFieldIsFid )
    {
      if ( attr == 0 )
      {
        pushError( tr( "Cannot delete feature id column" ) );
        res = false;
        break;
      }
      else
      {
        --attr;
      }
    }
    if ( mOgrLayer->DeleteField( attr ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error deleting field %1: %2" ).arg( attr ).arg( CPLGetLastErrorMsg() ) );
      res = false;
    }
  }
  loadFields();

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return res;
}

bool QgsOgrProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  bool result = true;
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    int fieldIndex = renameIt.key();
    if ( fieldIndex < 0 || fieldIndex >= mAttributeFields.count() )
    {
      pushError( tr( "Invalid attribute index" ) );
      result = false;
      continue;
    }
    if ( mAttributeFields.indexFromName( renameIt.value() ) >= 0 )
    {
      //field name already in use
      pushError( tr( "Error renaming field %1: name '%2' already exists" ).arg( fieldIndex ).arg( renameIt.value() ) );
      result = false;
      continue;
    }
    int ogrFieldIndex = fieldIndex;
    if ( mFirstFieldIsFid )
    {
      ogrFieldIndex -= 1;
      if ( ogrFieldIndex < 0 )
      {
        pushError( tr( "Invalid attribute index" ) );
        result = false;
        continue;
      }
    }

    //type does not matter, it will not be used
    gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( textEncoding()->fromUnicode( renameIt.value() ), OFTReal ) );
    if ( mOgrLayer->AlterFieldDefn( ogrFieldIndex, fld.get(), ALTER_NAME_FLAG ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error renaming field %1: %2" ).arg( fieldIndex ).arg( CPLGetLastErrorMsg() ) );
      result = false;
    }
  }
  loadFields();

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return result;
}

bool QgsOgrProvider::startTransaction()
{
  bool inTransaction = false;
  if ( mTransaction == nullptr && mOgrLayer->TestCapability( OLCTransactions ) )
  {
    // A transaction might already be active, so be robust on failed
    // StartTransaction.
    CPLPushErrorHandler( CPLQuietErrorHandler );
    inTransaction = ( mOgrLayer->StartTransaction() == OGRERR_NONE );
    CPLPopErrorHandler();
  }
  return inTransaction;
}


bool QgsOgrProvider::commitTransaction()
{
  if ( mOgrLayer->CommitTransaction() != OGRERR_NONE )
  {
    pushError( tr( "OGR error committing transaction: %1" ).arg( CPLGetLastErrorMsg() ) );
    return false;
  }
  return true;
}


bool QgsOgrProvider::rollbackTransaction()
{
  if ( mOgrLayer->RollbackTransaction() != OGRERR_NONE )
  {
    pushError( tr( "OGR error rolling back transaction: %1" ).arg( CPLGetLastErrorMsg() ) );
    return false;
  }
  return true;
}

bool QgsOgrProvider::_setSubsetString( const QString &theSQL, bool updateFeatureCount, bool updateCapabilities, bool hasExistingRef )
{
  QgsCPLErrorHandler handler;

  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !mOgrOrigLayer )
    return false;

  if ( theSQL == mSubsetString && mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::Uncounted ) )
    return true;

  const bool subsetStringHasChanged { theSQL != mSubsetString };

  if ( !theSQL.isEmpty() )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mutex = nullptr;
#else
    QRecursiveMutex *mutex = nullptr;
#endif
    OGRLayerH layer = mOgrOrigLayer->getHandleAndMutex( mutex );
    GDALDatasetH ds = mOgrOrigLayer->getDatasetHandleAndMutex( mutex );
    OGRLayerH subsetLayerH;
    {
      QMutexLocker locker( mutex );
      subsetLayerH = QgsOgrProviderUtils::setSubsetString( layer, ds, textEncoding(), theSQL );
    }
    if ( !subsetLayerH )
    {
      pushError( tr( "OGR[%1] error %2: %3" ).arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );
      return false;
    }
    if ( layer != subsetLayerH )
    {
      mOgrSqlLayer = QgsOgrProviderUtils::getSqlLayer( mOgrOrigLayer.get(), subsetLayerH, theSQL );
      Q_ASSERT( mOgrSqlLayer.get() );
      mOgrLayer = mOgrSqlLayer.get();
    }
    else
    {
      mOgrSqlLayer.reset();
      mOgrLayer = mOgrOrigLayer.get();
    }
  }
  else
  {
    mOgrSqlLayer.reset();
    mOgrLayer = mOgrOrigLayer.get();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mutex = nullptr;
#else
    QRecursiveMutex *mutex = nullptr;
#endif
    OGRLayerH layer = mOgrOrigLayer->getHandleAndMutex( mutex );
    {
      QMutexLocker locker( mutex );
      OGR_L_SetAttributeFilter( layer, nullptr );
    }
  }
  mSubsetString = theSQL;

  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), mFilePath );

  if ( !mLayerName.isNull() )
  {
    parts.insert( QStringLiteral( "layerName" ), mLayerName );
  }
  else if ( mIsSubLayer && mLayerIndex >= 0 )
  {
    parts.insert( QStringLiteral( "layerId" ), mLayerIndex );
  }

  if ( !mSubsetString.isEmpty() )
  {
    parts.insert( QStringLiteral( "subset" ), mSubsetString );
  }

  if ( mOgrGeometryTypeFilter != wkbUnknown )
  {
    parts.insert( QStringLiteral( "geometryType" ), QgsOgrProviderUtils::ogrWkbGeometryTypeName( mOgrGeometryTypeFilter ) );
  }

  if ( !mOpenOptions.isEmpty() )
  {
    parts.insert( QStringLiteral( "openOptions" ), mOpenOptions );
  }

  QString uri = QgsOgrProviderMetadata().encodeUri( parts );
  if ( uri != dataSourceUri() )
  {
    if ( hasExistingRef )
      QgsOgrConnPool::instance()->unref( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
    setDataSourceUri( uri );
    if ( hasExistingRef )
      QgsOgrConnPool::instance()->ref( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  }

  mOgrLayer->ResetReading();

  mRefreshFeatureCount = updateFeatureCount;

  // check the validity of the layer if subset string has changed
  if ( subsetStringHasChanged )
  {
    loadFields();
  }

  invalidateCachedExtent( false );

  // Changing the filter may change capabilities
  if ( updateCapabilities )
    computeCapabilities();

  emit dataChanged();

  return true;

}


bool QgsOgrProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  bool returnValue = true;

  clearMinMaxCache();

  setRelevantFields( true, attributeIndexes() );

  const bool inTransaction = startTransaction();

  // Some drivers may need to call ResetReading() after GetFeature(), such
  // as GPKG in GDAL < 2.3.0 to avoid letting the database in a locked state.
  // But this is undesirable in general, so don't do this when we know that
  // we don't need to.
  bool mayNeedResetReadingAfterGetFeature = true;
  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    mayNeedResetReadingAfterGetFeature = false;
  }
  else if ( mGDALDriverName == QLatin1String( "GPKG" ) )
  {
    mayNeedResetReadingAfterGetFeature = false;
  }

  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();

    const QgsAttributeMap &attr = it.value();
    if ( attr.isEmpty() )
      continue;

    gdal::ogr_feature_unique_ptr of( mOgrLayer->GetFeature( FID_TO_NUMBER( fid ) ) );
    if ( !of )
    {
      pushError( tr( "Feature %1 for attribute update not found." ).arg( fid ) );
      continue;
    }

    if ( mayNeedResetReadingAfterGetFeature )
    {
      mOgrLayer->ResetReading();
    }

    QgsLocaleNumC l;

    for ( QgsAttributeMap::const_iterator it2 = attr.begin(); it2 != attr.end(); ++it2 )
    {
      int f = it2.key();
      if ( mFirstFieldIsFid )
      {
        if ( f == 0 )
        {
          if ( it2->toLongLong() != fid )
          {
            pushError( tr( "Changing feature id of feature %1 is not allowed." ).arg( fid ) );
            returnValue = false;
          }
          continue;
        }
        else
        {
          --f;
        }
      }

      OGRFieldDefnH fd = OGR_F_GetFieldDefnRef( of.get(), f );
      if ( !fd )
      {
        pushError( tr( "Field %1 of feature %2 doesn't exist." ).arg( f ).arg( fid ) );
        continue;
      }

      OGRFieldType type = OGR_Fld_GetType( fd );
      if ( it2->isNull() || ( type != OFTString && ( ( it2->type() != QVariant::List && it2->type() != QVariant::StringList && it2->toString().isEmpty() ) || ( it2->type() == QVariant::List && it2->toList().empty() ) || ( it2->type() == QVariant::StringList && it2->toStringList().empty() ) ) ) )
      {
// Starting with GDAL 2.2, there are 2 concepts: unset fields and null fields
// whereas previously there was only unset fields. For a GeoJSON output,
// leaving a field unset will cause it to not appear at all in the output
// feature.
// When all features of a layer have a field unset, this would cause the
// field to not be present at all in the output, and thus on reading to
// have disappeared. #16812
#ifdef OGRNullMarker
        OGR_F_SetFieldNull( of.get(), f );
#else
        OGR_F_UnsetField( of.get(), f );
#endif
      }
      else
      {
        switch ( type )
        {
          case OFTInteger:
            OGR_F_SetFieldInteger( of.get(), f, it2->toInt() );
            break;
          case OFTInteger64:
            OGR_F_SetFieldInteger64( of.get(), f, it2->toLongLong() );
            break;
          case OFTReal:
            OGR_F_SetFieldDouble( of.get(), f, it2->toDouble() );
            break;
          case OFTDate:
            OGR_F_SetFieldDateTime( of.get(), f,
                                    it2->toDate().year(),
                                    it2->toDate().month(),
                                    it2->toDate().day(),
                                    0, 0, 0,
                                    0 );
            break;
          case OFTTime:
            OGR_F_SetFieldDateTime( of.get(), f,
                                    0, 0, 0,
                                    it2->toTime().hour(),
                                    it2->toTime().minute(),
                                    it2->toTime().second(),
                                    0 );
            break;
          case OFTDateTime:
            OGR_F_SetFieldDateTime( of.get(), f,
                                    it2->toDateTime().date().year(),
                                    it2->toDateTime().date().month(),
                                    it2->toDateTime().date().day(),
                                    it2->toDateTime().time().hour(),
                                    it2->toDateTime().time().minute(),
                                    it2->toDateTime().time().second(),
                                    0 );
            break;
          case OFTString:
          {
            QString stringValue;
            if ( OGR_Fld_GetSubType( fd ) == OFSTJSON )
              stringValue = jsonStringValue( it2.value() );
            else
              stringValue = it2->toString();
            OGR_F_SetFieldString( of.get(), f, textEncoding()->fromUnicode( stringValue ).constData() );
            break;
          }

          case OFTBinary:
          {
            const QByteArray ba = it2->toByteArray();
            OGR_F_SetFieldBinary( of.get(), f, ba.size(), const_cast< GByte * >( reinterpret_cast< const GByte * >( ba.data() ) ) );
            break;
          }

          case OFTStringList:
          {
            QStringList list = it2->toStringList();
            int count = list.count();
            char **lst = new char *[count + 1];
            if ( count > 0 )
            {
              int pos = 0;
              for ( const QString &string : list )
              {
                lst[pos] = CPLStrdup( textEncoding()->fromUnicode( string ).data() );
                pos++;
              }
            }
            lst[count] = nullptr;
            OGR_F_SetFieldStringList( of.get(), f, lst );
            CSLDestroy( lst );
            break;
          }

          case OFTIntegerList:
          {
            const QVariantList list = it2->toList();
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
            OGR_F_SetFieldIntegerList( of.get(), f, count, lst );
            delete [] lst;
            break;
          }

          case OFTRealList:
          {
            const QVariantList list = it2->toList();
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
            OGR_F_SetFieldDoubleList( of.get(), f, count, lst );
            delete [] lst;
            break;
          }

          case OFTInteger64List:
          {
            const QVariantList list = it2->toList();
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
            OGR_F_SetFieldInteger64List( of.get(), f, count, lst );
            delete [] lst;
            break;
          }

          default:
            pushError( tr( "Type %1 of attribute %2 of feature %3 unknown." ).arg( type ).arg( fid ).arg( f ) );
            break;
        }
      }
    }

    if ( mOgrLayer->SetFeature( of.get() ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error setting feature %1: %2" ).arg( fid ).arg( CPLGetLastErrorMsg() ) );
      returnValue = false;
    }
  }

  if ( inTransaction )
  {
    if ( returnValue )
      returnValue = commitTransaction();
    else
      rollbackTransaction();
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  if ( !syncToDisc() )
  {
    pushError( tr( "OGR error syncing to disk: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
  return returnValue;
}

bool QgsOgrProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  setRelevantFields( true, attributeIndexes() );

  const bool inTransaction = startTransaction();

  // Some drivers may need to call ResetReading() after GetFeature(), such
  // as GPKG in GDAL < 2.3.0 to avoid letting the database in a locked state.
  // But this is undesirable in general, so don't do this when we know that
  // we don't need to.
  bool mayNeedResetReadingAfterGetFeature = true;
  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    mayNeedResetReadingAfterGetFeature = false;
  }
  else if ( mGDALDriverName == QLatin1String( "GPKG" ) )
  {
    mayNeedResetReadingAfterGetFeature = false;
  }

  bool returnvalue = true;
  for ( QgsGeometryMap::const_iterator it = geometry_map.constBegin(); it != geometry_map.constEnd(); ++it )
  {
    gdal::ogr_feature_unique_ptr theOGRFeature( mOgrLayer->GetFeature( FID_TO_NUMBER( it.key() ) ) );
    if ( !theOGRFeature )
    {
      pushError( tr( "OGR error changing geometry: feature %1 not found" ).arg( it.key() ) );
      returnvalue = false;
      continue;
    }

    if ( mayNeedResetReadingAfterGetFeature )
    {
      mOgrLayer->ResetReading(); // needed for SQLite-based to clear iterator, which could let the database in a locked state otherwise
    }

    OGRGeometryH newGeometry = nullptr;
    QByteArray wkb = it->asWkb();
    // We might receive null geometries. It is OK, but don't go through the
    // OGR_G_CreateFromWkb() route then
    if ( !wkb.isEmpty() )
    {
      //create an OGRGeometry
      if ( OGR_G_CreateFromWkb( reinterpret_cast<unsigned char *>( const_cast<char *>( wkb.constData() ) ),
                                mOgrLayer->GetSpatialRef(),
                                &newGeometry,
                                wkb.length() ) != OGRERR_NONE )
      {
        pushError( tr( "OGR error creating geometry for feature %1: %2" ).arg( it.key() ).arg( CPLGetLastErrorMsg() ) );
        OGR_G_DestroyGeometry( newGeometry );
        returnvalue = false;
        continue;
      }

      if ( !newGeometry )
      {
        pushError( tr( "OGR error in feature %1: geometry is null" ).arg( it.key() ) );
        returnvalue = false;
        continue;
      }

      newGeometry = ConvertGeometryIfNecessary( newGeometry );
    }

    //set the new geometry
    if ( OGR_F_SetGeometryDirectly( theOGRFeature.get(), newGeometry ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error setting geometry of feature %1: %2" ).arg( it.key() ).arg( CPLGetLastErrorMsg() ) );
      // Shouldn't happen normally. If it happens, ownership of the geometry
      // may be not really well defined, so better not destroy it, but just
      // the feature.
      returnvalue = false;
      continue;
    }


    if ( mOgrLayer->SetFeature( theOGRFeature.get() ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error setting feature %1: %2" ).arg( it.key() ).arg( CPLGetLastErrorMsg() ) );
      returnvalue = false;
      continue;
    }
    mShapefileMayBeCorrupted = true;

    invalidateCachedExtent( true );
  }

  if ( inTransaction )
  {
    if ( returnvalue )
      returnvalue = commitTransaction();
    else
      rollbackTransaction();
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  if ( !syncToDisc() )
  {
    pushError( tr( "OGR error syncing to disk: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
  return returnvalue;
}

bool QgsOgrProvider::createSpatialIndex()
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !mOgrOrigLayer )
    return false;
  if ( !doInitialActionsForEdition() )
    return false;

  QByteArray layerName = mOgrOrigLayer->name();
  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    QByteArray sql = QByteArray( "CREATE SPATIAL INDEX ON " ) + quotedIdentifier( layerName );  // quote the layer name so spaces are handled
    QgsDebugMsgLevel( QStringLiteral( "SQL: %1" ).arg( QString::fromUtf8( sql ) ), 2 );
    mOgrOrigLayer->ExecuteSQLNoReturn( sql );

    if ( !mFilePath.endsWith( QLatin1String( ".shp" ), Qt::CaseInsensitive ) )
      return true;

    QFileInfo fi( mFilePath );     // to get the base name
    //find out, if the .qix file is there
    return QFileInfo::exists( fi.path().append( '/' ).append( fi.completeBaseName() ).append( ".qix" ) );
  }
  else if ( mGDALDriverName == QLatin1String( "GPKG" ) ||
            mGDALDriverName == QLatin1String( "SQLite" ) )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mutex = nullptr;
#else
    QRecursiveMutex *mutex = nullptr;
#endif
    OGRLayerH layer = mOgrOrigLayer->getHandleAndMutex( mutex );
    QByteArray sql = QByteArray( "SELECT CreateSpatialIndex(" + quotedIdentifier( layerName ) + ","
                                 + quotedIdentifier( OGR_L_GetGeometryColumn( layer ) ) + ") " ); // quote the layer name so spaces are handled
    mOgrOrigLayer->ExecuteSQLNoReturn( sql );
    return true;
  }
  return false;
}

QString QgsOgrProvider::createIndexName( QString tableName, QString field )
{
  QRegularExpression safeExp( QStringLiteral( "[^a-zA-Z0-9]" ) );
  tableName.replace( safeExp, QStringLiteral( "_" ) );
  field.replace( safeExp, QStringLiteral( "_" ) );
  return tableName + "_" + field + "_idx";
}

bool QgsOgrProvider::createAttributeIndex( int field )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( field < 0 || field >= mAttributeFields.count() )
    return false;

  if ( !doInitialActionsForEdition() )
    return false;

  QByteArray quotedLayerName = quotedIdentifier( mOgrOrigLayer->name() );
  if ( mGDALDriverName == QLatin1String( "GPKG" ) ||
       mGDALDriverName == QLatin1String( "SQLite" ) )
  {
    if ( field == 0 && mFirstFieldIsFid )
    {
      // already an index on this field, no need to re-created
      return false;
    }

    QString indexName = createIndexName( mOgrOrigLayer->name(), fields().at( field ).name() );
    QByteArray createSql = "CREATE INDEX IF NOT EXISTS " + textEncoding()->fromUnicode( indexName ) + " ON " + quotedLayerName + " (" + textEncoding()->fromUnicode( fields().at( field ).name() ) + ")";
    mOgrOrigLayer->ExecuteSQLNoReturn( createSql );
    return true;
  }
  else
  {
    QByteArray dropSql = "DROP INDEX ON " + quotedLayerName;
    mOgrOrigLayer->ExecuteSQLNoReturn( dropSql );
    QByteArray createSql = "CREATE INDEX ON " + quotedLayerName + " USING " + textEncoding()->fromUnicode( fields().at( field ).name() );
    mOgrOrigLayer->ExecuteSQLNoReturn( createSql );

    QFileInfo fi( mFilePath );     // to get the base name
    //find out, if the .idm/.ind file is there
    QString idmFile( fi.path().append( '/' ).append( fi.completeBaseName() ).append( ".idm" ) );
    QString indFile( fi.path().append( '/' ).append( fi.completeBaseName() ).append( ".ind" ) );
    return QFile::exists( idmFile ) || QFile::exists( indFile );
  }
}

bool QgsOgrProvider::deleteFeatures( const QgsFeatureIds &id )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  const bool inTransaction = startTransaction();

  bool returnvalue = true;
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    if ( !deleteFeature( *it ) )
    {
      returnvalue = false;
    }
  }

  if ( inTransaction )
  {
    if ( returnvalue )
      returnvalue = commitTransaction();
    else
      rollbackTransaction();
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  if ( !syncToDisc() )
  {
    returnvalue = false;
  }

  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    // Shapefile behaves in a special way due to possible recompaction
    recalculateFeatureCount();
  }
  else
  {
    if ( mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::Uncounted ) &&
         mFeaturesCounted != static_cast< long long >( Qgis::FeatureCountState::UnknownCount ) )
    {
      if ( returnvalue )
        mFeaturesCounted -= id.size();
      else
        recalculateFeatureCount();
    }
  }

  clearMinMaxCache();

  invalidateCachedExtent( true );

  return returnvalue;
}

bool QgsOgrProvider::deleteFeature( QgsFeatureId id )
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( !doInitialActionsForEdition() )
    return false;

  if ( mOgrLayer->DeleteFeature( FID_TO_NUMBER( id ) ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error deleting feature %1: %2" ).arg( id ).arg( CPLGetLastErrorMsg() ) );
    return false;
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  mShapefileMayBeCorrupted = true;

  return true;
}

bool QgsOgrProvider::doInitialActionsForEdition()
{
  if ( !mValid )
    return false;

  // If mUpdateModeStackDepth > 0, it means that an updateMode is already active and that we have write access
  if ( mUpdateModeStackDepth == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Enter update mode implicitly" ), 2 );
    if ( !_enterUpdateMode( true ) )
      return false;
  }

  return true;
}

QgsVectorDataProvider::Capabilities QgsOgrProvider::capabilities() const
{
  return mCapabilities;
}

void QgsOgrProvider::computeCapabilities()
{
  QgsVectorDataProvider::Capabilities ability = QgsVectorDataProvider::Capabilities();
  bool updateModeActivated = false;

  // collect abilities reported by OGR
  if ( mOgrLayer )
  {

    // We want the layer in rw mode or capabilities will be wrong
    // If mUpdateModeStackDepth > 0, it means that an updateMode is already active and that we have write access
    if ( mUpdateModeStackDepth == 0 )
    {
      updateModeActivated = _enterUpdateMode( true );
    }

    // Whilst the OGR documentation (e.g. at
    // https://gdal.org/doxygen/classOGRLayer.html#aeedbda1a62f9b89b8e5f24332cf22286) states "The capability
    // codes that can be tested are represented as strings, but #defined
    // constants exists to ensure correct spelling", we always use strings
    // here.  This is because older versions of OGR don't always have all
    // the #defines we want to test for here.

    if ( mOgrLayer->TestCapability( "RandomRead" ) )
      // true if the GetFeature() method works *efficiently* for this layer.
      // TODO: Perhaps influence if QGIS caches into memory
      //       (vs read from disk every time) based on this setting.
    {
      // the latter flag is here just for compatibility
      ability |= QgsVectorDataProvider::SelectAtId;
    }

    if ( mWriteAccessPossible && mOgrLayer->TestCapability( "SequentialWrite" ) )
      // true if the CreateFeature() method works for this layer.
    {
      ability |= QgsVectorDataProvider::AddFeatures;
    }

    if ( mWriteAccessPossible && mOgrLayer->TestCapability( "DeleteFeature" ) )
      // true if this layer can delete its features
    {
      ability |= DeleteFeatures;
    }

    if ( mWriteAccessPossible && mOgrLayer->TestCapability( "RandomWrite" ) )
      // true if the SetFeature() method is operational on this layer.
    {
      // TODO According to http://shapelib.maptools.org/ (Shapefile C Library V1.2)
      // TODO "You can't modify the vertices of existing structures".
      // TODO Need to work out versions of shapelib vs versions of GDAL/OGR
      // TODO And test appropriately.

      ability |= ChangeAttributeValues;
      ability |= ChangeGeometries;
    }

#if 0
    if ( mOgrLayer->TestCapability( "FastFeatureCount" ) )
      // true if this layer can return a feature count
      // (via OGRLayer::GetFeatureCount()) efficiently ... ie. without counting
      // the features. In some cases this will return true until a spatial
      // filter is installed after which it will return false.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to count features.
    }

    if ( mOgrLayer->TestCapability( "FastSetNextByIndex" ) )
      // true if this layer can perform the SetNextByIndex() call efficiently.
    {
      // No use required for this QGIS release.
    }
#endif

    if ( mWriteAccessPossible && mOgrLayer->TestCapability( "CreateField" ) )
    {
      ability |= AddAttributes;
    }

    if ( mWriteAccessPossible && mOgrLayer->TestCapability( "DeleteField" ) )
    {
      ability |= DeleteAttributes;
    }

    if ( mWriteAccessPossible && mOgrLayer->TestCapability( "AlterFieldDefn" ) )
    {
      ability |= RenameAttributes;
    }

    if ( !mOgrLayer->TestCapability( OLCStringsAsUTF8 ) )
    {
      ability |= SelectEncoding;
    }

    // OGR doesn't handle shapefiles without attributes, ie. missing DBFs well, fixes #803
    if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
    {
      ability |= CreateSpatialIndex;
      ability |= CreateAttributeIndex;

      if ( ( ability & ChangeAttributeValues ) == 0 )
      {
        // on readonly shapes OGR reports that it can delete features although it can't RandomWrite
        ability &= ~( AddAttributes | DeleteFeatures );
      }
    }
    else if ( mGDALDriverName == QLatin1String( "GPKG" ) ||
              mGDALDriverName == QLatin1String( "SQLite" ) )
    {
      ability |= CreateSpatialIndex;
      ability |= CreateAttributeIndex;
    }

    /* Curve geometries are available in some drivers starting with GDAL 2.0 */
    if ( mOgrLayer->TestCapability( "CurveGeometries" ) )
    {
      ability |= CircularGeometries;
    }

    if ( mGDALDriverName == QLatin1String( "GPKG" ) )
    {
      //supports transactions
      ability |= TransactionSupport;
    }

    if ( GDALGetMetadataItem( mOgrLayer->driver(), GDAL_DCAP_FEATURE_STYLES, nullptr ) != nullptr )
    {
      ability |= FeatureSymbology;
      ability |= CreateRenderer;
    }
  }

  ability |= ReadLayerMetadata;
  ability |= ReloadData;

  if ( updateModeActivated )
    leaveUpdateMode();

  mCapabilities = ability;
}


QString QgsOgrProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsOgrProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString  QgsOgrProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsOgrProvider::crs() const
{
  QgsCoordinateReferenceSystem srs;
  if ( !mValid || ( mOGRGeomType == wkbNone ) )
    return srs;

  if ( OGRSpatialReferenceH spatialRefSys = mOgrLayer->GetSpatialRef() )
  {
    srs = QgsOgrUtils::OGRSpatialReferenceToCrs( spatialRefSys );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "no spatial reference found" ) );
  }

  return srs;
}

QSet<QVariant> QgsOgrProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;

  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
    return uniqueValues;

  const QgsField fld = mAttributeFields.at( index );
  if ( fld.name().isNull() )
  {
    return uniqueValues; //not a provider field
  }

  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  QByteArray sql = "SELECT DISTINCT " + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) );

  // GPKG/SQLite fid
  // For GPKG and SQLITE drivers PK fields are not exposed as real fields, (and OGR_F_GetFID only
  // works with GPKG), so we are adding an extra column that will become index 0
  // See https://github.com/qgis/QGIS/issues/29129
  if ( ( mGDALDriverName == QLatin1String( "GPKG" ) || mGDALDriverName == QLatin1String( "SQLite" ) )
       && mFirstFieldIsFid && index == 0 )
  {
    sql += ", " + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) ) + " AS fid2";
  }

  sql += " FROM " + quotedIdentifier( mOgrLayer->name() );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + textEncoding()->fromUnicode( mSubsetString );
  }

  sql += " ORDER BY " + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) ) + " ASC";

  QgsDebugMsgLevel( QStringLiteral( "SQL: %1" ).arg( textEncoding()->toUnicode( sql ) ), 2 );
  QgsOgrLayerUniquePtr l = mOgrLayer->ExecuteSQL( sql );
  if ( !l )
  {
    QgsDebugMsg( QStringLiteral( "Failed to execute SQL" ) );
    return QgsVectorDataProvider::uniqueValues( index, limit );
  }

  gdal::ogr_feature_unique_ptr f;
  bool ok = false;
  while ( f.reset( l->GetNextFeature() ), f )
  {
    const QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), fld, 0, textEncoding(), &ok );
    if ( ok )
      uniqueValues << res;

    if ( limit >= 0 && uniqueValues.size() >= limit )
      break;
  }

  return uniqueValues;
}

QStringList QgsOgrProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;

  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
    return results;

  QgsField fld = mAttributeFields.at( index );
  if ( fld.name().isNull() )
  {
    return results; //not a provider field
  }

  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  // uniqueStringsMatching() is supposed to be case insensitive, so use the
  // ILIKE operator when it is available.
  // Prior to GDAL 3.1, with OGR SQL, LIKE behaved like ILIKE
  bool supportsILIKE = false;
  {
    QByteArray sql = "SELECT 1 FROM ";
    sql += quotedIdentifier( mOgrLayer->name() );
    sql += " WHERE 'a' ILIKE 'A' LIMIT 1";
    QgsOgrLayerUniquePtr l = mOgrLayer->ExecuteSQL( sql );
    if ( l )
    {
      gdal::ogr_feature_unique_ptr f;
      f.reset( l->GetNextFeature() );
      supportsILIKE = f != nullptr;
    }
  }

  QByteArray sql = "SELECT DISTINCT " + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) );
  sql += " FROM " + quotedIdentifier( mOgrLayer->name() );

  sql += " WHERE " + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) );
  if ( supportsILIKE )
    sql += " ILIKE '%";
  else
    sql += " LIKE '%";
  sql += textEncoding()->fromUnicode( substring ) + "%'";

  if ( !mSubsetString.isEmpty() )
  {
    sql += " AND (" + textEncoding()->fromUnicode( mSubsetString ) + ')';
  }

  sql += " ORDER BY " + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) ) + " ASC";

  QgsDebugMsgLevel( QStringLiteral( "SQL: %1" ).arg( textEncoding()->toUnicode( sql ) ), 2 );
  QgsOgrLayerUniquePtr l = mOgrLayer->ExecuteSQL( sql );
  if ( !l )
  {
    QgsDebugMsg( QStringLiteral( "Failed to execute SQL" ) );
    return QgsVectorDataProvider::uniqueStringsMatching( index, substring, limit, feedback );
  }

  gdal::ogr_feature_unique_ptr f;
  while ( f.reset( l->GetNextFeature() ), f )
  {
    if ( OGR_F_IsFieldSetAndNotNull( f.get(), 0 ) )
      results << textEncoding()->toUnicode( OGR_F_GetFieldAsString( f.get(), 0 ) );

    if ( ( limit >= 0 && results.size() >= limit ) || ( feedback && feedback->isCanceled() ) )
      break;
  }

  return results;
}

QgsFeatureSource::SpatialIndexPresence QgsOgrProvider::hasSpatialIndex() const
{
  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  if ( mOgrLayer && mOgrLayer->TestCapability( OLCFastSpatialFilter ) )
    return QgsFeatureSource::SpatialIndexPresent;
  else if ( mOgrLayer )
    return QgsFeatureSource::SpatialIndexNotPresent;
  else
    return QgsFeatureSource::SpatialIndexUnknown;
}

Qgis::VectorLayerTypeFlags QgsOgrProvider::vectorLayerTypeFlags() const
{
  Qgis::VectorLayerTypeFlags flags;
  if ( mValid && mSubsetString.trimmed().startsWith( QStringLiteral( "SELECT" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    flags.setFlag( Qgis::VectorLayerTypeFlag::SqlQuery );
  }
  return flags;
}

QVariant QgsOgrProvider::minimumValue( int index ) const
{
  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
  {
    return QVariant();
  }

  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  const QgsField originalField = mAttributeFields.at( index );
  QgsField fld = originalField;

  // can't use native date/datetime types -- OGR converts these to string in the MAX return value
  if ( fld.type() == QVariant::DateTime || fld.type() == QVariant::Date )
  {
    fld.setType( QVariant::String );
  }

  // Don't quote column name (see https://trac.osgeo.org/gdal/ticket/5799#comment:9)
  QByteArray sql = "SELECT MIN(" + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) );
  sql += ") FROM " + quotedIdentifier( mOgrLayer->name() );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + textEncoding()->fromUnicode( mSubsetString );
  }

  QgsOgrLayerUniquePtr l = mOgrLayer->ExecuteSQL( sql );
  if ( !l )
  {
    QgsDebugMsg( QStringLiteral( "Failed to execute SQL: %1" ).arg( textEncoding()->toUnicode( sql ) ) );
    return QgsVectorDataProvider::minimumValue( index );
  }

  gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
  if ( !f )
  {
    return QVariant();
  }

  bool ok = false;
  QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), fld, 0, textEncoding(), &ok );
  if ( !ok )
    return QVariant();

  if ( res.type() != originalField.type() )
    res = convertValue( originalField.type(), res.toString() );

  if ( originalField.type() == QVariant::DateTime )
  {
    // ensure that we treat times as local time, to match behavior when iterating features
    QDateTime temp = res.toDateTime();
    temp.setTimeSpec( Qt::LocalTime );
    res = temp;
  }

  return res;
}

QVariant QgsOgrProvider::maximumValue( int index ) const
{
  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
  {
    return QVariant();
  }

  QgsCPLHTTPFetchOverrider oCPLHTTPFetcher( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( oCPLHTTPFetcher, QStringLiteral( "QgsOgrProvider" ) );

  const QgsField originalField = mAttributeFields.at( index );
  QgsField fld = originalField;

  // can't use native date/datetime types -- OGR converts these to string in the MAX return value
  if ( fld.type() == QVariant::DateTime || fld.type() == QVariant::Date )
  {
    fld.setType( QVariant::String );
  }

  // Don't quote column name (see https://trac.osgeo.org/gdal/ticket/5799#comment:9)
  QByteArray sql = "SELECT MAX(" + quotedIdentifier( textEncoding()->fromUnicode( fld.name() ) );
  sql += ") FROM " + quotedIdentifier( mOgrLayer->name() );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + textEncoding()->fromUnicode( mSubsetString );
  }

  QgsOgrLayerUniquePtr l = mOgrLayer->ExecuteSQL( sql );
  if ( !l )
  {
    QgsDebugMsg( QStringLiteral( "Failed to execute SQL: %1" ).arg( textEncoding()->toUnicode( sql ) ) );
    return QgsVectorDataProvider::maximumValue( index );
  }

  gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
  if ( !f )
  {
    return QVariant();
  }

  bool ok = false;
  QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), fld, 0, textEncoding(), &ok );
  if ( !ok )
    return QVariant();

  if ( res.type() != originalField.type() )
    res = convertValue( originalField.type(), res.toString() );

  if ( originalField.type() == QVariant::DateTime )
  {
    // ensure that we treat times as local time, to match behavior when iterating features
    QDateTime temp = res.toDateTime();
    temp.setTimeSpec( Qt::LocalTime );
    res = temp;
  }

  return res;
}

QByteArray QgsOgrProvider::quotedIdentifier( const QByteArray &field ) const
{
  return QgsOgrProviderUtils::quotedIdentifier( field, mGDALDriverName );
}

bool QgsOgrProvider::syncToDisc()
{
  QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );

  //for shapefiles, remove spatial index files and create a new index
  QgsOgrConnPool::instance()->unref( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  bool shapeIndex = false;
  if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
  {
    QString sbnIndexFile;
    QFileInfo fi( mFilePath );
    int suffixLength = fi.suffix().length();
    sbnIndexFile = mFilePath;
    sbnIndexFile.chop( suffixLength );
    sbnIndexFile.append( "sbn" );

    if ( QFile::exists( sbnIndexFile ) )
    {
      shapeIndex = true;
      close();
      QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
      QFile::remove( sbnIndexFile );
      open( OpenModeSameAsCurrent );
      if ( !mValid )
        return false;
    }
  }

  if ( mOgrLayer->SyncToDisk() != OGRERR_NONE )
  {
    pushError( tr( "OGR error syncing to disk: %1" ).arg( CPLGetLastErrorMsg() ) );
  }

  QgsOgrConnPool::instance()->ref( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  if ( shapeIndex )
  {
    return createSpatialIndex();
  }

  return true;
}

void QgsOgrProvider::recalculateFeatureCount() const
{
  if ( !mOgrLayer )
  {
    mFeaturesCounted = static_cast< long long >( Qgis::FeatureCountState::Uncounted );
    return;
  }

  OGRGeometryH filter = mOgrLayer->GetSpatialFilter();
  if ( filter )
  {
    filter = OGR_G_Clone( filter );
    mOgrLayer->SetSpatialFilter( nullptr );
  }

  // feature count returns number of features within current spatial filter
  // so we remove it if there's any and then put it back
  if ( mOgrGeometryTypeFilter == wkbUnknown )
  {
    mFeaturesCounted = mOgrLayer->GetApproxFeatureCount();
    if ( mFeaturesCounted == -1 )
    {
      mFeaturesCounted = static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
    }
  }
  else
  {
    mFeaturesCounted = 0;
    setRelevantFields( true, QgsAttributeList() );
    mOgrLayer->ResetReading();
    gdal::ogr_feature_unique_ptr fet;
    const OGRwkbGeometryType flattenGeomTypeFilter =
      QgsOgrProviderUtils::ogrWkbSingleFlatten( mOgrGeometryTypeFilter );
    while ( fet.reset( mOgrLayer->GetNextFeature() ), fet )
    {
      OGRGeometryH geom = OGR_F_GetGeometryRef( fet.get() );
      if ( geom )
      {
        OGRwkbGeometryType gType = OGR_G_GetGeometryType( geom );
        gType = QgsOgrProviderUtils::ogrWkbSingleFlatten( gType );
        if ( gType == flattenGeomTypeFilter ) mFeaturesCounted++;
      }
    }
    mOgrLayer->ResetReading();
    setRelevantFields( true, attributeIndexes() );
  }

  if ( filter )
  {
    mOgrLayer->SetSpatialFilter( filter );
  }

  QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
}

bool QgsOgrProvider::doesStrictFeatureTypeCheck() const
{
  // FIXME probably other drivers too...
  return mGDALDriverName != QLatin1String( "ESRI Shapefile" ) || ( mOGRGeomType == wkbPoint || mOGRGeomType == wkbPoint25D );
}

QgsFeatureRenderer *QgsOgrProvider::createRenderer( const QVariantMap & ) const
{
  if ( !( mCapabilities & FeatureSymbology ) )
    return nullptr;

  std::unique_ptr< QgsSymbol > defaultSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::geometryType( wkbType() ) ) );
  return new QgsEmbeddedSymbolRenderer( defaultSymbol.release() );
}

void QgsOgrProvider::open( OpenMode mode )
{
  bool openReadOnly = false;
  Q_ASSERT( !mOgrSqlLayer );
  Q_ASSERT( !mOgrOrigLayer );

  // Try to open using VSIFileHandler
  //   see http://trac.osgeo.org/gdal/wiki/UserDocs/ReadInZip
  QString vsiPrefix = QgsZipItem::vsiPrefix( dataSourceUri( true ) );
  if ( !vsiPrefix.isEmpty() || mFilePath.startsWith( QLatin1String( "/vsicurl/" ) ) )
  {
    // GDAL>=1.8.0 has write support for zip, but read and write operations
    // cannot be interleaved, so for now just use read-only.
    openReadOnly = true;
    if ( !mFilePath.startsWith( vsiPrefix ) )
    {
      mFilePath = vsiPrefix + mFilePath;
      setDataSourceUri( mFilePath );
    }
    QgsDebugMsgLevel( QStringLiteral( "Trying %1 syntax, mFilePath= %2" ).arg( vsiPrefix, mFilePath ), 2 );
  }

  QgsDebugMsgLevel( "mFilePath: " + mFilePath, 3 );
  QgsDebugMsgLevel( "mLayerIndex: " + QString::number( mLayerIndex ), 3 );
  QgsDebugMsgLevel( "mLayerName: " + mLayerName, 3 );
  QgsDebugMsgLevel( "mSubsetString: " + mSubsetString, 3 );
  CPLSetConfigOption( "GPX_ELE_AS_25D", "YES" );  // use GPX elevation as z values
  CPLSetConfigOption( "LIBKML_RESOLVE_STYLE", "YES" );  // resolve kml style urls from style tables to feature style strings
  if ( !CPLGetConfigOption( "OSM_USE_CUSTOM_INDEXING", nullptr ) )
  {
    // Disable custom/fast indexing by default, as it can prevent some .osm.pbf
    // files to be loaded.
    // See https://github.com/qgis/QGIS/issues/31062
    CPLSetConfigOption( "OSM_USE_CUSTOM_INDEXING", "NO" );
  }

  if ( mFilePath.startsWith( QLatin1String( "MySQL:" ) ) && !mLayerName.isEmpty() && !mFilePath.endsWith( ",tables=" + mLayerName ) )
  {
    mFilePath += ",tables=" + mLayerName;
  }

  if ( mode == OpenModeForceReadOnly )
    openReadOnly = true;
  else if ( mode == OpenModeSameAsCurrent && !mWriteAccess )
    openReadOnly = true;

  const bool bIsGpkg = QFileInfo( mFilePath ).suffix().compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0;

  // first try to open in update mode (unless specified otherwise)
  QString errCause;
  if ( !openReadOnly )
  {
    QStringList options( mOpenOptions );
    if ( !bIsGpkg && ( mode == OpenModeForceUpdateRepackOff || ( mDeferRepack && OpenModeSameAsCurrent ) ) )
    {
      options << "AUTO_REPACK=OFF";
    }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,2)
    if ( bIsGpkg && mode == OpenModeInitial )
    {
      // A hint to QgsOgrProviderUtils::GDALOpenWrapper() to not force WAL
      // as in OpenModeInitial we are not going to do anything besides getting capabilities
      // and re-opening in readonly mode.
      options << "DO_NOT_ENABLE_WAL=ON";
    }
#endif

    // We get the layer which was requested by the uri. The layername
    // has precedence over the layerid if both are given.
    if ( !mLayerName.isNull() )
    {
      mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, true, options, mLayerName, errCause, true );
    }
    else
    {
      mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, true, options, mLayerIndex, errCause, true );
    }
  }

  mValid = false;
  if ( mOgrOrigLayer )
  {
    mWriteAccess = true;
    mWriteAccessPossible = true;
  }
  else
  {
    mWriteAccess = false;
    if ( !openReadOnly )
    {
      QgsDebugMsg( QStringLiteral( "OGR failed to opened in update mode, trying in read-only mode" ) );
    }

    QStringList options( mOpenOptions );
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
    // assume trusted data to get more speed
    if ( mGDALDriverName == QLatin1String( "FlatGeobuf" ) &&
         !options.contains( QStringLiteral( "VERIFY_BUFFERS=YES" ) ) )
    {
      options << QStringLiteral( "VERIFY_BUFFERS=NO" );
    }
#endif

    // try to open read-only
    if ( !mLayerName.isNull() )
    {
      mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, false, options, mLayerName, errCause, true );
    }
    else
    {
      mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, false, options, mLayerIndex, errCause, true );
    }
  }

  if ( mOgrOrigLayer )
  {
    mGDALDriverName = mOgrOrigLayer->driverName();
    mShareSameDatasetAmongLayers = QgsOgrProviderUtils::canDriverShareSameDatasetAmongLayers( mGDALDriverName );

    QgsDebugMsgLevel( "OGR opened using Driver " + mGDALDriverName, 2 );

    mOgrLayer = mOgrOrigLayer.get();

    // check that the initial encoding setting is fit for this layer

    if ( mode == OpenModeInitial && mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
    {
      // determine encoding from shapefile cpg or LDID information, if possible
      QString shpEncoding;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
      shpEncoding = mOgrLayer->GetMetadataItem( QStringLiteral( "ENCODING_FROM_CPG" ), QStringLiteral( "SHAPEFILE" ) );
      if ( shpEncoding.isEmpty() )
        shpEncoding = mOgrLayer->GetMetadataItem( QStringLiteral( "ENCODING_FROM_LDID" ), QStringLiteral( "SHAPEFILE" ) );
#else
      shpEncoding = QgsOgrUtils::readShapefileEncoding( mFilePath );
#endif

      if ( !shpEncoding.isEmpty() )
        setEncoding( shpEncoding );
      else
        setEncoding( encoding() );
    }
    else
    {
      setEncoding( encoding() );
    }

    // Ensure subset is set (setSubsetString does nothing if the passed sql subset string is equal to mSubsetString, which is the case when reloading the dataset)
    QString origSubsetString = mSubsetString;
    mSubsetString.clear();
    // Block signals to avoid endless recursion reloadData -> emit dataChanged -> reloadData
    blockSignals( true );

    // Do not update capabilities: it will be done later

    // WARNING if this is the initial open - we don't already have a connection ref, and will be creating one later. So we *mustn't* grab an extra connection ref
    // while setting the subset string, or we'll be left with an extra reference which is never cleared.
    mValid = _setSubsetString( origSubsetString, false, false, mode != OpenModeInitial );

    blockSignals( false );
    if ( mValid )
    {
      if ( mode == OpenModeInitial )
      {
        computeCapabilities();
      }
      QgsDebugMsgLevel( QStringLiteral( "Data source is valid" ), 2 );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Data source is invalid (%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ), tr( "OGR" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( errCause + "(" + QString::fromUtf8( CPLGetLastErrorMsg() ) + ")", tr( "OGR" ) );
  }

  // For shapefiles or MapInfo .tab, so as to allow concurrent opening between
  // QGIS and MapInfo, we go back to read-only mode for now.
  // For GPKG too, so to open in read-only nolock mode for GDAL >= 3.4.2
  // We limit to those drivers as re-opening is relatively cheap (other drivers
  // like GeoJSON might do full content ingestion for example)
  if ( mValid && mode == OpenModeInitial && mWriteAccess &&
       ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) ||
         mGDALDriverName == QLatin1String( "MapInfo File" )
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,2)
         || mGDALDriverName == QLatin1String( "GPKG" )
#endif
       ) )
  {
    mOgrSqlLayer.reset();
    mOgrOrigLayer.reset();
    mOgrLayer = nullptr;
    mValid = false;

    // In the case where we deal with a shapefile, it is possible that it has
    // pre-existing holes in the DBF (see #15407), so do a packing at the first edit
    // action.
    if ( mGDALDriverName == QLatin1String( "ESRI Shapefile" ) )
    {
      mShapefileMayBeCorrupted = true;
    }

    // try to open read-only
    if ( !mLayerName.isNull() )
    {
      mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, false, mOpenOptions,  mLayerName, errCause, true );
    }
    else
    {
      mOgrOrigLayer = QgsOgrProviderUtils::getLayer( mFilePath, false, mOpenOptions, mLayerIndex, errCause, true );
    }

    mWriteAccess = false;
    mOgrLayer = mOgrOrigLayer.get();
    if ( mOgrLayer )
    {
      mValid = true;
      mDynamicWriteAccess = true;

      if ( !mSubsetString.isEmpty() )
      {
        // Do not update capabilities here
        // but ensure subset is set (setSubsetString does nothing if the passed sql subset string is equal to
        // mSubsetString, which is the case when reloading the dataset)
        QString origSubsetString = mSubsetString;
        mSubsetString.clear();
        mValid = _setSubsetString( origSubsetString, false, false );
      }
    }
  }

  // For debug/testing purposes
  if ( !mValid )
    setProperty( "_debug_open_mode", "invalid" );
  else if ( mWriteAccess )
    setProperty( "_debug_open_mode", "read-write" );
  else
    setProperty( "_debug_open_mode", "read-only" );

  mRefreshFeatureCount = true;
}

void QgsOgrProvider::close()
{
  if ( mWriteAccess && mForceRecomputeExtent )
    extent();

  mOgrSqlLayer.reset();
  mOgrOrigLayer.reset();
  mOgrLayer = nullptr;
  mValid = false;
  setProperty( "_debug_open_mode", "invalid" );

  invalidateCachedExtent( false );
}

void QgsOgrProvider::invalidateNetworkCache()
{
  if ( mFilePath.startsWith( QLatin1String( "/vsicurl/" ) )  ||
       mFilePath.startsWith( QLatin1String( "/vsis3/" ) ) ||
       mFilePath.startsWith( QLatin1String( "/vsigs/" ) ) ||
       mFilePath.startsWith( QLatin1String( "/vsiaz/" ) ) )
  {
    QgsDebugMsgLevel( QString( "Invalidating cache for %1" ).arg( mFilePath ), 3 );
    VSICurlPartialClearCache( mFilePath.toUtf8().constData() );
  }
}

void QgsOgrProvider::reloadProviderData()
{
  invalidateNetworkCache();
  mFeaturesCounted = static_cast< long long >( Qgis::FeatureCountState::Uncounted );
  bool wasValid = mValid;
  QgsOgrConnPool::instance()->invalidateConnections( QgsOgrProviderUtils::connectionPoolId( dataSourceUri( true ), mShareSameDatasetAmongLayers ) );
  close();
  open( OpenModeSameAsCurrent );
  if ( !mValid && wasValid )
    pushError( tr( "Cannot reopen datasource %1" ).arg( dataSourceUri() ) );
}

bool QgsOgrProvider::_enterUpdateMode( bool implicit )
{
  if ( !mWriteAccessPossible )
  {
    return false;
  }
  if ( mWriteAccess )
  {
    ++mUpdateModeStackDepth;
    return true;
  }
  if ( mUpdateModeStackDepth == 0 )
  {
    Q_ASSERT( mDynamicWriteAccess );
    QgsDebugMsgLevel( QStringLiteral( "Reopening %1 in update mode" ).arg( dataSourceUri() ), 2 );
    close();
    open( implicit ? OpenModeForceUpdate : OpenModeForceUpdateRepackOff );
    if ( !mOgrLayer || !mWriteAccess )
    {
      QgsMessageLog::logMessage( tr( "Cannot reopen datasource %1 in update mode" ).arg( dataSourceUri() ), tr( "OGR" ) );
      pushError( tr( "Cannot reopen datasource %1 in update mode" ).arg( dataSourceUri() ) );
      return false;
    }
  }
  ++mUpdateModeStackDepth;
  // For implicitly entered updateMode, don't defer repacking
  mDeferRepack = !implicit;
  return true;
}

bool QgsOgrProvider::leaveUpdateMode()
{
  if ( !mWriteAccessPossible )
  {
    return false;
  }
  --mUpdateModeStackDepth;
  if ( mUpdateModeStackDepth < 0 )
  {
    QgsMessageLog::logMessage( tr( "Unbalanced call to leaveUpdateMode() w.r.t. enterUpdateMode()" ), tr( "OGR" ) );
    mUpdateModeStackDepth = 0;
    return false;
  }
  if ( mDeferRepack && mUpdateModeStackDepth == 0 )
  {
    // Only repack once update mode is inactive
    if ( mShapefileMayBeCorrupted )
      repack();

    mShapefileMayBeCorrupted = false;
    mDeferRepack = false;
  }
  if ( !mDynamicWriteAccess )
  {
    // The GeoJSON driver only properly flushes stuff in all situations by
    // closing and re-opening. Starting with GDAL 2.3.1, it should be safe to
    // use GDALDatasetFlush().
    if ( mGDALDriverName == QLatin1String( "GeoJSON" ) )
    {
      // Backup fields since if we created new fields, but didn't populate it
      // with any feature yet, it will disappear.
      const QgsFields oldFields = mAttributeFields;
      reloadData();
      if ( mValid )
      {
        // Make sure that new fields we added, but didn't populate yet, are
        // recreated at the OGR level, otherwise we won't be able to populate
        // them.
        for ( const auto &field : oldFields )
        {
          int idx = mAttributeFields.lookupField( field.name() );
          if ( idx < 0 )
          {
            bool ignoreErrorOut = false;
            addAttributeOGRLevel( field, ignoreErrorOut );
            mAttributeFields.append( field );
          }
        }
      }
    }
    return true;
  }
  if ( mUpdateModeStackDepth == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Reopening %1 in read-only mode" ).arg( dataSourceUri() ), 2 );
    close();
    open( OpenModeForceReadOnly );
    if ( !mOgrLayer )
    {
      QgsMessageLog::logMessage( tr( "Cannot reopen datasource %1 in read-only mode" ).arg( dataSourceUri() ), tr( "OGR" ) );
      pushError( tr( "Cannot reopen datasource %1 in read-only mode" ).arg( dataSourceUri() ) );
      return false;
    }
  }
  return true;
}

bool QgsOgrProvider::isSaveAndLoadStyleToDatabaseSupported() const
{
  // We could potentially extend support for styling to other drivers
  // with multiple layer support.
  return mGDALDriverName == QLatin1String( "GPKG" ) ||
         mGDALDriverName == QLatin1String( "SQLite" );
}

bool QgsOgrProvider::isDeleteStyleFromDatabaseSupported() const
{
  return isSaveAndLoadStyleToDatabaseSupported();
}

QString QgsOgrProvider::fileVectorFilters() const
{
  return QgsOgrProviderUtils::fileVectorFilters();
}

///@endcond
