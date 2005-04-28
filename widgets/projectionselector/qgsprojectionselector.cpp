/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
/* $Id$ */
#include "qgsprojectionselector.h"

//standard includes
#include <iostream>
#include <cassert>
#include <sqlite3.h>
#include <cstdlib>

//qgis includes
#include "qgscsexception.h"
#include "qgsconfig.h"

//qt includes
#include <qapplication.h>
#include <qfile.h>
#include <qtextedit.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qprogressdialog.h> 
#include <qfileinfo.h>
#include <qdir.h>

//gdal and ogr includes
// XXX DO WE NEED THESE?
#include <ogr_api.h>
#include <ogr_spatialref.h>
#include <cpl_error.h>

// set the default coordinate system
static const char* defaultWktKey = "4326";

QgsProjectionSelector::QgsProjectionSelector( QWidget* parent , const char* name , WFlags fl  )
  : QgsProjectionSelectorBase( parent, "Projection Selector", fl )
{
// Get the package data path and set the full path name to the sqlite3 spatial reference
// database.
#if defined(Q_OS_MACX) || defined(WIN32)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  mSrsDatabaseFileName = PKGDATAPATH;
  mSrsDatabaseFileName += "/resources/srs.db";
  // Populate the projection list view
  getUserProjList();
  getProjList();
}

QgsProjectionSelector::~QgsProjectionSelector()
{
}
void QgsProjectionSelector::setSelectedWKT(QString theWKT)
{
  //get the srid given the wkt so we can pick the correct list item
#ifdef QGISDEBUG
  std::cout << "QgsProjectionSelector::setSelectedWKT called with \n" << theWKT << std::endl;
#endif
  //now delegate off to the rest of the work
  QListViewItemIterator myIterator (lstCoordinateSystems);
  while (myIterator.current()) 
  {
    if (myIterator.current()->text(0)==theWKT)
    {
      lstCoordinateSystems->setCurrentItem(myIterator.current());
      lstCoordinateSystems->ensureItemVisible(myIterator.current());
      return;
    }
    ++myIterator;
  }
}

void QgsProjectionSelector::setSelectedSRID(QString theSRID)
{
  QListViewItemIterator myIterator (lstCoordinateSystems);
  while (myIterator.current()) 
  {
    if (myIterator.current()->text(1)==theSRID)
    {
      lstCoordinateSystems->setCurrentItem(myIterator.current());
      lstCoordinateSystems->ensureItemVisible(myIterator.current());
      return;
    }
    ++myIterator;
  }

}

//note this line just returns the projection name!
QString QgsProjectionSelector::getSelectedWKT()
{
  // return the selected wkt name from the list view
  QListViewItem *lvi = lstCoordinateSystems->currentItem();
  if(lvi)
  {
    return lvi->text(0);
  }
  else
  {
    return QString::null;
  }
}
// Returns the whole wkt for the selected projection node
QString QgsProjectionSelector::getCurrentWKT()
{
  // Only return the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  //
  // Get the selected node
  QListViewItem *lvi = lstCoordinateSystems->currentItem();
  if(lvi)
  {
    // Make sure the selected node is a srs and not a top-level projection node
    std::cout << lvi->text(1) << std::endl; 
    if(lvi->text(1).length() > 0)
    {
    // set up the database
    // XXX We could probabaly hold the database open for the life of this object, 
    // assuming that it will never be used anywhere else. Given the low overhead,
    // opening it each time seems to be a reasonable approach at this time.
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(mSrsDatabaseFileName, &db);
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
    QString sql = "select srtext from spatial_ref_sys where srid = ";
    sql += lvi->text(1);

#ifdef QGISDEBUG
    std::cout << "Finding selected wkt using : " <<  sql << std::endl;
#endif
    rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
    // XXX Need to free memory from the error msg if one is set
    QString wkt;
    if(rc == SQLITE_OK)
    {
      // get the first row of the result set
      if(sqlite3_step(ppStmt) == SQLITE_ROW)
      {
        // get the wkt 
        wkt = (char*)sqlite3_column_text(ppStmt, 0);
      }
    }
    // close the statement
    sqlite3_finalize(ppStmt);
    // close the database
    sqlite3_close(db);
    // return the srs wkt
    return wkt;
    }
  }
  else
  {
    // No node is selected, return null
    return NULL;
  }

}

long QgsProjectionSelector::getCurrentSRID()
{
  if(lstCoordinateSystems->currentItem()->text(1).length() > 0)
  {
    return lstCoordinateSystems->currentItem()->text(1).toLong();
  }
  else
  {
    return NULL;
  }
}
void QgsProjectionSelector::getUserProjList()
{
#ifdef QGISDEBUG
  std::cout << "Fetching user projection list..." << std::endl;
#endif
  // User defined coordinate system node
  mUserProjList = new QListViewItem(lstCoordinateSystems,"User Defined Coordinate System");
  //determine where the user proj database lives for this user. If none is found an empty
  //now only will be shown
  QString myQGisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  // first we look for ~/.qgis/user_projections.db
  // if it doesnt exist we copy it in from the global resources dir
  QFileInfo myFileInfo;
  myFileInfo.setFile(myQGisSettingsDir+"user_projections.db");
  //return straight away if the user has not created any custom projections
  if ( !myFileInfo.exists( ) )
  {
#ifdef QGISDEBUG
  std::cout << "User projection db not found...skipping" << std::endl;
#endif
    return;
  }

  sqlite3      *myDatabase;
  char         *myErrorMessage = 0;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(myQGisSettingsDir+"user_projections.db").latin1(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist. But we checked earlier for its existance
    //     and aborted in that case. This is because we may be runnig from read only 
    //     media such as live cd and dont want to force trying to create a db.
    assert(myResult == 0);
  }

  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select description from tbl_user_projection order by description";
#ifdef QGISDEBUG
  std::cout << "User projection list sql" << mySql << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    QListViewItem *newItem;
    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      newItem = new QListViewItem(mUserProjList, (char *)sqlite3_column_text(myPreparedStatement,0));
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}   

void QgsProjectionSelector::getProjList()
{
  // Create the top-level nodes for the list view of projections
  //
  // Geographic coordinate system node
  mGeoList = new QListViewItem(lstCoordinateSystems,"Geographic Coordinate System");
  // Projected coordinate system node
  mProjList = new QListViewItem(lstCoordinateSystems,"Projected Coordinate System");

  //bail out in case the projections db does not exist
  //this is neccessary in case the pc is running linux with a 
  //read only filesystem because otherwise sqlite will try 
  //to create the db file on the fly
  
  QFileInfo myFileInfo;
  myFileInfo.setFile(mSrsDatabaseFileName);
  if ( !myFileInfo.exists( ) )
  {
    return;
  }

  // open the database containing the spatial reference data
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  rc = sqlite3_open(mSrsDatabaseFileName, &db);
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
  // get total count of records in the projection table
  QString sql = "select count(*) from srs_name";

  rc = sqlite3_prepare(db, sql, sql.length(), &ppStmt, &pzTail);
  assert(rc == SQLITE_OK);
  sqlite3_step(ppStmt);
  // Set the max for the progress dialog to the number of entries in the srs_name table
  int myEntriesCount = sqlite3_column_int(ppStmt, 0);
  sqlite3_finalize(ppStmt);

  // Set up the query to retreive the projection information needed to populate the list
  sql = "select * from srs_name order by name";
  rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
  // XXX Need to free memory from the error msg if one is set
  if(rc == SQLITE_OK)
  {
    QListViewItem *newItem;
    // set up the progress dialog
    int myProgress = 1;
    QProgressDialog myProgressBar( "Building Projections List...", 0, myEntriesCount,
        this, "progress", TRUE );
    // set initial value to 1
    myProgressBar.setProgress(myProgress);
    while(sqlite3_step(ppStmt) == SQLITE_ROW)
    {
      // only update the progress dialog every 10 records
      if((myProgress++ % 10) == 0)
      {
        myProgressBar.setProgress(myProgress++);
      }
      // check to see if the srs is geographic
      int isGeo = sqlite3_column_int(ppStmt, 3);
      if(isGeo)
      {      
        // this is a geographic coordinate system
        // Add it to the tree
        newItem = new QListViewItem(mGeoList, (char *)sqlite3_column_text(ppStmt,1));

        // display the spatial reference id in the second column of the list view
        newItem->setText(1,(char *)sqlite3_column_text(ppStmt, 0));
      }
      else
      {
        // This is a projected srs

        QListViewItem *node;
        // Fine the node for this type and add the projection to it
        // If the node doesn't exist, create it
        node = lstCoordinateSystems->findItem(QString((char*)sqlite3_column_text(ppStmt, 2)),0);
        if(node == 0)
        {
          // the node doesn't exist -- create it
          node = new QListViewItem(mProjList, (char*)sqlite3_column_text(ppStmt, 2));
        }

        // add the item, setting the projection name in the first column of the list view
        newItem = new QListViewItem(node, (char *)sqlite3_column_text(ppStmt,1));
        // set the srid in the second column on the list view
        newItem->setText(1,(char *)sqlite3_column_text(ppStmt, 0));
      }
    }
    // update the progress bar to 100% -- just for eye candy purposes (some people hate to
    // see a progress dialog end at 99%)
    myProgressBar.setProgress(myEntriesCount);
  }
  // close the sqlite3 statement
  sqlite3_finalize(ppStmt);
  // close the database
  sqlite3_close(db);
}   

// New coordinate system selected from the list
void QgsProjectionSelector::coordinateSystemSelected( QListViewItem * theItem)
{
  if(theItem->text(1).length() > 0)
  {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(mSrsDatabaseFileName, &db);
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
    QString sql = "select srtext from spatial_ref_sys where srid = ";
    sql += theItem->text(1);

    rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
    // XXX Need to free memory from the error msg if one is set
    QString wkt;
    if(rc == SQLITE_OK)
    {
      if(sqlite3_step(ppStmt) == SQLITE_ROW)
      {
        wkt = (char*)sqlite3_column_text(ppStmt, 0);
      }
    }
    // close the statement
    sqlite3_finalize(ppStmt);
    // close the database
    sqlite3_close(db);
#ifdef QGISDEBUG
    std::cout << "Item selected : " << theItem->text(0) << std::endl;
    std::cout << "Item selected full wkt : " << wkt << std::endl;
#endif
    assert(wkt.length() > 0);
    // reformat the wkt to improve the display in the textedit
    // box
    wkt = wkt.replace(",", ", ");
    teProjection->setText(wkt);
    // let anybody who's listening know about the change
    // XXX Is this appropriate here if the dialog is cancelled??
    emit wktSelected(wkt);

  }
}

