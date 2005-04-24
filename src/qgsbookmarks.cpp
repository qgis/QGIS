/***************************************************************************
               QgsBookmarks.cpp  - Spatial Bookmarks
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
//standard includes
#include <iostream>
#include <cassert>
#include <sqlite3.h>
#include <fstream>

#include <qfileinfo.h>
#include <qstring.h>
#include <qdir.h>
#include <qlistview.h>

#include "qgsbookmarks.h"

QgsBookmarks::QgsBookmarks(QWidget *parent, const char *name)
{
  // make sure the users database for bookmarks exists
  mQGisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  // first we look for ~/.qgis/qgis.db
  // if it doesnt exist we copy it in from the global resources dir
  QFileInfo myFileInfo;
  mUserDbPath = mQGisSettingsDir + "qgis.db";
  myFileInfo.setFile(mUserDbPath);
  if ( !myFileInfo.exists( ) )
  {
    // make sure the ~/.qgis dir exists first
    QDir myUserQGisDir;
    QString myPath = QDir::homeDirPath();
    myPath += "/.qgis";
    myUserQGisDir.setPath(myPath);
    //now make sure the users .qgis dir exists 
    makeDir(myUserQGisDir);
    // Get the package data path and set the full path name to the sqlite3 spatial reference
    // database.
#if defined(Q_OS_MACX) || defined(WIN32)
    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
    QString myMasterDatabaseFileName = PKGDATAPATH;
    myMasterDatabaseFileName += "/resources/qgis.db";
    //now copy the master file into the users .qgis dir
    std::ifstream myInputStream(myMasterDatabaseFileName.latin1() );

    if (! myInputStream)
    {
      std::cerr << "unable to open input file: "
        << myMasterDatabaseFileName << " --bailing out! \n";
      //XXX Do better error handling
      return ;
    }

    std::ofstream myOutputStream(QString(mQGisSettingsDir+"qgis.db").latin1());

    if (! myOutputStream)
    {
      std::cerr << "cannot open " << QString(mQGisSettingsDir+"qgis.db").latin1()  << "  for output\n";
      //XXX Do better error handling
      return ;
    }

    char myChar;
    while (myInputStream.get(myChar))
    {
      myOutputStream.put(myChar);
    }

  }
  // Note proper queens english on next line
  initialise();
}

// Destructor
QgsBookmarks::~QgsBookmarks()
{
}

// Initialise the bookmark tree from the database
void QgsBookmarks::initialise()
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
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
  QString sql = "select * from tbl_bookmarks";

  rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
  // XXX Need to free memory from the error msg if one is set
  if(rc == SQLITE_OK)
  {
    // get the first row of the result set
    while(sqlite3_step(ppStmt) == SQLITE_ROW)
    {
      QString name  = (char*)sqlite3_column_text(ppStmt, 1);
//        sqlite3_bind_parameter_index(ppStmt, "name"));
      std::cout << "Bookmark name: " << name << std::endl; 
      QListViewItem *lvi = new QListViewItem(lstBookmarks, name);
      lvi->setText(1, (char*)sqlite3_column_text(ppStmt, 2)); 
      // get the extents
      QString xMin = (char*)sqlite3_column_text(ppStmt, 3);
      QString yMin = (char*)sqlite3_column_text(ppStmt, 4);
      QString xMax = (char*)sqlite3_column_text(ppStmt, 5);
      QString yMax = (char*)sqlite3_column_text(ppStmt, 6);

      lvi->setText(2, xMin + ", " + yMin + ", " + xMax + ", " + yMax); 
    }
  }
  else
  {
    // XXX query failed -- warn the user some how
    std::cout << "Failed to get bookmarks: " << sqlite3_errmsg(db) << std::endl; 
  }
  // close the statement
  sqlite3_finalize(ppStmt);
  // close the database
  sqlite3_close(db);
  // return the srs wkt


}

// A recursive function to make a directory and its ancestors
// XXX Note we use this function in two places, one more and we
// XXX should consider making a utility class to contain this and
// XXX other, if any functions that are used across the application.
bool QgsBookmarks::makeDir(QDir &theQDir)
{
  if (theQDir.isRoot ())
  {
    //cannot create a root dir
    return (false);
  }

  QDir myBaseDir;
  QFileInfo myTempFileInfo;

  myTempFileInfo.setFile(theQDir.path());
  myBaseDir = myTempFileInfo.dir();

  if(!myBaseDir.exists() && !makeDir(myBaseDir))
  {
    return FALSE;
  }

  qDebug("attempting to create directory %s in %s", 
          (const char *)myTempFileInfo.fileName(),
          myBaseDir.path().latin1());

  return myBaseDir.mkdir(myTempFileInfo.fileName());
}

void QgsBookmarks::deleteBookmark()
{
  // get the current item
  QListViewItem *lvi = lstBookmarks->currentItem();
  lstBookmarks->takeItem(lvi);
  
}


