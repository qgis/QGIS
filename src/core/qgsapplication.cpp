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
#include "qgsrasterrendererregistry.h"
#include "qgsrendererregistry.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbollayerutils.h"
#include "qgscalloutsregistry.h"
#include "qgspluginlayerregistry.h"
#include "qgsclassificationmethodregistry.h"
#include "qgsmessagelog.h"
#include "qgsannotationregistry.h"
#include "qgssettings.h"
#include "qgsunittypes.h"
#include "qgsuserprofile.h"
#include "qgsuserprofilemanager.h"
#include "qgsreferencedgeometry.h"
#include "qgs3drendererregistry.h"
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

#include "gps/qgsgpsconnectionregistry.h"
#include "processing/qgsprocessingregistry.h"
#include "processing/models/qgsprocessingmodelchildparametersource.h"
#include "processing/models/qgsprocessingmodelchilddependency.h"

#include "layout/qgspagesizeregistry.h"

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

#ifndef Q_OS_WIN
#include <netinet/in.h>
#include <pwd.h>
#else
#include <winsock.h>
#include <windows.h>
#include <lmcons.h>
#define SECURITY_WIN32
#include <security.h>
#pragma comment( lib, "Secur32.lib" )
#endif

#include "qgsconfig.h"

#include <gdal.h>
#include <ogr_api.h>
#include <cpl_conv.h> // for setting gdal options
#include <sqlite3.h>

#if PROJ_VERSION_MAJOR>=6
#include <proj.h>
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
Q_GLOBAL_STATIC_WITH_ARGS( QString, sPlatformName, ( "desktop" ) )
Q_GLOBAL_STATIC( QString, sTranslation )

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
    mQtTranslator = new QTranslator();
    if ( mQtTranslator->load( QStringLiteral( "qt_" ) + *sTranslation(), QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) )
    {
      installTranslator( mQtTranslator );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "loading of qt translation failed %1/qt_%2" ).arg( QLibraryInfo::location( QLibraryInfo::TranslationsPath ), *sTranslation() ), 2 );
    }
  }

  mApplicationMembers = new ApplicationMembers();

  *sProfilePath() = profileFolder;
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

  qRegisterMetaType<QgsGeometry::Error>( "QgsGeometry::Error" );
  qRegisterMetaType<QgsProcessingFeatureSourceDefinition>( "QgsProcessingFeatureSourceDefinition" );
  qRegisterMetaType<QgsProcessingOutputLayerDefinition>( "QgsProcessingOutputLayerDefinition" );
  qRegisterMetaType<QgsUnitTypes::LayoutUnit>( "QgsUnitTypes::LayoutUnit" );
  qRegisterMetaType<QgsFeatureId>( "QgsFeatureId" );
  qRegisterMetaType<QgsFeatureIds>( "QgsFeatureIds" );
  qRegisterMetaType<QgsProperty>( "QgsProperty" );
  qRegisterMetaType<Qgis::MessageLevel>( "Qgis::MessageLevel" );
  qRegisterMetaType<QgsReferencedRectangle>( "QgsReferencedRectangle" );
  qRegisterMetaType<QgsReferencedPointXY>( "QgsReferencedPointXY" );
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
  qRegisterMetaType<QgsProcessingModelChildParameterSource>( "QgsProcessingModelChildParameterSource" );
  qRegisterMetaTypeStreamOperators<QgsProcessingModelChildParameterSource>( "QgsProcessingModelChildParameterSource" );
  qRegisterMetaType<QgsRemappingSinkDefinition>( "QgsRemappingSinkDefinition" );
  qRegisterMetaType<QgsProcessingModelChildDependency>( "QgsProcessingModelChildDependency" );
  qRegisterMetaType<QgsTextFormat>( "QgsTextFormat" );
  QMetaType::registerComparators<QgsProcessingModelChildDependency>();
  QMetaType::registerEqualsComparator<QgsProcessingFeatureSourceDefinition>();
  QMetaType::registerEqualsComparator<QgsProperty>();

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

#if PROJ_VERSION_MAJOR>=6
  // append local user-writable folder as a proj search path
  QStringList currentProjSearchPaths = QgsProjUtils::searchPaths();
  currentProjSearchPaths.append( qgisSettingsDirPath() + QStringLiteral( "proj" ) );
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
#endif


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
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( std::exception &e )
  {
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromLatin1( e.what() ) );
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( ... )
  {
    QgsDebugMsg( QStringLiteral( "Caught unhandled unknown exception" ) );
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

QIcon QgsApplication::getThemeIcon( const QString &name )
{
  QgsApplication *app = instance();
  if ( app && app->mIconCache.contains( name ) )
    return app->mIconCache.value( name );

  QIcon icon;

  QString myPreferredPath = activeThemePath() + QDir::separator() + name;
  QString myDefaultPath = defaultThemePath() + QDir::separator() + name;
  if ( QFile::exists( myPreferredPath ) )
  {
    icon = QIcon( myPreferredPath );
  }
  else if ( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesn't exist in the default theme either!
    icon = QIcon( myDefaultPath );
  }
  else
  {
    icon = QIcon();
  }

  if ( app )
    app->mIconCache.insert( name, icon );
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
    float scale = Qgis::UI_SCALE_FACTOR * app->fontMetrics().height() / 32.0;
    cursorIcon = QCursor( icon.pixmap( std::ceil( scale * 32 ), std::ceil( scale * 32 ) ), std::ceil( scale * activeX ), std::ceil( scale * activeY ) );
  }
  if ( app )
    app->mCursorCache.insert( cursor, cursorIcon );
  return cursorIcon;
}

// TODO: add some caching mechanism ?
QPixmap QgsApplication::getThemePixmap( const QString &name )
{
  QString myPreferredPath = activeThemePath() + QDir::separator() + name;
  QString myDefaultPath = defaultThemePath() + QDir::separator() + name;
  if ( QFile::exists( myPreferredPath ) )
  {
    return QPixmap( myPreferredPath );
  }
  else
  {
    //could still return an empty icon if it
    //doesn't exist in the default theme either!
    return QPixmap( myDefaultPath );
  }
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
      *sCfgIntDir() = appPath.split( '/', QString::SkipEmptyParts ).last();
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
  if ( themeName == QStringLiteral( "default" ) || !themes.contains( themeName ) )
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
  styledata.replace( QStringLiteral( "@theme_path" ), path );

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
    QRegularExpression regex( QStringLiteral( "(?<=[\\s:])([0-9\\.]+)(?=em)" ) );
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
#if PROJ_VERSION_MAJOR>=6
    QString tempCopy = QDir::tempPath() + "/srs6.db";
#else
    QString tempCopy = QDir::tempPath() + "/srs.db";
#endif

    if ( !QFile( tempCopy ).exists() )
    {
#if PROJ_VERSION_MAJOR>=6
      QFile f( buildSourcePath() + "/resources/srs6.db" );
#else
      QFile f( buildSourcePath() + "/resources/srs.db" );
#endif
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

QStringList QgsApplication::svgPaths()
{
  //local directories to search when looking for an SVG with a given basename
  //defined by user in options dialog
  QgsSettings settings;
  const QStringList pathList = settings.value( QStringLiteral( "svg/searchPathsForSVG" ) ).toStringList();

  // maintain user set order while stripping duplicates
  QStringList paths;
  for ( const QString &path : pathList )
  {
    if ( !paths.contains( path ) )
      paths.append( path );
  }
  for ( const QString &path : qgis::as_const( *sDefaultSvgPaths() ) )
  {
    if ( !paths.contains( path ) )
      paths.append( path );
  }

  return paths;
}

QStringList QgsApplication::layoutTemplatePaths()
{
  //local directories to search when looking for an template with a given basename
  //defined by user in options dialog
  QgsSettings settings;
  QStringList pathList = settings.value( QStringLiteral( "Layout/searchPathsForTemplates" ), QVariant(), QgsSettings::Core ).toStringList();

  return pathList;
}

QMap<QString, QString> QgsApplication::systemEnvVars()
{
  return *sSystemEnvVars();
}

QString QgsApplication::userStylePath()
{
  return qgisSettingsDirPath() + QStringLiteral( "symbology-style.db" );
}

QRegExp QgsApplication::shortNameRegExp()
{
  return QRegExp( "^[A-Za-z][A-Za-z0-9\\._-]*" );
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

  process.start( QStringLiteral( "whoami" ) );
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

QString QgsApplication::platform()
{
  return *sPlatformName();
}

QString QgsApplication::locale()
{
  QgsSettings settings;
  bool overrideLocale = settings.value( QStringLiteral( "locale/overrideFlag" ), false ).toBool();
  if ( overrideLocale )
  {
    QString locale = settings.value( QStringLiteral( "locale/userLocale" ), QString() ).toString();
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
  if ( instance() )
  {
    if ( !instance()->mAuthManager )
    {
      instance()->mAuthManager = QgsAuthManager::instance();
    }
    return instance()->mAuthManager;
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
  if ( instance() )
    delete instance()->mAuthManager;
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
                            "  font-family: 'Lato', 'Ubuntu', 'Lucida Grande', 'Segoe UI', 'Arial', sans-serif;"
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
                   "  border:10px solid black;"
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
                   "   border: solid 1px #eee;"
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

  QStringList targetElems = tPathUrl.split( '/', QString::SkipEmptyParts );
  QStringList aPathElems = aPathUrl.split( '/', QString::SkipEmptyParts );

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

  return aPathElems.join( QStringLiteral( "/" ) );
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

  QStringList srcElems = rPathUrl.split( '/', QString::SkipEmptyParts );
  QStringList targetElems = targetPathUrl.split( '/', QString::SkipEmptyParts );

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
  while ( ( pos = targetElems.indexOf( QStringLiteral( ".." ) ) ) > 0 )
  {
    // remove preceding element and ..
    targetElems.removeAt( pos - 1 );
    targetElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  targetElems.prepend( QString() );
#endif

  return targetElems.join( QStringLiteral( "/" ) );
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
  settings.setValue( QStringLiteral( "gdal/skipList" ), skippedGdalDrivers.join( QStringLiteral( " " ) ) );

  applyGdalSkippedDrivers();
}

void QgsApplication::registerGdalDriversFromSettings()
{
  QgsSettings settings;
  QString joinedList = settings.value( QStringLiteral( "gdal/skipList" ), QString() ).toString();
  QStringList myList;
  if ( !joinedList.isEmpty() )
  {
    myList = joinedList.split( ' ' );
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
  QString myDriverList = realDisabledDriverList.join( ' ' );
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

int QgsApplication::maxConcurrentConnectionsPerPool() const
{
  return CONN_POOL_MAX_CONCURRENT_CONNS;
}

void QgsApplication::setTranslation( const QString &translation )
{
  *sTranslation() = translation;
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

QgsDataItemProviderRegistry *QgsApplication::dataItemProviderRegistry()
{
  if ( instance() )
  {
    if ( !instance()->mDataItemProviderRegistry )
    {
      instance()->mDataItemProviderRegistry = new QgsDataItemProviderRegistry();
    }
    return instance()->mDataItemProviderRegistry;
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

QgsGpsConnectionRegistry *QgsApplication::gpsConnectionRegistry()
{
  return members()->mGpsConnectionRegistry;
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

QgsScaleBarRendererRegistry *QgsApplication::scaleBarRendererRegistry()
{
  return members()->mScaleBarRendererRegistry;
}

QgsProjectStorageRegistry *QgsApplication::projectStorageRegistry()
{
  return members()->mProjectStorageRegistry;
}

QgsLocalizedDataPathRegistry *QgsApplication::localizedDataPathRegistry()
{
  return members()->mLocalizedDataPathRegistry;
}

QgsApplication::ApplicationMembers::ApplicationMembers()
{
  // don't use initializer lists or scoped pointers - as more objects are added here we
  // will need to be careful with the order of creation/destruction
  mLocalizedDataPathRegistry = new QgsLocalizedDataPathRegistry();
  mMessageLog = new QgsMessageLog();
  QgsRuntimeProfiler *profiler = QgsRuntimeProfiler::threadLocalInstance();

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
    profiler->start( tr( "Setup GPS registry" ) );
    mGpsConnectionRegistry = new QgsGpsConnectionRegistry();
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
    profiler->start( tr( "Setup 3D renderer registry" ) );
    m3DRendererRegistry = new Qgs3DRendererRegistry();
    profiler->end();
  }
  {
    profiler->start( tr( "Setup project storage registry" ) );
    mProjectStorageRegistry = new QgsProjectStorageRegistry();
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
    profiler->start( tr( "Setup scalebar registry" ) );
    mScaleBarRendererRegistry = new QgsScaleBarRendererRegistry();
    profiler->end();
  }
}

QgsApplication::ApplicationMembers::~ApplicationMembers()
{
  delete mStyleModel;
  delete mScaleBarRendererRegistry;
  delete mValidityCheckRegistry;
  delete mActionScopeRegistry;
  delete m3DRendererRegistry;
  delete mAnnotationRegistry;
  delete mColorSchemeRegistry;
  delete mFieldFormatterRegistry;
  delete mGpsConnectionRegistry;
  delete mMessageLog;
  delete mPaintEffectRegistry;
  delete mPluginLayerRegistry;
  delete mProcessingRegistry;
  delete mProjectStorageRegistry;
  delete mPageSizeRegistry;
  delete mLayoutItemRegistry;
  delete mRasterRendererRegistry;
  delete mRendererRegistry;
  delete mSvgCache;
  delete mImageCache;
  delete mCalloutRegistry;
  delete mSymbolLayerRegistry;
  delete mTaskManager;
  delete mNetworkContentFetcherRegistry;
  delete mClassificationMethodRegistry;
  delete mNumericFormatRegistry;
  delete mBookmarkManager;
  delete mConnectionRegistry;
  delete mLocalizedDataPathRegistry;
}

QgsApplication::ApplicationMembers *QgsApplication::members()
{
  if ( instance() )
  {
    return instance()->mApplicationMembers;
  }
  else
  {
    static QMutex sMemberMutex( QMutex::Recursive );
    QMutexLocker lock( &sMemberMutex );
    if ( !sApplicationMembers )
      sApplicationMembers = new ApplicationMembers();
    return sApplicationMembers;
  }
}
