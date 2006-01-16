/***************************************************************************
    qgsapplication.cpp - Accessors for application-wide data
     --------------------------------------
    Date                 : 02-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsapplication.h"

#include <QDir>

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
#if defined(Q_WS_MACX) || defined(Q_WS_WIN32)
  mPrefixPath = applicationDirPath();
  mPluginPath = mPrefixPath + QString("/lib/qgis");
  mPkgDataPath = mPrefixPath + QString("/share/qgis");
#else
  mPrefixPath = PREFIX;
  mPluginPath = PLUGINPATH;
  mPkgDataPath = PKGDATAPATH;
#endif
  mThemePath = mPkgDataPath + QString("/themes/default/");
}

QgsApplication::~QgsApplication()
{}

/*!
  Set the theme path to the specified theme.
*/
void QgsApplication::selectTheme(const QString& theThemeName)
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
  Returns the path to the user qgis.db file.
*/
const QString QgsApplication::qgisUserDbFilePath()
{
  return QDir::homeDirPath() + QString("/.qgis/qgis.db");
}

/*!
  Returns the path to the splash screen image directory.
*/
const QString QgsApplication::splashPath()
{
  return mPkgDataPath + QString("/images/splash/");
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
