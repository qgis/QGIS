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

#include <qgsconfig.h>
#include <qgslogger.h>

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

QString QgsApplication::mPrefixPath;
QString QgsApplication::mPluginPath;
QString QgsApplication::mPkgDataPath;
QString QgsApplication::mThemePath;

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
QgsApplication::QgsApplication(int & argc, char ** argv, bool GUIenabled)
: QApplication(argc, argv, GUIenabled)
{
  QgsDebugMsg("\n**********************************");
  QgsDebugMsg("\nInitialising QgsApplication...");
#if defined(Q_WS_MACX) || defined(Q_WS_WIN32) || defined(WIN32)
  setPrefixPath(applicationDirPath(), true);
#else
  QDir myDir(applicationDirPath());
  myDir.cdUp();
  QString myPrefix = myDir.absolutePath();
  QgsDebugMsg("Prefix: " +  myPrefix.toLocal8Bit());
  setPrefixPath(myPrefix, true);
#endif
  QgsDebugMsg("\nPlugin Path:" + mPluginPath);
  QgsDebugMsg("\nPkgData Path:" + mPkgDataPath);
  QgsDebugMsg("\nTheme Path:" + mThemePath);
  QgsDebugMsg("\n**********************************\n");
}

QgsApplication::~QgsApplication()
{}

void QgsApplication::setPrefixPath(const QString thePrefixPath, bool useDefaultPaths)
{
  mPrefixPath = thePrefixPath;
  if (useDefaultPaths)
  {
    setPluginPath(mPrefixPath + QDir::separator() + QString(QGIS_PLUGIN_SUBDIR));
    setPkgDataPath(mPrefixPath + QDir::separator() + QString(QGIS_DATA_SUBDIR));
  }
}

void QgsApplication::setPluginPath(const QString thePluginPath)
{
  mPluginPath = thePluginPath;
  QgsDebugMsg("\n\n\n\n\n +++++++++++++++++++++++\n plugin path changed\n" + mPluginPath + "\n +++++++++++++++++ \n\n\n\n");
}

void QgsApplication::setPkgDataPath(const QString thePkgDataPath)
{
  mPkgDataPath = thePkgDataPath;
  mThemePath = mPkgDataPath + QString("/themes/default/");
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
const QString QgsApplication::themePath() 
{ 
  return mThemePath; 
}

/*!
  Set the theme path to the specified theme.
*/
void QgsApplication::selectTheme(const QString theThemeName)
{
  mThemePath = mPkgDataPath + QString("/themes/") + theThemeName + QString("/");
}

/*!
  Returns the path to the authors file.
*/
const QString QgsApplication::authorsFilePath()
{
  return mPkgDataPath + QString("/doc/AUTHORS");
}
/*!
  Returns the path to the sponsors file.
*/
const QString QgsApplication::sponsorsFilePath()
{
  return mPkgDataPath + QString("/doc/SPONSORS");
}
/*!
  Returns the path to the developer image directory.
*/
const QString QgsApplication::developerPath()
{
  return mPkgDataPath + QString("/images/developers/");
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
  return mPkgDataPath + QString("/i18n/");
}

/*!
  Returns the path to the master qgis.db file.
*/
const QString QgsApplication::qgisMasterDbFilePath()
{
  return mPkgDataPath + QString("/resources/qgis.db");
}

/*!
  Returns the path to the settings directory in user's home dir
 */
const QString QgsApplication::qgisSettingsDirPath()
{
  return QDir::homeDirPath() + QString("/.qgis/");
}

/*!
  Returns the path to the user qgis.db file.
*/
const QString QgsApplication::qgisUserDbFilePath()
{
  return qgisSettingsDirPath() + QString("qgis.db");
}

/*!
  Returns the path to the splash screen image directory.
*/
const QString QgsApplication::splashPath()
{
  return mPkgDataPath + QString("/images/splash/");
}

/*!
  Returns the path to the icons image directory.
*/
const QString QgsApplication::iconsPath()
{
  return mPkgDataPath + QString("/images/icons/");
}
/*!
  Returns the path to the srs.db file.
*/
const QString QgsApplication::srsDbFilePath()
{
  return mPkgDataPath + QString("/resources/srs.db");
}

/*!
  Returns the path to the svg directory.
*/
const QString QgsApplication::svgPath()
{
  return mPkgDataPath + QString("/svg/");
}

QgsApplication::endian_t QgsApplication::endian()
{
  return (htonl(1) == 1) ? XDR : NDR ;
}

void QgsApplication::initQgis()
{
  // set the provider plugin path (this creates provider registry)
  QgsProviderRegistry::instance(pluginPath());
  
  // create map layer registry if doesn't exist
  QgsMapLayerRegistry::instance();
}

void QgsApplication::exitQgis()
{
  delete QgsMapLayerRegistry::instance();
  delete QgsProviderRegistry::instance();
}

QString QgsApplication::reportStyleSheet()
{
  QString myStyle;
  myStyle = ".glossy{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #616161, stop: 0.5 #505050, stop: 0.6 #434343, stop:1 #656565);"
    "color: white;"
    "padding-left: 4px;"
    "padding-top: 20px;"
    "padding-bottom: 8px;"
    "border: 1px solid #6c6c6c;"
    "}"
  ".glossyBlue{ background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3087d3, stop: 0.5 #3794e2, stop: 0.6 #43a6f9, stop:1 #2f87d1);"
    "color: white;"
    "padding-left: 4px;"
    "padding-top: 20px;"
    "padding-bottom: 8px;"
    "border: 1px solid #44a7fb;"
    "}"
  "h1 {font-size : 22pt; }"
  "h2 {font-size : 18pt; }"
  "h3 {font-size : 14pt; }"
  ".glossyh3{ "
    "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #616161, stop: 0.5 #505050, stop: 0.6 #434343, stop:1 #656565);"
    "color: white; "
    "padding-left: 4px; "
    "padding-top: 20px;"
    "padding-bottom: 8px;  "
    "border: 1px solid #6c6c6c; }"
  ".headerCell, th {color:#466aa5; "
    "font-size : 12pt; "
    "font-weight: bold; "
    "width: 100%;"
    "align: left;"
    "}"
  ".parameterHeader {font-weight: bold;}"
  ".largeCell {color:#000000; font-size : 12pt;}"
  ".alternateCell {font-weight: bold;}"
  ".rocTable "
  "{"
  "  border-width: 1px 1px 1px 1px;"
  "  border-spacing: 2px;"
  "  border-style: solid solid solid solid;" //unsupported
  "  border-color: black black black black;" //unsupported
  "  border-collapse: separate;"
  "  background-color: white;"
  "}";
  return myStyle;
}
