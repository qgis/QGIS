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
/* $Id$ */

#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"

#include <QDir>
#include <QMessageBox>
#include <QPalette>
#include <QSettings>

#include "qgsconfig.h"

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <ogr_api.h>

QString QgsApplication::mPrefixPath;
QString QgsApplication::mPluginPath;
QString QgsApplication::mPkgDataPath;
QString QgsApplication::mThemeName;

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
QgsApplication::QgsApplication( int & argc, char ** argv, bool GUIenabled )
    : QApplication( argc, argv, GUIenabled )
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

QgsApplication::~QgsApplication()
{

}

bool QgsApplication::notify( QObject * receiver, QEvent * event )
{
  // Send event to receiver and catch unhandled exceptions
  bool done = true;
  try
  {
    done = QApplication::notify( receiver, event );
  }
  catch ( std::exception & e )
  {
    QMessageBox::critical( activeWindow(), tr( "Exception" ), e.what() );
  }
  return done;
}

void QgsApplication::setPrefixPath( const QString thePrefixPath, bool useDefaultPaths )
{
  mPrefixPath = thePrefixPath;
#if defined(_MSC_VER)
  if ( mPrefixPath.endsWith( "/bin" ) )
  {
    mPrefixPath.chop( 4 );
  }
#endif
  if ( useDefaultPaths )
  {
    setPluginPath( mPrefixPath + "/" + QString( QGIS_PLUGIN_SUBDIR ) );
    setPkgDataPath( mPrefixPath + "/" + QString( QGIS_DATA_SUBDIR ) );
  }
}

void QgsApplication::setPluginPath( const QString thePluginPath )
{
  mPluginPath = thePluginPath;
}

void QgsApplication::setPkgDataPath( const QString thePkgDataPath )
{
  mPkgDataPath = thePkgDataPath;
}

const QString QgsApplication::prefixPath()
{
  return mPrefixPath;
}
const QString QgsApplication::pluginPath()
{
  return mPluginPath;
}
const QString QgsApplication::pkgDataPath()
{
  return mPkgDataPath;
}
const QString QgsApplication::defaultThemePath()
{
  return mPkgDataPath + "/themes/default/";
}
const QString QgsApplication::activeThemePath()
{
  return mPkgDataPath + "/themes/" + mThemeName + "/";
}

/*!
  Set the theme path to the specified theme.
*/
void QgsApplication::setThemeName( const QString theThemeName )
{
  QString myPath = mPkgDataPath + "/themes/" + theThemeName + "/";
  //check it exists and if not roll back to default theme
  if ( QFile::exists( myPath ) )
  {
    mThemeName = theThemeName;
  }
  else
  {
    mThemeName = "default";
  }
}
/*!
 * Get the active theme name
 */
const QString QgsApplication::themeName()
{
  return mThemeName;
}
/*!
  Returns the path to the authors file.
*/
const QString QgsApplication::authorsFilePath()
{
  return mPkgDataPath + QString( "/doc/AUTHORS" );
}
/*!
  Returns the path to the contributors file.
*/
const QString QgsApplication::contributorsFilePath()
{
  return mPkgDataPath + QString( "/doc/CONTRIBUTORS" );
}
/*!
  Returns the path to the sponsors file.
*/
const QString QgsApplication::sponsorsFilePath()
{
  return mPkgDataPath + QString( "/doc/SPONSORS" );
}

/*!
  Returns the path to the donors file.
*/
const QString QgsApplication::donorsFilePath()
{
  return mPkgDataPath + QString( "/doc/DONORS" );
}

/*!
  Returns the path to the sponsors file.
  @note Added in QGIS 1.1
*/
const QString QgsApplication::translatorsFilePath()
{
  return mPkgDataPath + QString( "/doc/TRANSLATORS" );
}
/*!
  Returns the path to the developer image directory.
*/
const QString QgsApplication::developerPath()
{
  return mPkgDataPath + QString( "/images/developers/" );
}

/*!
  Returns the path to the help application.
*/
const QString QgsApplication::helpAppPath()
{
  QString helpAppPath = applicationDirPath();
#ifdef Q_OS_MACX
  helpAppPath += "/bin/qgis_help.app/Contents/MacOS";
#endif
  helpAppPath += "/qgis_help";
  return helpAppPath;
}
/*!
  Returns the path to the mapserverexport application.
*/
const QString QgsApplication::msexportAppPath()
{
  QString msexportAppPath = applicationDirPath();
#ifdef Q_OS_MACX
  msexportAppPath += "/bin/msexport.app/Contents/MacOS";
#endif
  msexportAppPath += "/msexport";
  return msexportAppPath;
}

/*!
  Returns the path to the translation directory.
*/
const QString QgsApplication::i18nPath()
{
  return mPkgDataPath + QString( "/i18n/" );
}

/*!
  Returns the path to the master qgis.db file.
*/
const QString QgsApplication::qgisMasterDbFilePath()
{
  return mPkgDataPath + QString( "/resources/qgis.db" );
}

/*!
  Returns the path to the settings directory in user's home dir
 */
const QString QgsApplication::qgisSettingsDirPath()
{
  return QDir::homePath() + QString( "/.qgis/" );
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
  return mPkgDataPath + QString( "/images/splash/" );
}

/*!
  Returns the path to the icons image directory.
*/
const QString QgsApplication::iconsPath()
{
  return mPkgDataPath + QString( "/images/icons/" );
}
/*!
  Returns the path to the srs.db file.
*/
const QString QgsApplication::srsDbFilePath()
{
  return mPkgDataPath + QString( "/resources/srs.db" );
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
  //additional default paths
  myPathList
    << mPkgDataPath + QString( "/svg/" )
    << qgisSettingsDirPath() + QString( "svg/" );
  return myPathList;

}

/*!
  Returns the path to the applications svg directories.
*/
const QString QgsApplication::svgPath()
{
  return mPkgDataPath + QString( "/svg/" );
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
  QString myState = QString( "Application state:\n"
                             "Prefix              : %1\n"
                             "Plugin Path         : %2\n"
                             "Package Data Path   : %3\n"
                             "Active Theme Name   : %4\n"
                             "Active Theme Path   : %5\n"
                             "Default Theme Path  : %6\n"
                             "SVG Search Paths    : %7\n"
                             "User DB Path        : %8\n" )
                    .arg( mPrefixPath )
                    .arg( mPluginPath )
                    .arg( mPkgDataPath )
                    .arg( themeName() )
                    .arg( activeThemePath() )
                    .arg( defaultThemePath() )
                    .arg( svgPaths().join( "\n" ) )
                    .arg( qgisMasterDbFilePath() );
  return myState;
}

QString QgsApplication::reportStyleSheet()
{
  //
  // Make the style sheet desktop preferences aware by using qappliation
  // palette as a basis for colours where appropriate
  //
  QColor myColor1 = palette().highlight().color();
  QColor myColor2 = myColor1;
  myColor2 = myColor2.lighter( 110 ); //10% lighter
  QString myStyle;
  myStyle = ".glossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
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
            "h1 {font-size : 22pt; }"
            "h2 {font-size : 18pt; }"
            "h3 {font-size : 14pt; }";
  return myStyle;
}

void QgsApplication::registerOgrDrivers()
{
  if ( 0 >= OGRGetDriverCount() )
  {
    OGRRegisterAll();
  }
}
