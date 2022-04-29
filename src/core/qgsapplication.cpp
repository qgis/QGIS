/***************************************************************************
    qgsapplication.cpp - Accessors for application-wide data
     --------------------------------------
    Date                 : 02-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslocalizeddatapathregistry.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgsannotationitemregistry.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnetworkcontentfetcherregistry.h"
#include "qgsnetworkreply.h"
#include "qgsproviderregistry.h"
#include "qgsexpression.h"
#include "qgsactionscoperegistry.h"
#include "qgsruntimeprofiler.h"
#include "qgstaskmanager.h"
#include "qgsnumericformatregistry.h"
#include "qgsfieldformatterregistry.h"
#include "qgsscalebarrendererregistry.h"
#include "qgssvgcache.h"
#include "qgsimagecache.h"
#include "qgssourcecache.h"
#include "qgscolorschemeregistry.h"
#include "qgspainteffectregistry.h"
#include "qgsprojectstorageregistry.h"
#include "qgsexternalstorageregistry.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrendererregistry.h"
#include "qgspointcloudrendererregistry.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbollayerutils.h"
#include "qgscalloutsregistry.h"
#include "qgspluginlayerregistry.h"
#include "qgsclassificationmethodregistry.h"
#include "qgsmessagelog.h"
#include "qgsannotationregistry.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgstiledownloadmanager.h"
#include "qgsunittypes.h"
#include "qgsuserprofile.h"
#include "qgsuserprofilemanager.h"
#include "qgsreferencedgeometry.h"
#include "qgs3drendererregistry.h"
#include "qgs3dsymbolregistry.h"
#include "qgslayoutrendercontext.h"
#include "qgssqliteutils.h"
#include "qgsstyle.h"
#include "qgsprojutils.h"
#include "qgsvaliditycheckregistry.h"
#include "qgsnewsfeedparser.h"
#include "qgsbookmarkmanager.h"
#include "qgsstylemodel.h"
#include "qgsconnectionregistry.h"
#include "qgsremappingproxyfeaturesink.h"
#include "qgsmeshlayer.h"
#include "qgsfeaturestore.h"
#include "qgslocator.h"
#include "qgsreadwritelocker.h"
#include "qgsbabelformatregistry.h"
#include "qgsdbquerylog.h"

#include "gps/qgsgpsconnectionregistry.h"
#include "processing/qgsprocessingregistry.h"
#include "processing/models/qgsprocessingmodelchildparametersource.h"
#include "processing/models/qgsprocessingmodelchilddependency.h"

#include "layout/qgspagesizeregistry.h"
#include "qgsrecentstylehandler.h"
#include "qgsdatetimefieldformatter.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#endif
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QPalette>
#include <QProcess>
#include <QProcessEnvironment>
#include <QIcon>
#include <QPixmap>
#include <QThreadPool>
#include <QLocale>
#include <QStyle>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QTextStream>
#include <QScreen>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QRecursiveMutex>
#endif

#ifndef Q_OS_WIN
#include <netinet/in.h>
#include <pwd.h>
#else
#include <winsock.h>
#include <windows.h>
#include <lmcons.h>
#define SECURITY_WIN32
#include <security.h>
#ifdef _MSC_VER
#pragma comment( lib, "Secur32.lib" )
#endif
#endif

#include "qgsconfig.h"

#include <gdal.h>
#include <ogr_api.h>
#include <cpl_conv.h> // for setting gdal options
#include <sqlite3.h>
#include <mutex>

#include <proj.h>

#if defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#endif

#define CONN_POOL_MAX_CONCURRENT_CONNS      4

QObject *ABISYM( QgsApplication::mFileOpenEventReceiver ) = nullptr;
bool ABISYM( QgsApplication::mInitialized ) = false;
bool ABISYM( QgsApplication::mRunningFromBuildDir ) = false;
const char *QgsApplication::QGIS_ORGANIZATION_NAME = "QGIS";
const char *QgsApplication::QGIS_ORGANIZATION_DOMAIN = "qgis.org";
const char *QgsApplication::QGIS_APPLICATION_NAME = "QGIS3";
QgsApplication::ApplicationMembers *QgsApplication::sApplicationMembers = nullptr;
QgsAuthManager *QgsApplication::sAuthManager = nullptr;
int ABISYM( QgsApplication::sMaxThreads ) = -1;

Q_GLOBAL_STATIC( QStringList, sFileOpenEventList )
Q_GLOBAL_STATIC( QString, sPrefixPath )
Q_GLOBAL_STATIC( QString, sPluginPath )
Q_GLOBAL_STATIC( QString, sPkgDataPath )
Q_GLOBAL_STATIC( QString, sLibraryPath )
Q_GLOBAL_STATIC( QString, sLibexecPath )
Q_GLOBAL_STATIC( QString, sQmlImportPath )
Q_GLOBAL_STATIC( QString, sThemeName )
Q_GLOBAL_STATIC( QString, sProfilePath )

Q_GLOBAL_STATIC( QStringList, sDefaultSvgPaths )
Q_GLOBAL_STATIC( QgsStringMap, sSystemEnvVars )
Q_GLOBAL_STATIC( QString, sConfigPath )

Q_GLOBAL_STATIC( QString, sBuildSourcePath )
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
Q_GLOBAL_STATIC( QString, sCfgIntDir )
#endif
Q_GLOBAL_STATIC( QString, sBuildOutputPath )
Q_GLOBAL_STATIC( QStringList, sGdalSkipList )
Q_GLOBAL_STATIC( QStringList, sDeferredSkippedGdalDrivers )
Q_GLOBAL_STATIC( QString, sAuthDbDirPath )

Q_GLOBAL_STATIC( QString, sUserName )
Q_GLOBAL_STATIC( QString, sUserFullName )
Q_GLOBAL_STATIC_WITH_ARGS( QString, sPlatformName, ( "external" ) )
Q_GLOBAL_STATIC( QString, sTranslation )

Q_GLOBAL_STATIC( QTemporaryDir, sIconCacheDir )

QgsApplication::QgsApplication( int &argc, char **argv, bool GUIenabled, const QString &profileFolder, const QString &platformName )
  : QApplication( argc, argv, GUIenabled )
{
  *sPlatformName() = platformName;

  if ( *sTranslation() != QLatin1String( "C" ) )
  {
    mQgisTranslator = new QTranslator();
    if ( mQgisTranslator->load( QStringLiteral( "qgis_" ) + *sTranslation(), i18nPath() ) )
    {
      installTranslator( mQgisTranslator );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "loading of qgis translation failed %1/qgis_%2" ).arg( i18nPath(), *sTranslation() ), 2 );
    }

    /* Translation file for Qt.
     * The strings from the QMenuBar context section are used by Qt/Mac to shift
     * the About, Preferences and Quit items to the Mac Application menu.
     * These items must be translated identically in both qt_ and qgis_ files.
     */
    QString qtTranslationsPath = QLibraryInfo::location( QLibraryInfo::TranslationsPath );
#ifdef __MINGW32__
    QString prefix = QDir( QString( "%1/../" ).arg( QApplication::applicationDirPath() ) ).absolutePath();
    qtTranslationsPath = prefix + qtTranslationsPath.mid( QLibraryInfo::location( QLibraryInfo::PrefixPath ).length() );
#endif

    mQtTranslator = new QTranslator();
    if ( mQtTranslator->load( QStringLiteral( "qt_" ) + *sTranslation(), qtTranslationsPath ) )
    {
      installTranslator( mQtTranslator );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "loading of qt translation failed %1/qt_%2" ).arg( qtTranslationsPath, *sTranslation() ), 2 );
    }

    mQtBaseTranslator = new QTranslator();
    if ( mQtBaseTranslator->load( QStringLiteral( "qtbase_" ) + *sTranslation(), qtTranslationsPath ) )
    {
      installTranslator( mQtBaseTranslator );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "loading of qtbase translation failed %1/qt_%2" ).arg( qtTranslationsPath, *sTranslation() ), 2 );
    }
  }

  mApplicationMembers = new ApplicationMembers();

  *sProfilePath() = profileFolder;

  connect( instance(), &QgsApplication::localeChanged, &QgsDateTimeFieldFormatter::applyLocaleChange );
}

void QgsApplication::init( QString profileFolder )
{
  if ( profileFolder.isEmpty() )
  {
    if ( getenv( "QGIS_CUSTOM_CONFIG_PATH" ) )
    {
      profileFolder = getenv( "QGIS_CUSTOM_CONFIG_PATH" );
    }
    else
    {
      profileFolder = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation ).value( 0 );
    }
    // This will normally get here for custom scripts that use QgsApplication.
    // This doesn't get this hit for QGIS Desktop because we setup the profile via main
    QString rootProfileFolder = QgsUserProfileManager::resolveProfilesFolder( profileFolder );
    QgsUserProfileManager manager( rootProfileFolder );
    QgsUserProfile *profile = manager.getProfile();
    profileFolder = profile->folder();
    delete profile;
  }

  *sProfilePath() = profileFolder;

  static std::once_flag sMetaTypesRegistered;
  std::call_once( sMetaTypesRegistered, []
  {
    qRegisterMetaType<QgsGeometry::Error>( "QgsGeometry::Error" );
    qRegisterMetaType<QgsDatabaseQueryLogEntry>( "QgsDatabaseQueryLogEntry" );
    qRegisterMetaType<QgsProcessingFeatureSourceDefinition>( "QgsProcessingFeatureSourceDefinition" );
    qRegisterMetaType<QgsProcessingOutputLayerDefinition>( "QgsProcessingOutputLayerDefinition" );
    qRegisterMetaType<QgsUnitTypes::LayoutUnit>( "QgsUnitTypes::LayoutUnit" );
    qRegisterMetaType<QgsFeatureId>( "QgsFeatureId" );
    qRegisterMetaType<QgsFields>( "QgsFields" );
    qRegisterMetaType<QgsFeatureIds>( "QgsFeatureIds" );
    qRegisterMetaType<QgsProperty>( "QgsProperty" );
    qRegisterMetaType<QgsFeatureStoreList>( "QgsFeatureStoreList" );
    qRegisterMetaType<Qgis::MessageLevel>( "Qgis::MessageLevel" );
    qRegisterMetaType<Qgis::BrowserItemState>( "Qgis::BrowserItemState" );
    qRegisterMetaType<QgsReferencedRectangle>( "QgsReferencedRectangle" );
    qRegisterMetaType<QgsReferencedPointXY>( "QgsReferencedPointXY" );
    qRegisterMetaType<QgsReferencedGeometry>( "QgsReferencedGeometry" );
    qRegisterMetaType<QgsLayoutRenderContext::Flags>( "QgsLayoutRenderContext::Flags" );
    qRegisterMetaType<QgsStyle::StyleEntity>( "QgsStyle::StyleEntity" );
    qRegisterMetaType<QgsCoordinateReferenceSystem>( "QgsCoordinateReferenceSystem" );
    qRegisterMetaType<QgsAuthManager::MessageLevel>( "QgsAuthManager::MessageLevel" );
    qRegisterMetaType<QgsNetworkRequestParameters>( "QgsNetworkRequestParameters" );
    qRegisterMetaType<QgsNetworkReplyContent>( "QgsNetworkReplyContent" );
    qRegisterMetaType<QgsGeometry>( "QgsGeometry" );
    qRegisterMetaType<QgsDatumTransform::GridDetails>( "QgsDatumTransform::GridDetails" );
    qRegisterMetaType<QgsDatumTransform::TransformDetails>( "QgsDatumTransform::TransformDetails" );
    qRegisterMetaType<QgsNewsFeedParser::Entry>( "QgsNewsFeedParser::Entry" );
    qRegisterMetaType<QgsRectangle>( "QgsRectangle" );
    qRegisterMetaType<QgsLocatorResult>( "QgsLocatorResult" );
    qRegisterMetaType<QgsProcessingModelChildParameterSource>( "QgsProcessingModelChildParameterSource" );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt6 documentation says these are not needed anymore (https://www.qt.io/blog/whats-new-in-qmetatype-qvariant) #spellok
    // TODO: when tests can run against Qt6 builds, check for any regressions
    qRegisterMetaTypeStreamOperators<QgsProcessingModelChildParameterSource>( "QgsProcessingModelChildParameterSource" );
#endif
    qRegisterMetaType<QgsRemappingSinkDefinition>( "QgsRemappingSinkDefinition" );
    qRegisterMetaType<QgsProcessingModelChildDependency>( "QgsProcessingModelChildDependency" );
    qRegisterMetaType<QgsTextFormat>( "QgsTextFormat" );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMetaType::registerComparators<QgsProcessingModelChildDependency>();
    QMetaType::registerEqualsComparator<QgsProcessingFeatureSourceDefinition>();
    QMetaType::registerEqualsComparator<QgsProperty>();
    QMetaType::registerEqualsComparator<QgsDateTimeRange>();
    QMetaType::registerEqualsComparator<QgsDateRange>();
#endif
    qRegisterMetaType<QPainter::CompositionMode>( "QPainter::CompositionMode" );
    qRegisterMetaType<QgsDateTimeRange>( "QgsDateTimeRange" );
    qRegisterMetaType<QList<QgsMapLayer *>>( "QList<QgsMapLayer*>" );
    qRegisterMetaType<QMap<QNetworkRequest::Attribute, QVariant>>( "QMap<QNetworkRequest::Attribute,QVariant>" );
    qRegisterMetaType<QMap<QNetworkRequest::KnownHeaders, QVariant>>( "QMap<QNetworkRequest::KnownHeaders,QVariant>" );
    qRegisterMetaType<QList<QNetworkReply::RawHeaderPair>>( "QList<QNetworkReply::RawHeaderPair>" );
  } );

  ( void ) resolvePkgPath();

  if ( ABISYM( mRunningFromBuildDir ) )
  {
    // we run from source directory - not installed to destination (specified prefix)
    *sPrefixPath() = QString(); // set invalid path
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    setPluginPath( *sBuildOutputPath() + '/' + QString( QGIS_PLUGIN_SUBDIR ) + '/' + *sCfgIntDir() );
#else
    setPluginPath( *sBuildOutputPath() + '/' + QStringLiteral( QGIS_PLUGIN_SUBDIR ) );
#endif
    setPkgDataPath( *sBuildOutputPath() + QStringLiteral( "/data" ) ); // in buildDir/data - used for: doc, resources, svg
    *sLibraryPath() = *sBuildOutputPath() + '/' + QGIS_LIB_SUBDIR + '/';
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    *sLibexecPath() = *sBuildOutputPath() + '/' + QGIS_LIBEXEC_SUBDIR + '/' + *sCfgIntDir() + '/';
#else
    *sLibexecPath() = *sBuildOutputPath() + '/' + QGIS_LIBEXEC_SUBDIR + '/';
#endif
#if defined( HAVE_QUICK )
    *sQmlImportPath() = *sBuildOutputPath() + '/' + QGIS_QML_SUBDIR + '/';
#endif
  }
  else
  {
    char *prefixPath = getenv( "QGIS_PREFIX_PATH" );
    if ( !prefixPath )
    {
      if ( sPrefixPath()->isNull() )
      {
#if defined(Q_OS_MACX) || defined(Q_OS_WIN)
        setPrefixPath( applicationDirPath(), true );
#elif defined(ANDROID)
        // this is "/data/data/org.qgis.qgis" in android
        QDir myDir( QDir::homePath() );
        myDir.cdUp();
        QString myPrefix = myDir.absolutePath();
        setPrefixPath( myPrefix, true );
#else
        QDir myDir( applicationDirPath() );
        // Fix for server which is one level deeper in /usr/lib/cgi-bin
        if ( applicationDirPath().contains( QStringLiteral( "cgi-bin" ) ) )
        {
          myDir.cdUp();
        }
        myDir.cdUp(); // Go from /usr/bin or /usr/lib (for server) to /usr
        QString myPrefix = myDir.absolutePath();
        setPrefixPath( myPrefix, true );
#endif
      }
    }
    else
    {
      setPrefixPath( prefixPath, true );
    }
  }

  *sConfigPath() = profileFolder + '/'; // make sure trailing slash is included
  *sDefaultSvgPaths() << qgisSettingsDirPath() + QStringLiteral( "svg/" );

  *sAuthDbDirPath() = qgisSettingsDirPath();
  if ( getenv( "QGIS_AUTH_DB_DIR_PATH" ) )
  {
    setAuthDatabaseDirPath( getenv( "QGIS_AUTH_DB_DIR_PATH" ) );
  }

  // store system environment variables passed to application, before they are adjusted
  QMap<QString, QString> systemEnvVarMap;
  QString passfile( QStringLiteral( "QGIS_AUTH_PASSWORD_FILE" ) ); // QString, for comparison

  const auto systemEnvironment = QProcessEnvironment::systemEnvironment().toStringList();
  for ( const QString &varStr : systemEnvironment )
  {
    int pos = varStr.indexOf( QLatin1Char( '=' ) );
    if ( pos == -1 )
      continue;
    QString varStrName = varStr.left( pos );
    QString varStrValue = varStr.mid( pos + 1 );
    if ( varStrName != passfile )
    {
      systemEnvVarMap.insert( varStrName, varStrValue );
    }
  }
  *sSystemEnvVars() = systemEnvVarMap;

  // append local user-writable folder as a proj search path
  QStringList currentProjSearchPaths = QgsProjUtils::searchPaths();
  currentProjSearchPaths.append( qgisSettingsDirPath() + QStringLiteral( "proj" ) );
#ifdef Q_OS_MACX
  // append bundled proj lib for MacOS
  QString projLib( QDir::cleanPath( pkgDataPath().append( "/proj" ) ) );
  if ( QFile::exists( projLib ) )
  {
    currentProjSearchPaths.append( projLib );
  }
#endif // Q_OS_MACX

  char **newPaths = new char *[currentProjSearchPaths.length()];
  for ( int i = 0; i < currentProjSearchPaths.count(); ++i )
  {
    newPaths[i] = CPLStrdup( currentProjSearchPaths.at( i ).toUtf8().constData() );
  }
  proj_context_set_search_paths( nullptr, currentProjSearchPaths.count(), newPaths );
  for ( int i = 0; i < currentProjSearchPaths.count(); ++i )
  {
    CPLFree( newPaths[i] );
  }
  delete [] newPaths;

  // allow Qt to search for Qt plugins (e.g. sqldrivers) in our plugin directory
  QCoreApplication::addLibraryPath( pluginPath() );

  // set max. thread count to -1
  // this should be read from QgsSettings but we don't know where they are at this point
  // so we read actual value in main.cpp
  ABISYM( sMaxThreads ) = -1;

  {
    QgsScopedRuntimeProfile profile( tr( "Load color schemes" ) );
    colorSchemeRegistry()->addDefaultSchemes();
    colorSchemeRegistry()->initStyleScheme();
  }

  {
    QgsScopedRuntimeProfile profile( tr( "Load bookmarks" ) );
    bookmarkManager()->initialize( QgsApplication::qgisSettingsDirPath() + "/bookmarks.xml" );
  }

  if ( !members()->mStyleModel )
    members()->mStyleModel = new QgsStyleModel( QgsStyle::defaultStyle() );

  ABISYM( mInitialized ) = true;
}

QgsApplication::~QgsApplication()
{
  delete mDataItemProviderRegistry;
  delete mApplicationMembers;
  delete mQgisTranslator;
  delete mQtTranslator;
  delete mQtBaseTranslator;

  // we do this here as well as in exitQgis() -- it's safe to call as often as we want,
  // and there's just a *chance* that someone hasn't properly called exitQgis prior to
  // this destructor...
  invalidateCaches();
}

void QgsApplication::invalidateCaches()
{
  // invalidate coordinate cache while the PROJ context held by the thread-locale
  // QgsProjContextStore object is still alive. Otherwise if this later object
  // is destroyed before the static variables of the cache, we might use freed memory.
  QgsCoordinateTransform::invalidateCache( true );
  QgsCoordinateReferenceSystem::invalidateCache( true );
  QgsEllipsoidUtils::invalidateCache( true );
}

QgsApplication *QgsApplication::instance()
{
  return qobject_cast<QgsApplication *>( QCoreApplication::instance() );
}

bool QgsApplication::event( QEvent *event )
{
  bool done = false;
  if ( event->type() == QEvent::FileOpen )
  {
    // handle FileOpen event (double clicking a file icon in Mac OS X Finder)
    if ( ABISYM( mFileOpenEventReceiver ) )
    {
      // Forward event to main window.
      done = notify( ABISYM( mFileOpenEventReceiver ), event );
    }
    else
    {
      // Store filename because receiver has not registered yet.
      // If QGIS has been launched by double clicking a file icon, FileOpen will be
      // the first event; the main window is not yet ready to handle the event.
      sFileOpenEventList()->append( static_cast<QFileOpenEvent *>( event )->file() );
      done = true;
    }
  }
  else
  {
    // pass other events to base class
    done = QApplication::event( event );
  }
  return done;
}

bool QgsApplication::notify( QObject *receiver, QEvent *event )
{
  bool done = false;
  // Crashes  in customization (especially on Mac), if we're not in the main/UI thread, see #5597
  if ( thread() == receiver->thread() )
    emit preNotify( receiver, event, &done );

  if ( done )
    return true;

  // Send event to receiver and catch unhandled exceptions
  done = true;
  try
  {
    done = QApplication::notify( receiver, event );
  }
  catch ( QgsException &e )
  {
    qCritical() << "Caught unhandled QgsException: " << e.what();
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( std::exception &e )
  {
    qCritical() << "Caught unhandled std::exception: " << e.what();
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( ... )
  {
    qCritical() << "Caught unhandled unknown exception";
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), tr( "unknown exception" ) );
  }

  return done;
}

QgsRuntimeProfiler *QgsApplication::profiler()
{
  return QgsRuntimeProfiler::threadLocalInstance();
}

void QgsApplication::setFileOpenEventReceiver( QObject *receiver )
{
  // Set receiver for FileOpen events
  ABISYM( mFileOpenEventReceiver ) = receiver;
  // Propagate any events collected before the receiver has registered.
  if ( sFileOpenEventList()->count() > 0 )
  {
    const QStringList fileOpenEventList = *sFileOpenEventList();
    for ( const QString &file : fileOpenEventList )
    {
      QFileOpenEvent foe( file );
      QgsApplication::sendEvent( ABISYM( mFileOpenEventReceiver ), &foe );
    }
    sFileOpenEventList()->clear();
  }
}

void QgsApplication::setPrefixPath( const QString &prefixPath, bool useDefaultPaths )
{
  *sPrefixPath() = prefixPath;
#if defined(Q_OS_WIN)
  if ( sPrefixPath()->endsWith( "/bin" ) )
  {
    sPrefixPath()->chop( 4 );
  }
#endif
  if ( useDefaultPaths && !ABISYM( mRunningFromBuildDir ) )
  {
    setPluginPath( *sPrefixPath() + '/' + QStringLiteral( QGIS_PLUGIN_SUBDIR ) );
    setPkgDataPath( *sPrefixPath() + '/' + QStringLiteral( QGIS_DATA_SUBDIR ) );
  }
  *sLibraryPath() = *sPrefixPath() + '/' + QGIS_LIB_SUBDIR + '/';
  *sLibexecPath() = *sPrefixPath() + '/' + QGIS_LIBEXEC_SUBDIR + '/';
#if defined( HAVE_QUICK )
  *sQmlImportPath() = *sPrefixPath() + '/' + QGIS_QML_SUBDIR + '/';
#endif
}

void QgsApplication::setPluginPath( const QString &pluginPath )
{
  *sPluginPath() = pluginPath;
}

void QgsApplication::setPkgDataPath( const QString &pkgDataPath )
{
  *sPkgDataPath() = pkgDataPath;

  QString mySvgPath = pkgDataPath + QStringLiteral( "/svg/" );

  // avoid duplicate entries
  if ( !sDefaultSvgPaths()->contains( mySvgPath ) )
    *sDefaultSvgPaths() << mySvgPath;
}

void QgsApplication::setDefaultSvgPaths( const QStringList &pathList )
{
  *sDefaultSvgPaths() = pathList;
}

void QgsApplication::setAuthDatabaseDirPath( const QString &authDbDirPath )
{
  QFileInfo fi( authDbDirPath );
  if ( fi.exists() && fi.isDir() && fi.isWritable() )
  {
    *sAuthDbDirPath() = fi.canonicalFilePath() + QDir::separator();
  }
}

QString QgsApplication::prefixPath()
{
#if 0
  if ( ABISYM( mRunningFromBuildDir ) )
  {
    static bool sOnce = true;
    if ( sOnce )
    {
      QgsMessageLogNotifyBlocker blockNotifications;
      ( void ) blockNotifications;
      qWarning( "!!! prefix path was requested, but it is not valid - we do not run from installed path !!!" );
    }
    sOnce = false;
  }
#endif

  return *sPrefixPath();
}
QString QgsApplication::pluginPath()
{
  return *sPluginPath();
}

QString QgsApplication::pkgDataPath()
{
  if ( sPkgDataPath()->isNull() )
    return resolvePkgPath();
  else
    return *sPkgDataPath();
}

QString QgsApplication::defaultThemePath()
{
  return QStringLiteral( ":/images/themes/default/" );
}
QString QgsApplication::activeThemePath()
{
  QString usersThemes = userThemesFolder() + QDir::separator() + themeName() + QDir::separator() + "icons/";
  QDir dir( usersThemes );
  if ( dir.exists() )
  {
    return usersThemes;
  }
  else
  {
    QString defaultThemes = defaultThemesFolder() + QDir::separator() + themeName() + QDir::separator() + "icons/";
    return defaultThemes;
  }
}

QString QgsApplication::appIconPath()
{
  return iconsPath() + QStringLiteral( "qgis-icon-60x60.png" );
}

int QgsApplication::maxThreads()
{
  return ABISYM( sMaxThreads );
}

QString QgsApplication::iconPath( const QString &iconFile )
{
  // try active theme
  QString path = activeThemePath();
  if ( QFile::exists( path + iconFile ) )
    return path + iconFile;

  // use default theme
  return defaultThemePath() + iconFile;
}

QIcon QgsApplication::getThemeIcon( const QString &name, const QColor &fillColor, const QColor &strokeColor )
{
  const QString cacheKey = ( name.startsWith( '/' ) ? name.mid( 1 ) : name )
                           + ( fillColor.isValid() ? QStringLiteral( "_%1" ).arg( fillColor.name( QColor::HexArgb ).mid( 1 ) ) :  QString() )
                           + ( strokeColor.isValid() ? QStringLiteral( "_%1" ).arg( strokeColor.name( QColor::HexArgb ).mid( 1 ) ) : QString() );
  QgsApplication *app = instance();
  if ( app && app->mIconCache.contains( cacheKey ) )
    return app->mIconCache.value( cacheKey );

  QIcon icon;
  const bool colorBased = fillColor.isValid() || strokeColor.isValid();

  auto iconFromColoredSvg = [ = ]( const QString & path ) -> QIcon
  {
    // sizes are unused here!
    const QByteArray svgContent = QgsApplication::svgCache()->svgContent( path, 16, fillColor, strokeColor, 1, 1 );

    const QString iconPath = sIconCacheDir()->filePath( cacheKey + QStringLiteral( ".svg" ) );
    QFile f( iconPath );
    if ( f.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      f.write( svgContent );
      f.close();
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Could not create colorized icon svg at %1" ).arg( iconPath ) );
      return QIcon();
    }

    return QIcon( f.fileName() );
  };

  QString preferredPath = activeThemePath() + QDir::separator() + name;
  QString defaultPath = defaultThemePath() + QDir::separator() + name;
  if ( QFile::exists( preferredPath ) )
  {
    if ( colorBased )
    {
      icon = iconFromColoredSvg( preferredPath );
    }
    else
    {
      icon = QIcon( preferredPath );
    }
  }
  else if ( QFile::exists( defaultPath ) )
  {
    //could still return an empty icon if it
    //doesn't exist in the default theme either!
    if ( colorBased )
    {
      icon = iconFromColoredSvg( defaultPath );
    }
    else
    {
      icon = QIcon( defaultPath );
    }
  }
  else
  {
    icon = QIcon();
  }

  if ( app )
    app->mIconCache.insert( cacheKey, icon );
  return icon;
}

QCursor QgsApplication::getThemeCursor( Cursor cursor )
{
  QgsApplication *app = instance();
  if ( app && app->mCursorCache.contains( cursor ) )
    return app->mCursorCache.value( cursor );

  // All calculations are done on 32x32 icons
  // Defaults to center, individual cursors may override
  int activeX = 16;
  int activeY = 16;

  QString name;
  switch ( cursor )
  {
    case ZoomIn:
      name = QStringLiteral( "mZoomIn.svg" );
      activeX = 13;
      activeY = 13;
      break;
    case ZoomOut:
      name = QStringLiteral( "mZoomOut.svg" );
      activeX = 13;
      activeY = 13;
      break;
    case Identify:
      activeX = 3;
      activeY = 6;
      name = QStringLiteral( "mIdentify.svg" );
      break;
    case CrossHair:
      name = QStringLiteral( "mCrossHair.svg" );
      break;
    case CapturePoint:
      name = QStringLiteral( "mCapturePoint.svg" );
      break;
    case Select:
      name = QStringLiteral( "mSelect.svg" );
      activeX = 6;
      activeY = 6;
      break;
    case Sampler:
      activeX = 5;
      activeY = 5;
      name = QStringLiteral( "mSampler.svg" );
      break;
      // No default
  }
  // It should never get here!
  Q_ASSERT( ! name.isEmpty( ) );

  QIcon icon = getThemeIcon( QStringLiteral( "cursors" ) + QDir::separator() + name );
  QCursor cursorIcon;
  // Check if an icon exists for this cursor (the O.S. default cursor will be used if it does not)
  if ( ! icon.isNull( ) )
  {
    // Apply scaling
    float scale = Qgis::UI_SCALE_FACTOR * QgsApplication::fontMetrics().height() / 32.0;
    cursorIcon = QCursor( icon.pixmap( std::ceil( scale * 32 ), std::ceil( scale * 32 ) ), std::ceil( scale * activeX ), std::ceil( scale * activeY ) );
  }
  if ( app )
    app->mCursorCache.insert( cursor, cursorIcon );
  return cursorIcon;
}

// TODO: add some caching mechanism ?
QPixmap QgsApplication::getThemePixmap( const QString &name, const QColor &foreColor, const QColor &backColor, const int size )
{
  const QString preferredPath = activeThemePath() + QDir::separator() + name;
  const QString defaultPath = defaultThemePath() + QDir::separator() + name;
  const QString path = QFile::exists( preferredPath ) ? preferredPath : defaultPath;
  if ( foreColor.isValid() || backColor.isValid() )
  {
    bool fitsInCache = false;
    const QImage image = svgCache()->svgAsImage( path, size, backColor, foreColor, 1, 1, fitsInCache );
    return QPixmap::fromImage( image );
  }

  return QPixmap( path );
}

void QgsApplication::setThemeName( const QString &themeName )
{
  *sThemeName() = themeName;
}

QString QgsApplication::resolvePkgPath()
{
  static QString appPath;
  if ( appPath.isNull() )
  {
    if ( QCoreApplication::instance() )
    {
      appPath = applicationDirPath();
    }
    else
    {
      qWarning( "Application path not initialized" );
    }
  }

  if ( !appPath.isNull() || getenv( "QGIS_PREFIX_PATH" ) )
  {
    QString prefix = getenv( "QGIS_PREFIX_PATH" ) ? getenv( "QGIS_PREFIX_PATH" ) : appPath;

    // check if QGIS is run from build directory (not the install directory)
    QFile f;
    // "/../../.." is for Mac bundled app in build directory
    static const QStringList paths { QStringList() << QString() << QStringLiteral( "/.." ) << QStringLiteral( "/bin" ) << QStringLiteral( "/../../.." ) };
    for ( const QString &path : paths )
    {
      f.setFileName( prefix + path + "/qgisbuildpath.txt" );
      if ( f.exists() )
        break;
    }
    if ( f.exists() && f.open( QIODevice::ReadOnly ) )
    {
      ABISYM( mRunningFromBuildDir ) = true;
      *sBuildSourcePath() = f.readLine().trimmed();
      *sBuildOutputPath() = f.readLine().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "Running from build directory!" ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "- source directory: %1" ).arg( sBuildSourcePath()->toUtf8().constData() ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "- output directory of the build: %1" ).arg( sBuildOutputPath()->toUtf8().constData() ), 4 );
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
      *sCfgIntDir() = prefix.split( '/', QString::SkipEmptyParts ).last();
      qDebug( "- cfg: %s", sCfgIntDir()->toUtf8().constData() );
#endif
    }
  }

  QString prefixPath;
  if ( getenv( "QGIS_PREFIX_PATH" ) )
    prefixPath = getenv( "QGIS_PREFIX_PATH" );
  else
  {
#if defined(ANDROID)
    // this is "/data/data/org.qgis.qgis" in android
    QDir dir( QDir::homePath() );
    dir.cdUp();
    prefixPath = dir.absolutePath();
#else

#if defined(Q_OS_MACX)
    prefixPath = appPath;
#elif defined(Q_OS_WIN)
    prefixPath = appPath;
    if ( prefixPath.endsWith( "/bin" ) )
      prefixPath.chop( 4 );
#else
    QDir dir( appPath );
    // Fix for server which is one level deeper in /usr/lib/cgi-bin
    if ( appPath.contains( QStringLiteral( "cgi-bin" ) ) )
    {
      dir.cdUp();
    }
    dir.cdUp(); // Go from /usr/bin or /usr/lib (for server) to /usr
    prefixPath = dir.absolutePath();
#endif
#endif
  }

  if ( ABISYM( mRunningFromBuildDir ) )
    return *sBuildOutputPath() + QStringLiteral( "/data" );
  else
    return prefixPath + '/' + QStringLiteral( QGIS_DATA_SUBDIR );
}

QString QgsApplication::themeName()
{
  return *sThemeName();
}

void QgsApplication::setUITheme( const QString &themeName )
{
  // Loop all style sheets, find matching name, load it.
  QHash<QString, QString> themes = QgsApplication::uiThemes();
  if ( themeName == QLatin1String( "default" ) || !themes.contains( themeName ) )
  {
    setThemeName( QStringLiteral( "default" ) );
    qApp->setStyleSheet( QString() );
    return;
  }

  QString path = themes.value( themeName );
  QString stylesheetname = path + "/style.qss";

  QFile file( stylesheetname );
  QFile variablesfile( path + "/variables.qss" );

  QFileInfo variableInfo( variablesfile );

  if ( !file.open( QIODevice::ReadOnly ) || ( variableInfo.exists() && !variablesfile.open( QIODevice::ReadOnly ) ) )
  {
    return;
  }

  QString styledata = file.readAll();
  styledata.replace( QLatin1String( "@theme_path" ), path );

  if ( variableInfo.exists() )
  {
    QTextStream in( &variablesfile );
    while ( !in.atEnd() )
    {
      QString line = in.readLine();
      // This is a variable
      if ( line.startsWith( '@' ) )
      {
        int index = line.indexOf( ':' );
        QString name = line.mid( 0, index );
        QString value = line.mid( index + 1, line.length() );
        styledata.replace( name, value );
      }
    }
    variablesfile.close();
  }
  file.close();

  if ( Qgis::UI_SCALE_FACTOR != 1.0 )
  {
    // apply OS-specific UI scale factor to stylesheet's em values
    int index = 0;
    const static QRegularExpression regex( QStringLiteral( "(?<=[\\s:])([0-9\\.]+)(?=em)" ) );
    QRegularExpressionMatch match = regex.match( styledata, index );
    while ( match.hasMatch() )
    {
      index = match.capturedStart();
      styledata.remove( index, match.captured( 0 ).length() );
      QString number = QString::number( match.captured( 0 ).toDouble() * Qgis::UI_SCALE_FACTOR );
      styledata.insert( index, number );
      index += number.length();
      match = regex.match( styledata, index );
    }
  }

  qApp->setStyleSheet( styledata );

  QFile palettefile( path + "/palette.txt" );
  QFileInfo paletteInfo( palettefile );
  if ( paletteInfo.exists() && palettefile.open( QIODevice::ReadOnly ) )
  {
    QPalette pal = qApp->palette();
    QTextStream in( &palettefile );
    while ( !in.atEnd() )
    {
      QString line = in.readLine();
      QStringList parts = line.split( ':' );
      if ( parts.count() == 2 )
      {
        int role = parts.at( 0 ).trimmed().toInt();
        QColor color = QgsSymbolLayerUtils::decodeColor( parts.at( 1 ).trimmed() );
        pal.setColor( static_cast< QPalette::ColorRole >( role ), color );
      }
    }
    palettefile.close();
    qApp->setPalette( pal );
  }

  setThemeName( themeName );
}

QHash<QString, QString> QgsApplication::uiThemes()
{
  QStringList paths = QStringList() << userThemesFolder() << defaultThemesFolder();
  QHash<QString, QString> mapping;
  mapping.insert( QStringLiteral( "default" ), QString() );
  const auto constPaths = paths;
  for ( const QString &path : constPaths )
  {
    QDir folder( path );
    QFileInfoList styleFiles = folder.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    const auto constStyleFiles = styleFiles;
    for ( const QFileInfo &info : constStyleFiles )
    {
      QFileInfo styleFile( info.absoluteFilePath() + "/style.qss" );
      if ( !styleFile.exists() )
        continue;

      QString name = info.baseName();
      QString path = info.absoluteFilePath();
      mapping.insert( name, path );
    }
  }
  return mapping;
}

QString QgsApplication::authorsFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/AUTHORS" );
}

QString QgsApplication::contributorsFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/CONTRIBUTORS" );
}
QString QgsApplication::developersMapFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/developersmap.html" );
}

QString QgsApplication::sponsorsFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/SPONSORS" );
}

QString QgsApplication::donorsFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/DONORS" );
}

QString QgsApplication::translatorsFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/TRANSLATORS" );
}

QString QgsApplication::licenceFilePath()
{
  return pkgDataPath() + QStringLiteral( "/doc/LICENSE" );
}

QString QgsApplication::i18nPath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
    return *sBuildOutputPath() + QStringLiteral( "/i18n/" );
  else
    return pkgDataPath() + QStringLiteral( "/i18n/" );
}

QString QgsApplication::metadataPath()
{
  return pkgDataPath() + QStringLiteral( "/resources/metadata-ISO/" );
}

QString QgsApplication::qgisMasterDatabaseFilePath()
{
  return pkgDataPath() + QStringLiteral( "/resources/qgis.db" );
}

QString QgsApplication::qgisSettingsDirPath()
{
  return *sConfigPath();
}

QString QgsApplication::qgisUserDatabaseFilePath()
{
  return qgisSettingsDirPath() + QStringLiteral( "qgis.db" );
}

QString QgsApplication::qgisAuthDatabaseFilePath()
{
  return *sAuthDbDirPath() + QStringLiteral( "qgis-auth.db" );
}

QString QgsApplication::splashPath()
{
  return QStringLiteral( ":/images/splash/" );
}

QString QgsApplication::iconsPath()
{
  return pkgDataPath() + QStringLiteral( "/images/icons/" );
}

QString QgsApplication::srsDatabaseFilePath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
  {
    QString tempCopy = QDir::tempPath() + "/srs6.db";

    if ( !QFile( tempCopy ).exists() )
    {
      QFile f( buildSourcePath() + "/resources/srs6.db" );
      if ( !f.copy( tempCopy ) )
      {
        qFatal( "Could not create temporary copy" );
      }
    }

    return tempCopy;
  }
  else
  {
    return pkgDataPath() + QStringLiteral( "/resources/srs.db" );
  }
}

void QgsApplication::setSvgPaths( const QStringList &svgPaths )
{
  settingsSearchPathsForSVG.setValue( svgPaths );
  members()->mSvgPathCacheValid = false;
}

QStringList QgsApplication::svgPaths()
{
  static QReadWriteLock lock;

  QgsReadWriteLocker locker( lock, QgsReadWriteLocker::Read );

  if ( members()->mSvgPathCacheValid )
  {
    return members()->mSvgPathCache;
  }
  else
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    //local directories to search when looking for an SVG with a given basename
    //defined by user in options dialog
    const QStringList pathList = settingsSearchPathsForSVG.value();

    // maintain user set order while stripping duplicates
    QStringList paths;
    for ( const QString &path : pathList )
    {
      if ( !paths.contains( path ) )
        paths.append( path );
    }
    for ( const QString &path : std::as_const( *sDefaultSvgPaths() ) )
    {
      if ( !paths.contains( path ) )
        paths.append( path );
    }
    members()->mSvgPathCache = paths;

    return paths;
  }
}

QStringList QgsApplication::layoutTemplatePaths()
{
  //local directories to search when looking for an template with a given basename
  //defined by user in options dialog
  return QgsLayout::settingsSearchPathForTemplates.value();
}

QMap<QString, QString> QgsApplication::systemEnvVars()
{
  return *sSystemEnvVars();
}

QString QgsApplication::userStylePath()
{
  return qgisSettingsDirPath() + QStringLiteral( "symbology-style.db" );
}

QRegularExpression QgsApplication::shortNameRegularExpression()
{
  const thread_local QRegularExpression regexp( QRegularExpression::anchoredPattern( QStringLiteral( "^[A-Za-z][A-Za-z0-9\\._-]*" ) ) );
  return regexp;
}

QString QgsApplication::userLoginName()
{
  if ( !sUserName()->isEmpty() )
    return *sUserName();

#ifdef _MSC_VER
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  if ( GetUserName( ( TCHAR * )name, &size ) )
  {
    *sUserName() = QString::fromLocal8Bit( name );
  }

#elif QT_CONFIG(process)
  QProcess process;

  process.start( QStringLiteral( "whoami" ), QStringList() );
  process.waitForFinished();
  *sUserName() = process.readAllStandardOutput().trimmed();
#endif

  if ( !sUserName()->isEmpty() )
    return *sUserName();

  //backup plan - use environment variables
  *sUserName() = qgetenv( "USER" );
  if ( !sUserName()->isEmpty() )
    return *sUserName();

  //last resort
  *sUserName() = qgetenv( "USERNAME" );
  return *sUserName();
}

QString QgsApplication::userFullName()
{
  if ( !sUserFullName()->isEmpty() )
    return *sUserFullName();

#ifdef _MSC_VER
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  //note - this only works for accounts connected to domain
  if ( GetUserNameEx( NameDisplay, ( TCHAR * )name, &size ) )
  {
    *sUserFullName() = QString::fromLocal8Bit( name );
  }

  //fall back to login name
  if ( sUserFullName()->isEmpty() )
    *sUserFullName() = userLoginName();
#elif defined(Q_OS_ANDROID) || defined(__MINGW32__)
  *sUserFullName() = QStringLiteral( "Not available" );
#else
  struct passwd *p = getpwuid( getuid() );

  if ( p )
  {
    QString gecosName = QString( p->pw_gecos );
    *sUserFullName() = gecosName.left( gecosName.indexOf( ',', 0 ) );
  }

#endif

  return *sUserFullName();
}

QString QgsApplication::osName()
{
#if defined(Q_OS_ANDROID)
  return QLatin1String( "android" );
#elif defined(Q_OS_MAC)
  return QLatin1String( "osx" );
#elif defined(Q_OS_WIN)
  return QLatin1String( "windows" );
#elif defined(Q_OS_LINUX)
  return QStringLiteral( "linux" );
#elif defined(Q_OS_FREEBSD)
  return QStringLiteral( "freebsd" );
#elif defined(Q_OS_OPENBSD)
  return QStringLiteral( "openbsd" );
#elif defined(Q_OS_NETBSD)
  return QStringLiteral( "netbsd" );
#elif defined(Q_OS_UNIX)
  return QLatin1String( "unix" );
#else
  return QLatin1String( "unknown" );
#endif
}

int QgsApplication::systemMemorySizeMb()
{
#if defined(Q_OS_ANDROID)
  return -1;
#elif defined(Q_OS_MAC)
  return -1;
#elif defined(Q_OS_WIN)
  MEMORYSTATUSEX memoryStatus;
  ZeroMemory( &memoryStatus, sizeof( MEMORYSTATUSEX ) );
  memoryStatus.dwLength = sizeof( MEMORYSTATUSEX );
  if ( GlobalMemoryStatusEx( &memoryStatus ) )
  {
    return memoryStatus.ullTotalPhys / ( 1024 * 1024 );
  }
  else
  {
    return -1;
  }
#elif defined(Q_OS_LINUX)
  constexpr int megabyte = 1024 * 1024;
  struct sysinfo si;
  sysinfo( &si );
  return si.totalram / megabyte;
#elif defined(Q_OS_FREEBSD)
  return -1;
#elif defined(Q_OS_OPENBSD)
  return -1;
#elif defined(Q_OS_NETBSD)
  return -1;
#elif defined(Q_OS_UNIX)
  return -1;
#else
  return -1;
#endif
}

QString QgsApplication::platform()
{
  return *sPlatformName();
}

QString QgsApplication::locale()
{
  if ( settingsLocaleOverrideFlag.value() )
  {
    QString locale = settingsLocaleUserLocale.value();
    // don't differentiate en_US and en_GB
    if ( locale.startsWith( QLatin1String( "en" ), Qt::CaseInsensitive ) )
    {
      return locale.left( 2 );
    }

    return locale;
  }
  else
  {
    return QLocale().name().left( 2 );
  }
}

void QgsApplication::setLocale( const QLocale &locale )
{
  QLocale::setDefault( locale );
  emit instance()->localeChanged();
}

QString QgsApplication::userThemesFolder()
{
  return qgisSettingsDirPath() + QStringLiteral( "/themes" );
}

QString QgsApplication::defaultStylePath()
{
  return pkgDataPath() + QStringLiteral( "/resources/symbology-style.xml" );
}

QString QgsApplication::defaultThemesFolder()
{
  return pkgDataPath() + QStringLiteral( "/resources/themes" );
}

QString QgsApplication::serverResourcesPath()
{
  return pkgDataPath() + QStringLiteral( "/resources/server/" );
}

QString QgsApplication::libraryPath()
{
  return *sLibraryPath();
}

QString QgsApplication::libexecPath()
{
  return *sLibexecPath();
}

QString QgsApplication::qmlImportPath()
{
  return *sQmlImportPath();
}

QgsApplication::endian_t QgsApplication::endian()
{
  return ( htonl( 1 ) == 1 ) ? XDR : NDR;
}

void QgsApplication::initQgis()
{
  if ( !ABISYM( mInitialized ) && QgsApplication::instance() )
  {
    init( *sProfilePath() );
  }

  // set the provider plugin path (this creates provider registry)
  QgsProviderRegistry::instance( pluginPath() );

  // create data item provider registry
  ( void )QgsApplication::dataItemProviderRegistry();

  // create project instance if doesn't exist
  QgsProject::instance();

  // Initialize authentication manager and connect to database
  authManager()->init( pluginPath(), qgisAuthDatabaseFilePath() );

  // Make sure we have a NAM created on the main thread.
  // Note that this might call QgsApplication::authManager to
  // setup the proxy configuration that's why it needs to be
  // called after the QgsAuthManager instance has been created
  QgsNetworkAccessManager::instance();

}

QgsAuthManager *QgsApplication::authManager()
{
  if ( auto *lInstance = instance() )
  {
    if ( !lInstance->mAuthManager )
    {
      lInstance->mAuthManager = QgsAuthManager::instance();
    }
    return lInstance->mAuthManager;
  }
  else
  {
    // no QgsApplication instance
    if ( !sAuthManager )
      sAuthManager = QgsAuthManager::instance();
    return sAuthManager;
  }
}


void QgsApplication::exitQgis()
{
  // make sure all threads are done before exiting
  QThreadPool::globalInstance()->waitForDone();

  // don't create to delete
  if ( auto *lInstance = instance() )
    delete lInstance->mAuthManager;
  else
    delete sAuthManager;

  //Ensure that all remaining deleteLater QObjects are actually deleted before we exit.
  //This isn't strictly necessary (since we're exiting anyway) but doing so prevents a lot of
  //LeakSanitiser noise which hides real issues
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

  //delete all registered functions from expression engine (see above comment)
  QgsExpression::cleanRegisteredFunctions();

  delete QgsProject::instance();

  // avoid creating instance just to delete it!
  if ( QgsProviderRegistry::exists() )
    delete QgsProviderRegistry::instance();

  invalidateCaches();

  QgsStyle::cleanDefaultStyle();

  // tear-down GDAL/OGR
  OGRCleanupAll();
  GDALDestroyDriverManager();
}

QString QgsApplication::showSettings()
{
  QString myEnvironmentVar( getenv( "QGIS_PREFIX_PATH" ) );
  QString myState = tr( "Application state:\n"
                        "QGIS_PREFIX_PATH env var:\t\t%1\n"
                        "Prefix:\t\t%2\n"
                        "Plugin Path:\t\t%3\n"
                        "Package Data Path:\t%4\n"
                        "Active Theme Name:\t%5\n"
                        "Active Theme Path:\t%6\n"
                        "Default Theme Path:\t%7\n"
                        "SVG Search Paths:\t%8\n"
                        "User DB Path:\t%9\n"
                        "Auth DB Path:\t%10\n" )
                    .arg( myEnvironmentVar,
                          prefixPath(),
                          pluginPath(),
                          pkgDataPath(),
                          themeName(),
                          activeThemePath(),
                          defaultThemePath(),
                          svgPaths().join( tr( "\n\t\t", "match indentation of application state" ) ),
                          qgisMasterDatabaseFilePath() )
                    .arg( qgisAuthDatabaseFilePath() );
  return myState;
}

QString QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType styleSheetType )
{
  //
  // Make the style sheet desktop preferences aware by using qapplication
  // palette as a basis for colors where appropriate
  //
  //  QColor myColor1 = palette().highlight().color();
  QColor myColor1( Qt::lightGray );
  QColor myColor2 = myColor1;
  myColor2 = myColor2.lighter( 110 ); //10% lighter
  QString myStyle;
  myStyle = QStringLiteral( ".overview{"
                            "  font: 1.82em;"
                            "  font-weight: bold;"
                            "}"
                            "body{"
                            "  background: white;"
                            "  color: black;"
                            "  font-family: 'Lato', 'Open Sans', 'Lucida Grande', 'Segoe UI', 'Arial', sans-serif;"
                            "  width: 100%;"
                            "}"
                            "h1{  background-color: #F6F6F6;"
                            "  color: #589632; " // from http://qgis.org/en/site/getinvolved/styleguide.html
                            "  font-size: x-large;  "
                            "  font-weight: normal;"
                            "  background: none;"
                            "  padding: 0.75em 0 0;"
                            "  margin: 0;"
                            "  line-height: 3em;"
                            "}"
                            "h2{  background-color: #F6F6F6;"
                            "  color: #589632; "  // from http://qgis.org/en/site/getinvolved/styleguide.html
                            "  font-size: medium;  "
                            "  font-weight: normal;"
                            "  background: none;"
                            "  padding: 0.75em 0 0;"
                            "  margin: 0;"
                            "  line-height: 1.1em;"
                            "}"
                            "h3{  background-color: #F6F6F6;"
                            "  color: #93b023;"  // from http://qgis.org/en/site/getinvolved/styleguide.html
                            "  font-weight: bold;"
                            "  font-size: large;"
                            "  text-align: left;"
                            "  border-bottom: 5px solid #DCEB5C;"
                            "}"
                            "h4{  background-color: #F6F6F6;"
                            "  color: #93b023;"  // from http://qgis.org/en/site/getinvolved/styleguide.html
                            "  font-weight: bold;"
                            "  font-size: medium;"
                            "  text-align: left;"
                            "}"
                            "h5{    background-color: #F6F6F6;"
                            "   color: #93b023;"  // from http://qgis.org/en/site/getinvolved/styleguide.html
                            "   font-weight: bold;"
                            "   font-size: small;"
                            "   text-align: left;"
                            "}"
                            "a{  color: #729FCF;"
                            "  font-family: arial,sans-serif;"
                            "}"
                            "label{  background-color: #FFFFCC;"
                            "  border: 1px solid black;"
                            "  margin: 1px;"
                            "  padding: 0px 3px; "
                            "  font-size: small;"
                            "}"
                            "th .strong {"
                            "  font-weight: bold;"
                            "}"
                            "hr {"
                            "  border: 0;"
                            "  height: 0;"
                            "  border-top: 1px solid black;"
                            "}"
                            ".list-view .highlight {"
                            "  text-align: left;"
                            "  border: 0px;"
                            "  width: 20%;"
                            "  padding-right: 15px;"
                            "  padding-left: 20px;"
                            "  font-weight: bold;"
                            "}"
                            ".tabular-view .odd-row {"
                            "  background-color: #f9f9f9;"
                            "}"
                            ".section {"
                            "  font-weight: bold;"
                            "  padding-top:25px;"
                            "}" );

  // We have some subtle differences between Qt based style and QWebKit style
  switch ( styleSheetType )
  {
    case StyleSheetType::Qt:
      myStyle += QStringLiteral(
                   ".tabular-view{ "
                   "  border-collapse: collapse;"
                   "  width: 95%;"
                   "}"
                   ".tabular-view th, .tabular-view td { "
                   "  border:1px solid black;"
                   "}" );
      break;

    case StyleSheetType::WebBrowser:
      myStyle += QStringLiteral(
                   "body { "
                   "   margin: auto;"
                   "   width: 97%;"
                   "}"
                   "table.tabular-view, table.list-view { "
                   "   border-collapse: collapse;"
                   "   table-layout:fixed;"
                   "   width: 100% !important;"
                   "   font-size: 90%;"
                   "}"
                   // Override
                   "h1 { "
                   "   line-height: inherit;"
                   "}"
                   "td, th {"
                   "   word-wrap: break-word; "
                   "   vertical-align: top;"
                   "}"
                   // Set first column width
                   ".list-view th:first-child, .list-view td:first-child {"
                   "   width: 20%;"
                   "}"
                   ".list-view.highlight { "
                   "   padding-left: inherit; "
                   "}"
                   // Set first column width for inner tables
                   ".tabular-view th:first-child, .tabular-view td:first-child { "
                   "   width: 20%; "
                   "}"
                   // Makes titles bg stand up
                   ".tabular-view th.strong { "
                   "   background-color: #eee; "
                   "}"
                   // Give some visual appearance to those ugly nested tables
                   ".tabular-view th, .tabular-view td { "
                   "   border: 1px solid #eee;"
                   "}"
                 );
      break;
  }

  return myStyle;
}

void QgsApplication::registerOgrDrivers()
{
  if ( 0 >= OGRGetDriverCount() )
  {
    OGRRegisterAll();
  }
}

QString QgsApplication::absolutePathToRelativePath( const QString &aPath, const QString &targetPath )
{
  QString aPathUrl = aPath;
  QString tPathUrl = targetPath;
#if defined( Q_OS_WIN )
  const Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  aPathUrl.replace( '\\', '/' );
  if ( aPathUrl.startsWith( "//" ) )
  {
    // keep UNC prefix
    aPathUrl = "\\\\" + aPathUrl.mid( 2 );
  }

  tPathUrl.replace( '\\', '/' );
  if ( tPathUrl.startsWith( "//" ) )
  {
    // keep UNC prefix
    tPathUrl = "\\\\" + tPathUrl.mid( 2 );
  }
#else
  const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList targetElems = tPathUrl.split( '/', QString::SkipEmptyParts );
  QStringList aPathElems = aPathUrl.split( '/', QString::SkipEmptyParts );
#else
  QStringList targetElems = tPathUrl.split( '/', Qt::SkipEmptyParts );
  QStringList aPathElems = aPathUrl.split( '/', Qt::SkipEmptyParts );
#endif

  targetElems.removeAll( QStringLiteral( "." ) );
  aPathElems.removeAll( QStringLiteral( "." ) );

  // remove common part
  int n = 0;
  while ( !aPathElems.isEmpty() &&
          !targetElems.isEmpty() &&
          aPathElems[0].compare( targetElems[0], cs ) == 0 )
  {
    aPathElems.removeFirst();
    targetElems.removeFirst();
    n++;
  }

  if ( n == 0 )
  {
    // no common parts; might not even be a file
    return aPathUrl;
  }

  if ( !targetElems.isEmpty() )
  {
    // go up to the common directory
    for ( int i = 0; i < targetElems.size(); i++ )
    {
      aPathElems.insert( 0, QStringLiteral( ".." ) );
    }
  }
  else
  {
    // let it start with . nevertheless,
    // so relative path always start with either ./ or ../
    aPathElems.insert( 0, QStringLiteral( "." ) );
  }

  return aPathElems.join( QLatin1Char( '/' ) );
}

QString QgsApplication::relativePathToAbsolutePath( const QString &rpath, const QString &targetPath )
{
  // relative path should always start with ./ or ../
  if ( !rpath.startsWith( QLatin1String( "./" ) ) && !rpath.startsWith( QLatin1String( "../" ) ) )
  {
    return rpath;
  }

  QString rPathUrl = rpath;
  QString targetPathUrl = targetPath;

#if defined(Q_OS_WIN)
  rPathUrl.replace( '\\', '/' );
  targetPathUrl.replace( '\\', '/' );

  bool uncPath = targetPathUrl.startsWith( "//" );
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList srcElems = rPathUrl.split( '/', QString::SkipEmptyParts );
  QStringList targetElems = targetPathUrl.split( '/', QString::SkipEmptyParts );
#else
  QStringList srcElems = rPathUrl.split( '/', Qt::SkipEmptyParts );
  QStringList targetElems = targetPathUrl.split( '/', Qt::SkipEmptyParts );
#endif

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    targetElems.insert( 0, "" );
    targetElems.insert( 0, "" );
  }
#endif

  // append source path elements
  targetElems << srcElems;
  targetElems.removeAll( QStringLiteral( "." ) );

  // resolve ..
  int pos;
  while ( ( pos = targetElems.indexOf( QLatin1String( ".." ) ) ) > 0 )
  {
    // remove preceding element and ..
    targetElems.removeAt( pos - 1 );
    targetElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  targetElems.prepend( QString() );
#endif

  return targetElems.join( QLatin1Char( '/' ) );
}

QString QgsApplication::buildSourcePath()
{
  return *sBuildSourcePath();
}

QString QgsApplication::buildOutputPath()
{
  return *sBuildOutputPath();
}

#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
QString QgsApplication::cfgIntDir()
{
  return *sCfgIntDir();
}
#endif

void QgsApplication::skipGdalDriver( const QString &driver )
{
  if ( sGdalSkipList()->contains( driver ) || driver.isEmpty() )
  {
    return;
  }
  *sGdalSkipList() << driver;
  applyGdalSkippedDrivers();
}

void QgsApplication::restoreGdalDriver( const QString &driver )
{
  if ( !sGdalSkipList()->contains( driver ) )
  {
    return;
  }
  int myPos = sGdalSkipList()->indexOf( driver );
  if ( myPos >= 0 )
  {
    sGdalSkipList()->removeAt( myPos );
  }
  applyGdalSkippedDrivers();
}

QStringList QgsApplication::skippedGdalDrivers()
{
  return *sGdalSkipList();
}

void QgsApplication::setSkippedGdalDrivers( const QStringList &skippedGdalDrivers,
    const QStringList &deferredSkippedGdalDrivers )
{
  *sGdalSkipList() = skippedGdalDrivers;
  *sDeferredSkippedGdalDrivers() = deferredSkippedGdalDrivers;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "gdal/skipDrivers" ), skippedGdalDrivers.join( QLatin1Char( ',' ) ) );

  applyGdalSkippedDrivers();
}

void QgsApplication::registerGdalDriversFromSettings()
{
  QgsSettings settings;
  QString joinedList, delimiter;
  if ( settings.contains( QStringLiteral( "gdal/skipDrivers" ) ) )
  {
    joinedList = settings.value( QStringLiteral( "gdal/skipDrivers" ), QString() ).toString();
    delimiter = QStringLiteral( "," );
  }
  else
  {
    joinedList = settings.value( QStringLiteral( "gdal/skipList" ), QString() ).toString();
    delimiter = QStringLiteral( " " );
  }
  QStringList myList;
  if ( !joinedList.isEmpty() )
  {
    myList = joinedList.split( delimiter );
  }
  *sGdalSkipList() = myList;
  applyGdalSkippedDrivers();
}

QStringList QgsApplication::deferredSkippedGdalDrivers()
{
  return *sDeferredSkippedGdalDrivers();
}

void QgsApplication::applyGdalSkippedDrivers()
{
  sGdalSkipList()->removeDuplicates();
  QStringList realDisabledDriverList;
  for ( const auto &driverName : *sGdalSkipList() )
  {
    if ( !sDeferredSkippedGdalDrivers()->contains( driverName ) )
      realDisabledDriverList << driverName;
  }
  QString myDriverList = realDisabledDriverList.join( ',' );
  QgsDebugMsgLevel( QStringLiteral( "Gdal Skipped driver list set to:" ), 2 );
  QgsDebugMsgLevel( myDriverList, 2 );
  CPLSetConfigOption( "GDAL_SKIP", myDriverList.toUtf8() );
  GDALAllRegister(); //to update driver list and skip missing ones
}

bool QgsApplication::createThemeFolder()
{
  QString folder = userThemesFolder();
  QDir myDir( folder );
  if ( !myDir.exists() )
  {
    myDir.mkpath( folder );
  }

  return true;
}

void QgsApplication::copyPath( const QString &src, const QString &dst )
{
  QDir dir( src );
  if ( ! dir.exists() )
    return;

  const auto subDirectories = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  for ( const QString &d : subDirectories )
  {
    QString dst_path = dst + QDir::separator() + d;
    dir.mkpath( dst_path );
    copyPath( src + QDir::separator() + d, dst_path );
  }

  const auto files = dir.entryList( QDir::Files );
  for ( const QString &f : files )
  {
    QFile::copy( src + QDir::separator() + f, dst + QDir::separator() + f );
  }
}

QVariantMap QgsApplication::customVariables()
{
  //read values from QgsSettings
  QgsSettings settings;

  QVariantMap variables;

  //check if settings contains any variables
  settings.beginGroup( "variables" );
  QStringList childKeys = settings.childKeys();
  for ( QStringList::const_iterator it = childKeys.constBegin(); it != childKeys.constEnd(); ++it )
  {
    QString name = *it;
    variables.insert( name, settings.value( name ) );
  }

  return variables;
}

void QgsApplication::setCustomVariables( const QVariantMap &variables )
{
  QgsSettings settings;

  QVariantMap::const_iterator it = variables.constBegin();
  settings.beginGroup( "variables" );
  settings.remove( "" );
  for ( ; it != variables.constEnd(); ++it )
  {
    settings.setValue( it.key(), it.value() );
  }

  emit instance()->customVariablesChanged();
}

void QgsApplication::setCustomVariable( const QString &name, const QVariant &value )
{
  // save variable to settings
  QgsSettings settings;

  settings.setValue( QStringLiteral( "variables/" ) + name, value );

  emit instance()->customVariablesChanged();
}

int QgsApplication::scaleIconSize( int standardSize, bool applyDevicePixelRatio )
{
  QFontMetrics fm( ( QFont() ) );
  const double scale = 1.1 * standardSize / 24;
  int scaledIconSize = static_cast< int >( std::floor( std::max( Qgis::UI_SCALE_FACTOR * fm.height() * scale, static_cast< double >( standardSize ) ) ) );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  if ( applyDevicePixelRatio && QApplication::desktop() )
    scaledIconSize *= QApplication::desktop()->devicePixelRatio();
#else
  if ( applyDevicePixelRatio )
  {
    if ( QWidget *activeWindow = QApplication::activeWindow() )
      scaledIconSize *= ( activeWindow->screen() ? QApplication::activeWindow()->screen()->devicePixelRatio() : 1 );
  }
#endif
  return scaledIconSize;
}

int QgsApplication::maxConcurrentConnectionsPerPool() const
{
  return CONN_POOL_MAX_CONCURRENT_CONNS;
}

void QgsApplication::setTranslation( const QString &translation )
{
  *sTranslation() = translation;
}

QString QgsApplication::translation() const
{
  return *sTranslation();
}

void QgsApplication::collectTranslatableObjects( QgsTranslationContext *translationContext )
{
  emit requestForTranslatableObjects( translationContext );
}

QString QgsApplication::nullRepresentation()
{
  ApplicationMembers *appMembers = members();
  if ( appMembers->mNullRepresentation.isNull() )
  {
    appMembers->mNullRepresentation = QgsSettings().value( QStringLiteral( "qgis/nullValue" ), QStringLiteral( "NULL" ) ).toString();
  }
  return appMembers->mNullRepresentation;
}

void QgsApplication::setNullRepresentation( const QString &nullRepresentation )
{
  ApplicationMembers *appMembers = members();
  if ( !appMembers || appMembers->mNullRepresentation == nullRepresentation )
    return;

  appMembers->mNullRepresentation = nullRepresentation;
  QgsSettings().setValue( QStringLiteral( "qgis/nullValue" ), nullRepresentation );

  QgsApplication *app = instance();
  if ( app )
    emit app->nullRepresentationChanged();
}

QgsActionScopeRegistry *QgsApplication::actionScopeRegistry()
{
  return members()->mActionScopeRegistry;
}

bool QgsApplication::createDatabase( QString *errorMessage )
{
  // set a working directory up for gdal to write .aux.xml files into
  // for cases where the raster dir is read only to the user
  // if the env var is already set it will be used preferentially
  QString myPamPath = qgisSettingsDirPath() + QStringLiteral( "gdal_pam/" );
  QDir myDir( myPamPath );
  if ( !myDir.exists() )
  {
    myDir.mkpath( myPamPath ); //fail silently
  }

#if defined(Q_OS_WIN)
  CPLSetConfigOption( "GDAL_PAM_PROXY_DIR", myPamPath.toUtf8() );
#else
  //under other OS's we use an environment var so the user can
  //override the path if he likes
  int myChangeFlag = 0; //whether we want to force the env var to change
  setenv( "GDAL_PAM_PROXY_DIR", myPamPath.toUtf8(), myChangeFlag );
#endif

  // Check qgis.db and make private copy if necessary
  QFile qgisPrivateDbFile( QgsApplication::qgisUserDatabaseFilePath() );

  // first we look for ~/.qgis/qgis.db
  if ( !qgisPrivateDbFile.exists() )
  {
    // if it doesn't exist we copy it in from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDatabaseFilePath();
    QFile masterFile( qgisMasterDbFileName );

    // Must be sure there is destination directory ~/.qgis
    QDir().mkpath( QgsApplication::qgisSettingsDirPath() );

    //now copy the master file into the users .qgis dir
    bool isDbFileCopied = masterFile.copy( qgisPrivateDbFile.fileName() );

    if ( !isDbFileCopied )
    {
      if ( errorMessage )
      {
        *errorMessage = tr( "[ERROR] Can not make qgis.db private copy" );
      }
      return false;
    }

    QFile::Permissions perms = QFile( qgisPrivateDbFile.fileName() ).permissions();
    if ( !( perms & QFile::WriteOwner ) )
    {
      if ( !qgisPrivateDbFile.setPermissions( perms | QFile::WriteOwner ) )
      {
        if ( errorMessage )
        {
          *errorMessage = tr( "Can not make '%1' user writable" ).arg( qgisPrivateDbFile.fileName() );
        }
        return false;
      }
    }
  }
  else
  {
    // migrate if necessary
    sqlite3_database_unique_ptr database;
    if ( database.open( QgsApplication::qgisUserDatabaseFilePath() ) != SQLITE_OK )
    {
      if ( errorMessage )
      {
        *errorMessage = tr( "Could not open qgis.db" );
      }
      return false;
    }

    char *errmsg = nullptr;
    int res = sqlite3_exec( database.get(), "SELECT srs_id FROM tbl_srs LIMIT 0", nullptr, nullptr, &errmsg );
    if ( res != SQLITE_OK )
    {
      sqlite3_free( errmsg );

      // qgis.db is missing tbl_srs, create it
      if ( sqlite3_exec( database.get(),
                         "DROP INDEX IF EXISTS idx_srsauthid;"
                         "CREATE TABLE tbl_srs ("
                         "srs_id INTEGER PRIMARY KEY,"
                         "description text NOT NULL,"
                         "projection_acronym text NOT NULL,"
                         "ellipsoid_acronym NOT NULL,"
                         "parameters text NOT NULL,"
                         "srid integer,"
                         "auth_name varchar,"
                         "auth_id varchar,"
                         "is_geo integer NOT NULL,"
                         "deprecated boolean,"
                         "wkt text);"
                         "CREATE INDEX idx_srsauthid on tbl_srs(auth_name,auth_id);", nullptr, nullptr, &errmsg ) != SQLITE_OK )
      {
        if ( errorMessage )
        {
          *errorMessage = tr( "Creation of missing tbl_srs in the private qgis.db failed.\n%1" ).arg( QString::fromUtf8( errmsg ) );
        }
        sqlite3_free( errmsg );
        return false;
      }
    }
    else
    {
      // test if wkt column exists in database
      res = sqlite3_exec( database.get(), "SELECT wkt FROM tbl_srs LIMIT 0", nullptr, nullptr, &errmsg );
      if ( res != SQLITE_OK )
      {
        // need to add wkt column
        sqlite3_free( errmsg );
        if ( sqlite3_exec( database.get(),
                           "DROP INDEX IF EXISTS idx_srsauthid;"
                           "DROP TABLE IF EXISTS tbl_srs_bak;"
                           "ALTER TABLE tbl_srs RENAME TO tbl_srs_bak;"
                           "CREATE TABLE tbl_srs ("
                           "srs_id INTEGER PRIMARY KEY,"
                           "description text NOT NULL,"
                           "projection_acronym text NOT NULL,"
                           "ellipsoid_acronym NOT NULL,"
                           "parameters text NOT NULL,"
                           "srid integer,"
                           "auth_name varchar,"
                           "auth_id varchar,"
                           "is_geo integer NOT NULL,"
                           "deprecated boolean,"
                           "wkt text);"
                           "CREATE INDEX idx_srsauthid on tbl_srs(auth_name,auth_id);"
                           "INSERT INTO tbl_srs(srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) SELECT srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,'','',is_geo,0 FROM tbl_srs_bak;"
                           "DROP TABLE tbl_srs_bak", nullptr, nullptr, &errmsg ) != SQLITE_OK )
        {
          if ( errorMessage )
          {
            *errorMessage = tr( "Migration of private qgis.db failed.\n%1" ).arg( QString::fromUtf8( errmsg ) );
          }
          sqlite3_free( errmsg );
          return false;
        }
      }
    }

    res = sqlite3_exec( database.get(), "SELECT acronym FROM tbl_projection LIMIT 0", nullptr, nullptr, &errmsg );
    if ( res != SQLITE_OK )
    {
      sqlite3_free( errmsg );

      // qgis.db is missing tbl_projection, create it
      if ( sqlite3_exec( database.get(),
                         "CREATE TABLE tbl_projection ("
                         "acronym varchar(20) NOT NULL PRIMARY KEY,"
                         "name varchar(255) NOT NULL default '',"
                         "notes varchar(255) NOT NULL default '',"
                         "parameters varchar(255) NOT NULL default ''"
                         ")", nullptr, nullptr, &errmsg ) != SQLITE_OK )
      {
        if ( errorMessage )
        {
          *errorMessage = tr( "Creation of missing tbl_projection in the private qgis.db failed.\n%1" ).arg( QString::fromUtf8( errmsg ) );
        }
        sqlite3_free( errmsg );
        return false;
      }
    }

    res = sqlite3_exec( database.get(), "SELECT epsg FROM tbl_srs LIMIT 0", nullptr, nullptr, &errmsg );
    if ( res == SQLITE_OK )
    {
      // epsg column exists => need migration
      if ( sqlite3_exec( database.get(),
                         "DROP INDEX IF EXISTS idx_srsauthid;"
                         "DROP TABLE IF EXISTS tbl_srs_bak;"
                         "ALTER TABLE tbl_srs RENAME TO tbl_srs_bak;"
                         "CREATE TABLE tbl_srs ("
                         "srs_id INTEGER PRIMARY KEY,"
                         "description text NOT NULL,"
                         "projection_acronym text NOT NULL,"
                         "ellipsoid_acronym NOT NULL,"
                         "parameters text NOT NULL,"
                         "srid integer,"
                         "auth_name varchar,"
                         "auth_id varchar,"
                         "is_geo integer NOT NULL,"
                         "deprecated boolean,"
                         "wkt text);"
                         "CREATE INDEX idx_srsauthid on tbl_srs(auth_name,auth_id);"
                         "INSERT INTO tbl_srs(srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) SELECT srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,'','',is_geo,0 FROM tbl_srs_bak;"
                         "DROP TABLE tbl_srs_bak", nullptr, nullptr, &errmsg ) != SQLITE_OK )
      {
        if ( errorMessage )
        {
          *errorMessage = tr( "Migration of private qgis.db failed.\n%1" ).arg( QString::fromUtf8( errmsg ) );
        }
        sqlite3_free( errmsg );
        return false;
      }
    }
    else
    {
      sqlite3_free( errmsg );
    }

    if ( sqlite3_exec( database.get(), "DROP VIEW vw_srs", nullptr, nullptr, &errmsg ) != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "vw_srs didn't exists in private qgis.db: %1" ).arg( errmsg ) );
    }

    if ( sqlite3_exec( database.get(),
                       "CREATE VIEW vw_srs AS"
                       " SELECT"
                       " a.description AS description"
                       ",a.srs_id AS srs_id"
                       ",a.is_geo AS is_geo"
                       ",coalesce(b.name,a.projection_acronym) AS name"
                       ",a.parameters AS parameters"
                       ",a.auth_name AS auth_name"
                       ",a.auth_id AS auth_id"
                       ",a.deprecated AS deprecated"
                       " FROM tbl_srs a"
                       " LEFT OUTER JOIN tbl_projection b ON a.projection_acronym=b.acronym"
                       " ORDER BY coalesce(b.name,a.projection_acronym),a.description", nullptr, nullptr, &errmsg ) != SQLITE_OK )
    {
      if ( errorMessage )
      {
        *errorMessage = tr( "Update of view in private qgis.db failed.\n%1" ).arg( QString::fromUtf8( errmsg ) );
      }
      sqlite3_free( errmsg );
      return false;
    }
  }
  return true;
}

void QgsApplication::setMaxThreads( int maxThreads )
{
  QgsDebugMsgLevel( QStringLiteral( "maxThreads: %1" ).arg( maxThreads ), 2 );

  // make sure value is between 1 and #cores, if not set to -1 (use #cores)
  // 0 could be used to disable any parallel processing
  if ( maxThreads < 1 || maxThreads > QThread::idealThreadCount() )
    maxThreads = -1;

  // save value
  ABISYM( sMaxThreads ) = maxThreads;

  // if -1 use #cores
  if ( maxThreads == -1 )
    maxThreads = QThread::idealThreadCount();

  // set max thread count in QThreadPool
  QThreadPool::globalInstance()->setMaxThreadCount( maxThreads );
  QgsDebugMsgLevel( QStringLiteral( "set QThreadPool max thread count to %1" ).arg( QThreadPool::globalInstance()->maxThreadCount() ), 2 );
}

QgsTaskManager *QgsApplication::taskManager()
{
  return members()->mTaskManager;
}

QgsSettingsRegistryCore *QgsApplication::settingsRegistryCore()
{
  return members()->mSettingsRegistryCore;
}

QgsColorSchemeRegistry *QgsApplication::colorSchemeRegistry()
{
  return members()->mColorSchemeRegistry;
}

QgsPaintEffectRegistry *QgsApplication::paintEffectRegistry()
{
  return members()->mPaintEffectRegistry;
}

QgsRendererRegistry *QgsApplication::rendererRegistry()
{
  return members()->mRendererRegistry;
}

QgsRasterRendererRegistry *QgsApplication::rasterRendererRegistry()
{
  return members()->mRasterRendererRegistry;
}

QgsPointCloudRendererRegistry *QgsApplication::pointCloudRendererRegistry()
{
  return members()->mPointCloudRendererRegistry;
}

QgsDataItemProviderRegistry *QgsApplication::dataItemProviderRegistry()
{
  if ( auto *lInstance = instance() )
  {
    if ( !instance()->mDataItemProviderRegistry )
    {
      lInstance->mDataItemProviderRegistry = new QgsDataItemProviderRegistry();
    }
    return lInstance->mDataItemProviderRegistry;
  }
  else
  {
    // no QgsApplication instance
    static QgsDataItemProviderRegistry *sDataItemProviderRegistry = nullptr;
    if ( !sDataItemProviderRegistry )
      sDataItemProviderRegistry = new QgsDataItemProviderRegistry();
    return sDataItemProviderRegistry;
  }
}

QgsCoordinateReferenceSystemRegistry *QgsApplication::coordinateReferenceSystemRegistry()
{
  return members()->mCrsRegistry;
}

QgsSvgCache *QgsApplication::svgCache()
{
  return members()->mSvgCache;
}

QgsImageCache *QgsApplication::imageCache()
{
  return members()->mImageCache;
}

QgsSourceCache *QgsApplication::sourceCache()
{
  return members()->mSourceCache;
}

QgsNetworkContentFetcherRegistry *QgsApplication::networkContentFetcherRegistry()
{
  return members()->mNetworkContentFetcherRegistry;
}

QgsValidityCheckRegistry *QgsApplication::validityCheckRegistry()
{
  return members()->mValidityCheckRegistry;
}

QgsSymbolLayerRegistry *QgsApplication::symbolLayerRegistry()
{
  return members()->mSymbolLayerRegistry;
}

QgsCalloutRegistry *QgsApplication::calloutRegistry()
{
  return members()->mCalloutRegistry;
}

QgsLayoutItemRegistry *QgsApplication::layoutItemRegistry()
{
  return members()->mLayoutItemRegistry;
}

QgsAnnotationItemRegistry *QgsApplication::annotationItemRegistry()
{
  return members()->mAnnotationItemRegistry;
}

QgsGpsConnectionRegistry *QgsApplication::gpsConnectionRegistry()
{
  return members()->mGpsConnectionRegistry;
}

QgsBabelFormatRegistry *QgsApplication::gpsBabelFormatRegistry()
{
  return members()->mGpsBabelFormatRegistry;
}

QgsPluginLayerRegistry *QgsApplication::pluginLayerRegistry()
{
  return members()->mPluginLayerRegistry;
}

QgsClassificationMethodRegistry *QgsApplication::classificationMethodRegistry()
{
  return members()->mClassificationMethodRegistry;
}

QgsBookmarkManager *QgsApplication::bookmarkManager()
{
  return members()->mBookmarkManager;
}

QgsTileDownloadManager *QgsApplication::tileDownloadManager()
{
  return members()->mTileDownloadManager;
}

QgsRecentStyleHandler *QgsApplication::recentStyleHandler()
{
  return members()->mRecentStyleHandler;
}

QgsDatabaseQueryLog *QgsApplication::databaseQueryLog()
{
  return members()->mQueryLogger;
}

QgsStyleModel *QgsApplication::defaultStyleModel()
{
  return members()->mStyleModel;
}

QgsMessageLog *QgsApplication::messageLog()
{
  return members()->mMessageLog;
}

QgsProcessingRegistry *QgsApplication::processingRegistry()
{
  return members()->mProcessingRegistry;
}

QgsConnectionRegistry *QgsApplication::connectionRegistry()
{
  return members()->mConnectionRegistry;
}

QgsPageSizeRegistry *QgsApplication::pageSizeRegistry()
{
  return members()->mPageSizeRegistry;
}

QgsAnnotationRegistry *QgsApplication::annotationRegistry()
{
  return members()->mAnnotationRegistry;
}

QgsNumericFormatRegistry *QgsApplication::numericFormatRegistry()
{
  return members()->mNumericFormatRegistry;
}

QgsFieldFormatterRegistry *QgsApplication::fieldFormatterRegistry()
{
  return members()->mFieldFormatterRegistry;
}

Qgs3DRendererRegistry *QgsApplication::renderer3DRegistry()
{
  return members()->m3DRendererRegistry;
}

Qgs3DSymbolRegistry *QgsApplication::symbol3DRegistry()
{
  return members()->m3DSymbolRegistry;
}

QgsScaleBarRendererRegistry *QgsApplication::scaleBarRendererRegistry()
{
  return members()->mScaleBarRendererRegistry;
}

QgsProjectStorageRegistry *QgsApplication::projectStorageRegistry()
{
  return members()->mProjectStorageRegistry.get();
}

QgsExternalStorageRegistry *QgsApplication::externalStorageRegistry()
{
  return members()->mExternalStorageRegistry;
}

QgsLocalizedDataPathRegistry *QgsApplication::localizedDataPathRegistry()
{
  return members()->mLocalizedDataPathRegistry;
}

QgsApplication::ApplicationMembers::ApplicationMembers()
{
  // don't use initializer lists or scoped pointers - as more objects are added here we
  // will need to be careful with the order of creation/destruction
  mSettingsRegistryCore = new QgsSettingsRegistryCore();
  mLocalizedDataPathRegistry = new QgsLocalizedDataPathRegistry();
  mMessageLog = new QgsMessageLog();
  QgsRuntimeProfiler *profiler = QgsRuntimeProfiler::threadLocalInstance();

  {
    profiler->start( tr( "Create query logger" ) );
    mQueryLogger = new QgsDatabaseQueryLog();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup coordinate reference system registry" ) );
    mCrsRegistry = new QgsCoordinateReferenceSystemRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Create connection registry" ) );
    mConnectionRegistry = new QgsConnectionRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup task manager" ) );
    mTaskManager = new QgsTaskManager();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup action scope registry" ) );
    mActionScopeRegistry = new QgsActionScopeRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup numeric formats" ) );
    mNumericFormatRegistry = new QgsNumericFormatRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup field formats" ) );
    mFieldFormatterRegistry = new QgsFieldFormatterRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup SVG cache" ) );
    mSvgCache = new QgsSvgCache();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup image cache" ) );
    mImageCache = new QgsImageCache();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup source cache" ) );
    mSourceCache = new QgsSourceCache();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup color scheme registry" ) );
    mColorSchemeRegistry = new QgsColorSchemeRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup paint effect" ) );
    mPaintEffectRegistry = new QgsPaintEffectRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup symbol layer registry" ) );
    mSymbolLayerRegistry = new QgsSymbolLayerRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Recent style handler" ) );
    mRecentStyleHandler = new QgsRecentStyleHandler();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup callout registry" ) );
    mCalloutRegistry = new QgsCalloutRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup renderer registry" ) );
    mRendererRegistry = new QgsRendererRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup raster renderer registry" ) );
    mRasterRendererRegistry = new QgsRasterRendererRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup point cloud renderer registry" ) );
    mPointCloudRendererRegistry = new QgsPointCloudRendererRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup GPS registry" ) );
    mGpsConnectionRegistry = new QgsGpsConnectionRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup GPSBabel format registry" ) );
    mGpsBabelFormatRegistry = new QgsBabelFormatRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup plugin layer registry" ) );
    mPluginLayerRegistry = new QgsPluginLayerRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup Processing registry" ) );
    mProcessingRegistry = new QgsProcessingRegistry();
    profiler->end();
  }
  mPageSizeRegistry = new QgsPageSizeRegistry();
  {
    profiler->start( tr( "Setup layout item registry" ) );
    mLayoutItemRegistry = new QgsLayoutItemRegistry();
    mLayoutItemRegistry->populate();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup annotation registry" ) );
    mAnnotationRegistry = new QgsAnnotationRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup annotation item registry" ) );
    mAnnotationItemRegistry = new QgsAnnotationItemRegistry();
    mAnnotationItemRegistry->populate();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup 3D symbol registry" ) );
    m3DSymbolRegistry = new Qgs3DSymbolRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup 3D renderer registry" ) );
    m3DRendererRegistry = new Qgs3DRendererRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup project storage registry" ) );
    mProjectStorageRegistry.reset( new QgsProjectStorageRegistry() );
    profiler->end();
  }
  {
    profiler->start( tr( "Setup external storage registry" ) );
    mExternalStorageRegistry = new QgsExternalStorageRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup network content cache" ) );
    mNetworkContentFetcherRegistry = new QgsNetworkContentFetcherRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup layout check registry" ) );
    mValidityCheckRegistry = new QgsValidityCheckRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup classification registry" ) );
    mClassificationMethodRegistry = new QgsClassificationMethodRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup bookmark manager" ) );
    mBookmarkManager = new QgsBookmarkManager( nullptr );
    profiler->end();
  }
  {
    profiler->start( tr( "Setup tile download manager" ) );
    mTileDownloadManager = new QgsTileDownloadManager();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup scalebar registry" ) );
    mScaleBarRendererRegistry = new QgsScaleBarRendererRegistry();
    profiler->end();
  }
}

QgsApplication::ApplicationMembers::~ApplicationMembers()
{
  delete mStyleModel;
  delete mTileDownloadManager;
  delete mScaleBarRendererRegistry;
  delete mValidityCheckRegistry;
  delete mActionScopeRegistry;
  delete m3DRendererRegistry;
  delete m3DSymbolRegistry;
  delete mAnnotationRegistry;
  delete mColorSchemeRegistry;
  delete mFieldFormatterRegistry;
  delete mGpsConnectionRegistry;
  delete mGpsBabelFormatRegistry;
  delete mMessageLog;
  delete mPaintEffectRegistry;
  delete mPluginLayerRegistry;
  delete mProcessingRegistry;
  delete mPageSizeRegistry;
  delete mAnnotationItemRegistry;
  delete mLayoutItemRegistry;
  delete mPointCloudRendererRegistry;
  delete mRasterRendererRegistry;
  delete mRendererRegistry;
  delete mSvgCache;
  delete mImageCache;
  delete mSourceCache;
  delete mCalloutRegistry;
  delete mRecentStyleHandler;
  delete mSymbolLayerRegistry;
  delete mExternalStorageRegistry;
  delete mTaskManager;
  delete mNetworkContentFetcherRegistry;
  delete mClassificationMethodRegistry;
  delete mNumericFormatRegistry;
  delete mBookmarkManager;
  delete mConnectionRegistry;
  delete mLocalizedDataPathRegistry;
  delete mCrsRegistry;
  delete mQueryLogger;
  delete mSettingsRegistryCore;
}

QgsApplication::ApplicationMembers *QgsApplication::members()
{
  if ( auto *lInstance = instance() )
  {
    return lInstance->mApplicationMembers;
  }
  else
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static QMutex sMemberMutex( QMutex::Recursive );
#else
    static QRecursiveMutex sMemberMutex;
#endif
    QMutexLocker lock( &sMemberMutex );
    if ( !sApplicationMembers )
      sApplicationMembers = new ApplicationMembers();
    return sApplicationMembers;
  }
}
