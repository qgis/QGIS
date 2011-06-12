
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
  if ( SQLITE_OK == rc )
  {
    // prepare the sql statement
    QString sql;
    QTextStream sqlStream( &sql );
    // use '17 g' format; SmartNotation is default
    sqlStream.setRealNumberPrecision( 17 );
    sqlStream << "insert into tbl_bookmarks values(null,'" <<
    // fix occurrences of single-quote
    mName.replace( '\'', "''" ) << "','" <<
    mProjectTitle.replace( '\'', "''" ) << "'," <<
    mViewExtent.xMinimum() << "," <<
    mViewExtent.yMinimum() << "," <<
    mViewExtent.xMaximum() << "," <<
    mViewExtent.yMaximum() << "," <<
    mSrid << ")";

    QgsDebugMsg( QString( "Storing bookmark using: %1" ).arg( sql ) );

    char * errmsg = 0;
    rc = sqlite3_exec( db, sql.toUtf8(), NULL, NULL, &errmsg );
    if ( rc != SQLITE_OK )
    {
      // XXX query failed -- warn the user some how
      QgsDebugMsg( QString( "Failed to store bookmark: %1" ).arg( errmsg ) );
      sqlite3_free( errmsg );
    }
    sqlite3_close( db );
  }
  else
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( db ) ) );

    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( rc == 0 );
  }
}
