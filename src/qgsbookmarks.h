/***************************************************************************
               QgsBookmarks.h  - Spatial Bookmarks
                             -------------------
    begin                : 2005-04-23
    copyright            : (C) 2005 Gary Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#ifndef QGSBOOKMARKS_H
#define QGSBOOKMARKS_H
#include "qgsbookmarksbase.uic.h"
class QString;
class QDir;
class QWidget;
class sqlite3;
class QgsBookmarks : public QgsBookmarksBase{
  Q_OBJECT
public:
 QgsBookmarks(QWidget *parent=0, const char *name=0);
 ~QgsBookmarks();
public slots:
 void deleteBookmark();
 void zoomToBookmark();
 int connectDb();
 void refreshBookmarks();

private:
 QWidget *mParent;
 bool makeDir(QDir &theQDir);
 void initialise();
 QString mUserDbPath;
 QString mQGisSettingsDir;
 sqlite3 *db;


};
#endif // QGSBOOKMARKS_H

