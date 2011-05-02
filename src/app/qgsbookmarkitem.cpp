
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
#include <sqlite3.h>
#include <QTextStream>

#include <cassert>

#include "qgsrectangle.h"
#include "qgsbookmarkitem.h"
#include "qgslogger.h"

QgsBookmarkItem::QgsBookmarkItem( QString name, QString projectTitle,
                                  QgsRectangle viewExtent, int srid, QString dbPath )
    : mName( name ), mProjectTitle( projectTitle ), mViewExtent( viewExtent ),
    mSrid( srid ), mUserDbPath( dbPath )
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
  int rc;
  QgsDebugMsg( QString( "Opening user database: %1" ).arg( mUserDbPath ) );
  rc = sqlite3_open( mUserDbPath.toUtf8().data(), &db );
  if ( rc )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( db ) ) );

    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( rc == 0 );
  }
  // prepare the sql statement
  const char *pzTail;
  sqlite3_stmt *ppStmt;
  QString sql;
  QTextStream sqlStream( &sql );
  sqlStream << "insert into tbl_bookmarks values(null,'" <<
  mName << "','" <<
  mProjectTitle << "'," <<
  mViewExtent.xMinimum() << "," <<
  mViewExtent.yMinimum() << "," <<
  mViewExtent.xMaximum() << "," <<
  mViewExtent.yMaximum() << "," <<
  mSrid << ")";

  QgsDebugMsg( QString( "Storing bookmark using: %1" ).arg( sql ) );

  QByteArray sqlData = sql.toUtf8();

  rc = sqlite3_prepare( db, sqlData.constData(), sqlData.size(), &ppStmt, &pzTail );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    // get the first row of the result set
    if ( sqlite3_step( ppStmt ) != SQLITE_DONE )
    {

      // XXX query failed -- warn the user some how
      QgsDebugMsg( QString( "Failed to store bookmark: %1" ).arg( sqlite3_errmsg( db ) ) );
    }
    // close the statement
    sqlite3_finalize( ppStmt );
    // close the database
    sqlite3_close( db );
  }


}
