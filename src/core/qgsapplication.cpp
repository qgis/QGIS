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
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"
#include "qgsexception.h"
#include "qgsgeometry.h"

#include <QDir>
#include <QFile>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QPalette>
#include <QSettings>
#include <QIcon>
#include <QPixmap>

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

#include "qgsconfig.h"

#include <gdal.h>
#include <ogr_api.h>
#include <cpl_conv.h> // for setting gdal options

QObject * ABISYM( QgsApplication::mFileOpenEventReceiver );
QStringList ABISYM( QgsApplication::mFileOpenEventList );
QString ABISYM( QgsApplication::mPrefixPath );
QString ABISYM( QgsApplication::mPluginPath );
QString ABISYM( QgsApplication::mPkgDataPath );
QString ABISYM( QgsApplication::mLibraryPath );
QString ABISYM( QgsApplication::mLibexecPath );
QString ABISYM( QgsApplication::mThemeName );
QStringList ABISYM( QgsApplication::mDefaultSvgPaths );
QString ABISYM( QgsApplication::mConfigPath );
bool ABISYM( QgsApplication::mRunningFromBuildDir ) = false;
QString ABISYM( QgsApplication::mBuildSourcePath );
#ifdef _MSC_VER
QString ABISYM( QgsApplication::mCfgIntDir );
#endif
QString ABISYM( QgsApplication::mBuildOutputPath );
QStringList ABISYM( QgsApplication::mGdalSkipList );

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
QgsApplication::QgsApplication( int & argc, char ** argv, bool GUIenabled, QString customConfigPath )
    : QApplication( argc, argv, GUIenabled )
{
  init( customConfigPath ); // init can also be called directly by e.g. unit tests that don't inherit QApplication.
}
void QgsApplication::init( QString customConfigPath )
{
  if ( customConfigPath.isEmpty() )
  {
    customConfigPath = QDir::homePath() + QString( "/.qgis/" );
  }
  qRegisterMetaType<QgsGeometry::Error>( "QgsGeometry::Error" );

  // check if QGIS is run from build directory (not the install directory)
  QDir appDir( applicationDirPath() );
#ifndef _MSC_VER
#define SOURCE_PATH "source_path.txt"
#else
#define SOURCE_PATH "../source_path.txt"
#endif
  if ( appDir.exists( SOURCE_PATH ) )
  {
    QFile f( applicationDirPath() + "/" + SOURCE_PATH );
    if ( f.open( QIODevice::ReadOnly ) )
    {
      ABISYM( mRunningFromBuildDir ) = true;
      ABISYM( mBuildSourcePath ) = f.readAll();
#if _MSC_VER
      QStringList elems = applicationDirPath().split( "/", QString::SkipEmptyParts );
      ABISYM( mCfgIntDir ) = elems.last();
      ABISYM( mBuildOutputPath ) = applicationDirPath() + "/../..";
#elif defined(Q_WS_MACX)
      ABISYM( mBuildOutputPath ) = applicationDirPath();
#else
      ABISYM( mBuildOutputPath ) = applicationDirPath() + "/.."; // on linux
#endif
      qDebug( "Running from build directory!" );
      qDebug( "- source directory: %s", ABISYM( mBuildSourcePath ).toAscii().data() );
      qDebug( "- output directory of the build: %s", ABISYM( mBuildOutputPath ).toAscii().data() );
    }
  }

  if ( ABISYM( mRunningFromBuildDir ) )
  {
    // we run from source directory - not installed to destination (specified prefix)
    ABISYM( mPrefixPath ) = QString(); // set invalid path
#ifdef _MSC_VER
    setPluginPath( ABISYM( mBuildOutputPath ) + "/" + QString( QGIS_PLUGIN_SUBDIR ) + "/" + ABISYM( mCfgIntDir ) );
#else
    setPluginPath( ABISYM( mBuildOutputPath ) + "/" + QString( QGIS_PLUGIN_SUBDIR ) );
#endif
    setPkgDataPath( ABISYM( mBuildSourcePath ) ); // directly source path - used for: doc, resources, svg
    ABISYM( mLibraryPath ) = ABISYM( mBuildOutputPath ) + "/" + QGIS_LIB_SUBDIR + "/";
    ABISYM( mLibexecPath ) = ABISYM( mBuildOutputPath ) + "/" + QGIS_LIBEXEC_SUBDIR + "/";
  }
  else
  {
    char *prefixPath = getenv( "QGIS_PREFIX_PATH" );
    if ( !prefixPath )
    {
#if defined(Q_WS_MACX) || defined(Q_WS_WIN32) || defined(WIN32)
      setPrefixPath( applicationDirPath(), true );
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
    ABISYM( mConfigPath ) = customConfigPath + "/"; // make sure trailing slash is included
  }

  ABISYM( mDefaultSvgPaths ) << qgisSettingsDirPath() + QString( "svg/" );

  // set a working directory up for gdal to write .aux.xml files into
  // for cases where the raster dir is read only to the user
  // if the env var is already set it will be used preferentially
  QString myPamPath = qgisSettingsDirPath() + QString( "gdal_pam/" );
  QDir myDir( myPamPath );
  if ( !myDir.exists() )
  {
    myDir.mkpath( myPamPath ); //fail silently
  }


#if defined(Q_WS_WIN32) || defined(WIN32)
  CPLSetConfigOption( "GDAL_PAM_PROXY_DIR", myPamPath.toUtf8() );
#else
  //under other OS's we use an environment var so the user can
  //override the path if he likes
  int myChangeFlag = 0; //whether we want to force the env var to change
  setenv( "GDAL_PAM_PROXY_DIR", myPamPath.toUtf8(), myChangeFlag );
#endif
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
    QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( std::exception & e )
  {
    QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  catch ( ... )
  {
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

void QgsApplication::setPrefixPath( const QString thePrefixPath, bool useDefaultPaths )
{
  ABISYM( mPrefixPath ) = thePrefixPath;
#if defined(_MSC_VER)
  if ( ABISYM( mPrefixPath ).endsWith( "/bin" ) )
  {
    ABISYM( mPrefixPath ).chop( 4 );
  }
#endif
  if ( useDefaultPaths )
  {
    setPluginPath( ABISYM( mPrefixPath ) + "/" + QString( QGIS_PLUGIN_SUBDIR ) );
    setPkgDataPath( ABISYM( mPrefixPath ) + "/" + QString( QGIS_DATA_SUBDIR ) );
  }
  ABISYM( mLibraryPath ) = ABISYM( mPrefixPath ) + "/" + QGIS_LIB_SUBDIR + "/";
  ABISYM( mLibexecPath ) = ABISYM( mPrefixPath ) + "/" + QGIS_LIBEXEC_SUBDIR + "/";
}

void QgsApplication::setPluginPath( const QString thePluginPath )
{
  ABISYM( mPluginPath ) = thePluginPath;
}

void QgsApplication::setPkgDataPath( const QString thePkgDataPath )
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

const QString QgsApplication::prefixPath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
  {
    qWarning( "!!! prefix path was requested, but it is not valid - we do not run from installed path !!!" );
  }

  return ABISYM( mPrefixPath );
}
const QString QgsApplication::pluginPath()
{
  return ABISYM( mPluginPath );
}
const QString QgsApplication::pkgDataPath()
{
  return ABISYM( mPkgDataPath );
}
const QString QgsApplication::defaultThemePath()
{
  return ":/images/themes/default/";
}
const QString QgsApplication::activeThemePath()
{
  return ":/images/themes/" + themeName() + "/";
}


QString QgsApplication::iconPath( QString iconFile )
{
  // try active theme
  QString path = activeThemePath();
  if ( QFile::exists( path + iconFile ) )
    return path + iconFile;

  // use default theme
  return defaultThemePath() + iconFile;
}

QIcon QgsApplication::getThemeIcon( const QString theName )
{
  QString myPreferredPath = activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = defaultThemePath() + QDir::separator() + theName;
  if ( QFile::exists( myPreferredPath ) )
  {
    return QIcon( myPreferredPath );
  }
  else if ( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( myDefaultPath );
  }
  else
  {
    return QIcon();
  }
}

// TODO: add some caching mechanism ?
QPixmap QgsApplication::getThemePixmap( const QString theName )
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
void QgsApplication::setThemeName( const QString theThemeName )
{
  QString myPath = ":/images/themes/" + theThemeName + "/";
  //check it exists and if not roll back to default theme
  if ( QFile::exists( myPath ) )
  {
    ABISYM( mThemeName ) = theThemeName;
  }
  else
  {
    ABISYM( mThemeName ) = "default";
  }
}
/*!
 * Get the active theme name
 */
const QString QgsApplication::themeName()
{
  return ABISYM( mThemeName );
}
/*!
  Returns the path to the authors file.
*/
const QString QgsApplication::authorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QString( "/doc/AUTHORS" );
}
/*!
  Returns the path to the contributors file.
*/
const QString QgsApplication::contributorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QString( "/doc/CONTRIBUTORS" );
}
/*!
  Returns the path to the sponsors file.
*/
const QString QgsApplication::sponsorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QString( "/doc/SPONSORS" );
}

/*!
  Returns the path to the donors file.
*/
const QString QgsApplication::donorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QString( "/doc/DONORS" );
}

/*!
  Returns the path to the sponsors file.
  @note Added in QGIS 1.1
*/
const QString QgsApplication::translatorsFilePath()
{
  return ABISYM( mPkgDataPath ) + QString( "/doc/TRANSLATORS" );
}

const QString QgsApplication::developerPath()
{
  return QString(); // developer images are no longer shipped!
}

/*!
  Returns the path to the help application.
*/
const QString QgsApplication::helpAppPath()
{
  QString helpAppPath;
#ifdef Q_OS_MACX
  helpAppPath = applicationDirPath() + "/bin/qgis_help.app/Contents/MacOS";
#else
  helpAppPath = libexecPath();
#endif
  helpAppPath += "/qgis_help";
  return helpAppPath;
}
/*!
  Returns the path to the translation directory.
*/
const QString QgsApplication::i18nPath()
{
  if ( ABISYM( mRunningFromBuildDir ) )
    return ABISYM( mBuildOutputPath ) + QString( "/i18n" );
  else
    return ABISYM( mPkgDataPath ) + QString( "/i18n/" );
}

/*!
  Returns the path to the master qgis.db file.
*/
const QString QgsApplication::qgisMasterDbFilePath()
{
  return ABISYM( mPkgDataPath ) + QString( "/resources/qgis.db" );
}

/*!
  Returns the path to the settings directory in user's home dir
 */
const QString QgsApplication::qgisSettingsDirPath()
{
  return ABISYM( mConfigPath );
}

/*!
  Returns the path to the user qgis.db file.
*/
const QString QgsApplication::qgisUserDbFilePath()
{
  return qgisSettingsDirPath() + QString( "qgis.db" );
}

/*!
  Returns the path to the splash screen image directory.
*/
const QString QgsApplication::splashPath()
{
  return QString( ":/images/splash/" );
}

/*!
  Returns the path to the icons image directory.
*/
const QString QgsApplication::iconsPath()
{
  return ABISYM( mPkgDataPath ) + QString( "/images/icons/" );
}
/*!
  Returns the path to the srs.db file.
*/
const QString QgsApplication::srsDbFilePath()
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
    return ABISYM( mPkgDataPath ) + QString( "/resources/srs.db" );
  }
}

/*!
  Returns the paths to the svg directories.
*/
const QStringList QgsApplication::svgPaths()
{
  //local directories to search when looking for an SVG with a given basename
  //defined by user in options dialog
  QSettings settings;
  QStringList myPathList;
  QString myPaths = settings.value( "svg/searchPathsForSVG", "" ).toString();
  if ( !myPaths.isEmpty() )
  {
    myPathList = myPaths.split( "|" );
  }

  myPathList << ABISYM( mDefaultSvgPaths );
  return myPathList;
}

/*!
  Returns the path to the applications svg directories.
*/
const QString QgsApplication::svgPath()
{
  QString svgSubDir( ABISYM( mRunningFromBuildDir ) ? "/images/svg/" : "/svg/" );
  return ABISYM( mPkgDataPath ) + svgSubDir;
}

const QString QgsApplication::userStyleV2Path()
{
  return qgisSettingsDirPath() + QString( "symbology-ng-style.xml" );
}

const QString QgsApplication::defaultStyleV2Path()
{
  return ABISYM( mPkgDataPath ) + QString( "/resources/symbology-ng-style.xml" );
}

const QString QgsApplication::libraryPath()
{
  return ABISYM( mLibraryPath );
}

const QString QgsApplication::libexecPath()
{
  return ABISYM( mLibexecPath );
}

QgsApplication::endian_t QgsApplication::endian()
{
  return ( htonl( 1 ) == 1 ) ? XDR : NDR ;
}

void QgsApplication::initQgis()
{
  // set the provider plugin path (this creates provider registry)
  QgsProviderRegistry::instance( pluginPath() );

  // create map layer registry if doesn't exist
  QgsMapLayerRegistry::instance();
}

void QgsApplication::exitQgis()
{
  delete QgsMapLayerRegistry::instance();
  delete QgsProviderRegistry::instance();
}

QString QgsApplication::showSettings()
{
  QString myState = tr( "Application state:\n"
                        "Prefix:\t\t%1\n"
                        "Plugin Path:\t\t%2\n"
                        "Package Data Path:\t%3\n"
                        "Active Theme Name:\t%4\n"
                        "Active Theme Path:\t%5\n"
                        "Default Theme Path:\t%6\n"
                        "SVG Search Paths:\t%7\n"
                        "User DB Path:\t%8\n" )
                    .arg( prefixPath() )
                    .arg( pluginPath() )
                    .arg( pkgDataPath() )
                    .arg( themeName() )
                    .arg( activeThemePath() )
                    .arg( defaultThemePath() )
                    .arg( svgPaths().join( tr( "\n\t\t", "match indentation of application state" ) ) )
                    .arg( qgisMasterDbFilePath() );
  return myState;
}

QString QgsApplication::reportStyleSheet()
{
  //
  // Make the style sheet desktop preferences aware by using qappliation
  // palette as a basis for colors where appropriate
  //
  QColor myColor1 = palette().highlight().color();
  QColor myColor2 = myColor1;
  myColor2 = myColor2.lighter( 110 ); //10% lighter
  QString myStyle;
  myStyle = "p.glossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop: 0 " + myColor1.name()  + ","
            "stop: 0.1 " + myColor2.name() + ","
            "stop: 0.5 " + myColor1.name()  + ","
            "stop: 0.9 " + myColor2.name() + ","
            "stop: 1 " + myColor1.name() + ");"
            "color: white;"
            "padding-left: 4px;"
            "padding-top: 20px;"
            "padding-bottom: 8px;"
            "border: 1px solid #6c6c6c;"
            "}"
            "th.glossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop: 0 " + myColor1.name()  + ","
            "stop: 0.1 " + myColor2.name() + ","
            "stop: 0.5 " + myColor1.name()  + ","
            "stop: 0.9 " + myColor2.name() + ","
            "stop: 1 " + myColor1.name() + ");"
            "color: white;"
            "border: 1px solid #6c6c6c;"
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

QString QgsApplication::absolutePathToRelativePath( QString aPath, QString targetPath )
{
#if defined( Q_OS_WIN )
  const Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  aPath.replace( "\\", "/" );
  if ( aPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    aPath = "\\\\" + aPath.mid( 2 );
  }

  targetPath.replace( "\\", "/" );
  if ( targetPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    targetPath = "\\\\" + targetPath.mid( 2 );
  }
#else
  const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

  QStringList targetElems = targetPath.split( "/", QString::SkipEmptyParts );
  QStringList aPathElems = aPath.split( "/", QString::SkipEmptyParts );

  targetElems.removeAll( "." );
  aPathElems.removeAll( "." );

  // remove common part
  int n = 0;
  while ( aPathElems.size() > 0 &&
          targetElems.size() > 0 &&
          aPathElems[0].compare( targetElems[0], cs ) == 0 )
  {
    aPathElems.removeFirst();
    targetElems.removeFirst();
    n++;
  }

  if ( n == 0 )
  {
    // no common parts; might not even be a file
    return aPath;
  }

  if ( targetElems.size() > 0 )
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

QString QgsApplication::relativePathToAbsolutePath( QString rpath, QString targetPath )
{
  // relative path should always start with ./ or ../
  if ( !rpath.startsWith( "./" ) && !rpath.startsWith( "../" ) )
  {
    return rpath;
  }

#if defined(Q_OS_WIN)
  rpath.replace( "\\", "/" );
  targetPath.replace( "\\", "/" );

  bool uncPath = targetPath.startsWith( "//" );
#endif

  QStringList srcElems = rpath.split( "/", QString::SkipEmptyParts );
  QStringList targetElems = targetPath.split( "/", QString::SkipEmptyParts );

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

void QgsApplication::skipGdalDriver( QString theDriver )
{
  if ( ABISYM( mGdalSkipList ).contains( theDriver ) || theDriver.isEmpty() )
  {
    return;
  }
  ABISYM( mGdalSkipList ) << theDriver;
  applyGdalSkippedDrivers();
}

void QgsApplication::restoreGdalDriver( QString theDriver )
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
