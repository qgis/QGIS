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

QString QgsApplication::mPkgDataPath;

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
  mPkgDataPath = QCoreApplication::applicationDirPath() + QString("/share/qgis");
#else
  mPkgDataPath = PKGDATAPATH;
#endif
}

QgsApplication::~QgsApplication()
{}

/*!
  Returns the path to the translation files.
*/
const QString QgsApplication::i18nPath()
{
  return mPkgDataPath + QString("/i18n/");
}

/*!
  Returns the path to the splash screen images.
*/
const QString QgsApplication::splashPath()
{
  return mPkgDataPath + QString("/images/splash/");
}

/*!
  Returns the path to the theme images.
*/
const QString QgsApplication::themePath()
{
  return mPkgDataPath + QString("/themes/");
}
