/***************************************************************************
                          qgscontexthelp.cpp
                    Display context help for a dialog
                             -------------------
    begin                : 2005-06-19
    copyright            : (C) 2005 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#include <qstring.h>
#include <qdir.h>
#include <qtextbrowser.h>
#include <sqlite3.h>
#include "qgscontexthelp.h"
#include <cassert>
QgsContextHelp::QgsContextHelp(QString &contextId, QWidget *parent, const char *name)
  : QgsContextHelpBase(parent, name)
{
  // get the help content from the database and display the document
  //
  QString qgisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  QString userDbPath = qgisSettingsDir + "qgis_help.db";
  int rc = connectDb(userDbPath);
  if(rc == SQLITE_OK)
  {
    sqlite3_stmt *ppStmt;
    const char *pzTail;
    // build the sql statement
    QString sql = "select help_text from tbl_help where context_id = " + contextId;
    rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
    if(rc == SQLITE_OK)
    {
      if(sqlite3_step(ppStmt) == SQLITE_ROW){
        // there should only be one row returned
        // Set the browser text to the record from the database
        txtBrowser->setText((char*)sqlite3_column_text(ppStmt, 0));
      }
    }
    // close the statement
    sqlite3_finalize(ppStmt);
    // close the database
    sqlite3_close(db);
  }
}
QgsContextHelp::~QgsContextHelp()
{
}
void QgsContextHelp::linkClicked ( const QString &link )
{
}
int QgsContextHelp::connectDb(QString &userDbPath)
{

  char *zErrMsg = 0;
  int rc;
  rc = sqlite3_open(userDbPath, &db);
  if(rc)
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(db) << std::endl;

    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert(rc == 0);
  }
  return rc;
}
