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

#include "qgsapplication.h"
#include "qgsgdalutils.h"
#include "qgsgeopackagedataitems.h"
#include "qgsgeopackageprojectstorage.h"
#include "qgsgeopackageproviderconnection.h"
#include "qgslayermetadataproviderregistry.h"
#include "qgsmessagelog.h"
#include "qgsogrconnpool.h"
#include "qgsogrdbconnection.h"
#include "qgsogrlayermetadataprovider.h"
#include "qgsogrprovider.h"
#include "qgsogrtransaction.h"
#include "qgsprojectstorageregistry.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsvectorfilewriter.h"

#include "moc_qgsogrprovidermetadata.cpp"

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include <gdal.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QDirIterator>

///@cond PRIVATE


#define TEXT_PROVIDER_KEY u"ogr"_s
#define TEXT_PROVIDER_DESCRIPTION u"OGR data provider"_s

QgsDataProvider *QgsOgrProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options,
    Qgis::DataProviderReadFlags flags )
{
  return new QgsOgrProvider( uri, options, flags );
}

Qgis::VectorExportResult QgsOgrProviderMetadata::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    Qgis::WkbType wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> &oldToNewAttrIdxMap,
    QString &errorMessage,
    const QMap<QString, QVariant> *options, QString &createdLayerUri )
{
  return QgsOgrProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           &oldToNewAttrIdxMap, createdLayerUri, &errorMessage, options
         );
}

bool QgsOgrProviderMetadata::createDatabase( const QString &uri, QString &errorMessage )
{
  errorMessage.clear();

  const QVariantMap parts = decodeUri( uri );
  const QString path = parts.value( u"path"_s ).toString();
  if ( path.isEmpty() )
  {
    errorMessage = tr( "Invalid database path specified" );
    return false;
  }
  else if ( QFileInfo::exists( path ) )
  {
    errorMessage = tr( "A file already exists with the specified database path" );
    return false;
  }

  const QString extension = QFileInfo( path ).completeSuffix();
  const QString driverName = QgsVectorFileWriter::driverForExtension( extension );
  if ( driverName.isEmpty() )
  {
    errorMessage = tr( "The file extension %1 is not supported for database creation" ).arg( extension );
    return false;
  }

  OGRSFDriverH poDriver = OGRGetDriverByName( driverName.toLocal8Bit().constData() );
  if ( !poDriver )
  {
    errorMessage = tr( "OGR driver for '%1' not found (OGR error: %2)" )
                   .arg( driverName,
                         QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return false;
  }

  char **metadata = GDALGetMetadata( poDriver, nullptr );

  if ( !CSLFetchBoolean( metadata, GDAL_DCAP_VECTOR, false )
       || !CSLFetchBoolean( metadata, GDAL_DCAP_CREATE, false ) )
  {
    errorMessage = tr( "The %1 driver does not support database creation" )
                   .arg( driverName );
    return false;
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  if ( !CSLFetchBoolean( metadata, GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, false ) )
  {
    errorMessage = tr( "The %1 driver does not support database creation" )
                   .arg( driverName );
    return false;
  }
#endif


  gdal::ogr_datasource_unique_ptr hDS( OGR_Dr_CreateDataSource( poDriver, path.toUtf8().constData(), nullptr ) );
  if ( !hDS )
  {
    errorMessage = tr( "Creation of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return false;
  }

  return true;
}

QVariantMap QgsOgrProviderMetadata::decodeUri( const QString &uri ) const
{
  QString path = uri;
  QString layerName;
  QString subset;
  QString geometryType;
  QString uniqueGeometryType;
  QStringList openOptions;
  QVariantMap credentialOptions;
  QString databaseName;
  QString authcfg;

  int layerId = -1;

  const thread_local QRegularExpression authcfgRegex( " authcfg='([^']+)'" );
  QRegularExpressionMatch match;
  if ( path.contains( authcfgRegex, &match ) )
  {
    path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    authcfg = match.captured( 1 );
  }

  QString vsiPrefix = QgsGdalUtils::vsiPrefixForPath( path );
  QString vsiSuffix;
  if ( path.startsWith( vsiPrefix, Qt::CaseInsensitive ) )
  {
    path = path.mid( vsiPrefix.count() );

    const thread_local QRegularExpression vsiRegex( u"(?:\\.zip|\\.tar|\\.gz|\\.tar\\.gz|\\.tgz)([^|]+)"_s );
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
    const thread_local QRegularExpression geometryTypeRegex( u"\\|geometrytype=([a-zA-Z0-9]*)"_s, QRegularExpression::PatternOption::CaseInsensitiveOption );
    const thread_local QRegularExpression uniqueGeometryTypeRegex( u"\\|uniqueGeometryType=([a-z]*)"_s, QRegularExpression::PatternOption::CaseInsensitiveOption );
    const thread_local QRegularExpression layerNameRegex( u"\\|layername=([^|]*)"_s, QRegularExpression::PatternOption::CaseInsensitiveOption );
    const thread_local QRegularExpression layerIdRegex( u"\\|layerid=([^|]*)"_s, QRegularExpression::PatternOption::CaseInsensitiveOption );
    const thread_local QRegularExpression subsetRegex( u"\\|subset=((?:.*[\r\n]*)*)\\Z"_s );
    const thread_local QRegularExpression openOptionRegex( u"\\|option:([^|]*)"_s );
    const thread_local QRegularExpression credentialOptionRegex( u"\\|credential:([^|]*)"_s );
    const thread_local QRegularExpression credentialOptionKeyValueRegex( u"(.*?)=(.*)"_s );

    // we first try to split off the geometry type component, if that's present. That's a known quantity which
    // will never be more than a-z characters
    match = geometryTypeRegex.match( path );
    if ( match.hasMatch() )
    {
      geometryType = match.captured( 1 );
      path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    }

    match = uniqueGeometryTypeRegex.match( path );
    if ( match.hasMatch() )
    {
      uniqueGeometryType = match.captured( 1 );
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

    while ( true )
    {
      const QRegularExpressionMatch match = credentialOptionRegex.match( path );
      if ( match.hasMatch() )
      {
        const QRegularExpressionMatch keyValueMatch = credentialOptionKeyValueRegex.match( match.captured( 1 ) );
        if ( keyValueMatch.hasMatch() )
        {
          credentialOptions.insert( keyValueMatch.captured( 1 ), keyValueMatch.captured( 2 ) );
        }
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
  if ( uri.startsWith( u"MySQL"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"PostgreSQL"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"MSSQL"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"ODBC"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"PGeo"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"SDE"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"OGDI"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"Ingres"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"IDB"_s, Qt::CaseSensitivity::CaseInsensitive ) ||
       uri.startsWith( u"OCI"_s, Qt::CaseSensitivity::CaseInsensitive ) )
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
  uriComponents.insert( u"path"_s, path );
  uriComponents.insert( u"layerName"_s, layerName );
  uriComponents.insert( u"layerId"_s, layerId > -1 && layerName.isEmpty() ? layerId : QVariant() ) ;
  if ( !subset.isEmpty() )
    uriComponents.insert( u"subset"_s, subset );
  if ( !geometryType.isEmpty() )
    uriComponents.insert( u"geometryType"_s, geometryType );
  if ( !uniqueGeometryType.isEmpty() )
    uriComponents.insert( u"uniqueGeometryType"_s, uniqueGeometryType );
  if ( !databaseName.isEmpty() )
    uriComponents.insert( u"databaseName"_s, databaseName );
  if ( !openOptions.isEmpty() )
    uriComponents.insert( u"openOptions"_s, openOptions );
  if ( !credentialOptions.isEmpty() )
    uriComponents.insert( u"credentialOptions"_s, credentialOptions );
  if ( !vsiPrefix.isEmpty() )
    uriComponents.insert( u"vsiPrefix"_s, vsiPrefix );
  if ( !vsiSuffix.isEmpty() )
    uriComponents.insert( u"vsiSuffix"_s, vsiSuffix );
  if ( !authcfg.isEmpty() )
    uriComponents.insert( u"authcfg"_s, authcfg );
  return uriComponents;
}

QString QgsOgrProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString vsiPrefix = parts.value( u"vsiPrefix"_s ).toString();
  const QString vsiSuffix = parts.value( u"vsiSuffix"_s ).toString();
  const QString path = parts.value( u"path"_s ).toString();
  const QString layerName = parts.value( u"layerName"_s ).toString();
  const QString layerId = parts.value( u"layerId"_s ).toString();
  const QString subset = parts.value( u"subset"_s ).toString();
  const QString geometryType = parts.value( u"geometryType"_s ).toString();
  const QString authcfg = parts.value( u"authcfg"_s ).toString();
  const QStringList openOptions = parts.value( u"openOptions"_s ).toStringList();
  const QString uniqueGeometryType = parts.value( u"uniqueGeometryType"_s ).toString();

  QString uri = vsiPrefix + path + vsiSuffix
                + ( !layerName.isEmpty() ? u"|layername=%1"_s.arg( layerName ) : !layerId.isEmpty() ? u"|layerid=%1"_s.arg( layerId ) : QString() )
                + ( !geometryType.isEmpty() ? u"|geometrytype=%1"_s.arg( geometryType ) : QString() );
  if ( !uniqueGeometryType.isEmpty() )
    uri += u"|uniqueGeometryType=%1"_s.arg( uniqueGeometryType );
  for ( const QString &openOption : openOptions )
  {
    uri += "|option:"_L1;
    uri += openOption;
  }

  const QVariantMap credentialOptions = parts.value( u"credentialOptions"_s ).toMap();
  for ( auto it = credentialOptions.constBegin(); it != credentialOptions.constEnd(); ++it )
  {
    if ( !it.value().toString().isEmpty() )
    {
      uri += u"|credential:%1=%2"_s.arg( it.key(), it.value().toString() );
    }
  }

  if ( !subset.isEmpty() )
    uri += u"|subset=%1"_s.arg( subset );
  if ( !authcfg.isEmpty() )
    uri += u" authcfg='%1'"_s.arg( authcfg );
  return uri;
}

QString QgsOgrProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QString src = uri;
  QStringList theURIParts = src.split( '|' );
  theURIParts[0] = context.pathResolver().writePath( theURIParts[0] );
  src = theURIParts.join( '|'_L1 );
  return src;
}

QString QgsOgrProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QString src = uri;
  QStringList theURIParts = src.split( '|' );
  theURIParts[0] = context.pathResolver().readPath( theURIParts[0] );
  src = theURIParts.join( '|'_L1 );
  return src;
}

QList<QgsDataItemProvider *> QgsOgrProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsGeoPackageDataItemProvider;
  return providers;
}

static QgsOgrLayerUniquePtr LoadDataSourceAndLayer( const QString &uri, bool updateMode, QString &filePath, QString &errCause )
{
  bool isSubLayer;
  int layerIndex;
  QString layerName;
  QString subsetString;
  OGRwkbGeometryType ogrGeometryType;
  QStringList openOptions;
  QVariantMap credentialOptions;
  filePath = QgsOgrProviderUtils::analyzeURI( uri,
             isSubLayer,
             layerIndex,
             layerName,
             subsetString,
             ogrGeometryType,
             openOptions,
             credentialOptions );

  if ( updateMode )
  {
    if ( !layerName.isEmpty() )
    {
      return QgsOgrProviderUtils::getLayer( filePath, true, QStringList(), layerName, errCause, true );
    }
    else
    {
      return QgsOgrProviderUtils::getLayer( filePath, true, QStringList(), layerIndex, errCause, true );
    }
  }
  else
  {
    if ( !layerName.isEmpty() )
    {
      return QgsOgrProviderUtils::getLayer( filePath, layerName, errCause );
    }
    else
    {
      return QgsOgrProviderUtils::getLayer( filePath, layerIndex, errCause );
    }
  }
}

bool QgsOgrProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  QString filePath;
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, false, filePath, errorCause );
  if ( !userLayer )
    return false;

  QRecursiveMutex *mutex = nullptr;
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex );
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );
  QString layerName = QString( OGR_L_GetName( hUserLayer ) );
  QString geomColumn = QString( OGR_L_GetGeometryColumn( hUserLayer ) );

  return QgsOgrUtils::styleExists( hDS, layerName, geomColumn, styleId, errorCause );
}
bool QgsOgrProviderMetadata::saveStyle(
  const QString &uri, const QString &qmlStyle, const QString &sldStyle,
  const QString &styleName, const QString &styleDescription,
  const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QString filePath;
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, true, filePath, errCause );
  if ( !userLayer )
    return false;

  QRecursiveMutex *mutex = nullptr;
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex );
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );
  QString layerName = QString( OGR_L_GetName( hUserLayer ) );
  QString geomColumn = QString( OGR_L_GetGeometryColumn( hUserLayer ) );

  return QgsOgrUtils::saveStyle( hDS, layerName, geomColumn, qmlStyle, sldStyle, styleName, styleDescription, uiFileContent, useAsDefault, errCause );
}

bool QgsOgrProviderMetadata::deleteStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QString filePath;
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, true, filePath, errCause );
  if ( !userLayer )
    return false;
  QRecursiveMutex *mutex = nullptr;
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

  return QgsOgrUtils::deleteStyleById( hDS, styleId, errCause );
}

QString QgsOgrProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QString name;
  return loadStoredStyle( uri, name, errCause );
}

QString QgsOgrProviderMetadata::loadStoredStyle( const QString &uri, QString &styleName, QString &errCause )
{
  QString filePath;
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, false, filePath, errCause );
  if ( !userLayer )
    return QString();

  QRecursiveMutex *mutex = nullptr;
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex );
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker lock( mutex );
  QString layerName = QString( OGR_L_GetName( hUserLayer ) );
  QString geomColumn = QString( OGR_L_GetGeometryColumn( hUserLayer ) );

  return QgsOgrUtils::loadStoredStyle( hDS, layerName, geomColumn, styleName, errCause );
}

int QgsOgrProviderMetadata::listStyles(
  const QString &uri, QStringList &ids, QStringList &names,
  QStringList &descriptions, QString &errCause )
{
  QString filePath;
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, false, filePath, errCause );
  if ( !userLayer )
    return -1;

  QRecursiveMutex *mutex = nullptr;
  OGRLayerH hUserLayer = userLayer->getHandleAndMutex( mutex );
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  Q_UNUSED( hDS );
  QMutexLocker locker( mutex );
  QString layerName = QString( OGR_L_GetName( hUserLayer ) );
  QString geomColumn = QString( OGR_L_GetGeometryColumn( hUserLayer ) );

  return QgsOgrUtils::listStyles( hDS, layerName, geomColumn, ids, names, descriptions, errCause );
}

QString QgsOgrProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QString filePath;
  QgsOgrLayerUniquePtr userLayer = LoadDataSourceAndLayer( uri, false, filePath, errCause );
  if ( !userLayer )
    return QString();

  QRecursiveMutex *mutex = nullptr;
  GDALDatasetH hDS = userLayer->getDatasetHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

  return QgsOgrUtils::getStyleById( hDS, styleId, errCause );
}

bool QgsOgrProviderMetadata::saveLayerMetadata( const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage )
{
  const QVariantMap parts = decodeUri( uri );
  const QString path = parts.value( u"path"_s ).toString();
  if ( !path.isEmpty() && QFileInfo::exists( path ) )
  {
    // export metadata to XML
    QDomImplementation domImplementation;
    QDomDocumentType documentType = domImplementation.createDocumentType( u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s );
    QDomDocument document( documentType );

    QDomElement rootNode = document.createElement( u"qgis"_s );
    rootNode.setAttribute( u"version"_s, Qgis::version() );
    document.appendChild( rootNode );

    if ( !metadata.writeMetadataXml( rootNode, document ) )
    {
      errorMessage = QObject::tr( "Error exporting metadata to XML" );
      return false;
    }

    QString metadataXml;
    QTextStream textStream( &metadataXml );
    document.save( textStream, 2 );

    if ( path.endsWith( ".gpkg"_L1, Qt::CaseInsensitive ) )
    {
      const QString layerName = parts.value( u"layerName"_s ).toString();
      QgsOgrLayerUniquePtr userLayer;
      userLayer = QgsOgrProviderUtils::getLayer( path, true, QStringList(), layerName, errorMessage, true );
      if ( !userLayer )
        return false;

      QRecursiveMutex *mutex = nullptr;
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
      if ( GDALSetMetadataItem( hLayer, "QGIS_VERSION", Qgis::version().toUtf8().constData(), nullptr ) == CE_None )
      {
        // so far so good, ready to throw the whole of the QGIS layer XML into the metadata table!

        // first we need to check if there's already a corresponding entry in gpkg_metadata -- if so, we need to update it.
        QString sql = QStringLiteral( "SELECT id from gpkg_metadata LEFT JOIN gpkg_metadata_reference ON "
                                      "(gpkg_metadata_reference.table_name = %1 AND gpkg_metadata.id = gpkg_metadata_reference.md_file_id) "
                                      "WHERE md_standard_uri = %2 and reference_scope = %3" ).arg(
                        QgsSqliteUtils::quotedString( layerName ),
                        QgsSqliteUtils::quotedString( u"http://mrcc.com/qgis.dtd"_s ),
                        QgsSqliteUtils::quotedString( u"table"_s ) );
        int existingRowId = -1;
        if ( QgsOgrLayerUniquePtr l = userLayer->ExecuteSQL( sql.toUtf8().constData() ) )
        {
          // retrieve inserted row id
          gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
          if ( f )
          {
            bool ok = false;
            QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), QgsField( QString(), QMetaType::Type::QString ), 0, nullptr, &ok );
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
          sql = u"UPDATE gpkg_metadata SET metadata=%1 where id=%2;"_s.arg(
                  QgsSqliteUtils::quotedString( metadataXml ) ).arg( existingRowId );
          userLayer->ExecuteSQLNoReturn( sql.toUtf8().constData() );
          if ( CPLGetLastErrorType() != CE_None )
          {
            errorMessage = u"%1 (%2): %3"_s.arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() );
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
          sql = u"INSERT INTO gpkg_metadata (md_scope, md_standard_uri, mime_type, metadata) VALUES (%1,%2,%3,%4);"_s
                .arg( QgsSqliteUtils::quotedString( u"dataset"_s ),
                      QgsSqliteUtils::quotedString( u"http://mrcc.com/qgis.dtd"_s ),
                      QgsSqliteUtils::quotedString( u"text/xml"_s ),
                      QgsSqliteUtils::quotedString( metadataXml ) );
          userLayer->ExecuteSQLNoReturn( sql.toUtf8().constData() );

          sql = u"SELECT last_insert_rowid();"_s;
          int lastRowId = -1;
          if ( QgsOgrLayerUniquePtr  l = userLayer->ExecuteSQL( sql.toUtf8().constData() ) )
          {
            // retrieve inserted row id
            gdal::ogr_feature_unique_ptr f( l->GetNextFeature() );
            if ( f )
            {
              bool ok = false;
              QVariant res = QgsOgrUtils::getOgrFeatureAttribute( f.get(), QgsField( QString(), QMetaType::Type::QString ), 0, nullptr, &ok );
              if ( !ok )
              {
                return false;
              }
              lastRowId = res.toInt();

              sql = u"INSERT INTO gpkg_metadata_reference (reference_scope, table_name, md_file_id) VALUES (%1,%2,%3);"_s
                    .arg( QgsSqliteUtils::quotedString( u"table"_s ),
                          QgsSqliteUtils::quotedString( layerName ) )
                    .arg( lastRowId );
              userLayer->ExecuteSQLNoReturn( sql.toUtf8().constData() );

              // Remove QGIS_VERSION now that we are done
              GDALSetMetadataItem( hLayer, "QGIS_VERSION", nullptr, nullptr );
              return true;
            }
          }
          errorMessage = u"Could not retrieve gpkg_metadata row id"_s;
          return false;
        }
      }
      else
      {
        errorMessage = u"%1 (%2): %3"_s.arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() );
        return false;
      }
    }
    else
    {
      // file based, but not a geopackage -- store as .qmd sidecar file instead
      // (possibly there's other formats outside of GPKG which also has some native means of storing metadata,
      // which could be added for those formats before we resort to the sidecar approach!)

      QString adjustedPath = path;
      const QStringList excludedExtensions = { ".gz"_L1, ".zip"_L1, ".tar"_L1, ".tgz"_L1, ".tar.gz"_L1 };
      for ( const QString &excludedExtension : excludedExtensions )
      {
        if ( adjustedPath.endsWith( excludedExtension, Qt::CaseInsensitive ) )
        {
          adjustedPath.chop( excludedExtension.size() );
          break;
        }
      }

      const QFileInfo fi( adjustedPath );
      const QString qmdFileName = fi.dir().filePath( fi.completeBaseName() + u".qmd"_s );
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
    QgsMessageLog::logMessage( QObject::tr( "Cannot open transaction on %1, since it is not currently opened" ).arg( connString ),
                               QObject::tr( "OGR" ), Qgis::MessageLevel::Critical );
    return nullptr;
  }

  return new QgsOgrTransaction( connString, std::move( ds ) );
}

QgsGeoPackageProjectStorage *gGeoPackageProjectStorage = nullptr;   // when not null it is owned by QgsApplication::projectStorageRegistry()
QgsOgrLayerMetadataProvider *gOgrLayerMetadataProvider = nullptr;   // when not null it is owned by QgsApplication::layerMetadataProviderRegistry()

void QgsOgrProviderMetadata::initProvider()
{
  Q_ASSERT( !gGeoPackageProjectStorage );
  gGeoPackageProjectStorage = new QgsGeoPackageProjectStorage;
  QgsApplication::projectStorageRegistry()->registerProjectStorage( gGeoPackageProjectStorage );  // takes ownership
  Q_ASSERT( !gOgrLayerMetadataProvider );
  gOgrLayerMetadataProvider = new QgsOgrLayerMetadataProvider();
  QgsApplication::layerMetadataProviderRegistry()->registerLayerMetadataProvider( gOgrLayerMetadataProvider );  // takes ownership
}


void QgsOgrProviderMetadata::cleanupProvider()
{
  QgsApplication::projectStorageRegistry()->unregisterProjectStorage( gGeoPackageProjectStorage );  // destroys the object
  gGeoPackageProjectStorage = nullptr;
  QgsApplication::layerMetadataProviderRegistry()->unregisterLayerMetadataProvider( gOgrLayerMetadataProvider );
  gOgrLayerMetadataProvider = nullptr;
  QgsOgrConnPool::cleanupInstance();
  // NOTE: QgsApplication takes care of
  // calling OGRCleanupAll();
}



QgsOgrProviderMetadata::QgsOgrProviderMetadata()
  : QgsProviderMetadata( TEXT_PROVIDER_KEY, TEXT_PROVIDER_DESCRIPTION )
{

}

QIcon QgsOgrProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconVector.svg"_s );
}

QString QgsOgrProviderMetadata::filters( Qgis::FileFilterType type )
{
  switch ( type )
  {
    case Qgis::FileFilterType::Vector:
      return QgsOgrProviderUtils::fileVectorFilters();

    case Qgis::FileFilterType::Raster:
    case Qgis::FileFilterType::Mesh:
    case Qgis::FileFilterType::MeshDataset:
    case Qgis::FileFilterType::PointCloud:
    case Qgis::FileFilterType::VectorTile:
    case Qgis::FileFilterType::TiledScene:
      return QString();
  }
  return QString();
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsOgrProviderMetadata::capabilities() const
{
  return QuerySublayers | CreateDatabase;
}

bool QgsOgrProviderMetadata::uriIsBlocklisted( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( !parts.contains( u"path"_s ) )
    return false;

  QFileInfo fi( parts.value( u"path"_s ).toString() );
  const QString suffix = fi.completeSuffix();

  // internal details only
  if ( suffix.compare( "shp.xml"_L1, Qt::CaseInsensitive ) == 0 )
    return true;

  return false;
}

QList<QgsProviderSublayerDetails> QgsOgrProviderMetadata::querySublayers( const QString &u, Qgis::SublayerQueryFlags flags, QgsFeedback *feedback ) const
{
  QString uri = QgsOgrProviderUtils::expandAuthConfig( u );
  QStringList options { u"@LIST_ALL_TABLES=YES"_s };
  QVariantMap uriParts = decodeUri( uri );

  // Try to open using VSIFileHandler
  const QString vsiPrefix = QgsGdalUtils::vsiPrefixForPath( uriParts.value( u"path"_s ).toString() );
  if ( !vsiPrefix.isEmpty() && ( uriParts.value( u"vsiPrefix"_s ).toString().isEmpty()
                                 || ( QgsGdalUtils::isVsiArchivePrefix( vsiPrefix ) && uriParts.value( u"vsiPrefix"_s ).toString() != vsiPrefix ) ) )
  {
    if ( !uri.startsWith( vsiPrefix ) )
    {
      // update zip etc to use vsi prefix if it wasn't explicitly specified
      uri = vsiPrefix + uri;
      uriParts = decodeUri( uri );
    }
  }

  if ( !uriParts.value( u"vsiPrefix"_s ).toString().isEmpty()
       && uriParts.value( u"vsiSuffix"_s ).toString().isEmpty()
       && QgsGdalUtils::isVsiArchivePrefix( uriParts.value( u"vsiPrefix"_s ).toString() ) )
  {
    // get list of files inside archive file
    QgsDebugMsgLevel( u"Open file %1 with gdal vsi"_s.arg( vsiPrefix + uriParts.value( u"path"_s ).toString() ), 3 );
    char **papszSiblingFiles = VSIReadDirRecursive( QString( vsiPrefix + uriParts.value( u"path"_s ).toString() ).toUtf8().constData() );
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
        if ( info.suffix().compare( "dbf"_L1, Qt::CaseInsensitive ) == 0 )
        {
          if ( files.contains( file.left( file.size() - 4 ) + ".shp" ) )
            continue;
        }
        if ( info.completeSuffix().compare( "shp.xml"_L1, Qt::CaseInsensitive ) == 0
             || info.completeSuffix().compare( "shx"_L1, Qt::CaseInsensitive ) == 0 )
        {
          continue;
        }

        // skip directories (files ending with /)
        if ( file.right( 1 ) != "/"_L1 )
        {
          uriParts.insert( u"vsiSuffix"_s, u"/%1"_s.arg( file ) );
          res << querySublayers( encodeUri( uriParts ), flags, feedback );
        }
      }
      CSLDestroy( papszSiblingFiles );
      return res;
    }
  }

  const QStringList dirExtensions = QgsOgrProviderUtils::directoryExtensions();

  const QString path = uriParts.value( u"path"_s ).toString();
  const QFileInfo pathInfo( path );
  const QString suffix = uriParts.value( u"vsiSuffix"_s ).toString().isEmpty()
                         ? pathInfo.suffix().toLower()
                         : QFileInfo( uriParts.value( u"vsiSuffix"_s ).toString() ).suffix().toLower();
  bool isOgrSupportedDirectory = pathInfo.isDir() && dirExtensions.contains( suffix );
  const Qgis::VsiHandlerType vsiHandlerType = QgsGdalUtils::vsiHandlerType( uriParts.value( u"vsiPrefix"_s ).toString() );

  bool forceDeepScanDir = false;
  if ( pathInfo.isDir() && !isOgrSupportedDirectory )
  {
    QDirIterator it( path, { u"*.adf"_s, u"*.ADF"_s }, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    forceDeepScanDir = it.hasNext();
  }

  if ( ( flags & Qgis::SublayerQueryFlag::FastScan ) && ( pathInfo.isFile() || pathInfo.isDir() || vsiHandlerType == Qgis::VsiHandlerType::Cloud ) && !forceDeepScanDir )
  {
    // fast scan, so we don't actually try to open the dataset and instead just check the extension alone
    const QStringList fileExtensions = QgsOgrProviderUtils::fileExtensions();

    // allow only normal files or supported directories to continue
    if ( !isOgrSupportedDirectory && !pathInfo.isFile() && vsiHandlerType != Qgis::VsiHandlerType::Cloud )
      return {};

    if ( !fileExtensions.contains( suffix ) && !dirExtensions.contains( suffix ) )
    {
      bool matches = false;
      const QStringList wildcards = QgsOgrProviderUtils::wildcards();
      for ( const QString &wildcard : wildcards )
      {
        const thread_local QRegularExpression rx( QRegularExpression::wildcardToRegularExpression( wildcard ), QRegularExpression::CaseInsensitiveOption );
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
    if ( pathInfo.fileName().compare( "metadata.xml"_L1, Qt::CaseInsensitive ) == 0 )
    {
      const QDir dir  = pathInfo.dir();
      if ( dir.exists( u"tdenv9.adf"_s )
           || dir.exists( u"tdenv.adf"_s )
           || dir.exists( u"TDENV9.ADF"_s )
           || dir.exists( u"TDENV.ADF"_s ) )
        return {};
    }

    // if file is trivial to read then there's no need to rely on
    // the extension only scan here -- avoiding it always gives us the correct data type
    // and sublayer visibility
    if ( !QgsGdalUtils::pathIsCheapToOpen( path ) )
    {
      // if this is a VRT file make sure it is vector VRT
      if ( suffix == "vrt"_L1 && !QgsGdalUtils::vrtMatchesLayerType( path, Qgis::LayerType::Vector ) )
      {
        return {};
      }

      QgsProviderSublayerDetails details;
      details.setType( Qgis::LayerType::Vector );
      details.setProviderKey( u"ogr"_s );
      details.setUri( uri );
      details.setName( uriParts.value( u"vsiSuffix"_s ).toString().isEmpty()
                       ? QgsProviderUtils::suggestLayerNameFromFilePath( path )
                       : QgsProviderUtils::suggestLayerNameFromFilePath( uriParts.value( u"vsiSuffix"_s ).toString() ) );
      if ( QgsGdalUtils::multiLayerFileExtensions().contains( suffix ) )
      {
        // uri may contain sublayers, but query flags prevent us from examining them
        details.setSkippedContainerScan( true );
      }
      return {details};
    }
  }

  const QString originalUriLayerName = uriParts.value( u"layerName"_s ).toString();
  int layerId = 0;
  bool originalUriLayerIdWasSpecified = false;
  const int uriLayerId = uriParts.value( u"layerId"_s ).toInt( &originalUriLayerIdWasSpecified );
  if ( originalUriLayerIdWasSpecified )
    layerId = uriLayerId;

  Qgis::WkbType originalGeometryTypeFilter = Qgis::WkbType::Unknown;
  bool originalUriGeometryTypeWasSpecified = false;
  const QString originalGeometryTypeString = uriParts.value( u"geometryType"_s ).toString();
  if ( !originalGeometryTypeString.isEmpty() )
  {
    originalGeometryTypeFilter = QgsOgrUtils::ogrGeometryTypeToQgsWkbType(
                                   QgsOgrProviderUtils::ogrWkbGeometryTypeFromName( originalGeometryTypeString )
                                 );
    originalUriGeometryTypeWasSpecified = true;
  }

  QString errCause;

  QVariantMap firstLayerUriParts;
  if ( !uriParts.value( u"vsiPrefix"_s ).toString().isEmpty() )
    firstLayerUriParts.insert( u"vsiPrefix"_s, uriParts.value( u"vsiPrefix"_s ) );
  if ( !uriParts.value( u"vsiSuffix"_s ).toString().isEmpty() )
    firstLayerUriParts.insert( u"vsiSuffix"_s, uriParts.value( u"vsiSuffix"_s ) );

  const QVariantMap credentialOptions = uriParts.value( u"credentialOptions"_s ).toMap();
  if ( !credentialOptions.isEmpty() && !uriParts.value( u"vsiPrefix"_s ).toString().isEmpty() )
  {
    const thread_local QRegularExpression bucketRx( u"^(.*)/"_s );
    const QRegularExpressionMatch bucketMatch = bucketRx.match( uriParts.value( u"path"_s ).toString() );
    if ( bucketMatch.hasMatch() )
    {
      QgsGdalUtils::applyVsiCredentialOptions( uriParts.value( u"vsiPrefix"_s ).toString(), bucketMatch.captured( 1 ), credentialOptions );
    }
  }

  firstLayerUriParts.insert( u"path"_s, uriParts.value( u"path"_s ) );

  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  QgsOgrLayerUniquePtr firstLayer = QgsOgrProviderUtils::getLayer( encodeUri( firstLayerUriParts ), false, options, layerId, errCause, true );
  CPLPopErrorHandler();

  if ( !firstLayer )
    return {};

  QRecursiveMutex *mutex = nullptr;
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
                                               !firstLayer );
        if ( !layer )
          continue;
      }

      QgsOgrLayer *sublayer = i == 0 ? firstLayer.get() : layer.get();
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
    if ( originalUriGeometryTypeWasSpecified && res.at( i ).wkbType() == Qgis::WkbType::Unknown )
    {
      res[ i ].setWkbType( originalGeometryTypeFilter );
      parts.insert( u"geometryType"_s, originalGeometryTypeString );
      res[i].setUri( encodeUri( parts ) );
    }

    if ( !parts.value( u"layerName"_s ).toString().isEmpty() ||
         !parts.value( u"layerId"_s ).toString().isEmpty() )
      continue;

    bool isAlwaysSingleLayerDataset = false;
    const QFileInfo fi( parts.value( u"path"_s ).toString() );
    if ( fi.isFile() )
    {
      const QString ext = fi.suffix().toLower();
      isAlwaysSingleLayerDataset = ext == "shp"_L1 ||
                                   ext == "mif"_L1 ||
                                   ext == "tab"_L1 ||
                                   ext == "csv"_L1 ||
                                   ext == "geojson"_L1;
    }
    if ( isAlwaysSingleLayerDataset )
      continue;

    parts.insert( u"layerName"_s, res.at( i ).name() );
    res[i].setUri( encodeUri( parts ) );
  }

  if ( !originalUriLayerName.isEmpty() )
  {
    // remove non-matching, unwanted layers
    res.erase( std::remove_if( res.begin(), res.end(), [this, originalUriLayerName]( const QgsProviderSublayerDetails & sublayer )
    {
      const QVariantMap uriParts = decodeUri( sublayer.uri() );
      return uriParts.value( u"layerName"_s ).toString() != originalUriLayerName && sublayer.name() != originalUriLayerName;
    } ), res.end() );
  }

  if ( originalUriLayerIdWasSpecified )
  {
    // remove non-matching, unwanted layers by layer id
    res.erase( std::remove_if( res.begin(), res.end(), [uriLayerId]( const QgsProviderSublayerDetails & sublayer )
    {
      return sublayer.layerNumber() != uriLayerId;
    } ), res.end() );
  }

  if ( originalUriGeometryTypeWasSpecified )
  {
    // remove non-matching, unwanted layers by geometry type
    res.erase( std::remove_if( res.begin(), res.end(), [originalGeometryTypeFilter]( const QgsProviderSublayerDetails & sublayer )
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
  const QString path = uriParts.value( u"path"_s ).toString();

  if ( path.isEmpty() )
    return {};

  const QFileInfo fileInfo( path );
  const QString suffix = fileInfo.suffix();

  static QMap< QString, QStringList > sExtensions
  {
    {
      u"shp"_s, {
        u"shx"_s,
        u"dbf"_s,
        u"sbn"_s,
        u"sbx"_s,
        u"prj"_s,
        u"idm"_s,
        u"ind"_s,
        u"qix"_s,
        u"cpg"_s,
        u"qpj"_s,
        u"shp.xml"_s,
      }
    },
    {
      u"tab"_s, {
        u"dat"_s,
        u"id"_s,
        u"map"_s,
        u"ind"_s,
        u"tda"_s,
        u"tin"_s,
        u"tma"_s,
        u"lda"_s,
        u"lin"_s,
        u"lma"_s,
      }
    },
    {
      u"mif"_s, {
        u"mid"_s,
      }
    },
    {
      u"gml"_s, {
        u"gfs"_s,
        u"xsd"_s,
      }
    },
    {
      u"csv"_s, {
        u"csvt"_s,
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

QList<Qgis::LayerType> QgsOgrProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
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
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( uri );
  const QString path = parts.value( u"path"_s ).toString();
  const QFileInfo fi( path );
  if ( fi.suffix().compare( "gpkg"_L1, Qt::CaseInsensitive ) == 0 )
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

#undef TEXT_PROVIDER_KEY
#undef TEXT_PROVIDER_DESCRIPTION

///@endcond
