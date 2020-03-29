/***************************************************************************
                    qgsproviderregistry.cpp  -  Singleton class for
                    registering data providers.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
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

#include "qgsproviderregistry.h"

#include <QString>
#include <QDir>
#include <QLibrary>

#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsdataitemprovider.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsprovidermetadata.h"
#include "qgsvectorlayer.h"
#include "qgsvectortileprovidermetadata.h"
#include "qgsproject.h"
#include "providers/memory/qgsmemoryprovider.h"
#include "providers/gdal/qgsgdalprovider.h"
#include "providers/ogr/qgsogrprovider.h"
#include "providers/meshmemory/qgsmeshmemorydataprovider.h"
#ifdef HAVE_STATIC_PROVIDERS
#include "qgswmsprovider.h"
#include "qgspostgresprovider.h"
#endif

static QgsProviderRegistry *sInstance = nullptr;

QgsProviderRegistry *QgsProviderRegistry::instance( const QString &pluginPath )
{
  if ( !sInstance )
  {
    static QMutex sMutex;
    QMutexLocker locker( &sMutex );
    if ( !sInstance )
    {
      sInstance = new QgsProviderRegistry( pluginPath );
    }
  }
  return sInstance;
} // QgsProviderRegistry::instance


/**
 * Convenience function for finding any existing data providers that match "providerKey"

  Necessary because [] map operator will create a QgsProviderMetadata
  instance.  Also you cannot use the map [] operator in const members for that
  very reason.  So there needs to be a convenient way to find a data provider
  without accidentally adding a null meta data item to the metadata map.
*/
static
QgsProviderMetadata *findMetadata_( const QgsProviderRegistry::Providers &metaData,
                                    const QString &providerKey )
{
  // first do case-sensitive match
  QgsProviderRegistry::Providers::const_iterator i =
    metaData.find( providerKey );

  if ( i != metaData.end() )
  {
    return i->second;
  }

  // fallback to case-insensitive match
  for ( auto it = metaData.begin(); it != metaData.end(); ++it )
  {
    if ( providerKey.compare( it->first, Qt::CaseInsensitive ) == 0 )
      return it->second;
  }

  return nullptr;
}

QgsProviderRegistry::QgsProviderRegistry( const QString &pluginPath )
{
  // At startup, examine the libs in the qgis/lib dir and store those that
  // are a provider shared lib
  // check all libs in the current plugin directory and get name and descriptions
  //TODO figure out how to register and identify data source plugin for a specific
  //TODO layer type
#if 0
  char **argv = qApp->argv();
  QString appDir = argv[0];
  int bin = appDir.findRev( "/bin", -1, false );
  QString baseDir = appDir.left( bin );
  QString mLibraryDirectory = baseDir + "/lib";
#endif
  mLibraryDirectory.setPath( pluginPath );
  init();
}

void QgsProviderRegistry::init()
{
  // add static providers
  Q_NOWARN_DEPRECATED_PUSH
  mProviders[ QgsMemoryProvider::providerKey() ] = new QgsProviderMetadata( QgsMemoryProvider::providerKey(), QgsMemoryProvider::providerDescription(), &QgsMemoryProvider::createProvider );
  mProviders[ QgsMeshMemoryDataProvider::providerKey() ] = new QgsProviderMetadata( QgsMeshMemoryDataProvider::providerKey(), QgsMeshMemoryDataProvider::providerDescription(), &QgsMeshMemoryDataProvider::createProvider );
  Q_NOWARN_DEPRECATED_POP
  mProviders[ QgsGdalProvider::providerKey() ] = new QgsGdalProviderMetadata();
  mProviders[ QgsOgrProvider::providerKey() ] = new QgsOgrProviderMetadata();
  QgsProviderMetadata *vt = new QgsVectorTileProviderMetadata();
  mProviders[ vt->key() ] = vt;
#ifdef HAVE_STATIC_PROVIDERS
  mProviders[ QgsWmsProvider::providerKey() ] = new QgsWmsProviderMetadata();
  mProviders[ QgsPostgresProvider::providerKey() ] = new QgsPostgresProviderMetadata();
#endif

  // add dynamic providers
#ifdef HAVE_STATIC_PROVIDERS
  QgsDebugMsg( QStringLiteral( "Forced only static providers" ) );
#else
  typedef QgsProviderMetadata *factory_function( );

  mLibraryDirectory.setSorting( QDir::Name | QDir::IgnoreCase );
  mLibraryDirectory.setFilter( QDir::Files | QDir::NoSymLinks );

#if defined(Q_OS_WIN) || defined(__CYGWIN__)
  mLibraryDirectory.setNameFilters( QStringList( "*.dll" ) );
#elif defined(ANDROID)
  mLibraryDirectory.setNameFilters( QStringList( "*provider.so" ) );
#else
  mLibraryDirectory.setNameFilters( QStringList( QStringLiteral( "*.so" ) ) );
#endif

  QgsDebugMsg( QStringLiteral( "Checking %1 for provider plugins" ).arg( mLibraryDirectory.path() ) );

  if ( mLibraryDirectory.count() == 0 )
  {
    QgsDebugMsg( QStringLiteral( "No dynamic QGIS data provider plugins found in:\n%1\n" ).arg( mLibraryDirectory.path() ) );
  }

  // provider file regex pattern, only files matching the pattern are loaded if the variable is defined
  QString filePattern = getenv( "QGIS_PROVIDER_FILE" );
  QRegExp fileRegexp;
  if ( !filePattern.isEmpty() )
  {
    fileRegexp.setPattern( filePattern );
  }

  typedef std::vector<QgsProviderMetadata *> *multiple_factory_function();

  const auto constEntryInfoList = mLibraryDirectory.entryInfoList();
  for ( const QFileInfo &fi : constEntryInfoList )
  {
    if ( !fileRegexp.isEmpty() )
    {
      if ( fileRegexp.indexIn( fi.fileName() ) == -1 )
      {
        QgsDebugMsg( "provider " + fi.fileName() + " skipped because doesn't match pattern " + filePattern );
        continue;
      }
    }

    QLibrary myLib( fi.filePath() );
    if ( !myLib.load() )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (lib not loadable): %2" ).arg( myLib.fileName(), myLib.errorString() ) );
      continue;
    }

    QFunctionPointer multi_func = myLib.resolve( QStringLiteral( "multipleProviderMetadataFactory" ).toLatin1().data() );
    multiple_factory_function *multi_function = reinterpret_cast< multiple_factory_function * >( cast_to_fptr( multi_func ) );
    if ( multi_function )
    {
      std::vector<QgsProviderMetadata *> *metadatas = multi_function();
      for ( const auto meta : *metadatas )
      {
        if ( findMetadata_( mProviders, meta->key() ) )
        {
          QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (key %2 already registered)" ).arg( myLib.fileName() ).arg( meta->key() ) );
          delete meta;
          continue;
        }
        // add this provider to the provider map
        mProviders[meta->key()] = meta;
      }
      delete metadatas;
    }
    else
    {
      QFunctionPointer func = myLib.resolve( QStringLiteral( "providerMetadataFactory" ).toLatin1().data() );
      factory_function *function = reinterpret_cast< factory_function * >( cast_to_fptr( func ) );
      if ( !function )
      {
        QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (no providerMetadataFactory method)" ).arg( myLib.fileName() ) );
        continue;
      }

      QgsProviderMetadata *meta = function();
      if ( !meta )
      {
        QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (no metadata returned)" ).arg( myLib.fileName() ) );
        continue;
      }

      if ( findMetadata_( mProviders, meta->key() ) )
      {
        QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (key %2 already registered)" ).arg( myLib.fileName() ).arg( meta->key() ) );
        delete meta;
        continue;
      }
      // add this provider to the provider map
      mProviders[meta->key()] = meta;
    }
  }
#endif
  QgsDebugMsg( QStringLiteral( "Loaded %1 providers (%2) " ).arg( mProviders.size() ).arg( providerList().join( ';' ) ) );

  // now initialize all providers
  for ( Providers::const_iterator it = mProviders.begin(); it != mProviders.end(); ++it )
  {
    const QString &key = it->first;
    Q_UNUSED( key );  // avoid unused variable warning in release build
    QgsProviderMetadata *meta = it->second;

    // now get vector file filters, if any
    QString fileVectorFilters = meta->filters( QgsProviderMetadata::FilterType::FilterVector );
    if ( !fileVectorFilters.isEmpty() )
    {
      mVectorFileFilters += fileVectorFilters;
      QgsDebugMsgLevel( QStringLiteral( "Checking %1: ...loaded OK (%2 file filters)" ).arg( key ).arg( fileVectorFilters.split( ";;" ).count() ), 2 );
    }

    // now get raster file filters, if any
    QString fileRasterFilters = meta->filters( QgsProviderMetadata::FilterType::FilterRaster );
    if ( !fileRasterFilters.isEmpty() )
    {
      QgsDebugMsgLevel( "raster filters: " + fileRasterFilters, 2 );
      mRasterFileFilters += fileRasterFilters;
      QgsDebugMsgLevel( QStringLiteral( "Checking %1: ...loaded OK (%2 file filters)" ).arg( key ).arg( fileRasterFilters.split( ";;" ).count() ), 2 );
    }

    // now get mesh file filters, if any
    QString fileMeshFilters = meta->filters( QgsProviderMetadata::FilterType::FilterMesh );
    if ( !fileMeshFilters.isEmpty() )
    {
      mMeshFileFilters += fileMeshFilters;
      QgsDebugMsgLevel( QStringLiteral( "Checking %1: ...loaded OK (%2 file mesh filters)" ).arg( key ).arg( mMeshFileFilters.split( ";;" ).count() ), 2 );

    }

    QString fileMeshDatasetFilters = meta->filters( QgsProviderMetadata::FilterType::FilterMeshDataset );
    if ( !fileMeshDatasetFilters.isEmpty() )
    {
      mMeshDatasetFileFilters += fileMeshDatasetFilters;
      QgsDebugMsgLevel( QStringLiteral( "Checking %1: ...loaded OK (%2 file dataset filters)" ).arg( key ).arg( mMeshDatasetFileFilters.split( ";;" ).count() ), 2 );
    }

    // call initProvider() - allows provider to register its services to QGIS
    meta->initProvider();
  }

  // load database drivers (only OGR)
  mDatabaseDrivers = QgsOgrProviderUtils::databaseDrivers();

  // load directory drivers (only OGR)
  mDirectoryDrivers =  QgsOgrProviderUtils::directoryDrivers();

  // load protocol drivers (only OGR)
  mProtocolDrivers =  QgsOgrProviderUtils::protocolDrivers();
} // QgsProviderRegistry ctor


// typedef for the unload dataprovider function
typedef void cleanupProviderFunction_t();

void QgsProviderRegistry::clean()
{
  // avoid recreating a new project just to clean it
  if ( QgsProject::sProject )
    QgsProject::instance()->removeAllMapLayers();

  Providers::const_iterator it = mProviders.begin();

  while ( it != mProviders.end() )
  {
    QgsDebugMsgLevel( QStringLiteral( "cleanup:%1" ).arg( it->first ), 5 );
    it->second->cleanupProvider();
    delete it->second;
    ++it;
  }
  mProviders.clear();
}

bool QgsProviderRegistry::exists()
{
  return static_cast< bool >( sInstance );
}

QgsProviderRegistry::~QgsProviderRegistry()
{
  clean();
  if ( sInstance == this )
    sInstance = nullptr;
}

QString QgsProviderRegistry::library( QString const &providerKey ) const
{
  QgsProviderMetadata *md = findMetadata_( mProviders, providerKey );

  if ( md )
  {
    Q_NOWARN_DEPRECATED_PUSH
    return md->library();
    Q_NOWARN_DEPRECATED_POP
  }

  return QString();
}

QString QgsProviderRegistry::pluginList( bool asHTML ) const
{
  Providers::const_iterator it = mProviders.begin();

  if ( mProviders.empty() )
    return QObject::tr( "No data provider plugins are available. No vector layers can be loaded" );

  QString list;

  if ( asHTML )
    list += QLatin1String( "<ol>" );

  while ( it != mProviders.end() )
  {
    if ( asHTML )
      list += QLatin1String( "<li>" );

    list += it->second->description();

    if ( asHTML )
      list += QLatin1String( "<br></li>" );
    else
      list += '\n';

    ++it;
  }

  if ( asHTML )
    list += QLatin1String( "</ol>" );

  return list;
}

void QgsProviderRegistry::setLibraryDirectory( QDir const &path )
{
  mLibraryDirectory = path;
  clean();
  init();
}

QDir QgsProviderRegistry::libraryDirectory() const
{
  return mLibraryDirectory;
}


/* Copied from QgsVectorLayer::setDataProvider
 *  TODO: Make it work in the generic environment
 *
 *  TODO: Is this class really the best place to put a data provider loader?
 *        It seems more sensible to provide the code in one place rather than
 *        in qgsrasterlayer, qgsvectorlayer, serversourceselect, etc.
 */
QgsDataProvider *QgsProviderRegistry::createProvider( QString const &providerKey, QString const &dataSource, const QgsDataProvider::ProviderOptions &options )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  QgsProviderMetadata *metadata = findMetadata_( mProviders, providerKey );
  if ( !metadata )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid data provider %1" ).arg( providerKey ) );
    return nullptr;
  }

  return metadata->createProvider( dataSource, options );
}

int QgsProviderRegistry::providerCapabilities( const QString &providerKey ) const
{
  const QList< QgsDataItemProvider * > itemProviders = dataItemProviders( providerKey );
  int ret = QgsDataProvider::NoDataCapabilities;
  //concat flags
  for ( const QgsDataItemProvider *itemProvider : itemProviders )
  {
    ret = ret | itemProvider->capabilities();
  }
  return ret;
}

QVariantMap QgsProviderRegistry::decodeUri( const QString &providerKey, const QString &uri )
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->decodeUri( uri );
  else
    return QVariantMap();
}

QString QgsProviderRegistry::encodeUri( const QString &providerKey, const QVariantMap &parts )
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->encodeUri( parts );
  else
    return QString();
}

QgsVectorLayerExporter::ExportError QgsProviderRegistry::createEmptyLayer( const QString &providerKey,
    const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite, QMap<int, int> &oldToNewAttrIdxMap,
    QString &errorMessage,
    const QMap<QString, QVariant> *options )
{
  QgsVectorLayerExporter::ExportError ret;

  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->createEmptyLayer( uri, fields, wkbType, srs, overwrite, oldToNewAttrIdxMap, errorMessage, options );
  else
  {
    ret = QgsVectorLayerExporter::ErrInvalidProvider;
    errorMessage = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
  }

  return ret;
}

QgsRasterDataProvider *QgsProviderRegistry::createRasterDataProvider( const QString &providerKey, const QString &uri, const QString &format,
    int nBands, Qgis::DataType type, int width, int height,
    double *geoTransform, const QgsCoordinateReferenceSystem &crs,
    const QStringList &createOptions )
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->createRasterDataProvider( uri, format, nBands, type, width, height, geoTransform, crs, createOptions );
  else
    return nullptr;
}

QList<QPair<QString, QString> > QgsProviderRegistry::pyramidResamplingMethods( const QString &providerKey )
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->pyramidResamplingMethods();
  else
    return QList<QPair<QString, QString> >();
}

QList<QgsDataItemProvider *> QgsProviderRegistry::dataItemProviders( const QString &providerKey ) const
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->dataItemProviders();
  else
    return QList<QgsDataItemProvider *>();
}

int QgsProviderRegistry::listStyles( const QString &providerKey, const QString &uri, QStringList &ids, QStringList &names, QStringList &descriptions, QString &errCause )
{
  int res = -1;
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
  {
    res = meta->listStyles( uri, ids, names, descriptions, errCause );
  }
  else
  {
    errCause = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
  }
  return res;
}

QString QgsProviderRegistry::getStyleById( const QString &providerKey, const QString &uri, QString styleId, QString &errCause )
{
  QString ret;
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
  {
    ret = meta->getStyleById( uri, styleId, errCause );
  }
  else
  {
    errCause = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
  }
  return ret;
}

bool QgsProviderRegistry::deleteStyleById( const QString &providerKey, const QString &uri, QString styleId, QString &errCause )
{
  bool ret( false );

  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->deleteStyleById( uri, styleId, errCause );
  else
  {
    errCause = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
  }
  return ret;
}

bool QgsProviderRegistry::saveStyle( const QString &providerKey, const QString &uri, const QString &qmlStyle,
                                     const QString &sldStyle, const QString &styleName, const QString &styleDescription,
                                     const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  bool ret( false );
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    ret = meta->saveStyle( uri, qmlStyle, sldStyle, styleName, styleDescription,
                           uiFileContent, useAsDefault, errCause );
  else
  {
    errCause = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
  }
  return ret;
}

QString QgsProviderRegistry::loadStyle( const QString &providerKey, const QString &uri, QString &errCause )
{
  QString ret;
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    ret = meta->loadStyle( uri, errCause );
  else
  {
    errCause = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
  }
  return ret;
}

bool QgsProviderRegistry::createDb( const QString &providerKey, const QString &dbPath, QString &errCause )
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->createDb( dbPath, errCause );
  else
  {
    errCause = QStringLiteral( "Resolving createDb(...) failed" );
    return false;
  }
}

QgsTransaction *QgsProviderRegistry::createTransaction( const QString &providerKey, const QString &connString )
{
  QgsProviderMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->createTransaction( connString );
  else
    return nullptr;
}

QWidget *QgsProviderRegistry::createSelectionWidget( const QString &providerKey,
    QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  Q_UNUSED( providerKey );
  Q_UNUSED( parent );
  Q_UNUSED( fl );
  Q_UNUSED( widgetMode );
  QgsDebugMsg( "deprecated call - use QgsGui::sourceSelectProviderRegistry()->createDataSourceWidget() instead" );
  return nullptr;
}

QFunctionPointer QgsProviderRegistry::function( QString const &providerKey,
    QString const &functionName )
{
  Q_NOWARN_DEPRECATED_PUSH
  QString lib = library( providerKey );
  Q_NOWARN_DEPRECATED_POP
  if ( lib.isEmpty() )
    return nullptr;

  QLibrary myLib( lib );

  QgsDebugMsg( "Library name is " + myLib.fileName() );

  if ( myLib.load() )
  {
    return myLib.resolve( functionName.toLatin1().data() );
  }
  else
  {
    QgsDebugMsg( "Cannot load library: " + myLib.errorString() );
    return nullptr;
  }
}

QLibrary *QgsProviderRegistry::createProviderLibrary( QString const &providerKey ) const
{
  Q_NOWARN_DEPRECATED_PUSH
  QString lib = library( providerKey );
  Q_NOWARN_DEPRECATED_POP
  if ( lib.isEmpty() )
    return nullptr;

  std::unique_ptr< QLibrary > myLib( new QLibrary( lib ) );

  QgsDebugMsg( "Library name is " + myLib->fileName() );

  if ( myLib->load() )
    return myLib.release();

  QgsDebugMsg( "Cannot load library: " + myLib->errorString() );

  return nullptr;
}

void QgsProviderRegistry::registerGuis( QWidget * )
{
  QgsDebugMsg( "deprecated - use QgsGui::providerGuiRegistry() instead." );
}

bool QgsProviderRegistry::registerProvider( QgsProviderMetadata *providerMetadata )
{
  if ( providerMetadata )
  {
    if ( mProviders.find( providerMetadata->key() ) == mProviders.end() )
    {
      mProviders[ providerMetadata->key() ] = providerMetadata;
      return true;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Cannot register provider metadata: a provider with the same key (%1) was already registered!" ).arg( providerMetadata->key() ), 2 );
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Trying to register a null metadata provider!" ), 2 );
  }
  return false;
}

QString QgsProviderRegistry::fileVectorFilters() const
{
  return mVectorFileFilters;
}

QString QgsProviderRegistry::fileRasterFilters() const
{
  return mRasterFileFilters;
}

QString QgsProviderRegistry::fileMeshFilters() const
{
  return mMeshFileFilters;
}

QString QgsProviderRegistry::fileMeshDatasetFilters() const
{
  return mMeshDatasetFileFilters;
}

QString QgsProviderRegistry::databaseDrivers() const
{
  return mDatabaseDrivers;
}

QString QgsProviderRegistry::directoryDrivers() const
{
  return mDirectoryDrivers;
}

QString QgsProviderRegistry::protocolDrivers() const
{
  return mProtocolDrivers;
}

QStringList QgsProviderRegistry::providerList() const
{
  QStringList lst;
  for ( Providers::const_iterator it = mProviders.begin(); it != mProviders.end(); ++it )
  {
    lst.append( it->first );
  }
  return lst;
}

QgsProviderMetadata *QgsProviderRegistry::providerMetadata( const QString &providerKey ) const
{
  return findMetadata_( mProviders, providerKey );
}
