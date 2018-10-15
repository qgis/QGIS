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
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsprovidermetadata.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "providers/memory/qgsmemoryprovider.h"
#include "mesh/qgsmeshmemorydataprovider.h"

// typedefs for provider plugin functions of interest
typedef QString providerkey_t();
typedef QString description_t();
typedef bool    isprovider_t();
typedef QString fileVectorFilters_t();
typedef void buildsupportedrasterfilefilter_t( QString &fileFiltersString );
typedef QString databaseDrivers_t();
typedef QString directoryDrivers_t();
typedef QString protocolDrivers_t();
typedef void initProviderFunction_t();
typedef QVariantMap decodeUri_t( const QString &uri );

//typedef int dataCapabilities_t();
//typedef QgsDataItem * dataItem_t(QString);

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
  mLibraryDirectory = pluginPath;
  init();
}


void QgsProviderRegistry::init()
{
  // add standard providers
  mProviders[ QgsMemoryProvider::providerKey() ] = new QgsProviderMetadata( QgsMemoryProvider::providerKey(), QgsMemoryProvider::providerDescription(), &QgsMemoryProvider::createProvider );
  mProviders[ QgsMeshMemoryDataProvider::providerKey() ] = new QgsProviderMetadata( QgsMeshMemoryDataProvider::providerKey(), QgsMeshMemoryDataProvider::providerDescription(), &QgsMeshMemoryDataProvider::createProvider );

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
    QString msg = QObject::tr( "No QGIS data provider plugins found in:\n%1\n" ).arg( mLibraryDirectory.path() );
    msg += QObject::tr( "No vector layers can be loaded. Check your QGIS installation" );

    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( QObject::tr( "No Data Providers" ) );
    output->setMessage( msg, QgsMessageOutput::MessageText );
    output->showMessage();
    return;
  }

  // provider file regex pattern, only files matching the pattern are loaded if the variable is defined
  QString filePattern = getenv( "QGIS_PROVIDER_FILE" );
  QRegExp fileRegexp;
  if ( !filePattern.isEmpty() )
  {
    fileRegexp.setPattern( filePattern );
  }

  Q_FOREACH ( const QFileInfo &fi, mLibraryDirectory.entryInfoList() )
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

    //MH: Added a further test to detect non-provider plugins linked to provider plugins.
    //Only pure provider plugins have 'type' not defined
    isprovider_t *hasType = reinterpret_cast< isprovider_t * >( cast_to_fptr( myLib.resolve( "type" ) ) );
    if ( hasType )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (has type method)" ).arg( myLib.fileName() ) );
      continue;
    }

    // get the description and the key for the provider plugin
    isprovider_t *isProvider = reinterpret_cast< isprovider_t * >( cast_to_fptr( myLib.resolve( "isProvider" ) ) );
    if ( !isProvider )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (no isProvider method)" ).arg( myLib.fileName() ) );
      continue;
    }

    // check to see if this is a provider plugin
    if ( !isProvider() )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (not a provider)" ).arg( myLib.fileName() ) );
      continue;
    }

    // looks like a provider. get the key and description
    description_t *pDesc = reinterpret_cast< description_t * >( cast_to_fptr( myLib.resolve( "description" ) ) );
    if ( !pDesc )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (no description method)" ).arg( myLib.fileName() ) );
      continue;
    }

    providerkey_t *pKey = reinterpret_cast< providerkey_t * >( cast_to_fptr( myLib.resolve( "providerKey" ) ) );
    if ( !pKey )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (no providerKey method)" ).arg( myLib.fileName() ) );
      continue;
    }

    // add this provider to the provider map
    mProviders[pKey()] = new QgsProviderMetadata( pKey(), pDesc(), myLib.fileName() );

    // load database drivers
    databaseDrivers_t *pDatabaseDrivers = reinterpret_cast< databaseDrivers_t * >( cast_to_fptr( myLib.resolve( "databaseDrivers" ) ) );
    if ( pDatabaseDrivers )
    {
      mDatabaseDrivers = pDatabaseDrivers();
    }

    // load directory drivers
    directoryDrivers_t *pDirectoryDrivers = reinterpret_cast< directoryDrivers_t * >( cast_to_fptr( myLib.resolve( "directoryDrivers" ) ) );
    if ( pDirectoryDrivers )
    {
      mDirectoryDrivers = pDirectoryDrivers();
    }

    // load protocol drivers
    protocolDrivers_t *pProtocolDrivers = reinterpret_cast< protocolDrivers_t * >( cast_to_fptr( myLib.resolve( "protocolDrivers" ) ) );
    if ( pProtocolDrivers )
    {
      mProtocolDrivers = pProtocolDrivers();
    }

    // now get vector file filters, if any
    fileVectorFilters_t *pFileVectorFilters = reinterpret_cast< fileVectorFilters_t * >( cast_to_fptr( myLib.resolve( "fileVectorFilters" ) ) );
    if ( pFileVectorFilters )
    {
      QString fileVectorFilters = pFileVectorFilters();

      if ( !fileVectorFilters.isEmpty() )
        mVectorFileFilters += fileVectorFilters;

      QgsDebugMsg( QStringLiteral( "Checking %1: ...loaded OK (%2 file filters)" ).arg( myLib.fileName() ).arg( fileVectorFilters.split( ";;" ).count() ) );
    }

    // now get raster file filters, if any
    // this replaces deprecated QgsRasterLayer::buildSupportedRasterFileFilter
    buildsupportedrasterfilefilter_t *pBuild =
      reinterpret_cast< buildsupportedrasterfilefilter_t * >( cast_to_fptr( myLib.resolve( "buildSupportedRasterFileFilter" ) ) );
    if ( pBuild )
    {
      QString fileRasterFilters;
      pBuild( fileRasterFilters );

      QgsDebugMsg( "raster filters: " + fileRasterFilters );
      if ( !fileRasterFilters.isEmpty() )
        mRasterFileFilters += fileRasterFilters;

      QgsDebugMsg( QStringLiteral( "Checking %1: ...loaded OK (%2 file filters)" ).arg( myLib.fileName() ).arg( fileRasterFilters.split( ";;" ).count() ) );
    }

    // call initProvider() if such function is available - allows provider to register its services to QGIS
    initProviderFunction_t *initFunc = reinterpret_cast< initProviderFunction_t * >( cast_to_fptr( myLib.resolve( "initProvider" ) ) );
    if ( initFunc )
      initFunc();
  }
} // QgsProviderRegistry ctor


// typedef for the unload dataprovider function
typedef void cleanupProviderFunction_t();

void QgsProviderRegistry::clean()
{
  QgsProject::instance()->removeAllMapLayers();

  Providers::const_iterator it = mProviders.begin();

  while ( it != mProviders.end() )
  {
    QgsDebugMsgLevel( QStringLiteral( "cleanup:%1" ).arg( it->first ), 5 );
    QString lib = it->second->library();
    if ( !lib.isEmpty() )
    {
      QLibrary myLib( lib );
      if ( myLib.isLoaded() )
      {
        cleanupProviderFunction_t *cleanupFunc = reinterpret_cast< cleanupProviderFunction_t * >( cast_to_fptr( myLib.resolve( "cleanupProvider" ) ) );
        if ( cleanupFunc )
          cleanupFunc();
      }
    }
    delete it->second;
    ++it;
  }
  mProviders.clear();
}

QgsProviderRegistry::~QgsProviderRegistry()
{
  clean();
  if ( sInstance == this )
    sInstance = nullptr;
}


/**
 * Convenience function for finding any existing data providers that match "providerKey"

  Necessary because [] map operator will create a QgsProviderMetadata
  instance.  Also you cannot use the map [] operator in const members for that
  very reason.  So there needs to be a convenient way to find a data provider
  without accidentally adding a null meta data item to the metadata map.
*/
static
QgsProviderMetadata *findMetadata_( QgsProviderRegistry::Providers const &metaData,
                                    QString const &providerKey )
{
  QgsProviderRegistry::Providers::const_iterator i =
    metaData.find( providerKey );

  if ( i != metaData.end() )
  {
    return i->second;
  }

  return nullptr;
} // findMetadata_



QString QgsProviderRegistry::library( QString const &providerKey ) const
{
  QgsProviderMetadata *md = findMetadata_( mProviders, providerKey );

  if ( md )
  {
    return md->library();
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



// typedef for the QgsDataProvider class factory
typedef QgsDataProvider *classFactoryFunction_t( const QString *, const QgsDataProvider::ProviderOptions &options );


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

  const QgsProviderMetadata *metadata = providerMetadata( providerKey );
  if ( !metadata )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid data provider %1" ).arg( providerKey ) );
    return nullptr;
  }

  if ( metadata->createFunction() )
  {
    return metadata->createFunction()( dataSource, options );
  }

  // load the plugin
  QString lib = library( providerKey );

#ifdef TESTPROVIDERLIB
  const char *cLib = lib.toUtf8();

  // test code to help debug provider loading problems
  //  void *handle = dlopen(cLib, RTLD_LAZY);
  void *handle = dlopen( cOgrLib, RTLD_LAZY | RTLD_GLOBAL );
  if ( !handle )
  {
    QgsLogger::warning( "Error in dlopen" );
  }
  else
  {
    QgsDebugMsg( "dlopen succeeded" );
    dlclose( handle );
  }

#endif
  // load the data provider
  QLibrary myLib( lib );

  QgsDebugMsg( "Library name is " + myLib.fileName() );
  if ( !myLib.load() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load %1: %2" ).arg( lib, myLib.errorString() ) );
    return nullptr;
  }

  classFactoryFunction_t *classFactory = reinterpret_cast< classFactoryFunction_t * >( cast_to_fptr( myLib.resolve( "classFactory" ) ) );
  if ( !classFactory )
  {
    QgsDebugMsg( QStringLiteral( "Failed to load %1: no classFactory method" ).arg( lib ) );
    return nullptr;
  }

  QgsDataProvider *dataProvider = classFactory( &dataSource, options );
  if ( !dataProvider )
  {
    QgsMessageLog::logMessage( QObject::tr( "Unable to instantiate the data provider plugin %1" ).arg( lib ) );
    myLib.unload();
    return nullptr;
  }

  QgsDebugMsg( QStringLiteral( "Instantiated the data provider plugin: %1" ).arg( dataProvider->name() ) );
  return dataProvider;
} // QgsProviderRegistry::setDataProvider

QVariantMap QgsProviderRegistry::decodeUri( const QString &providerKey, const QString &uri )
{
  std::unique_ptr< QLibrary > library( createProviderLibrary( providerKey ) );
  if ( !library )
  {
    return QVariantMap();
  }

  decodeUri_t *decodeUri = reinterpret_cast< decodeUri_t *>( cast_to_fptr( library->resolve( "decodeUri" ) ) );
  if ( !decodeUri )
  {
    return QVariantMap();
  }
  return decodeUri( uri );
}

int QgsProviderRegistry::providerCapabilities( const QString &providerKey ) const
{
  std::unique_ptr< QLibrary > library( createProviderLibrary( providerKey ) );
  if ( !library )
  {
    return QgsDataProvider::NoDataCapabilities;
  }

  dataCapabilities_t *dataCapabilities = reinterpret_cast< dataCapabilities_t *>( cast_to_fptr( library->resolve( "dataCapabilities" ) ) );
  if ( !dataCapabilities )
  {
    return QgsDataProvider::NoDataCapabilities;
  }

  return dataCapabilities();
}

// This should be QWidget, not QDialog
typedef QWidget *selectFactoryFunction_t( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode );

QWidget *QgsProviderRegistry::createSelectionWidget( const QString &providerKey,
    QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  selectFactoryFunction_t *selectFactory =
    reinterpret_cast< selectFactoryFunction_t * >( cast_to_fptr( function( providerKey, "selectWidget" ) ) );

  if ( !selectFactory )
    return nullptr;

  return selectFactory( parent, fl, widgetMode );
}

QFunctionPointer QgsProviderRegistry::function( QString const &providerKey,
    QString const &functionName )
{
  QString lib = library( providerKey );
  if ( lib.isEmpty() )
    return nullptr;

  QLibrary myLib( library( providerKey ) );

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
  QString lib = library( providerKey );
  if ( lib.isEmpty() )
    return nullptr;

  std::unique_ptr< QLibrary > myLib( new QLibrary( lib ) );

  QgsDebugMsg( "Library name is " + myLib->fileName() );

  if ( myLib->load() )
    return myLib.release();

  QgsDebugMsg( "Cannot load library: " + myLib->errorString() );

  return nullptr;
}

void QgsProviderRegistry::registerGuis( QWidget *parent )
{
  typedef void registerGui_function( QWidget * parent );

  Q_FOREACH ( const QString &provider, providerList() )
  {
    registerGui_function *registerGui = reinterpret_cast< registerGui_function * >( cast_to_fptr( function( provider, "registerGui" ) ) );

    if ( !registerGui )
      continue;

    registerGui( parent );
  }
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

const QgsProviderMetadata *QgsProviderRegistry::providerMetadata( const QString &providerKey ) const
{
  return findMetadata_( mProviders, providerKey );
}
