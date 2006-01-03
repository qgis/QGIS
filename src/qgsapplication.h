/***************************************************************************
    qgsapplication.h - Accessors for application-wide data
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
#ifndef QGSAPPLICATION_H
#define QGSAPPLICATION_H

#include <QApplication>

class QgsApplication: public QApplication
{
  public:
    QgsApplication(int & argc, char ** argv, bool GUIenabled);
    virtual ~QgsApplication();

    //! Returns the path to the translation files.
    static const QString i18nPath();

    //! Returns the path to the splash screen images.
    static const QString splashPath();

    //! Returns the path to the theme images.
    static const QString themePath();

    //! Returns the common root of all data path.
    static const QString& pkgDataPath() { return mPkgDataPath; }

  private:
    static QString mPkgDataPath;
};

#endif
