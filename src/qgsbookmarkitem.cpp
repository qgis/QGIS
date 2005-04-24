
/***************************************************************************
               QgsBookmarkItem.h  - Spatial Bookmark Item
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
#include <iostream>
#include <sqlite3.h>
#include <qstring.h>
#include <qtextstream.h>

#include <cassert>

#include "qgsrect.h"
#include "qgsbookmarkitem.h"

QgsBookmarkItem::QgsBookmarkItem(QString name, QString projectTitle, 
      QgsRect viewExtent, int srid, QString dbPath)
      : mName(name), mProjectTitle(projectTitle), mViewExtent(viewExtent),
      mSrid(srid), mUserDbPath(dbPath)
{
}
QgsBookmarkItem::~QgsBookmarkItem()
{
}
  void QgsBookmarkItem::store()
{
  // To store the bookmark we have to open the database and insert
  // the record using the parameters set in the constructor

  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
#ifdef QGISDEBUG 
  std::cout << "Opening user database: " << mUserDbPath << std::endl; 
#endif 
  rc = sqlite3_open(mUserDbPath, &db);
  if(rc)
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(db) << std::endl;

    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert(rc == 0);
  }
  // prepare the sql statement
  const char *pzTail;
  sqlite3_stmt *ppStmt;
  char *pzErrmsg;
  QString sql;
  QTextOStream sqlStream(&sql);
  sqlStream << "insert into tbl_bookmarks values(null,'" <<
    mName << "','" <<
    mProjectTitle << "'," <<
    mViewExtent.xMin() << "," <<
    mViewExtent.yMin() << "," <<
    mViewExtent.xMax() << "," <<
    mViewExtent.yMax() << "," <<
    mSrid << ")";

#ifdef QGISDEBUG 
  std::cout << "Storing bookmark using: " << sql << std::endl; 
#endif 
  rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
  // XXX Need to free memory from the error msg if one is set
  if(rc == SQLITE_OK)
  {
    // get the first row of the result set
    if(sqlite3_step(ppStmt) != SQLITE_DONE)
    {

      // XXX query failed -- warn the user some how
      std::cout << "Failed to store bookmark: " << sqlite3_errmsg(db) << std::endl; 
    }
    // close the statement
    sqlite3_finalize(ppStmt);
    // close the database
    sqlite3_close(db);
  }


}
