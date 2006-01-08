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
#include "qgsbookmarks.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsmapcanvas.h"
 
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

//standard includes
#include <iostream>
#include <cassert>
#include <sqlite3.h>
#include <fstream>

QgsBookmarks::QgsBookmarks(QWidget *parent, const char *name)
#ifdef Q_OS_MACX
  // Mac modeless dialog dosn't have correct window type if parent is specified
  : QDialog(NULL),
#else
  // Specifying parent suppresses separate taskbar entry for dialog
  : QDialog(parent),
#endif
  mParent(parent)
{
  setupUi(this);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(reject()));

  // make sure the users database for bookmarks exists
  mQGisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  mUserDbPath = mQGisSettingsDir + "qgis.db";
  // create the database (if it exists - no problem)
  createDatabase(); 
  // Note proper queens english on next line
  initialise();

  // connect the slot up to catch when a new bookmark is added
  connect(mParent, SIGNAL(bookmarkAdded()), this, SLOT(refreshBookmarks()));
}

// Destructor
QgsBookmarks::~QgsBookmarks()
{
}

void QgsBookmarks::refreshBookmarks()
{
  lstBookmarks->clear();
  initialise();
}
// Initialise the bookmark tree from the database
void QgsBookmarks::initialise()
{
  int rc = connectDb();
  if(rc == SQLITE_OK)
  {
    // prepare the sql statement
    const char *pzTail;
    sqlite3_stmt *ppStmt;
    QString sql = "select * from tbl_bookmarks";

    rc = sqlite3_prepare(db, sql.utf8(), sql.length(), &ppStmt, &pzTail);
    // XXX Need to free memory from the error msg if one is set
    if(rc == SQLITE_OK)
    {
      // get the first row of the result set
      while(sqlite3_step(ppStmt) == SQLITE_ROW)
      {
        QString name = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 1));
        //        sqlite3_bind_parameter_index(ppStmt, "name"));
        std::cout << "Bookmark name: " << name.toLocal8Bit().data() << std::endl; 
        Q3ListViewItem *lvi = new Q3ListViewItem(lstBookmarks, name);
        // set the project name
        lvi->setText(1, QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 2))); 
        // get the extents
        QString xMin = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 3));
        QString yMin = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 4));
        QString xMax = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 5));
        QString yMax = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 6));
        // set the extents
        lvi->setText(2, xMin + ", " + yMin + ", " + xMax + ", " + yMax); 
        // set the id
        lvi->setText(3, QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 0)));
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
          (const char *)myTempFileInfo.fileName().toLocal8Bit().data(),
          (const char *)myBaseDir.path().toLocal8Bit().data());

  return myBaseDir.mkdir(myTempFileInfo.fileName());
}

void QgsBookmarks::on_btnDelete_clicked()
{
  // get the current item
  Q3ListViewItem *lvi = lstBookmarks->currentItem();
  if(lvi)
  {
    // make sure the user really wants to delete this bookmark
    if(0 == QMessageBox::information(this,tr("Really Delete?"),
          tr("Are you sure you want to delete the ") + lvi->text(0) +
          tr(" bookmark?"), tr("&Yes"), tr("&No"), QString::null, 0, 1))  
    {
      // remove it from the listview
      lstBookmarks->takeItem(lvi);
      // delete it from the database
      int rc = connectDb();
      if(rc == SQLITE_OK)
      {
        sqlite3_stmt *ppStmt;
        const char *pzTail;
        // build the sql statement
        QString sql = "delete from tbl_bookmarks where bookmark_id = " + lvi->text(3);
        rc = sqlite3_prepare(db, sql.utf8(), sql.length(), &ppStmt, &pzTail);
        if(rc == SQLITE_OK)
        {
          rc = sqlite3_step(ppStmt);
          if(rc != SQLITE_OK)
          {
            // XXX Provide popup message on failure?
            std::cout << "Failed to delete " << lvi->text(0).toLocal8Bit().data()
              << " bookmark from the database" << std::endl; 
          }
          else
          {
            // XXX Provide popup message on failure?
            std::cout << "Failed to delete " << lvi->text(0).toLocal8Bit().data()
              << " bookmark from the database" << std::endl; 
          }
        }

        // close the statement
        sqlite3_finalize(ppStmt);
        // close the database
        sqlite3_close(db);
      }
    }
  }
}

void QgsBookmarks::on_btnZoomTo_clicked()
{
  zoomToBookmark();
}

void QgsBookmarks::on_lstBookmarks_doubleClicked(Q3ListViewItem *lvi)
{
  zoomToBookmark();
}

void QgsBookmarks::zoomToBookmark()
{
	// Need to fetch the extent for the selected bookmark and then redraw
	// the map
  // get the current item
  Q3ListViewItem *lvi = lstBookmarks->currentItem();
  if(!lvi)
  {
      return;
  }
  // get the extent from the database
  int rc = connectDb();
  if(rc == SQLITE_OK)
  {
    sqlite3_stmt *ppStmt;
    const char *pzTail;
    // build the sql statement
    QString sql = "select xmin, ymin, xmax, ymax from tbl_bookmarks where bookmark_id = " + lvi->text(3);
    rc = sqlite3_prepare(db, sql.utf8(), sql.length(), &ppStmt, &pzTail);
    if(rc == SQLITE_OK)
    {
      if(sqlite3_step(ppStmt) == SQLITE_ROW){
        // get the extents from the resultset
        QString xmin  = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 0));
        QString ymin  = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 1));
        QString xmax  = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 2));
        QString ymax  = QString::fromUtf8((const char *)sqlite3_column_text(ppStmt, 3));
        // set the extent to the bookmark
        dynamic_cast<QgisApp*>(mParent)->setExtent(QgsRect(xmin.toDouble(),
              ymin.toDouble(),
              xmax.toDouble(),
              ymax.toDouble()));
        // redraw the map
        dynamic_cast<QgisApp*>(mParent)->getInterface()->getMapCanvas()->refresh();


      }
    }

    // close the statement
    sqlite3_finalize(ppStmt);
    // close the database
    sqlite3_close(db);
  }
  
}

int QgsBookmarks::connectDb()
{

  int rc;
  rc = sqlite3_open(mUserDbPath.toLocal8Bit().data(), &db);
  if(rc)
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(db) << std::endl;

    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert(rc == 0);
  }
  return rc;
}
bool QgsBookmarks::createDatabase()
{
  // make sure the users database for bookmarks exists
  QString qgisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  // first we look for ~/.qgis/qgis.db
  // if it doesnt exist we copy it in from the global resources dir
  QFileInfo myFileInfo;
  myFileInfo.setFile(qgisSettingsDir + "qgis.db");
  if ( !myFileInfo.exists( ) )
  {
#ifdef QGISDEBUG 
    std::cout << "The qgis.db does not exist in $HOME/.qgis" << std::endl; 
#endif 
    // make sure the ~/.qgis dir exists first
    QDir myUserQGisDir;
    QString myPath = QDir::homeDirPath();
    myPath += "/.qgis";
    myUserQGisDir.setPath(myPath);
#ifdef QGISDEBUG 
    std::cout << "Using " << myPath.toLocal8Bit().data() << " as path for qgis.db" << std::endl; 
#endif 
    //now make sure the users .qgis dir exists 
    makeDir(myUserQGisDir);
    // Get the full path name to the sqlite3 spatial reference database.
    QString myMasterDatabaseFileName = QgsApplication::qgisMasterDbFilePath();
    //now copy the master file into the users .qgis dir
    std::ifstream myInputStream(myMasterDatabaseFileName.toLocal8Bit().data() );

    if (! myInputStream)
    {
      std::cerr << "unable to open input file: "
        << myMasterDatabaseFileName.toLocal8Bit().data() << " --bailing out! \n";
      //XXX Do better error handling
      return false;
    }

    std::ofstream myOutputStream(QString(qgisSettingsDir+"qgis.db").toLocal8Bit().data());

    if (! myOutputStream)
    {
      std::cerr << "cannot open " << QString(qgisSettingsDir+"qgis.db").toLocal8Bit().data()  << "  for output\n";
      //XXX Do better error handling
      return false;
    }

    char myChar;
    while (myInputStream.get(myChar))
    {
      myOutputStream.put(myChar);
    }

  }
  return true;
}

void QgsBookmarks::on_btnHelp_clicked()
{
  QgsContextHelp::run(context_id);
}
