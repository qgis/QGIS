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
#include "qgscrscache.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproviderregistry.h"
#include "qgsexpression.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QPalette>
#include <QProcess>
#include <QSettings>
#include <QIcon>
#include <QPixmap>
#include <QThreadPool>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#include <pwd.h>
#else
#include <winsock.h>
#include <windows.h>
#include <Lmcons.h>
#define SECURITY_WIN32
#include <Security.h>
#pragma comment( lib, "Secur32.lib" )
#endif

#include "qgsconfig.h"

#include <gdal.h>
#include <ogr_api.h>
#include <cpl_conv.h> // for setting gdal options
#include <sqlite3.h>

QObject * ABISYM( QgsApplication::mFileOpenEventReceiver );
QStringList ABISYM( QgsApplication::mFileOpenEventList );
QString ABISYM( QgsApplication::mPrefixPath );
QString ABISYM( QgsApplication::mPluginPath );
QString ABISYM( QgsApplication::mPkgDataPath );
QString ABISYM( QgsApplication::mLibraryPath );
QString ABISYM( QgsApplication::mLibexecPath );
QString ABISYM( QgsApplication::mThemeName );
QString ABISYM( QgsApplication::mUIThemeName );
QStringList ABISYM( QgsApplication::mDefaultSvgPaths );
QMap<QString, QString> ABISYM( QgsApplication::mSystemEnvVars );
QString ABISYM( QgsApplication::mConfigPath );
bool ABISYM( QgsApplication::mRunningFromBuildDir ) = false;
QString ABISYM( QgsApplication::mBuildSourcePath );
#ifdef _MSC_VER
QString ABISYM( QgsApplication::mCfgIntDir );
#endif
QString ABISYM( QgsApplication::mBuildOutputPath );
QStringList ABISYM( QgsApplication::mGdalSkipList );
int ABISYM( QgsApplication::mMaxThreads );
QString ABISYM( QgsApplication::mAuthDbDirPath );

QString QgsApplication::sUserName;
QString QgsApplication::sUserFullName;
QString QgsApplication::sPlatformName = "desktop";

const char* QgsApplication::QGIS_ORGANIZATION_NAME = "QGIS";
const char* QgsApplication::QGIS_ORGANIZATION_DOMAIN = "qgis.org";
const char* QgsApplication::QGIS_APPLICATION_NAME = "QGIS2";

/*!
  \class QgsApplication
  \brief The QgsApplication class manages application-wide information.

  This is a subclass of QApplication and should be instantiated in place of
  QApplication. Most methods are static in keeping witn the design of QApplication.

  This class hides platform-specific path information and provides
  a portable way of referencing specific files and directories.
  Ideally, hard-coded paths should appear only here and not in other modules
  so that platform-conditional code is minimized and paths are easier
  to change due to centralization.
*/
QgsApplication::QgsApplication( int & argc, char ** argv, bool GUIenabled, const QString& customConfigPath, const QString& platformName )
    : QApplication( argc, argv, GUIenabled )
{
  sPlatformName = platformName;

  init( customConfigPath ); // init can also be called directly by e.g. unit tests that don't inherit QApplication.
}

void QgsApplication::init( QString customConfigPath )
{
  if ( customConfigPath.isEmpty() )
  {
    if ( getenv( "QGIS_CUSTOM_CONFIG_PATH" ) )
    {
      customConfigPath = getenv( "QGIS_CUSTOM_CONFIG_PATH" );
    }
    else
    {
      customConfigPath = QString( "%1/.qgis%2/" ).arg( QDir::homePath() ).arg( QGis::QGIS_VERSION_INT / 10000 );
    }
  }

  qRegisterMetaType<QgsGeometry::Error>( "QgsGeometry::Error" );

  QString prefixPath( getenv( "QGIS_PREFIX_PATH" ) ? getenv( "QGIS_PREFIX_PATH" ) : applicationDirPath() );
  // QgsDebugMsg( QString( "prefixPath(): %1" ).arg( prefixPath ) );

  // check if QGIS is run from build directory (not the install directory)
  QFile f;
  // "/../../.." is for Mac bundled app in build directory
  Q_FOREACH ( const QString& path, QStringList() << "" << "/.." << "/bin" << "/../../.." )
  {
    f.setFileName( prefixPath + path + "/qgisbuildpath.txt" );
    if ( f.exists() )
      break;
  }
  if ( f.exists() && f.open( QIODevice::ReadOnly ) )
  {
    ABISYM( mRunningFromBuildDir ) = true;
    ABISYM( mBuildSourcePath ) = f.readLine().trimmed();
    ABISYM( mBuildOutputPath ) = f.readLine().trimmed();
    qDebug( "Running from build directory!" );
    qDebug( "- source directory: %s", ABISYM( mBuildSourcePath ).toUtf8().data() );
    qDebug( "- output directory of the build: %s", ABISYM( mBuildOutputPath ).toUtf8().data() );
#ifdef _MSC_VER
    ABISYM( mCfgIntDir ) = prefixPath.split( '/', QString::SkipEmptyParts ).last();
    qDebug( "- cfg: %s", ABISYM( mCfgIntDir ).toUtf8().data() );
#endif
  }

  if ( ABISYM( mRunningFromBuildDir ) )
  {
    // we run from source directory - not installed to destination (specified prefix)
    ABISYM( mPrefixPath ) = QString(); // set invalid path
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    setPluginPath( ABISYM( mBuildOutputPath ) + '/' + QString( QGIS_PLUGIN_SUBDIR ) + '/' + ABISYM( mCfgIntDir ) );
#else
    setPluginPath( ABISYM( mBuildOutputPath ) + '/' + QString( QGIS_PLUGIN_SUBDIR ) );
#endif
    setPkgDataPath( ABISYM( mBuildSourcePath ) ); // directly source path - used for: doc, resources, svg
    ABISYM( mLibraryPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_LIB_SUBDIR + '/';
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    ABISYM( mLibexecPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_LIBEXEC_SUBDIR + '/' + ABISYM( mCfgIntDir ) + '/';
#else
    ABISYM( mLibexecPath ) = ABISYM( mBuildOutputPath ) + '/' + QGIS_LIBEXEC_SUBDIR + '/';
#endif
  }
  else
  {
    char *prefixPath = getenv( "QGIS_PREFIX_PATH" );
    if ( !prefixPath )
    {
#if defined(Q_OS_MACX) || defined(Q_OS_WIN)
      setPrefixPath( applicationDirPath(), true );
#elif defined(ANDROID)
      // this is  "/data/data/org.qgis.qgis" in android
      QDir myDir( QDir::homePath() );
      myDir.cdUp();
      QString myPrefix = myDir.absolutePath();
      setPrefixPath( myPrefix, true );
#else
      QDir myDir( applicationDirPath() );
      myDir.cdUp();
      QString myPrefix = myDir.absolutePath();
      setPrefixPath( myPrefix, true );
#endif
    }
    else
    {
      setPrefixPath( prefixPath, true );
    }
  }

  if ( !customConfigPath.isEmpty() )
  {
    ABISYM( mConfigPath ) = customConfigPath + '/'; // make sure trailing slash is included
  }

  ABISYM( mDefaultSvgPaths ) << qgisSettingsDirPath() + QLatin1String( "svg/" );

  ABISYM( mAuthDbDirPath ) = qgisSettingsDirPath();
  if ( getenv( "QGIS_AUTH_DB_DIR_PATH" ) )
  {
    setAuthDbDirPath( getenv( "QGIS_AUTH_DB_DIR_PATH" ) );
  }


  // store system environment variables passed to application, before they are adjusted
  QMap<QString, QString> systemEnvVarMap;
  QString passfile( "QGIS_AUTH_PASSWORD_FILE" ); // QString, for comparison
  Q_FOREACH ( const QString &varStr, QProcess::systemEnvironment() )
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
  // this should be read from QSettings but we don't know where they are at this point
  // so we read actual value in main.cpp
  ABISYM( mMaxThreads ) = -1;
}

QgsApplication::~QgsApplication()
{
}

bool QgsApplication::event( QEvent * event )
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

bool QgsApplication::notify( QObject * receiver, QEvent * event )
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
  catch ( QgsException & e )
  {
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( std::exception & e )
  {
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromAscii( e.what() ) );
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( ... )
  {
    QgsDebugMsg( "Caught unhandled unknown exception" );
    if ( qApp->thread() == QThread::currentThread() )
      QMessageBox::critical( activeWindow(), tr( "Exception" ), tr( "unknown exception" ) );
  }

  return done;
}

void QgsApplication::setFileOpenEventReceiver( QObject * receiver )
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

void QgsApplication::setPrefixPath( const QString &thePrefixPath, bool useDefaultPaths )
{
  ABISYM( mPrefixPath ) = thePrefixPath;
#if defined(_MSC_VER)
  if ( ABISYM( mPrefixPath ).endsWith( "/bin" ) )
  {
    ABISYM( mPrefixPath ).chop( 4 );
  }
#endif
  if ( useDefaultPaths && !ABISYM( mRunningFromBuildDir ) )
  {
    setPluginPath( ABISYM( mPrefixPath ) + '/' + QString( QGIS_PLUGIN_SUBDIR ) );
    setPkgDataPath( ABISYM( mPrefixPath ) + '/' + QString( QGIS_DATA_SUBDIR ) );
  }
  ABISYM( mLibraryPath ) = ABISYM( mPrefixPath ) + '/' + QGIS_LIB_SUBDIR + '/';
  ABISYM( mLibexecPath ) = ABISYM( mPrefixPath ) + '/' + QGIS_LIBEXEC_SUBDIR + '/';
}

void QgsApplication::setPluginPath( const QString &thePluginPath )
{
  ABISYM( mPluginPath ) = thePluginPath;
}

void QgsApplication::setPkgDataPath( const QString &thePkgDataPath )
{
  ABISYM( mPkgDataPath ) = thePkgDataPath;
  QString mySvgPath = thePkgDataPath + ( ABISYM( mRunningFromBuildDir ) ? "/images/svg/" : "/svg/" );
  // avoid duplicate entries
  if ( !ABISYM( mDefaultSvgPaths ).contains( mySvgPath ) )
    ABISYM( mDefaultSvgPaths ) << mySvgPath;
}

void QgsApplication::setDefaultSvgPaths( const QStringList& pathList )
{
  ABISYM( mDefaultSvgPaths ) = pathList;
}

void QgsApplication::setAuthDbDirPath( const QString& theAuthDbDirPath )
{
  QFileInfo fi( theAuthDbDirPath );
  if ( fi.exists() && fi.isDir() && fi.isWritable() )
  {
    ABISYM( mAuthDbDirPath ) = fi.canonicalFilePath() + QDir::separator();
  }
}

QString QgsApplication::prefixPath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
  {
    static bool once = true;
    if ( once )
      qWarning( "!!! prefix path was requested, but it is not valid - we do not run from installed path !!!" );
    once = false;
  }

  return ABISYM( mPrefixPath );
}
QString QgsApplication::pluginPath()
{
  return ABISYM( mPluginPath );
}
QString QgsApplication::pkgDataPath()
{
  return ABISYM( mPkgDataPath );
}
QString QgsApplication::defaultThemePath()
{
  return ":/images/themes/default/";
}
QString QgsApplication::activeThemePath()
{
  return userThemesFolder() + QDir::separator() + themeName() + QDir::separator() + "icons/";
}

QString QgsApplication::appIconPath()
{
  return iconsPath() + ( QDate::currentDate().month() == 12 ? tr( "qgis-icon-60x60_xmas.png", "December application icon" ) : QString( "qgis-icon-60x60.png" ) );
}

QString QgsApplication::iconPath( const QString& iconFile )
{
  // try active theme
  QString path = activeThemePath();
  if ( QFile::exists( path + iconFile ) )
    return path + iconFile;

  // use default theme
  return defaultThemePath() + iconFile;
}

QIcon QgsApplication::getThemeIcon( const QString &theName )
{
  QgsApplication* app = qobject_cast<QgsApplication*>( instance() );
  if ( app && app->mIconCache.contains( theName ) )
    return app->mIconCache.value( theName );

  QIcon icon;

  QString myPreferredPath = activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = defaultThemePath() + QDir::separator() + theName;
  if ( QFile::exists( myPreferredPath ) )
  {
    icon = QIcon( myPreferredPath );
  }
  else if ( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    icon = QIcon( myDefaultPath );
  }
  else
  {
    icon = QIcon();
  }

  if ( app )
    app->mIconCache.insert( theName, icon );
  return icon;
}

// TODO: add some caching mechanism ?
QPixmap QgsApplication::getThemePixmap( const QString &theName )
{
  QString myPreferredPath = activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = defaultThemePath() + QDir::separator() + theName;
  if ( QFile::exists( myPreferredPath ) )
  {
    return QPixmap( myPreferredPath );
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QPixmap( myDefaultPath );
  }
}

/*!
  Set the theme path to the specified theme.
*/
void QgsApplication::setThemeName( const QString &theThemeName )
{
  ABISYM( mThemeName ) = theThemeName;
}
/*!
 * Get the active theme name
 */
QString QgsApplication::themeName()
{
  return ABISYM( mThemeName );
}

void QgsApplication::setUITheme( const QString &themeName )
{
  // Loop all style sheets, find matching name, load it.
  QHash<QString, QString> themes = QgsApplication::uiThemes();
  QString themename = themeName;
  if ( !themes.contains( themename ) )
    themename = "default";

  QString path = themes[themename];
  QString stylesheetname = path + "/style.qss";
  QString autostylesheet = stylesheetname + ".auto";

  QFile file( stylesheetname );
  QFile variablesfile( path + "/variables.qss" );
  QFile fileout( autostylesheet );

  QFileInfo variableInfo( variablesfile );

  if ( variableInfo.exists() && variablesfile.open( QIODevice::ReadOnly ) )
  {
    if ( !file.open( QIODevice::ReadOnly ) || !fileout.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      return;
    }

    QHash<QString, QString> variables;
    QString styledata = file.readAll();
    QTextStream in( &variablesfile );
    while ( !in.atEnd() )
    {
      QString line = in.readLine();
      // This is is a variable
      if ( line.startsWith( '@' ) )
      {
        int index = line.indexOf( ':' );
        QString name = line.mid( 0, index );
        QString value = line.mid( index + 1, line.length() );
        styledata.replace( name, value );
      }
    }
    variablesfile.close();
    QTextStream out( &fileout );
    out << styledata;
    fileout.close();
    file.close();
    stylesheetname = autostylesheet;
  }

  QString styleSheet = QLatin1String( "file:///" );
  styleSheet.append( stylesheetname );
  qApp->setStyleSheet( styleSheet );
  setThemeName( themename );
}

QHash<QString, QString> QgsApplication::uiThemes()
{
  QStringList paths = QStringList() << userThemesFolder();
  QHash<QString, QString> mapping;
  mapping.insert( "default", "" );
  Q_FOREACH ( const QString& path, paths )
  {
    QDir folder( path );
    QFileInfoList styleFiles = folder.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    Q_FOREACH ( const QFileInfo& info, styleFiles )
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

/*!
  Returns the path to the authors file.
*/
QString QgsApplication::authorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/AUTHORS" );
}
/*!
  Returns the path to the contributors file.
*/
QString QgsApplication::contributorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/CONTRIBUTORS" );
}
QString QgsApplication::developersMapFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/developersmap.html" );
}

/*!
  Returns the path to the sponsors file.
*/
QString QgsApplication::sponsorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/SPONSORS" );
}

/*!
  Returns the path to the donors file.
*/
QString QgsApplication::donorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/DONORS" );
}

/** Returns the path to the sponsors file. */
QString QgsApplication::translatorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/TRANSLATORS" );
}

/** Returns the path to the licence file. */
QString QgsApplication::licenceFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/doc/LICENSE" );
}

/*!
  Returns the path to the help application.
*/
QString QgsApplication::helpAppPath()
{
  QString helpAppPath;
#ifdef Q_OS_MACX
  helpAppPath = applicationDirPath() + "/bin/qgis_help.app/Contents/MacOS";
#else
  helpAppPath = libexecPath();
#endif
  helpAppPath += "/qgis_help";
#ifdef Q_OS_WIN
  helpAppPath += ".exe";
#endif
  return helpAppPath;
}
/*!
  Returns the path to the translation directory.
*/
QString QgsApplication::i18nPath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
    return ABISYM( mBuildOutputPath ) + QLatin1String( "/i18n" );
  else
    return ABISYM( mPkgDataPath ) + QLatin1String( "/i18n/" );
}

/*!
  Returns the path to the master qgis.db file.
*/
QString QgsApplication::qgisMasterDbFilePath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/resources/qgis.db" );
}

/*!
  Returns the path to the settings directory in user's home dir
 */
QString QgsApplication::qgisSettingsDirPath()
{
  return ABISYM( mConfigPath );
}

/*!
  Returns the path to the user qgis.db file.
*/
QString QgsApplication::qgisUserDbFilePath()
{
  return qgisSettingsDirPath() + QLatin1String( "qgis.db" );
}

/*!
  Returns the path to the user authentication database file: qgis-auth.db.
*/
QString QgsApplication::qgisAuthDbFilePath()
{
  return ABISYM( mAuthDbDirPath ) + QLatin1String( "qgis-auth.db" );
}

/*!
  Returns the path to the splash screen image directory.
*/
QString QgsApplication::splashPath()
{
  return QString( ":/images/splash/" );
}

/*!
  Returns the path to the icons image directory.
*/
QString QgsApplication::iconsPath()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/images/icons/" );
}
/*!
  Returns the path to the srs.db file.
*/
QString QgsApplication::srsDbFilePath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
  {
    QString tempCopy = QDir::tempPath() + "/srs.db";

    if ( !QFile( tempCopy ).exists() )
    {
      QFile f( ABISYM( mPkgDataPath ) + "/resources/srs.db" );
      if ( !f.copy( tempCopy ) )
      {
        qFatal( "Could not create temporary copy" );
      }
    }

    return tempCopy;
  }
  else
  {
    return ABISYM( mPkgDataPath ) + QLatin1String( "/resources/srs.db" );
  }
}

/*!
  Returns the paths to the svg directories.
*/
QStringList QgsApplication::svgPaths()
{
  //local directories to search when looking for an SVG with a given basename
  //defined by user in options dialog
  QSettings settings;
  QStringList myPathList;
  QString myPaths = settings.value( "svg/searchPathsForSVG", QString() ).toString();
  if ( !myPaths.isEmpty() )
  {
    myPathList = myPaths.split( '|' );
  }

  myPathList << ABISYM( mDefaultSvgPaths );
  return myPathList;
}

/*!
  Returns the paths to the composer template directories.
*/
QStringList QgsApplication::composerTemplatePaths()
{
  //local directories to search when looking for an SVG with a given basename
  //defined by user in options dialog
  QSettings settings;
  QStringList myPathList;
  QString myPaths = settings.value( "composer/searchPathsForTemplates", QString() ).toString();
  if ( !myPaths.isEmpty() )
  {
    myPathList = myPaths.split( '|' );
  }

  return myPathList;
}

QString QgsApplication::userStyleV2Path()
{
  return qgisSettingsDirPath() + QLatin1String( "symbology-ng-style.db" );
}

QRegExp QgsApplication::shortNameRegExp()
{
  return QRegExp( "^[A-Za-z][A-Za-z0-9\\._-]*" );
}

QString QgsApplication::userLoginName()
{
  if ( !sUserName.isEmpty() )
    return sUserName;

#ifdef Q_OS_WIN
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  if ( GetUserName(( TCHAR* )name, &size ) )
  {
    sUserName = QString( name );
  }

#else
  QProcess process;

  process.start( "whoami" );
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

#ifdef Q_OS_WIN
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  //note - this only works for accounts connected to domain
  if ( GetUserNameEx( NameDisplay, ( TCHAR* )name, &size ) )
  {
    sUserFullName = QString( name );
  }

  //fall back to login name
  if ( sUserFullName.isEmpty() )
    sUserFullName = userLoginName();
#elif defined(Q_OS_ANDROID)
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
  return QLatin1String( "linux" );
#else
  return QLatin1String( "unknown" );
#endif
}

QString QgsApplication::platform()
{
  return sPlatformName;
}

QString QgsApplication::userThemesFolder()
{
  return qgisSettingsDirPath() + QLatin1String( "/themes" );
}

QString QgsApplication::defaultStyleV2Path()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/resources/symbology-ng-style.db" );
}

QString QgsApplication::defaultThemesFolder()
{
  return ABISYM( mPkgDataPath ) + QLatin1String( "/resources/themes" );
}

QString QgsApplication::libraryPath()
{
  return ABISYM( mLibraryPath );
}

QString QgsApplication::libexecPath()
{
  return ABISYM( mLibexecPath );
}

QgsApplication::endian_t QgsApplication::endian()
{
  return ( htonl( 1 ) == 1 ) ? XDR : NDR;
}

void QgsApplication::initQgis()
{
  // set the provider plugin path (this creates provider registry)
  QgsProviderRegistry::instance( pluginPath() );

  // create map layer registry if doesn't exist
  QgsMapLayerRegistry::instance();

  // initialize authentication manager and connect to database
  QgsAuthManager::instance()->init( pluginPath() );
}

void QgsApplication::exitQgis()
{
  delete QgsAuthManager::instance();

  //Ensure that all remaining deleteLater QObjects are actually deleted before we exit.
  //This isn't strictly necessary (since we're exiting anyway) but doing so prevents a lot of
  //LeakSanitiser noise which hides real issues
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

  delete QgsProviderRegistry::instance();

  //delete all registered functions from expression engine (see above comment)
  QgsExpression::cleanRegisteredFunctions();

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
                          qgisMasterDbFilePath() )
                    .arg( qgisAuthDbFilePath() );
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
  myStyle = "p.glossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "  stop: 0 " + myColor1.name()  + ","
            "  stop: 0.1 " + myColor2.name() + ","
            "  stop: 0.5 " + myColor1.name()  + ","
            "  stop: 0.9 " + myColor2.name() + ","
            "  stop: 1 " + myColor1.name() + ");"
            "  color: black;"
            "  padding-left: 4px;"
            "  padding-top: 20px;"
            "  padding-bottom: 8px;"
            "  border: 1px solid #6c6c6c;"
            "}"
            "p.subheaderglossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "  stop: 0 " + myColor1.name()  + ","
            "  stop: 0.1 " + myColor2.name() + ","
            "  stop: 0.5 " + myColor1.name()  + ","
            "  stop: 0.9 " + myColor2.name() + ","
            "  stop: 1 " + myColor1.name() + ");"
            "  font-weight: bold;"
            "  font-size: medium;"
            "  line-height: 1.1em;"
            "  width: 100%;"
            "  color: black;"
            "  padding-left: 4px;"
            "  padding-right: 4px;"
            "  padding-top: 20px;"
            "  padding-bottom: 8px;"
            "  border: 1px solid #6c6c6c;"
            "}"
            "th.glossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "  stop: 0 " + myColor1.name()  + ","
            "  stop: 0.1 " + myColor2.name() + ","
            "  stop: 0.5 " + myColor1.name()  + ","
            "  stop: 0.9 " + myColor2.name() + ","
            "  stop: 1 " + myColor1.name() + ");"
            "  color: black;"
            "  border: 1px solid #6c6c6c;"
            "}"
            ".overview{ font: 1.82em; font-weight: bold;}"
            "body{  background: white;"
            "  color: black;"
            "  font-family: arial,sans-serif;"
            "}"
            "h1{  background-color: #F6F6F6;"
            "  color: #8FB171; "
            "  font-size: x-large;  "
            "  font-weight: normal;"
            "  font-family: luxi serif, georgia, times new roman, times, serif;"
            "  background: none;"
            "  padding: 0.75em 0 0;"
            "  margin: 0;"
            "  line-height: 3em;"
            "}"
            "h2{  background-color: #F6F6F6;"
            "  color: #8FB171; "
            "  font-size: medium;  "
            "  font-weight: normal;"
            "  font-family: luxi serif, georgia, times new roman, times, serif;"
            "  background: none;"
            "  padding: 0.75em 0 0;"
            "  margin: 0;"
            "  line-height: 1.1em;"
            "}"
            "h3{  background-color: #F6F6F6;"
            "  color: #729FCF;"
            "  font-family: luxi serif, georgia, times new roman, times, serif;"
            "  font-weight: bold;"
            "  font-size: large;"
            "  text-align: right;"
            "  border-bottom: 5px solid #DCEB5C;"
            "}"
            "h4{  background-color: #F6F6F6;"
            "  color: #729FCF;"
            "  font-family: luxi serif, georgia, times new roman, times, serif;"
            "  font-weight: bold;"
            "  font-size: medium;"
            "  text-align: right;"
            "}"
            "h5{    background-color: #F6F6F6;"
            "   color: #729FCF;"
            "   font-family: luxi serif, georgia, times new roman, times, serif;"
            "   font-weight: bold;"
            "   font-size: small;"
            "   text-align: right;"
            "}"
            "a{  color: #729FCF;"
            "  font-family: arial,sans-serif;"
            "  font-size: small;"
            "}"
            "label{  background-color: #FFFFCC;"
            "  border: 1px solid black;"
            "  margin: 1px;"
            "  padding: 0px 3px; "
            "  font-size: small;"
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

QString QgsApplication::absolutePathToRelativePath( const QString& aPath, const QString& targetPath )
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

  targetElems.removeAll( "." );
  aPathElems.removeAll( "." );

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
      aPathElems.insert( 0, ".." );
    }
  }
  else
  {
    // let it start with . nevertheless,
    // so relative path always start with either ./ or ../
    aPathElems.insert( 0, "." );
  }

  return aPathElems.join( "/" );
}

QString QgsApplication::relativePathToAbsolutePath( const QString& rpath, const QString& targetPath )
{
  // relative path should always start with ./ or ../
  if ( !rpath.startsWith( "./" ) && !rpath.startsWith( "../" ) )
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
  targetElems.removeAll( "." );

  // resolve ..
  int pos;
  while (( pos = targetElems.indexOf( ".." ) ) > 0 )
  {
    // remove preceding element and ..
    targetElems.removeAt( pos - 1 );
    targetElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  targetElems.prepend( "" );
#endif

  return targetElems.join( "/" );
}

void QgsApplication::skipGdalDriver( const QString& theDriver )
{
  if ( ABISYM( mGdalSkipList ).contains( theDriver ) || theDriver.isEmpty() )
  {
    return;
  }
  ABISYM( mGdalSkipList ) << theDriver;
  applyGdalSkippedDrivers();
}

void QgsApplication::restoreGdalDriver( const QString& theDriver )
{
  if ( !ABISYM( mGdalSkipList ).contains( theDriver ) )
  {
    return;
  }
  int myPos = ABISYM( mGdalSkipList ).indexOf( theDriver );
  if ( myPos >= 0 )
  {
    ABISYM( mGdalSkipList ).removeAt( myPos );
  }
  applyGdalSkippedDrivers();
}

void QgsApplication::applyGdalSkippedDrivers()
{
  ABISYM( mGdalSkipList ).removeDuplicates();
  QString myDriverList = ABISYM( mGdalSkipList ).join( " " );
  QgsDebugMsg( "Gdal Skipped driver list set to:" );
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

  copyPath( defaultThemesFolder(), userThemesFolder() );
  return true;
}

void QgsApplication::copyPath( const QString& src, const QString& dst )
{
  QDir dir( src );
  if ( ! dir.exists() )
    return;

  Q_FOREACH ( const QString& d, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    QString dst_path = dst + QDir::separator() + d;
    dir.mkpath( dst_path );
    copyPath( src + QDir::separator() + d, dst_path );
  }

  Q_FOREACH ( const QString& f, dir.entryList( QDir::Files ) )
  {
    QFile::copy( src + QDir::separator() + f, dst + QDir::separator() + f );
  }
}

bool QgsApplication::createDB( QString *errorMessage )
{
  // set a working directory up for gdal to write .aux.xml files into
  // for cases where the raster dir is read only to the user
  // if the env var is already set it will be used preferentially
  QString myPamPath = qgisSettingsDirPath() + QLatin1String( "gdal_pam/" );
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
  QFile qgisPrivateDbFile( QgsApplication::qgisUserDbFilePath() );

  // first we look for ~/.qgis/qgis.db
  if ( !qgisPrivateDbFile.exists() )
  {
    // if it doesnt exist we copy it in from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDbFilePath();
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
  }
  else
  {
    // migrate if necessary
    sqlite3 *db;
    if ( sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().constData(), &db ) != SQLITE_OK )
    {
      if ( errorMessage )
      {
        *errorMessage = tr( "Could not open qgis.db" );
      }
      return false;
    }

    char *errmsg;
    int res = sqlite3_exec( db, "SELECT epsg FROM tbl_srs LIMIT 0", nullptr, nullptr, &errmsg );
    if ( res == SQLITE_OK )
    {
      // epsg column exists => need migration
      if ( sqlite3_exec( db,
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
        sqlite3_close( db );
        return false;
      }
    }
    else
    {
      sqlite3_free( errmsg );
    }

    if ( sqlite3_exec( db, "DROP VIEW vw_srs", nullptr, nullptr, &errmsg ) != SQLITE_OK )
    {
      QgsDebugMsg( QString( "vw_srs didn't exists in private qgis.db: %1" ).arg( errmsg ) );
    }

    if ( sqlite3_exec( db,
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
      sqlite3_close( db );
      return false;
    }

    sqlite3_close( db );
  }
  return true;
}

void QgsApplication::setMaxThreads( int maxThreads )
{
  QgsDebugMsg( QString( "maxThreads: %1" ).arg( maxThreads ) );

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
  QgsDebugMsg( QString( "set QThreadPool max thread count to %1" ).arg( QThreadPool::globalInstance()->maxThreadCount() ) );
}

