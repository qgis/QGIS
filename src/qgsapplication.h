/***************************************************************************
    qgsapplication.h - Accessors for application-wide data
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
#ifndef QGSAPPLICATION_H
#define QGSAPPLICATION_H

#include <QApplication>

class QgsApplication: public QApplication
{
  public:
    QgsApplication(int & argc, char ** argv, bool GUIenabled);
    virtual ~QgsApplication();

    //! Set the theme path to the specified theme.
    static void selectTheme(const QString& theThemeName);

    //! Returns the path to the authors file.
    static const QString authorsFilePath();

    //! Returns the path to the developer image directory.
    static const QString developerPath();

    //! Returns the path to the help application.
    static const QString helpAppPath();

    //! Returns the path to the translation directory.
    static const QString i18nPath();

    //! Returns the path to the master qgis.db file.
    static const QString qgisMasterDbFilePath();

    //! Returns the path to the splash screen image directory.
    static const QString splashPath();

    //! Returns the path to the srs.db file.
    static const QString srsDbFilePath();

    //! Returns the path to the svg directory.
    static const QString svgPath();

    //! Returns the path to the application prefix directory.
    static const QString& prefixPath() { return mPrefixPath; }

    //! Returns the path to the application plugin directory.
    static const QString& pluginPath() { return mPluginPath; }

    //! Returns the common root path of all application data directories.
    static const QString& pkgDataPath() { return mPkgDataPath; }

    //! Returns the path to the current theme directory.
    static const QString& themePath() { return mThemePath; }

  private:
    static QString mPrefixPath;
    static QString mPluginPath;
    static QString mPkgDataPath;
    static QString mThemePath;
};

#endif
