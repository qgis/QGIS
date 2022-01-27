/***************************************************************************
                     qgsogrprovidermetadata.cpp
begin                : June 2021
copyright            : (C) 2021 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrprovidermetadata.h"
#include "qgsogrprovider.h"
#include "qgsgeopackagedataitems.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgsogrtransaction.h"
#include "qgsgeopackageprojectstorage.h"
#include "qgsapplication.h"
#include "qgsogrconnpool.h"
#include "qgsprojectstorageregistry.h"
#include "qgsgeopackageproviderconnection.h"
#include "qgsogrdbconnection.h"
#include "qgsprovidersublayerdetails.h"
#include "qgszipitem.h"
#include "qgsproviderutils.h"
#include "qgsgdalutils.h"
#include "qgsproviderregistry.h"

#include <gdal.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QDirIterator>

///@cond PRIVATE


#define TEXT_PROVIDER_KEY QStringLiteral( "ogr" )
#define TEXT_PROVIDER_DESCRIPTION QStringLiteral( "OGR data provider" )

QgsDataProvider *QgsOgrProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
{
  return new QgsOgrProvider( uri, options, flags );
}

Qgis::VectorExportResult QgsOgrProviderMetadata::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> &oldToNewAttrIdxMap,
    QString &errorMessage,
    const QMap<QString, QVariant> *options )
{
  return QgsOgrProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           &oldToNewAttrIdxMap, &errorMessage, options
         );
}

QVariantMap QgsOgrProviderMetadata::decodeUri( const QString &uri ) const
{
  QString path = uri;
  QString layerName;
  QString subset;
  QString geometryType;
  QStringList openOptions;
  QString databaseName;
  QString authcfg;

  int layerId = -1;

  const QRegularExpression authcfgRegex( " authcfg='([^']+)'" );
  QRegularExpressionMatch match;
  if ( path.contains( authcfgRegex, &match ) )
  {
    path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    authcfg = match.captured( 1 );
  }

  QString vsiPrefix = qgsVsiPrefix( path );
  QString vsiSuffix;
  if ( path.startsWith( vsiPrefix, Qt::CaseInsensitive ) )
  {
    path = path.mid( vsiPrefix.count() );

    const QRegularExpression vsiRegex( QStringLiteral( "(?:\\.zip|\\.tar|\\.gz|\\.tar\\.gz|\\.tgz)([^|]+)" ) );
    QRegularExpressionMatch match = vsiRegex.match( path );
    if ( match.hasMatch() )
    {
      vsiSuffix = match.captured( 1 );
      path = path.remove( match.capturedStart( 1 ), match.capturedLength( 1 ) );
    }
  }
  else
  {
    vsiPrefix.clear();
  }

  if ( path.contains( '|' ) )
  {
    const QRegularExpression geometryTypeRegex( QStringLiteral( "\\|geometrytype=([a-zA-Z0-9]*)" ), QRegularExpression::PatternOption::CaseInsensitiveOption );
    const QRegularExpression layerNameRegex( QStringLiteral( "\\|layername=([^|]*)" ), QRegularExpression::PatternOption::CaseInsensitiveOption );
    const QRegularExpression layerIdRegex( QStringLiteral( "\\|layerid=([^|]*)" ), QRegularExpression::PatternOption::CaseInsensitiveOption );
    const QRegularExpression subsetRegex( QStringLiteral( "\\|subset=((?:.*[\r\n]*)*)\\Z" ) );
    const QRegularExpression openOptionRegex( QStringLiteral( "\\|option:([^|]*)" ) );


    // we first try to split off the geometry type component, if that's present. That's a known quantity which
    // will never be more than a-z characters
    match = geometryTypeRegex.match( path );
    if ( match.hasMatch() )
    {
      geometryType = match.captured( 1 );
      path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    }

    // next, we try to find and strip out the layerid/layername component. (This logic is based on the assumption
    // that a layer name doesn't contain a | character!)
    // we prefer layer names over layer ids, since they are persistent..
    match = layerNameRegex.match( path );
    if ( match.hasMatch() )
    {
      layerName = match.captured( 1 );
      path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    }

    match = layerIdRegex.match( path );
    if ( match.hasMatch() )
    {
      layerId = match.captured( 1 ).toInt();
      path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    }

    while ( true )
    {
      match = openOptionRegex.match( path );
      if ( match.hasMatch() )
      {
        openOptions << match.captured( 1 );
        path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
      }
      else
      {
        break;
      }
    }

    // lastly, try to parse out the subset component. This is the biggest unknown, because it's
    // quite possible that a subset string will contain a | character. If we've already parsed
    // out all the other known |xxx=yyy tags, then we can safely assume that everything from "|subset=" to the
    // end of the path is part of the subset filter
    match = subsetRegex.match( path );
    if ( match.hasMatch() )
    {
      subset = match.captured( 1 );
      path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    }
  }

  // Handles DB connections extracting database name if possible
  // Example: MySQL:database_name,host=localhost,port=3306 authcfg='f8wwfx8'
  if ( uri.startsWith( QStringLiteral( "MySQL" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "PostgreSQL" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "MSSQL" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "ODBC" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "PGeo" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "SDE" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "OGDI" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "Ingres" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "IDB" ), Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( QStringLiteral( "OCI" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    auto parts( path.split( ':' ) );
    if ( parts.count( ) > 1 )
    {
      auto dataParts( parts.at( 1 ).split( ',' ) );
      if ( dataParts.count() > 0 )
        databaseName = dataParts.at( 0 );
    }
  }

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  uriComponents.insert( QStringLiteral( "layerName" ), layerName );
  uriComponents.insert( QStringLiteral( "layerId" ), layerId > -1 && layerName.isEmpty() ? layerId : QVariant() ) ;
  if ( !subset.isEmpty() )
    uriComponents.insert( QStringLiteral( "subset" ), subset );
  if ( !geometryType.isEmpty() )
    uriComponents.insert( QStringLiteral( "geometryType" ), geometryType );
  if ( !databaseName.isEmpty() )
    uriComponents.insert( QStringLiteral( "databaseName" ), databaseName );
  if ( !openOptions.isEmpty() )
    uriComponents.insert( QStringLiteral( "openOptions" ), openOptions );
  if ( !vsiPrefix.isEmpty() )
    uriComponents.insert( QStringLiteral( "vsiPrefix" ), vsiPrefix );
  if ( !vsiSuffix.isEmpty() )
    uriComponents.insert( QStringLiteral( "vsiSuffix" ), vsiSuffix );
  if ( !authcfg.isEmpty() )
    uriComponents.insert( QStringLiteral( "authcfg" ), authcfg );
  return uriComponents;
}

QString QgsOgrProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString vsiPrefix = parts.value( QStringLiteral( "vsiPrefix" ) ).toString();
  const QString vsiSuffix = parts.value( QStringLiteral( "vsiSuffix" ) ).toString();
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  const QString layerName = parts.value( QStringLiteral( "layerName" ) ).toString();
  const QString layerId = parts.value( QStringLiteral( "layerId" ) ).toString();
  const QString subset = parts.value( QStringLiteral( "subset" ) ).toString();
  const QString geometryType = parts.value( QStringLiteral( "geometryType" ) ).toString();
  const QString authcfg = parts.value( QStringLiteral( "authcfg" ) ).toString();
  const QStringList openOptions = parts.value( QStringLiteral( "openOptions" ) ).toStringList();

  QString uri = vsiPrefix + path + vsiSuffix
                + ( !layerName.isEmpty() ? QStringLiteral( "|layername=%1" ).arg( layerName ) : !layerId.isEmpty() ? QStringLiteral( "|layerid=%1" ).arg( layerId ) : QString() )
                + ( !geometryType.isEmpty() ? QStringLiteral( "|geometrytype=%1" ).arg( geometryType ) : QString() );
  for ( const QString &openOption : openOptions )
  {
    uri += QLatin1String( "|option:" );
    uri += openOption;
  }
  if ( !subset.isEmpty() )
    uri += QStringLiteral( "|subset=%1" ).arg( subset );
  if ( !authcfg.isEmpty() )
    uri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );
  return uri;
}

QList<QgsDataItemProvider *> QgsOgrProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsGeoPackageDataItemProvider;
  return providers;
}

static QgsOgrLayerUniquePtr LoadDataSourceAndLayer( const QString &uri, QString &errCause )
{
  bool isSubLayer;
  int layerIndex;
  QString layerName;
  QString subsetString;
  OGRwkbGeometryType ogrGeometryType;
  QStringList openOptions;
  QString filePath = QgsOgrProviderUtils::analyzeURI( uri,
                     isSubLayer,
                     layerIndex,
                     layerName,
                     subsetString,
                     ogrGeometryType,
                     openOptions );

  if ( !layerName.isEmpty() )
  {
    return QgsOgrProviderUtils::getLayer( filePath, true, QStringList(), layerName, errCause, true );
  }
  else
  {
    return QgsOgrProviderUtils::getLayer( filePath, true, QStringList(), layerIndex, errCause, true );
  }
}

bool QgsOgrProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  errorCause.clear();

  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, errorCause );
  if ( !userLayer )
    return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex = nullptr;
#else
  QRecursiveMutex *mutex = nullptr;
#endif
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex );
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

  // check if layer_styles table exists
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
    return false;

  const QString realStyleId = styleId.isEmpty() ? QString( OGR_L_GetName( hUserLayer ) ) : styleId;

  const QString checkQuery = QStringLiteral( "f_table_schema=''"
                             " AND f_table_name=%1"
                             " AND f_geometry_column=%2"
                             " AND styleName=%3" )
                             .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetName( hUserLayer ) ) ),
                                   QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetGeometryColumn( hUserLayer ) ) ),
                                   QgsOgrProviderUtils::quotedValue( realStyleId ) );
  OGR_L_SetAttributeFilter( hLayer, checkQuery.toUtf8().constData() );
  OGR_L_ResetReading( hLayer );
  gdal::ogr_feature_unique_ptr hFeature( OGR_L_GetNextFeature( hLayer ) );
  OGR_L_ResetReading( hLayer );

  if ( hFeature )
    return true;

  return false;
}

bool QgsOgrProviderMetadata::saveStyle(
  const QString &uri, const QString &qmlStyle, const QString &sldStyle,
  const QString &styleName, const QString &styleDescription,
  const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, errCause );
  if ( !userLayer )
    return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex = nullptr;
#else
  QRecursiveMutex *mutex = nullptr;
#endif
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex );
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

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
    styleName.isEmpty() ? QString( OGR_L_GetName( hUserLayer ) ) : styleName;

  OGRFeatureDefnH hLayerDefn = OGR_L_GetLayerDefn( hLayer );

  if ( useAsDefault )
  {
    QString oldDefaultQuery = QStringLiteral( "useAsDefault = 1 AND f_table_schema=''"
                              " AND f_table_name=%1"
                              " AND f_geometry_column=%2" )
                              .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetName( hUserLayer ) ) ) )
                              .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetGeometryColumn( hUserLayer ) ) ) );
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
        QgsDebugMsg( QStringLiteral( "Could not unset previous useAsDefault style" ) );
      }
    }
  }

  QString checkQuery = QStringLiteral( "f_table_schema=''"
                                       " AND f_table_name=%1"
                                       " AND f_geometry_column=%2"
                                       " AND styleName=%3" )
                       .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetName( hUserLayer ) ) ) )
                       .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetGeometryColumn( hUserLayer ) ) ) )
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
                          OGR_L_GetName( hUserLayer ) );
    OGR_F_SetFieldString( hFeature.get(),
                          OGR_FD_GetFieldIndex( hLayerDefn, "f_geometry_column" ),
                          OGR_L_GetGeometryColumn( hUserLayer ) );
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

bool QgsOgrProviderMetadata::deleteStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  bool deleted;

  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, errCause );
  if ( !userLayer )
    return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex = nullptr;
#else
  QRecursiveMutex *mutex = nullptr;
#endif
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

  // check if layer_styles table already exist
  OGRLayerH hLayer = GDALDatasetGetLayerByName( hDS, "layer_styles" );
  if ( !hLayer )
  {
    errCause = QObject::tr( "Connection to database failed: %1" ).arg( dsUri.uri() );
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

static
bool LoadDataSourceLayerStylesAndLayer( const QString &uri,
                                        QgsOgrLayerUniquePtr &layerStyles,
                                        QgsOgrLayerUniquePtr &userLayer,
                                        QString &errCause )
{
  bool isSubLayer;
  int layerIndex;
  QString layerName;
  QString subsetString;
  OGRwkbGeometryType ogrGeometryType;
  QStringList openOptions;
  QString filePath = QgsOgrProviderUtils::analyzeURI( uri,
                     isSubLayer,
                     layerIndex,
                     layerName,
                     subsetString,
                     ogrGeometryType,
                     openOptions );

  layerStyles =
    QgsOgrProviderUtils::getLayer( filePath, "layer_styles", errCause );
  userLayer = nullptr;
  if ( !layerStyles )
  {
    errCause = QObject::tr( "Cannot find layer_styles layer" );
    return false;
  }

  if ( !layerName.isEmpty() )
  {
    userLayer = QgsOgrProviderUtils::getLayer( filePath, layerName, errCause );
  }
  else
  {
    userLayer = QgsOgrProviderUtils::getLayer( filePath, layerIndex, errCause );
  }
  if ( !userLayer )
  {
    layerStyles.reset();
    return false;
  }
  return true;
}


QString QgsOgrProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QgsOgrLayerUniquePtr layerStyles;
  QgsOgrLayerUniquePtr userLayer;
  if ( !LoadDataSourceLayerStylesAndLayer( uri, layerStyles, userLayer, errCause ) )
  {
    return QString();
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex1 = nullptr;
  QMutex *mutex2 = nullptr;
#else
  QRecursiveMutex *mutex1 = nullptr;
  QRecursiveMutex *mutex2 = nullptr;
#endif
  OGRLayerH hLayer = layerStyles->getHandleAndMutex( mutex1 );
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex2 );
  QMutexLocker lock1( mutex1 );
  QMutexLocker lock2( mutex2 );

  QString selectQmlQuery = QStringLiteral( "f_table_schema=''"
                           " AND f_table_name=%1"
                           " AND f_geometry_column=%2"
                           " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                           ",update_time DESC LIMIT 1" )
                           .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetName( hUserLayer ) ) ) )
                           .arg( QgsOgrProviderUtils::quotedValue( QString( OGR_L_GetGeometryColumn( hUserLayer ) ) ) );
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

    }
  }
  OGR_L_ResetReading( hLayer );

  return styleQML;
}

int QgsOgrProviderMetadata::listStyles(
  const QString &uri, QStringList &ids, QStringList &names,
  QStringList &descriptions, QString &errCause )
{
  bool isSubLayer;
  int layerIndex;
  QString layerName;
  QString subsetString;
  OGRwkbGeometryType ogrGeometryType;
  QStringList openOptions;
  QString filePath = QgsOgrProviderUtils::analyzeURI( uri,
                     isSubLayer,
                     layerIndex,
                     layerName,
                     subsetString,
                     ogrGeometryType,
                     openOptions );

  QgsOgrLayerUniquePtr userLayer;
  if ( !layerName.isEmpty() )
  {
    userLayer = QgsOgrProviderUtils::getLayer( filePath, layerName, errCause );
  }
  else
  {
    userLayer = QgsOgrProviderUtils::getLayer( filePath, layerIndex, errCause );
  }
  if ( !userLayer )
  {
    return -1;
  }

  QgsOgrLayerUniquePtr layerStyles =
    QgsOgrProviderUtils::getLayer( filePath, "layer_styles", errCause );
  if ( !layerStyles )
  {
    QgsMessageLog::logMessage( QObject::tr( "No styles available on DB" ) );
    errCause = QObject::tr( "No styles available on DB" );
    return 0;
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex1 = nullptr;
  QMutex *mutex2 = nullptr;
#else
  QRecursiveMutex *mutex1 = nullptr;
  QRecursiveMutex *mutex2 = nullptr;
#endif

  OGRLayerH hLayer = layerStyles->getHandleAndMutex( mutex1 );
  QMutexLocker lock1( mutex1 );
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex2 );
  QMutexLocker lock2( mutex2 );

  if ( OGR_L_GetFeatureCount( hLayer, TRUE ) == 0 )
  {
    QgsMessageLog::logMessage( QObject::tr( "No styles available on DB" ) );
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
    if ( tableName == QString::fromUtf8( OGR_L_GetName( hUserLayer ) ) &&
         geometryColumn == QString::fromUtf8( OGR_L_GetGeometryColumn( hUserLayer ) ) )
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
      qlonglong ts = second + minute * 60 + hour * 3600 + day * 24 * 3600 +
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

QString QgsOgrProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsOgrLayerUniquePtr layerStyles;
  QgsOgrLayerUniquePtr userLayer;
  if ( !LoadDataSourceLayerStylesAndLayer( uri, layerStyles, userLayer, errCause ) )
  {
    return QString();
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex1 = nullptr;
#else
  QRecursiveMutex *mutex1 = nullptr;
#endif

  OGRLayerH hLayer = layerStyles->getHandleAndMutex( mutex1 );
  QMutexLocker lock1( mutex1 );

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

bool QgsOgrProviderMetadata::saveLayerMetadata( const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage )
{
  const QVariantMap parts = decodeUri( uri );
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  if ( !path.isEmpty() && QFileInfo::exists( path ) )
  {
    // export metadata to XML
    QDomImplementation domImplementation;
    QDomDocumentType documentType = domImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
    QDomDocument document( documentType );

    QDomElement rootNode = document.createElement( QStringLiteral( "qgis" ) );
    rootNode.setAttribute( QStringLiteral( "version" ), Qgis::version() );
    document.appendChild( rootNode );

    if ( !metadata.writeMetadataXml( rootNode, document ) )
    {
      errorMessage = QObject::tr( "Error exporting metadata to XML" );
      return false;
    }

    QString metadataXml;
    QTextStream textStream( &metadataXml );
    document.save( textStream, 2 );

    QFileInfo fi( path );
    if ( fi.suffix().compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0 )
    {
      const QString layerName = parts.value( QStringLiteral( "layerName" ) ).toString();
      QgsOgrLayerUniquePtr userLayer;
      userLayer = QgsOgrProviderUtils::getLayer( path, true, QStringList(), layerName, errorMessage, true );
      if ( !userLayer )
        return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      QMutex *mutex = nullptr;
#else
      QRecursiveMutex *mutex = nullptr;
#endif
      // Returns native OGRLayerH object with the mutex to lock when using it
      OGRLayerH hLayer = userLayer->getHandleAndMutex( mutex );
      QMutexLocker locker( mutex );

      // These are special keys which get stored into the gpkg_contents table
      if ( !metadata.abstract().isEmpty() )
        GDALSetMetadataItem( hLayer, "DESCRIPTION", metadata.abstract().toUtf8().constData(), nullptr );
      if ( !metadata.identifier().isEmpty() )
        GDALSetMetadataItem( hLayer, "IDENTIFIER", metadata.identifier().toUtf8().constData(), nullptr );

      // we write a simple piece of GDAL metadata too -- this is solely to delegate responsibility of
      // creating all the metadata tables to GDAL! We will remove it once done.
      if ( GDALSetMetadataItem( hLayer, "QGIS_VERSION", Qgis::version().toLocal8Bit().constData(), nullptr ) == CE_None )
      {
        // so far so good, ready to throw the whole of the QGIS layer XML into the metadata table!

        // first we need to check if there's already a corresponding entry in gpkg_metadata -- if so, we need to update it.
        QString sql = QStringLiteral( "SELECT id from gpkg_metadata LEFT JOIN gpkg_metadata_reference ON "
                                      "(gpkg_metadata_reference.table_name = %1 AND gpkg_metadata.id = gpkg_metadata_reference.md_file_id) "
                                      "WHERE md_standard_uri = %2 and reference_scope = %3" ).arg(
                        QgsSqliteUtils::quotedString( layerName ),
                        QgsSqliteUtils::quotedString( QStringLiteral( "http://mrcc.com/qgis.dtd" ) ),
                        QgsSqliteUtils::quotedString( QStringLiteral( "table" ) ) );
        int existingRowId = -1;
        if ( QgsOgrLayerUniquePtr l = userLayer->ExecuteSQL( sql.toLocal8Bit().constData() ) )
        {
          // retrieve inserted row id
          gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
          if ( f )
          {
            bool ok = false;
            QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), QgsField( QString(), QVariant::String ), 0, nullptr, &ok );
            if ( ok )
            {
              existingRowId = res.toInt( &ok );
              if ( !ok )
                existingRowId = -1;
            }
          }
        }

        if ( existingRowId >= 0 )
        {
          // update existing row
          sql = QStringLiteral( "UPDATE gpkg_metadata SET metadata=%1 where id=%2;" ).arg(
                  QgsSqliteUtils::quotedString( metadataXml ) ).arg( existingRowId );
          userLayer->ExecuteSQLNoReturn( sql.toLocal8Bit().constData() );
          if ( CPLGetLastErrorType() != CE_None )
          {
            errorMessage = QStringLiteral( "%1 (%2): %3" ).arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() );
            return false;
          }
          else
          {
            // Remove QGIS_VERSION now that we are done
            GDALSetMetadataItem( hLayer, "QGIS_VERSION", nullptr, nullptr );
            return true;
          }
        }
        else
        {
          // insert new details in metadata tables
          sql = QStringLiteral( "INSERT INTO gpkg_metadata (md_scope, md_standard_uri, mime_type, metadata) VALUES (%1,%2,%3,%4);" )
                .arg( QgsSqliteUtils::quotedString( QStringLiteral( "dataset" ) ),
                      QgsSqliteUtils::quotedString( QStringLiteral( "http://mrcc.com/qgis.dtd" ) ),
                      QgsSqliteUtils::quotedString( QStringLiteral( "text/xml" ) ),
                      QgsSqliteUtils::quotedString( metadataXml ) );
          userLayer->ExecuteSQLNoReturn( sql.toLocal8Bit().constData() );

          sql = QStringLiteral( "SELECT last_insert_rowid();" );
          int lastRowId = -1;
          if ( QgsOgrLayerUniquePtr  l = userLayer->ExecuteSQL( sql.toLocal8Bit().constData() ) )
          {
            // retrieve inserted row id
            gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
            if ( f )
            {
              bool ok = false;
              QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), QgsField( QString(), QVariant::String ), 0, nullptr, &ok );
              if ( !ok )
              {
                return false;
              }
              lastRowId = res.toInt();

              sql = QStringLiteral( "INSERT INTO gpkg_metadata_reference (reference_scope, table_name, md_file_id) VALUES (%1,%2,%3);" )
                    .arg( QgsSqliteUtils::quotedString( QStringLiteral( "table" ) ),
                          QgsSqliteUtils::quotedString( layerName ) )
                    .arg( lastRowId );
              userLayer->ExecuteSQLNoReturn( sql.toLocal8Bit().constData() );

              // Remove QGIS_VERSION now that we are done
              GDALSetMetadataItem( hLayer, "QGIS_VERSION", nullptr, nullptr );
              return true;
            }
          }
          errorMessage = QStringLiteral( "Could not retrieve gpkg_metadata row id" );
          return false;
        }
      }
      else
      {
        errorMessage = QStringLiteral( "%1 (%2): %3" ).arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() );
        return false;
      }
    }
    else
    {
      // file based, but not a geopackage -- store as .qmd sidecar file instead
      // (possibly there's other formats outside of GPKG which also has some native means of storing metadata,
      // which could be added for those formats before we resort to the sidecar approach!)
      const QString qmdFileName = fi.dir().filePath( fi.completeBaseName() + QStringLiteral( ".qmd" ) );
      QFile qmdFile( qmdFileName );
      if ( qmdFile.open( QFile::WriteOnly | QFile::Truncate ) )
      {
        QTextStream fileStream( &qmdFile );
        fileStream << metadataXml;
        qmdFile.close();
        return true;
      }
      else
      {
        errorMessage = tr( "ERROR: Failed to created default metadata file as %1. Check file permissions and retry." ).arg( qmdFileName );
        return false;
      }
    }
  }

  throw QgsNotSupportedException( QObject::tr( "Storing metadata for the specified uri is not supported" ) );
}

QgsTransaction *QgsOgrProviderMetadata::createTransaction( const QString &connString )
{
  auto ds = QgsOgrProviderUtils::getAlreadyOpenedDataset( connString );
  if ( !ds )
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot open transaction on %1, since it is is not currently opened" ).arg( connString ),
                               QObject::tr( "OGR" ), Qgis::MessageLevel::Critical );
    return nullptr;
  }

  return new QgsOgrTransaction( connString, ds );
}

QgsGeoPackageProjectStorage *gGeoPackageProjectStorage = nullptr;   // when not null it is owned by QgsApplication::projectStorageRegistry()

void QgsOgrProviderMetadata::initProvider()
{
  Q_ASSERT( !gGeoPackageProjectStorage );
  gGeoPackageProjectStorage = new QgsGeoPackageProjectStorage;
  QgsApplication::projectStorageRegistry()->registerProjectStorage( gGeoPackageProjectStorage );  // takes ownership
}


void QgsOgrProviderMetadata::cleanupProvider()
{
  QgsApplication::projectStorageRegistry()->unregisterProjectStorage( gGeoPackageProjectStorage );  // destroys the object
  gGeoPackageProjectStorage = nullptr;
  QgsOgrConnPool::cleanupInstance();
  // NOTE: QgsApplication takes care of
  // calling OGRCleanupAll();
}



QgsOgrProviderMetadata::QgsOgrProviderMetadata()
  : QgsProviderMetadata( TEXT_PROVIDER_KEY, TEXT_PROVIDER_DESCRIPTION )
{

}

QString QgsOgrProviderMetadata::filters( FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterVector:
      return QgsOgrProviderUtils::fileVectorFilters();

    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
    case QgsProviderMetadata::FilterType::FilterPointCloud:
      return QString();
  }
  return QString();
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsOgrProviderMetadata::capabilities() const
{
  return QuerySublayers;
}

bool QgsOgrProviderMetadata::uriIsBlocklisted( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( !parts.contains( QStringLiteral( "path" ) ) )
    return false;

  QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
  const QString suffix = fi.completeSuffix();

  // internal details only
  if ( suffix.compare( QLatin1String( "shp.xml" ), Qt::CaseInsensitive ) == 0 )
    return true;

  return false;
}

QList<QgsProviderSublayerDetails> QgsOgrProviderMetadata::querySublayers( const QString &u, Qgis::SublayerQueryFlags flags, QgsFeedback *feedback ) const
{
  QString uri = QgsOgrProviderUtils::expandAuthConfig( u );
  QStringList options { QStringLiteral( "@LIST_ALL_TABLES=YES" ) };
  QVariantMap uriParts = decodeUri( uri );

  // Try to open using VSIFileHandler
  QString vsiPrefix = QgsZipItem::vsiPrefix( uriParts.value( QStringLiteral( "path" ) ).toString() );
  if ( !vsiPrefix.isEmpty() && uriParts.value( QStringLiteral( "vsiPrefix" ) ).toString().isEmpty() )
  {
    if ( !uri.startsWith( vsiPrefix ) )
    {
      // update zip etc to use vsi prefix if it wasn't explicitly specified
      uri = vsiPrefix + uri;
      uriParts = decodeUri( uri );
    }
  }

  if ( !uriParts.value( QStringLiteral( "vsiPrefix" ) ).toString().isEmpty()
       && uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty() )
  {
    // get list of files inside archive file
    QgsDebugMsgLevel( QStringLiteral( "Open file %1 with gdal vsi" ).arg( vsiPrefix + uriParts.value( QStringLiteral( "path" ) ).toString() ), 3 );
    char **papszSiblingFiles = VSIReadDirRecursive( QString( vsiPrefix + uriParts.value( QStringLiteral( "path" ) ).toString() ).toLocal8Bit().constData() );
    if ( papszSiblingFiles )
    {
      QList<QgsProviderSublayerDetails> res;

      QStringList files;
      for ( int i = 0; papszSiblingFiles[i]; i++ )
      {
        files << papszSiblingFiles[i];
      }

      for ( const QString &file : std::as_const( files ) )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        // ugly hack to remove .dbf file if there is a .shp file
        QFileInfo info( file );
        if ( info.suffix().compare( QLatin1String( "dbf" ), Qt::CaseInsensitive ) == 0 )
        {
          if ( files.contains( file.left( file.size() - 4 ) + ".shp" ) )
            continue;
        }
        if ( info.completeSuffix().compare( QLatin1String( "shp.xml" ), Qt::CaseInsensitive ) == 0
             || info.completeSuffix().compare( QLatin1String( "shx" ), Qt::CaseInsensitive ) == 0 )
        {
          continue;
        }

        // skip directories (files ending with /)
        if ( file.right( 1 ) != QLatin1String( "/" ) )
        {
          uriParts.insert( QStringLiteral( "vsiSuffix" ), QStringLiteral( "/%1" ).arg( file ) );
          res << querySublayers( encodeUri( uriParts ), flags, feedback );
        }
      }
      CSLDestroy( papszSiblingFiles );
      return res;
    }
  }

  const QStringList dirExtensions = QgsOgrProviderUtils::directoryExtensions();

  const QString path = uriParts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo pathInfo( path );
  const QString suffix = uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty()
                         ? pathInfo.suffix().toLower()
                         : QFileInfo( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString() ).suffix().toLower();
  bool isOgrSupportedDirectory = pathInfo.isDir() && dirExtensions.contains( suffix );

  bool forceDeepScanDir = false;
  if ( pathInfo.isDir() && !isOgrSupportedDirectory )
  {
    QDirIterator it( path, { QStringLiteral( "*.adf" ), QStringLiteral( "*.ADF" ) }, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    forceDeepScanDir = it.hasNext();
  }

  if ( ( flags & Qgis::SublayerQueryFlag::FastScan ) && ( pathInfo.isFile() || pathInfo.isDir() ) && !forceDeepScanDir )
  {
    // fast scan, so we don't actually try to open the dataset and instead just check the extension alone
    const QStringList fileExtensions = QgsOgrProviderUtils::fileExtensions();

    // allow only normal files or supported directories to continue
    if ( !isOgrSupportedDirectory && !pathInfo.isFile() )
      return {};

    if ( !fileExtensions.contains( suffix ) && !dirExtensions.contains( suffix ) )
    {
      bool matches = false;
      const QStringList wildcards = QgsOgrProviderUtils::wildcards();
      for ( const QString &wildcard : wildcards )
      {
        const QRegularExpression rx( QRegularExpression::wildcardToRegularExpression( wildcard ), QRegularExpression::CaseInsensitiveOption );
        if ( rx.match( pathInfo.fileName() ).hasMatch() )
        {
          matches = true;
          break;
        }
      }
      if ( !matches )
        return {};
    }

    // metadata.xml file next to tdenv?.adf files is a subcomponent of an ESRI tin layer alone, shouldn't be exposed
    if ( pathInfo.fileName().compare( QLatin1String( "metadata.xml" ), Qt::CaseInsensitive ) == 0 )
    {
      const QDir dir  = pathInfo.dir();
      if ( dir.exists( QStringLiteral( "tdenv9.adf" ) )
           || dir.exists( QStringLiteral( "tdenv.adf" ) )
           || dir.exists( QStringLiteral( "TDENV9.ADF" ) )
           || dir.exists( QStringLiteral( "TDENV.ADF" ) ) )
        return {};
    }

    // if file is trivial to read then there's no need to rely on
    // the extension only scan here -- avoiding it always gives us the correct data type
    // and sublayer visibility
    if ( !QgsGdalUtils::pathIsCheapToOpen( path ) )
    {
      // if this is a VRT file make sure it is vector VRT
      if ( suffix == QLatin1String( "vrt" ) && !QgsGdalUtils::vrtMatchesLayerType( path, QgsMapLayerType::VectorLayer ) )
      {
        return {};
      }

      QgsProviderSublayerDetails details;
      details.setType( QgsMapLayerType::VectorLayer );
      details.setProviderKey( QStringLiteral( "ogr" ) );
      details.setUri( uri );
      details.setName( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty()
                       ? QgsProviderUtils::suggestLayerNameFromFilePath( path )
                       : QgsProviderUtils::suggestLayerNameFromFilePath( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString() ) );
      if ( QgsGdalUtils::multiLayerFileExtensions().contains( suffix ) )
      {
        // uri may contain sublayers, but query flags prevent us from examining them
        details.setSkippedContainerScan( true );
      }
      return {details};
    }
  }

  const QString originalUriLayerName = uriParts.value( QStringLiteral( "layerName" ) ).toString();
  int layerId = 0;
  bool originalUriLayerIdWasSpecified = false;
  const int uriLayerId = uriParts.value( QStringLiteral( "layerId" ) ).toInt( &originalUriLayerIdWasSpecified );
  if ( originalUriLayerIdWasSpecified )
    layerId = uriLayerId;

  QgsWkbTypes::Type originalGeometryTypeFilter = QgsWkbTypes::Unknown;
  bool originalUriGeometryTypeWasSpecified = false;
  const QString originalGeometryTypeString = uriParts.value( QStringLiteral( "geometryType" ) ).toString();
  if ( !originalGeometryTypeString.isEmpty() )
  {
    originalGeometryTypeFilter = QgsOgrUtils::ogrGeometryTypeToQgsWkbType(
                                   QgsOgrProviderUtils::ogrWkbGeometryTypeFromName( originalGeometryTypeString )
                                 );
    originalUriGeometryTypeWasSpecified = true;
  }

  QString errCause;

  QVariantMap firstLayerUriParts;
  if ( !uriParts.value( QStringLiteral( "vsiPrefix" ) ).toString().isEmpty() )
    firstLayerUriParts.insert( QStringLiteral( "vsiPrefix" ), uriParts.value( QStringLiteral( "vsiPrefix" ) ) );
  if ( !uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty() )
    firstLayerUriParts.insert( QStringLiteral( "vsiSuffix" ), uriParts.value( QStringLiteral( "vsiSuffix" ) ) );
  firstLayerUriParts.insert( QStringLiteral( "path" ), uriParts.value( QStringLiteral( "path" ) ) );

  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  QgsOgrLayerUniquePtr firstLayer = QgsOgrProviderUtils::getLayer( encodeUri( firstLayerUriParts ), false, options, layerId, errCause, true );
  CPLPopErrorHandler();

  if ( !firstLayer )
    return {};

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QMutex *mutex = nullptr;
#else
  QRecursiveMutex *mutex = nullptr;
#endif
  GDALDatasetH hDS = firstLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

  const QString driverName = firstLayer->driverName();

  const int layerCount = firstLayer->GetLayerCount();

  QList<QgsProviderSublayerDetails> res;
  if ( layerCount == 1 )
  {
    res << QgsOgrProviderUtils::querySubLayerList( 0, firstLayer.get(), hDS, driverName, flags, uri, true, feedback );
  }
  else
  {
    // In case there is no free opened dataset in the cache, keep the first
    // layer alive while we iterate over the other layers, so that we can
    // reuse the same dataset. Can help in a particular with a FileGDB with
    // the FileGDB driver
    for ( int i = 0; i < layerCount; i++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      if ( originalUriLayerIdWasSpecified && i != uriLayerId )
        continue;

      QString errCause;
      QgsOgrLayerUniquePtr layer;

      if ( i != 0 )
      {
        layer = QgsOgrProviderUtils::getLayer( firstLayer->datasetName(),
                                               false,
                                               firstLayer->options(),
                                               i,
                                               errCause,
                                               // do not check timestamp beyond the first
                                               // layer
                                               firstLayer == nullptr );
        if ( !layer )
          continue;
      }

      QgsOgrLayer *sublayer = i == 0 ? firstLayer.get() : layer.get();
      if ( !sublayer )
        continue;

      const QString layerName = QString::fromUtf8( sublayer->name() );
      if ( !originalUriLayerName.isEmpty() && layerName != originalUriLayerName )
        continue;

      res << QgsOgrProviderUtils::querySubLayerList( i, sublayer, hDS, driverName, flags, uri, false, feedback );
    }
  }

  // Systematically add a layerName= option to all OGR sublayers in case
  // the current single layer dataset becomes layer a multi-layer one.
  // (Except for a few select extensions, known to be always single layer dataset!)
  for ( int i = 0; i < res.count(); ++i )
  {
    QVariantMap parts = decodeUri( res.at( i ).uri() );
    if ( originalUriGeometryTypeWasSpecified && res.at( i ).wkbType() == QgsWkbTypes::Unknown )
    {
      res[ i ].setWkbType( originalGeometryTypeFilter );
      parts.insert( QStringLiteral( "geometryType" ), originalGeometryTypeString );
      res[i].setUri( encodeUri( parts ) );
    }

    if ( !parts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() ||
         !parts.value( QStringLiteral( "layerId" ) ).toString().isEmpty() )
      continue;

    bool isAlwaysSingleLayerDataset = false;
    const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
    if ( fi.isFile() )
    {
      const QString ext = fi.suffix().toLower();
      isAlwaysSingleLayerDataset = ext == QLatin1String( "shp" ) ||
                                   ext == QLatin1String( "mif" ) ||
                                   ext == QLatin1String( "tab" ) ||
                                   ext == QLatin1String( "csv" ) ||
                                   ext == QLatin1String( "geojson" );
    }
    if ( isAlwaysSingleLayerDataset )
      continue;

    parts.insert( QStringLiteral( "layerName" ), res.at( i ).name() );
    res[i].setUri( encodeUri( parts ) );
  }

  if ( !originalUriLayerName.isEmpty() )
  {
    // remove non-matching, unwanted layers
    res.erase( std::remove_if( res.begin(), res.end(), [ = ]( const QgsProviderSublayerDetails & sublayer )
    {
      const QVariantMap uriParts = decodeUri( sublayer.uri() );
      return uriParts.value( QStringLiteral( "layerName" ) ).toString() != originalUriLayerName && sublayer.name() != originalUriLayerName;
    } ), res.end() );
  }

  if ( originalUriLayerIdWasSpecified )
  {
    // remove non-matching, unwanted layers by layer id
    res.erase( std::remove_if( res.begin(), res.end(), [ = ]( const QgsProviderSublayerDetails & sublayer )
    {
      return sublayer.layerNumber() != uriLayerId;
    } ), res.end() );
  }

  if ( originalUriGeometryTypeWasSpecified )
  {
    // remove non-matching, unwanted layers by geometry type
    res.erase( std::remove_if( res.begin(), res.end(), [ = ]( const QgsProviderSublayerDetails & sublayer )
    {
      return sublayer.wkbType() != originalGeometryTypeFilter;
    } ), res.end() );
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  // retrieve layer paths
  if ( GDALGroupH rootGroup = GDALDatasetGetRootGroup( hDS ) )
  {
    std::function< void( GDALGroupH, const QStringList & ) > recurseGroup;
    recurseGroup = [&recurseGroup, &res]( GDALGroupH group, const QStringList & currentPath )
    {
      if ( char **vectorLayerNames = GDALGroupGetVectorLayerNames( group, nullptr ) )
      {
        const QStringList layers = QgsOgrUtils::cStringListToQStringList( vectorLayerNames );
        CSLDestroy( vectorLayerNames );
        // attach path to matching layers
        for ( const QString &layer : layers )
        {
          for ( int i = 0; i < res.size(); ++i )
          {
            if ( res.at( i ).name() == layer )
            {
              res[i].setPath( currentPath );
            }
          }
        }
      }

      if ( char **subgroupNames = GDALGroupGetGroupNames( group, nullptr ) )
      {
        for ( int i = 0; subgroupNames[i]; ++i )
        {
          if ( GDALGroupH subgroup = GDALGroupOpenGroup( group, subgroupNames[i], nullptr ) )
          {
            recurseGroup( subgroup, QStringList( currentPath ) << QString::fromUtf8( subgroupNames[i] ) );
            GDALGroupRelease( subgroup );
          }
        }
        CSLDestroy( subgroupNames );
      }
    };

    recurseGroup( rootGroup, {} );
    GDALGroupRelease( rootGroup );
  }
#endif

  return res;
}

QStringList QgsOgrProviderMetadata::sidecarFilesForUri( const QString &uri ) const
{
  const QVariantMap uriParts = decodeUri( uri );
  const QString path = uriParts.value( QStringLiteral( "path" ) ).toString();

  if ( path.isEmpty() )
    return {};

  const QFileInfo fileInfo( path );
  const QString suffix = fileInfo.suffix();

  static QMap< QString, QStringList > sExtensions
  {
    {
      QStringLiteral( "shp" ), {
        QStringLiteral( "shx" ),
        QStringLiteral( "dbf" ),
        QStringLiteral( "sbn" ),
        QStringLiteral( "sbx" ),
        QStringLiteral( "prj" ),
        QStringLiteral( "idm" ),
        QStringLiteral( "ind" ),
        QStringLiteral( "qix" ),
        QStringLiteral( "cpg" ),
        QStringLiteral( "qpj" ),
        QStringLiteral( "shp.xml" ),
      }
    },
    {
      QStringLiteral( "tab" ), {
        QStringLiteral( "dat" ),
        QStringLiteral( "id" ),
        QStringLiteral( "map" ),
        QStringLiteral( "ind" ),
        QStringLiteral( "tda" ),
        QStringLiteral( "tin" ),
        QStringLiteral( "tma" ),
        QStringLiteral( "lda" ),
        QStringLiteral( "lin" ),
        QStringLiteral( "lma" ),
      }
    },
    {
      QStringLiteral( "mif" ), {
        QStringLiteral( "mid" ),
      }
    },
    {
      QStringLiteral( "gml" ), {
        QStringLiteral( "gfs" ),
        QStringLiteral( "xsd" ),
      }
    },
    {
      QStringLiteral( "csv" ), {
        QStringLiteral( "csvt" ),
      }
    },
  };

  QStringList res;
  for ( auto it = sExtensions.constBegin(); it != sExtensions.constEnd(); ++it )
  {
    if ( suffix.compare( it.key(), Qt::CaseInsensitive ) == 0 )
    {
      for ( const QString &ext : it.value() )
      {
        res.append( fileInfo.dir().filePath( fileInfo.completeBaseName() + '.' + ext ) );
      }
    }
  }
  return res;
}

QMap<QString, QgsAbstractProviderConnection *> QgsOgrProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsGeoPackageProviderConnection, QgsOgrDbConnection>( cached );
}

QgsAbstractProviderConnection *QgsOgrProviderMetadata::createConnection( const QString &connName )
{
  return new QgsGeoPackageProviderConnection( connName );
}

QgsAbstractProviderConnection *QgsOgrProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri );
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( path );
  if ( fi.suffix().compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0 )
    return new QgsGeoPackageProviderConnection( uri, configuration );
  else
    return new QgsOgrProviderConnection( uri, configuration );
}

void QgsOgrProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsGeoPackageProviderConnection>( name );
}

void QgsOgrProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn, const QString &name )
{
  saveConnectionProtected( conn, name );
}

QgsProviderMetadata::ProviderCapabilities QgsOgrProviderMetadata::providerCapabilities() const
{
  return FileBasedUris | SaveLayerMetadata;
}

///@endcond
