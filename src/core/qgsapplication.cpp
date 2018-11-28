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
#include "qgsdataitemproviderregistry.h"
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgslayoutitemregistry.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnetworkcontentfetcherregistry.h"
#include "qgsproviderregistry.h"
#include "qgsexpression.h"
#include "qgsactionscoperegistry.h"
#include "qgsruntimeprofiler.h"
#include "qgstaskmanager.h"
#include "qgsfieldformatterregistry.h"
#include "qgssvgcache.h"
#include "qgsimagecache.h"
#include "qgscolorschemeregistry.h"
#include "qgspainteffectregistry.h"
#include "qgsprojectstorageregistry.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrendererregistry.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbollayerutils.h"
#include "qgspluginlayerregistry.h"
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
#include "qgsvaliditycheckregistry.h"

#include "gps/qgsgpsconnectionregistry.h"
#include "processing/qgsprocessingregistry.h"

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

#define CONN_POOL_MAX_CONCURRENT_CONNS      4

QObject *ABISYM( QgsApplication::mFileOpenEventReceiver );
QStringList ABISYM( QgsApplication::mFileOpenEventList );
QString ABISYM( QgsApplication::mPrefixPath );
QString ABISYM( QgsApplication::mPluginPath );
QString ABISYM( QgsApplication::mPkgDataPath );
QString ABISYM( QgsApplication::mLibraryPath );
QString ABISYM( QgsApplication::mLibexecPath );
QString ABISYM( QgsApplication::mQmlImportPath );
QString ABISYM( QgsApplication::mThemeName );
QString ABISYM( QgsApplication::mUIThemeName );
QString ABISYM( QgsApplication::mProfilePath );

QStringList ABISYM( QgsApplication::mDefaultSvgPaths );
QMap<QString, QString> ABISYM( QgsApplication::mSystemEnvVars );
QString ABISYM( QgsApplication::mConfigPath );

bool ABISYM( QgsApplication::mInitialized ) = false;
bool ABISYM( QgsApplication::mRunningFromBuildDir ) = false;
QString ABISYM( QgsApplication::mBuildSourcePath );
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
QString ABISYM( QgsApplication::mCfgIntDir );
#endif
QString ABISYM( QgsApplication::mBuildOutputPath );
QStringList ABISYM( QgsApplication::mGdalSkipList );
int ABISYM( QgsApplication::mMaxThreads );
QString ABISYM( QgsApplication::mAuthDbDirPath );

QString QgsApplication::sUserName;
QString QgsApplication::sUserFullName;
QString QgsApplication::sPlatformName = QStringLiteral( "desktop" );
QString QgsApplication::sTranslation;

const char *QgsApplication::QGIS_ORGANIZATION_NAME = "QGIS";
const char *QgsApplication::QGIS_ORGANIZATION_DOMAIN = "qgis.org";
const char *QgsApplication::QGIS_APPLICATION_NAME = "QGIS3";

QgsApplication::ApplicationMembers *QgsApplication::sApplicationMembers = nullptr;

QgsApplication::QgsApplication( int &argc, char **argv, bool GUIenabled, const QString &profileFolder, const QString &platformName )
  : QApplication( argc, argv, GUIenabled )
{
  sPlatformName = platformName;

  if ( sTranslation != QLatin1String( "C" ) )
  {
    mQgisTranslator = new QTranslator();
    if ( mQgisTranslator->load( QStringLiteral( "qgis_" ) + sTranslation, i18nPath() ) )
    {
      installTranslator( mQgisTranslator );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "loading of qgis translation failed %1/qgis_%2" ).arg( i18nPath(), sTranslation ) );
    }

    /* Translation file for Qt.
     * The strings from the QMenuBar context section are used by Qt/Mac to shift
     * the About, Preferences and Quit items to the Mac Application menu.
     * These items must be translated identically in both qt_ and qgis_ files.
     */
    mQtTranslator = new QTranslator();
    if ( mQtTranslator->load( QStringLiteral( "qt_" ) + sTranslation, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) )
    {
      installTranslator( mQtTranslator );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "loading of qt translation failed %1/qt_%2" ).arg( QLibraryInfo::location( QLibraryInfo::TranslationsPath ), sTranslation ) );
    }
  }

  mApplicationMembers = new ApplicationMembers();

  ABISYM( mProfilePath ) = profileFolder;
}

void QgsApplication::init( QString profileFolder )
{
  if ( profileFolder.isEmpty() )
  {
    if ( getenv( "QGIS_CUSTOM_CONFIG_PATH" ) )
    {
      QString envProfileFolder = getenv( "QGIS_CUSTOM_CONFIG_PATH" );
      profileFolder = envProfileFolder + QDir::separator() + "profiles";
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

  ABISYM( mProfilePath ) = profileFolder;

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

  ( void ) resolvePkgPath();

  if ( ABISYM( mRunningFromBuildDir ) )
  {
    // we run from source directory - not installed to destination (specified prefix)
    ABISYM( mPrefixPath ) = QString(); // set invalid path
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    setPluginPath( ABISYM( mBuildOutputPath ) + '/' + QString( QGIS_PLUGIN_SUBDIR ) + '/' + ABISYM( mCfgIntDir ) );
#else
    setPluginPath( ABISYM( mBuildOutputPath ) + '/' + QStringLiteral( QGIS_PLUGIN_SUBDIR ) );
#endif
    setPkgDataPath( ABISYM( mBuildSourcePath ) ); // directly source path - used for: doc, resources, svg
    ABISYM( mLibraryPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_LIB_SUBDIR + '/';
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    ABISYM( mLibexecPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_LIBEXEC_SUBDIR + '/' + ABISYM( mCfgIntDir ) + '/';
#else
    ABISYM( mLibexecPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_LIBEXEC_SUBDIR + '/';
#endif
#if defined( HAVE_QUICK )
    ABISYM( mQmlImportPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_QML_SUBDIR + '/';
#endif
  }
  else
  {
    char *prefixPath = getenv( "QGIS_PREFIX_PATH" );
    if ( !prefixPath )
    {
      if ( ABISYM( mPrefixPath ).isNull() )
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

  ABISYM( mConfigPath ) = profileFolder + '/'; // make sure trailing slash is included
  ABISYM( mDefaultSvgPaths ) << qgisSettingsDirPath() + QStringLiteral( "svg/" );

  ABISYM( mAuthDbDirPath ) = qgisSettingsDirPath();
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
  ABISYM( mSystemEnvVars ) = systemEnvVarMap;

  // allow Qt to search for Qt plugins (e.g. sqldrivers) in our plugin directory
  QCoreApplication::addLibraryPath( pluginPath() );

  // set max. thread count to -1
  // this should be read from QgsSettings but we don't know where they are at this point
  // so we read actual value in main.cpp
  ABISYM( mMaxThreads ) = -1;

  colorSchemeRegistry()->addDefaultSchemes();
  colorSchemeRegistry()->initStyleScheme();

  ABISYM( mInitialized ) = true;
}

QgsApplication::~QgsApplication()
{
  delete mDataItemProviderRegistry;
  delete mApplicationMembers;
  delete mQgisTranslator;
  delete mQtTranslator;
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
      ABISYM( mFileOpenEventList ).append( static_cast<QFileOpenEvent *>( event )->file() );
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
  return members()->mProfiler;
}

void QgsApplication::setFileOpenEventReceiver( QObject *receiver )
{
  // Set receiver for FileOpen events
  ABISYM( mFileOpenEventReceiver ) = receiver;
  // Propagate any events collected before the receiver has registered.
  if ( ABISYM( mFileOpenEventList ).count() > 0 )
  {
    QStringListIterator i( ABISYM( mFileOpenEventList ) );
    while ( i.hasNext() )
    {
      QFileOpenEvent foe( i.next() );
      QgsApplication::sendEvent( ABISYM( mFileOpenEventReceiver ), &foe );
    }
    ABISYM( mFileOpenEventList ).clear();
  }
}

void QgsApplication::setPrefixPath( const QString &prefixPath, bool useDefaultPaths )
{
  ABISYM( mPrefixPath ) = prefixPath;
#if defined(_MSC_VER)
  if ( ABISYM( mPrefixPath ).endsWith( "/bin" ) )
  {
    ABISYM( mPrefixPath ).chop( 4 );
  }
#endif
  if ( useDefaultPaths && !ABISYM( mRunningFromBuildDir ) )
  {
    setPluginPath( ABISYM( mPrefixPath ) + '/' + QStringLiteral( QGIS_PLUGIN_SUBDIR ) );
    setPkgDataPath( ABISYM( mPrefixPath ) + '/' + QStringLiteral( QGIS_DATA_SUBDIR ) );
  }
  ABISYM( mLibraryPath ) = ABISYM( mPrefixPath ) + '/' + QGIS_LIB_SUBDIR + '/';
  ABISYM( mLibexecPath ) = ABISYM( mPrefixPath ) + '/' + QGIS_LIBEXEC_SUBDIR + '/';
#if defined( HAVE_QUICK )
  ABISYM( mQmlImportPath ) = ABISYM( mPrefixPath ) + '/' + QGIS_QML_SUBDIR + '/';
#endif
}

void QgsApplication::setPluginPath( const QString &pluginPath )
{
  ABISYM( mPluginPath ) = pluginPath;
}

void QgsApplication::setPkgDataPath( const QString &pkgDataPath )
{
  ABISYM( mPkgDataPath ) = pkgDataPath;
  QString mySvgPath = pkgDataPath + ( ABISYM( mRunningFromBuildDir ) ? "/images/svg/" : "/svg/" );
  // avoid duplicate entries
  if ( !ABISYM( mDefaultSvgPaths ).contains( mySvgPath ) )
    ABISYM( mDefaultSvgPaths ) << mySvgPath;
}

void QgsApplication::setDefaultSvgPaths( const QStringList &pathList )
{
  ABISYM( mDefaultSvgPaths ) = pathList;
}

void QgsApplication::setAuthDatabaseDirPath( const QString &authDbDirPath )
{
  QFileInfo fi( authDbDirPath );
  if ( fi.exists() && fi.isDir() && fi.isWritable() )
  {
    ABISYM( mAuthDbDirPath ) = fi.canonicalFilePath() + QDir::separator();
  }
}

QString QgsApplication::prefixPath()
{
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

  return ABISYM( mPrefixPath );
}
QString QgsApplication::pluginPath()
{
  return ABISYM( mPluginPath );
}
QString QgsApplication::pkgDataPath()
{
  if ( ABISYM( mPkgDataPath ).isNull() )
    return resolvePkgPath();
  else
    return ABISYM( mPkgDataPath );
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
  ABISYM( mThemeName ) = themeName;
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
    Q_FOREACH ( const QString &path, QStringList() << "" << "/.." << "/bin" << "/../../.." )
    {
      f.setFileName( prefix + path + "/qgisbuildpath.txt" );
      if ( f.exists() )
        break;
    }
    if ( f.exists() && f.open( QIODevice::ReadOnly ) )
    {
      ABISYM( mRunningFromBuildDir ) = true;
      ABISYM( mBuildSourcePath ) = f.readLine().trimmed();
      ABISYM( mBuildOutputPath ) = f.readLine().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "Running from build directory!" ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "- source directory: %1" ).arg( ABISYM( mBuildSourcePath ).toUtf8().constData() ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "- output directory of the build: %1" ).arg( ABISYM( mBuildOutputPath ).toUtf8().constData() ), 4 );
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
      ABISYM( mCfgIntDir ) = appPath.split( '/', QString::SkipEmptyParts ).last();
      qDebug( "- cfg: %s", ABISYM( mCfgIntDir ).toUtf8().constData() );
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

#if defined(Q_OS_MACX) || defined(Q_OS_WIN)
    prefixPath = appPath;
#if defined(_MSC_VER)
    if ( prefixPath.endsWith( "/bin" ) )
      prefixPath.chop( 4 );
#endif
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
    return ABISYM( mBuildSourcePath );
  else
    return prefixPath + '/' + QStringLiteral( QGIS_DATA_SUBDIR );
}

QString QgsApplication::themeName()
{
  return ABISYM( mThemeName );
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
  Q_FOREACH ( const QString &path, paths )
  {
    QDir folder( path );
    QFileInfoList styleFiles = folder.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    Q_FOREACH ( const QFileInfo &info, styleFiles )
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
    return ABISYM( mBuildOutputPath ) + QStringLiteral( "/i18n/" );
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
  return ABISYM( mConfigPath );
}

QString QgsApplication::qgisUserDatabaseFilePath()
{
  return qgisSettingsDirPath() + QStringLiteral( "qgis.db" );
}

QString QgsApplication::qgisAuthDatabaseFilePath()
{
  return ABISYM( mAuthDbDirPath ) + QStringLiteral( "qgis-auth.db" );
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
    QString tempCopy = QDir::tempPath() + "/srs.db";

    if ( !QFile( tempCopy ).exists() )
    {
      QFile f( pkgDataPath() + "/resources/srs.db" );
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
  QStringList pathList = settings.value( QStringLiteral( "svg/searchPathsForSVG" ) ).toStringList();

  // maintain user set order while stripping duplicates
  QStringList paths;
  Q_FOREACH ( const QString &path, pathList )
  {
    if ( !paths.contains( path ) )
      paths.append( path );
  }
  Q_FOREACH ( const QString &path, ABISYM( mDefaultSvgPaths ) )
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
  if ( !sUserName.isEmpty() )
    return sUserName;

#ifdef _MSC_VER
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  if ( GetUserName( ( TCHAR * )name, &size ) )
  {
    sUserName = QString( name );
  }

#elif QT_CONFIG(process)
  QProcess process;

  process.start( QStringLiteral( "whoami" ) );
  process.waitForFinished();
  sUserName = process.readAllStandardOutput().trimmed();
#endif

  if ( !sUserName.isEmpty() )
    return sUserName;

  //backup plan - use environment variables
  sUserName = qgetenv( "USER" );
  if ( !sUserName.isEmpty() )
    return sUserName;

  //last resort
  sUserName = qgetenv( "USERNAME" );
  return sUserName;
}

QString QgsApplication::userFullName()
{
  if ( !sUserFullName.isEmpty() )
    return sUserFullName;

#ifdef _MSC_VER
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  //note - this only works for accounts connected to domain
  if ( GetUserNameEx( NameDisplay, ( TCHAR * )name, &size ) )
  {
    sUserFullName = QString( name );
  }

  //fall back to login name
  if ( sUserFullName.isEmpty() )
    sUserFullName = userLoginName();
#elif defined(Q_OS_ANDROID) || defined(__MINGW32__)
  sUserFullName = "Not available";
#else
  struct passwd *p = getpwuid( getuid() );

  if ( p )
  {
    QString gecosName = QString( p->pw_gecos );
    sUserFullName = gecosName.left( gecosName.indexOf( ',', 0 ) );
  }

#endif

  return sUserFullName;
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
#else
  return QLatin1String( "unknown" );
#endif
}

QString QgsApplication::platform()
{
  return sPlatformName;
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
  return ABISYM( mLibraryPath );
}

QString QgsApplication::libexecPath()
{
  return ABISYM( mLibexecPath );
}

QString QgsApplication::qmlImportPath()
{
  return ABISYM( mQmlImportPath );
}

QgsApplication::endian_t QgsApplication::endian()
{
  return ( htonl( 1 ) == 1 ) ? XDR : NDR;
}

void QgsApplication::initQgis()
{
  if ( !ABISYM( mInitialized ) && QgsApplication::instance() )
  {
    init( ABISYM( mProfilePath ) );
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
    static QgsAuthManager *sAuthManager = nullptr;
    if ( !sAuthManager )
      sAuthManager = QgsAuthManager::instance();
    return sAuthManager;
  }
}


void QgsApplication::exitQgis()
{
  delete QgsApplication::authManager();

  //Ensure that all remaining deleteLater QObjects are actually deleted before we exit.
  //This isn't strictly necessary (since we're exiting anyway) but doing so prevents a lot of
  //LeakSanitiser noise which hides real issues
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

  //delete all registered functions from expression engine (see above comment)
  QgsExpression::cleanRegisteredFunctions();

  delete QgsProject::instance();

  delete QgsProviderRegistry::instance();

  // invalidate coordinate cache while the PROJ context held by the thread-locale
  // QgsProjContextStore object is still alive. Otherwise if this later object
  // is destroyed before the static variables of the cache, we might use freed memory.
  QgsCoordinateTransform::invalidateCache();

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

QString QgsApplication::reportStyleSheet()
{
  //
  // Make the style sheet desktop preferences aware by using qappliation
  // palette as a basis for colors where appropriate
  //
//  QColor myColor1 = palette().highlight().color();
  QColor myColor1( Qt::lightGray );
  QColor myColor2 = myColor1;
  myColor2 = myColor2.lighter( 110 ); //10% lighter
  QString myStyle;
  myStyle = ".overview{"
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
            "  text-align: right;"
            "  border-bottom: 5px solid #DCEB5C;"
            "}"
            "h4{  background-color: #F6F6F6;"
            "  color: #93b023;"  // from http://qgis.org/en/site/getinvolved/styleguide.html
            "  font-weight: bold;"
            "  font-size: medium;"
            "  text-align: right;"
            "}"
            "h5{    background-color: #F6F6F6;"
            "   color: #93b023;"  // from http://qgis.org/en/site/getinvolved/styleguide.html
            "   font-weight: bold;"
            "   font-size: small;"
            "   text-align: right;"
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
            ".section {"
            "  font-weight: bold;"
            "  padding-top:25px;"
            "}"
            ".list-view .highlight {"
            "  text-align: right;"
            "  border: 0px;"
            "  width: 20%;"
            "  padding-right: 15px;"
            "  padding-left: 20px;"
            "  font-weight: bold;"
            "}"
            "th .strong {"
            "  font-weight: bold;"
            "}"
            ".tabular-view{ "
            "  border-collapse: collapse;"
            "  width: 95%;"
            "}"
            ".tabular-view th, .tabular-view td { "
            "  border:10px solid black;"
            "}"
            ".tabular-view .odd-row{"
            "  background-color: #f9f9f9;"
            "}"
            "hr {"
            "  border: 0;"
            "  height: 0;"
            "  border-top: 1px solid black;"
            "}";
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

void QgsApplication::skipGdalDriver( const QString &driver )
{
  if ( ABISYM( mGdalSkipList ).contains( driver ) || driver.isEmpty() )
  {
    return;
  }
  ABISYM( mGdalSkipList ) << driver;
  applyGdalSkippedDrivers();
}

void QgsApplication::restoreGdalDriver( const QString &driver )
{
  if ( !ABISYM( mGdalSkipList ).contains( driver ) )
  {
    return;
  }
  int myPos = ABISYM( mGdalSkipList ).indexOf( driver );
  if ( myPos >= 0 )
  {
    ABISYM( mGdalSkipList ).removeAt( myPos );
  }
  applyGdalSkippedDrivers();
}

void QgsApplication::applyGdalSkippedDrivers()
{
  ABISYM( mGdalSkipList ).removeDuplicates();
  QString myDriverList = ABISYM( mGdalSkipList ).join( QStringLiteral( " " ) );
  QgsDebugMsg( QStringLiteral( "Gdal Skipped driver list set to:" ) );
  QgsDebugMsg( myDriverList );
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

  Q_FOREACH ( const QString &d, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    QString dst_path = dst + QDir::separator() + d;
    dir.mkpath( dst_path );
    copyPath( src + QDir::separator() + d, dst_path );
  }

  Q_FOREACH ( const QString &f, dir.entryList( QDir::Files ) )
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
    int res = sqlite3_exec( database.get(), "SELECT epsg FROM tbl_srs LIMIT 0", nullptr, nullptr, &errmsg );
    if ( res == SQLITE_OK )
    {
      // epsg column exists => need migration
      if ( sqlite3_exec( database.get(),
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
                         "deprecated boolean);"
                         "CREATE INDEX idx_srsauthid on tbl_srs(auth_name,auth_id);"
                         "INSERT INTO tbl_srs(srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) SELECT srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,'','',is_geo,0 FROM tbl_srs_bak;"
                         "DROP TABLE tbl_srs_bak", nullptr, nullptr, &errmsg ) != SQLITE_OK
         )
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
                       " ORDER BY coalesce(b.name,a.projection_acronym),a.description", nullptr, nullptr, &errmsg ) != SQLITE_OK
       )
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
  QgsDebugMsg( QStringLiteral( "maxThreads: %1" ).arg( maxThreads ) );

  // make sure value is between 1 and #cores, if not set to -1 (use #cores)
  // 0 could be used to disable any parallel processing
  if ( maxThreads < 1 || maxThreads > QThread::idealThreadCount() )
    maxThreads = -1;

  // save value
  ABISYM( mMaxThreads ) = maxThreads;

  // if -1 use #cores
  if ( maxThreads == -1 )
    maxThreads = QThread::idealThreadCount();

  // set max thread count in QThreadPool
  QThreadPool::globalInstance()->setMaxThreadCount( maxThreads );
  QgsDebugMsg( QStringLiteral( "set QThreadPool max thread count to %1" ).arg( QThreadPool::globalInstance()->maxThreadCount() ) );
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

QgsMessageLog *QgsApplication::messageLog()
{
  return members()->mMessageLog;
}

QgsProcessingRegistry *QgsApplication::processingRegistry()
{
  return members()->mProcessingRegistry;
}

QgsPageSizeRegistry *QgsApplication::pageSizeRegistry()
{
  return members()->mPageSizeRegistry;
}

QgsAnnotationRegistry *QgsApplication::annotationRegistry()
{
  return members()->mAnnotationRegistry;
}

QgsFieldFormatterRegistry *QgsApplication::fieldFormatterRegistry()
{
  return members()->mFieldFormatterRegistry;
}

Qgs3DRendererRegistry *QgsApplication::renderer3DRegistry()
{
  return members()->m3DRendererRegistry;
}

QgsProjectStorageRegistry *QgsApplication::projectStorageRegistry()
{
  return members()->mProjectStorageRegistry;
}

QgsApplication::ApplicationMembers::ApplicationMembers()
{
  // don't use initializer lists or scoped pointers - as more objects are added here we
  // will need to be careful with the order of creation/destruction
  mMessageLog = new QgsMessageLog();
  mProfiler = new QgsRuntimeProfiler();
  mTaskManager = new QgsTaskManager();
  mActionScopeRegistry = new QgsActionScopeRegistry();
  mFieldFormatterRegistry = new QgsFieldFormatterRegistry();
  mSvgCache = new QgsSvgCache();
  mImageCache = new QgsImageCache();
  mColorSchemeRegistry = new QgsColorSchemeRegistry();
  mPaintEffectRegistry = new QgsPaintEffectRegistry();
  mSymbolLayerRegistry = new QgsSymbolLayerRegistry();
  mRendererRegistry = new QgsRendererRegistry();
  mRasterRendererRegistry = new QgsRasterRendererRegistry();
  mGpsConnectionRegistry = new QgsGpsConnectionRegistry();
  mPluginLayerRegistry = new QgsPluginLayerRegistry();
  mProcessingRegistry = new QgsProcessingRegistry();
  mPageSizeRegistry = new QgsPageSizeRegistry();
  mLayoutItemRegistry = new QgsLayoutItemRegistry();
  mLayoutItemRegistry->populate();
  mAnnotationRegistry = new QgsAnnotationRegistry();
  m3DRendererRegistry = new Qgs3DRendererRegistry();
  mProjectStorageRegistry = new QgsProjectStorageRegistry();
  mNetworkContentFetcherRegistry = new QgsNetworkContentFetcherRegistry();
  mValidityCheckRegistry = new QgsValidityCheckRegistry();
}

QgsApplication::ApplicationMembers::~ApplicationMembers()
{
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
  delete mProfiler;
  delete mRasterRendererRegistry;
  delete mRendererRegistry;
  delete mSvgCache;
  delete mImageCache;
  delete mSymbolLayerRegistry;
  delete mTaskManager;
  delete mNetworkContentFetcherRegistry;
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
