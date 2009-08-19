/***************************************************************************
                          qgscustomprojectiondialog.cpp

                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Tim Sutton
    email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomprojectiondialog.h"

//qgis includes
#include "qgis.h" //<--magick numbers
#include "qgisapp.h" //<--theme icons
#include "qgsapplication.h"
#include "qgslogger.h"

//qt includes
#include <QFileInfo>
#include <QMessageBox>
#include <QLocale>

//stdc++ includes
#include <cassert>
#include <fstream>
#include <sqlite3.h>
#include "qgslogger.h"

//proj4 includes
extern "C"
{
#include <proj_api.h>
}


QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  pbnFirst->setIcon( QgisApp::getThemeIcon( "mIconFirst.png" ) );
  pbnPrevious->setIcon( QgisApp::getThemeIcon( "mIconPrevious.png" ) );
  pbnNext->setIcon( QgisApp::getThemeIcon( "mIconNext.png" ) );
  pbnLast->setIcon( QgisApp::getThemeIcon( "mIconLast.png" ) );
  pbnNew->setIcon( QgisApp::getThemeIcon( "mIconNew.png" ) );
  pbnSave->setIcon( QgisApp::getThemeIcon( "mActionFileSave.png" ) );
  pbnDelete->setIcon( QgisApp::getThemeIcon( "mIconDelete.png" ) );
  // user database is created at QGIS startup in QgisApp::createDB
  // we just check whether there is our database [MD]
  QFileInfo myFileInfo;
  myFileInfo.setFile( QgsApplication::qgisSettingsDirPath() );
  if ( !myFileInfo.exists( ) )
  {
    QgsDebugMsg( "The qgis.db does not exist" );
  }

  //
  // Setup member vars
  //
  mCurrentRecordId = "";

  //
  // Set up databound controls
  //

  // deprecated methods
  //getProjList();
  //getEllipsoidList();
  mRecordCountLong = getRecordCount();
  if ( mRecordCountLong > 0 )
    on_pbnFirst_clicked();
  else
    on_pbnNew_clicked();
  //automatically go to insert mode if there are not recs yet
  if ( mRecordCountLong < 1 )
  {
    on_pbnNew_clicked();
  }
}

QgsCustomProjectionDialog::~QgsCustomProjectionDialog()
{

}

void QgsCustomProjectionDialog::on_pbnDelete_clicked()
{

  if ( QMessageBox::Ok != QMessageBox::warning(
         this,
         tr( "Delete Projection Definition?" ),
         tr( "Deleting a projection definition is not reversable. Do you want to delete it?" ),
         QMessageBox::Ok | QMessageBox::Cancel ) )
  {
    return ;
  }

  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "delete from tbl_srs where srs_id='" + mCurrentRecordId + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  QgsDebugMsg( QString( "Query to delete current:%1" ).arg( mySql ) );
  if ( myResult == SQLITE_OK )
  {
    sqlite3_step( myPreparedStatement );
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  //move to an appropriate rec now this one is gone
  --mRecordCountLong;
  if ( mRecordCountLong < 1 )
  {
    on_pbnNew_clicked();
  }
  else if ( mCurrentRecordLong == 1 )
  {
    on_pbnFirst_clicked();
  }
  else if ( mCurrentRecordLong > mRecordCountLong )
  {
    on_pbnLast_clicked();
  }
  else
  {
    mCurrentRecordLong = mCurrentRecordLong - 2;
    on_pbnNext_clicked();
  }
  return ;
}

void QgsCustomProjectionDialog::on_pbnClose_clicked()
{
  close();
}




long QgsCustomProjectionDialog::getRecordCount()
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  long          myRecordCount = 0;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select count(*) from tbl_srs";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      QString myRecordCountString = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      myRecordCount = myRecordCountString.toLong();
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return myRecordCount;

}

QString QgsCustomProjectionDialog::getProjectionFamilyName( QString theProjectionFamilyAcronym )
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the PROJECTION list
  QString mySql = "select name from tbl_projection where acronym='" + theProjectionFamilyAcronym + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
      myName = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return myName;

}
QString QgsCustomProjectionDialog::getEllipsoidName( QString theEllipsoidAcronym )
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select name from tbl_ellipsoid where acronym='" + theEllipsoidAcronym + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
      myName = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return myName;

}
QString QgsCustomProjectionDialog::getProjectionFamilyAcronym( QString theProjectionFamilyName )
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the PROJECTION list
  QString mySql = "select acronym from tbl_projection where name='" + theProjectionFamilyName + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
      myName = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return myName;

}
QString QgsCustomProjectionDialog::getEllipsoidAcronym( QString theEllipsoidName )
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select acronym from tbl_ellipsoid where name='" + theEllipsoidName + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
      myName = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return myName;

}

void QgsCustomProjectionDialog::on_pbnFirst_clicked()
{
  QgsDebugMsg( "entered." );
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }

  QString mySql = "select * from tbl_srs order by srs_id limit 1";
  QgsDebugMsg( QString( "Query to move first:%1" ).arg( mySql ) );
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      mCurrentRecordId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      leName->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) ) );
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 4 ) ) );
      mCurrentRecordLong = 1;
      lblRecordNo->setText( QString::number( mCurrentRecordLong ) + " of " + QString::number( mRecordCountLong ) );
    }
  }
  else
  {
    QgsDebugMsg( QString( "pbnFirst query failed: %1" ).arg( mySql ) );

  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  //enable nav buttons as appropriate
  pbnFirst->setEnabled( false );
  pbnPrevious->setEnabled( false );
  //automatically go to insert mode if there are not recs yet
  if ( mRecordCountLong < 1 )
  {
    on_pbnNew_clicked();
    pbnDelete->setEnabled( false );
  }
  else if ( mCurrentRecordLong == mRecordCountLong )
  {
    pbnNext->setEnabled( false );
    pbnLast->setEnabled( false );
    pbnDelete->setEnabled( true );
  }
  else
  {
    pbnNext->setEnabled( true );
    pbnLast->setEnabled( true );
    pbnDelete->setEnabled( true );
  }
}


void QgsCustomProjectionDialog::on_pbnPrevious_clicked()
{
  QgsDebugMsg( "entered." );
  if ( mCurrentRecordLong <= 1 )
  {
    return;
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }

  QString mySql = "select * from tbl_srs where srs_id < " + mCurrentRecordId + " order by srs_id desc limit 1";
  QgsDebugMsg( QString( "Query to move previous:%1" ).arg( mySql ) );
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      mCurrentRecordId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      leName->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) ) );
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 4 ) ) ),
      --mCurrentRecordLong;
      lblRecordNo->setText( QString::number( mCurrentRecordLong ) + " of " + QString::number( mRecordCountLong ) );
    }
  }
  else
  {
    QgsDebugMsg( QString( "pbnPrevious query failed: %1" ).arg( mySql ) );

  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  //enable nav buttons as appropriate
  if ( mCurrentRecordLong <= 1 )
  {
    pbnFirst->setEnabled( false );
    pbnPrevious->setEnabled( false );
  }
  else
  {
    pbnFirst->setEnabled( true );
    pbnPrevious->setEnabled( true );
  }
  if ( mCurrentRecordLong == mRecordCountLong )
  {
    pbnNext->setEnabled( false );
    pbnLast->setEnabled( false );
  }
  else
  {
    pbnNext->setEnabled( true );
    pbnLast->setEnabled( true );
  }

}


void QgsCustomProjectionDialog::on_pbnNext_clicked()
{
  QgsDebugMsg( "entered." );
  if ( mCurrentRecordLong >= mRecordCountLong )
  {
    return;
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }

  QString mySql = "select * from tbl_srs where srs_id > " + mCurrentRecordId + " order by srs_id asc limit 1";
  QgsDebugMsg( QString( "Query to move next:%1" ).arg( mySql ) );
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      mCurrentRecordId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      leName->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) ) );
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 4 ) ) );
      ++mCurrentRecordLong;
      lblRecordNo->setText( QString::number( mCurrentRecordLong ) + " of " + QString::number( mRecordCountLong ) );
    }
  }
  else
  {
    QgsDebugMsg( QString( "pbnNext query failed: %1" ).arg( mySql ) );

  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  //enable nav buttons as appropriate
  if ( mCurrentRecordLong == mRecordCountLong )
  {
    pbnNext->setEnabled( false );
    pbnLast->setEnabled( false );
  }
  else
  {
    pbnNext->setEnabled( true );
    pbnLast->setEnabled( true );
  }
  if ( mRecordCountLong <= 1 )
  {
    pbnFirst->setEnabled( false );
    pbnPrevious->setEnabled( false );
  }
  else
  {
    pbnFirst->setEnabled( true );
    pbnPrevious->setEnabled( true );
  }

}


void QgsCustomProjectionDialog::on_pbnLast_clicked()
{
  QgsDebugMsg( "entered." );
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }

  QString mySql = "select * from tbl_srs order by srs_id desc limit 1";
  QgsDebugMsg( QString( "Query to move last:%1" ).arg( mySql ) );
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      mCurrentRecordId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      leName->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) ) );
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 4 ) ) );
      mCurrentRecordLong = mRecordCountLong;
      lblRecordNo->setText( QString::number( mCurrentRecordLong ) + " of " + QString::number( mRecordCountLong ) );
    }
  }
  else
  {
    QgsDebugMsg( QString( "pbnLast query failed: %1" ).arg( mySql ) );

  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  //enable nav buttons as appropriate
  pbnNext->setEnabled( false );
  pbnLast->setEnabled( false );
  if ( mRecordCountLong <= 1 )
  {
    pbnFirst->setEnabled( false );
    pbnPrevious->setEnabled( false );
  }
  else
  {
    pbnFirst->setEnabled( true );
    pbnPrevious->setEnabled( true );
  }
}


void QgsCustomProjectionDialog::on_pbnNew_clicked()
{
  if ( pbnNew->text() == tr( "Abort" ) )
  {
    //if we get here, user has aborted add record
    pbnNew->setIcon( QgisApp::getThemeIcon( "mIconNew.png" ) );
    //next line needed for new/abort logic
    pbnNew->setText( tr( "New" ) );
    //get back to the last used record before insert was pressed
    if ( mCurrentRecordId.isEmpty() )
    {
      on_pbnFirst_clicked();
    }
    else
    {
      mCurrentRecordLong = mLastRecordLong;
      on_pbnNext_clicked();
    }
  }
  else
  {
    //if we get here user has elected to add new record
    pbnFirst->setEnabled( false );
    pbnPrevious->setEnabled( false );
    pbnNext->setEnabled( false );
    pbnLast->setEnabled( false );
    pbnDelete->setEnabled( false );
    pbnNew->setIcon( QgisApp::getThemeIcon( "mIconNew.png" ) );
    //next line needed for new/abort logic
    pbnNew->setText( tr( "Abort" ) );
    //clear the controls
    leName->setText( "" );
    leParameters->setText( "" );
    //cboProjectionFamily->setCurrentItem(0);
    //cboEllipsoid->setCurrentItem(0);
    lblRecordNo->setText( "* of " + QString::number( mRecordCountLong ) );
    //remember the rec we are on in case the user aborts
    mLastRecordLong = mCurrentRecordLong;
    mCurrentRecordId = "";
  }

}


void QgsCustomProjectionDialog::on_pbnSave_clicked()
{
  QgsDebugMsg( "entered." );

  QString myName = leName->text();
  QString myParameters = leParameters->text();
  if ( myName.isEmpty() )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 projection definition is not valid." )
                              + tr( " Please give the projection a name before pressing save." ) );
    return;
  }
  if ( myParameters.isEmpty() )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 projection definition is not valid." )
                              + tr( " Please add the parameters before pressing save." ) );
    return;
  }


  //
  // Now make sure parameters have proj and ellipse
  //

  QString myProjectionAcronym  =  getProjFromParameters();
  QString myEllipsoidAcronym   =  getEllipseFromParameters();

  if ( myProjectionAcronym.isNull() )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 projection definition is not valid." )
                              + tr( " Please add a proj= clause before pressing save." ) );
    return;
  }

#if 0
  /** I am commenting this check out for now because of ticket #1146
   * In 1.0.0 we should consider doing more sophisticated checks or just
   * removing this commented block entirely. It is possible to set the
   * parameters for the earths figure in ways other than using ellps (which
   * is a convenience function in proj). For example the radius and flattenning
   * can be specified and various other parameter permutations. See the proj
   * manual section entitled 'Specifying the Earths Figure' for more details.
   * Tim Sutton */
  if ( myEllipsoidAcronym.isNull() )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.", "COMMENTED OUT" ) );
    return;
  }
#endif

  //
  // We must check the prj def is valid!
  // NOTE :  the test below may be bogus as the processes abpve emsired there
  // is always at least a projection and ellpsoid - which proj will parse as acceptible
  //

  projPJ myProj = pj_init_plus( leParameters->text().toLocal8Bit().data() );

  if ( myProj == NULL )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 projection definition is not valid." )
                              + tr( " Please correct before pressing save." ) );
    pj_free( myProj );
    return;

  }
  pj_free( myProj );


  /** TODO Check the projection is not a duplicate ! */


  //CREATE TABLE tbl_srs (
  //srs_id integer primary key,
  //description varchar(255) NOT NULL,
  //projection_acronym varchar(20) NOT NULL default '',
  //ellipsoid_acronym varchar(20) NOT NULL default '',
  //parameters varchar(80) NOT NULL default ''
  //);

  QString mySql;
  //insert a record if mode is enabled
  if ( pbnNew->text() == tr( "Abort" ) )
  {
    //if this is the first record we need to ensure that its srs_id is 10000. For
    //any rec after that sqlite3 will take care of the autonumering
    //this was done to support sqlite 3.0 as it does not yet support
    //the autoinc related system tables.
    if ( getRecordCount() == 0 )
    {
      mySql = QString( "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo) " )
              + " values (" + QString::number( USER_CRS_START_ID ) + ",'"
              + sqlSafeString( myName ) + "','" + myProjectionAcronym
              + "','" + myEllipsoidAcronym  + "','" + sqlSafeString( myParameters )
              + "',0)"; // <-- is_geo shamelessly hard coded for now
    }
    else
    {
      mySql = "insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo) values ('"
              + sqlSafeString( myName ) + "','" + myProjectionAcronym
              + "','" + myEllipsoidAcronym  + "','" + sqlSafeString( myParameters )
              + "',0)"; // <-- is_geo shamelessly hard coded for now
    }
  }
  else //user is updating an existing record
  {
    mySql = "update tbl_srs set description='" + sqlSafeString( myName )
            + "',projection_acronym='" + myProjectionAcronym
            + "',ellipsoid_acronym='" + myEllipsoidAcronym
            + "',parameters='" + sqlSafeString( myParameters ) + "' "
            + ",is_geo=0" // <--shamelessly hard coded for now
            + " where srs_id='" + mCurrentRecordId + "'"
            ;
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( sqlite3_errmsg( myDatabase ) ).arg( QgsApplication::qgisUserDbFilePath() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  QgsDebugMsg( QString( "Update or insert sql \n%1" ).arg( mySql ) );
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  sqlite3_step( myPreparedStatement );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( "Update or insert failed in custom projection dialog " );
  }
  //reinstate button if we were doing an insert
  else if ( pbnNew->text() == tr( "Abort" ) )
  {
    pbnNew->setText( tr( "New" ) );
    //get to the newly inserted record
    ++mRecordCountLong;
    mCurrentRecordLong = mRecordCountLong - 1;
    on_pbnLast_clicked();
  }

  sqlite3_finalize( myPreparedStatement );

  // If we have a projection acronym not in the user db previously, add it.
  // This is a must, or else we can't select it from the vw_srs table.
  // Actually, add it always and let the SQL PRIMARY KEY remove duplicates.

  sqlite3      *srsDatabase;
  const char   *srsTail;
  sqlite3_stmt *srsPreparedStatement;
  int           srsResult;

  //check the db is available
  srsResult = sqlite3_open( QgsApplication::srsDbFilePath().toUtf8().data(), &srsDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( srsDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the PROJECTION list
  QString srsSql = "select * from tbl_projection where acronym='" + myProjectionAcronym + "'";
  srsResult = sqlite3_prepare( srsDatabase, srsSql.toUtf8(), srsSql.length(), &srsPreparedStatement, &srsTail );
  // XXX Need to free memory from the error msg if one is set
  if ( srsResult == SQLITE_OK )
  {
    if ( sqlite3_step( srsPreparedStatement ) == SQLITE_ROW )
    {
      QgsDebugMsg( "Trying to insert projection" );
      // We have the result from system srs.db. Now insert into user db.
      mySql = QString( "INSERT INTO tbl_projection VALUES('%1','%2','%3','%4')" )
              .arg( QString::fromUtf8(( char * )sqlite3_column_text( srsPreparedStatement, 0 ) ) )
              .arg( QString::fromUtf8(( char * )sqlite3_column_text( srsPreparedStatement, 1 ) ) )
              .arg( QString::fromUtf8(( char * )sqlite3_column_text( srsPreparedStatement, 2 ) ) )
              .arg( QString::fromUtf8(( char * )sqlite3_column_text( srsPreparedStatement, 3 ) ) );
      myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.length(), &myPreparedStatement, &myTail );
      sqlite3_step( myPreparedStatement );
      if ( myResult != SQLITE_OK )
      {
        QgsDebugMsg( "Update or insert failed in custom projection dialog: " + mySql );
      }
      sqlite3_finalize( myPreparedStatement );
    }
  }

  // close the user and srs sqlite3 db
  sqlite3_close( myDatabase );
  sqlite3_finalize( srsPreparedStatement );
  sqlite3_close( srsDatabase );

  pbnDelete->setEnabled( true );
}

void QgsCustomProjectionDialog::on_pbnCalculate_clicked()
{
  QgsDebugMsg( "entered." );


  //
  // We must check the prj def is valid!
  //

  projPJ myProj = pj_init_plus( leTestParameters->text().toLocal8Bit().data() );

  QgsDebugMsg( QString( "My proj: %1" ).arg( leTestParameters->text() ) );

  if ( myProj == NULL )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 projection definition is not valid." ) );
    projectedX->setText( "" );
    projectedY->setText( "" );
    pj_free( myProj );
    return;

  }
  // Get the WGS84 coordinates
  bool okN, okE;
  double northing = northWGS84->text().toDouble( &okN ) * DEG_TO_RAD;
  double easthing = eastWGS84->text().toDouble( &okE )  * DEG_TO_RAD;

  if ( !okN || !okE )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "Northing and Easthing must be in decimal form." ) );
    projectedX->setText( "" );
    projectedY->setText( "" );
    pj_free( myProj );
    return;
  }

  projPJ wgs84Proj = pj_init_plus( GEOPROJ4.toLocal8Bit().data() ); //defined in qgis.h

  if ( wgs84Proj == NULL )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "Internal Error (source projection invalid?)" ) );
    projectedX->setText( "" );
    projectedY->setText( "" );
    pj_free( myProj );
    return;
  }

  double z = 0.0;

  int projResult = pj_transform( wgs84Proj, myProj, 1, 0, &easthing, &northing, &z );
  if ( projResult != 0 )
  {
    projectedX->setText( "Error" );
    projectedY->setText( "Error" );
    QgsDebugMsg( pj_strerrno( projResult ) );
  }
  else
  {
    QString tmp;

    tmp = QLocale::system().toString( northing, 'f', 4 );
    projectedX->setText( tmp );
    tmp = QLocale::system().toString( easthing, 'f', 4 );
    projectedY->setText( tmp );
  }

  //
  pj_free( myProj );
  pj_free( wgs84Proj );

}


QString QgsCustomProjectionDialog::getProjFromParameters()
{
  QgsDebugMsg( "entered." );
  QString myProj4String = leParameters->text();
  QRegExp myProjRegExp( "\\+proj=[a-zA-Z]*" );
  int myStart = 0;
  myStart = myProjRegExp.indexIn( myProj4String, myStart );
  if ( myStart == -1 )
  {
    QgsDebugMsg( "proj string supplied has no +proj argument!" );
    return NULL;
  }
  else
  {
    int myLength = myProjRegExp.matchedLength();
    QString myProjectionAcronym = myProj4String.mid( myStart + ( PROJ_PREFIX_LEN ), myLength - ( PROJ_PREFIX_LEN ) );//+1 for space
    return myProjectionAcronym;
  }
}

QString QgsCustomProjectionDialog::getEllipseFromParameters()
{
  QgsDebugMsg( "entered." );
  QString myProj4String = leParameters->text();
  QRegExp myEllipseRegExp( "\\+ellps=[a-zA-Z0-9\\-_]*" );
  int myStart = 0;
  myStart = myEllipseRegExp.indexIn( myProj4String, myStart );
  if ( myStart == -1 )
  {
    QgsDebugMsg( "proj string supplied has no +ellps!" );
    return NULL;
  }
  else //match was found
  {
    int myLength = myEllipseRegExp.matchedLength();
    QString myEllipsoidAcronym = myProj4String.mid( myStart + ( ELLPS_PREFIX_LEN ), myLength - ( ELLPS_PREFIX_LEN ) );
    return myEllipsoidAcronym;
  }
}

/*!
* \brief Make the string safe for use in SQL statements.
*  This involves escaping single quotes, double quotes, backslashes,
*  and optionally, percentage symbols.  Percentage symbols are used
*  as wildcards sometimes and so when using the string as part of the
*  LIKE phrase of a select statement, should be escaped.
* \arg const QString in The input string to make safe.
* \return The string made safe for SQL statements.
*/
const QString QgsCustomProjectionDialog::sqlSafeString( const QString theSQL )
{

  QString myRetval;
  QChar *it = ( QChar * )theSQL.unicode();
  for ( int i = 0; i < theSQL.length(); i++ )
  {
    if ( *it == '\"' )
    {
      myRetval += "\\\"";
    }
    else if ( *it == '\'' )
    {
      myRetval += "\\'";
    }
    else if ( *it == '\\' )
    {
      myRetval += "\\\\";
    }
    else if ( *it == '%' )
    {
      myRetval += "\\%";
    }
    else
    {
      myRetval += *it;
    }
    it++;
  }
  return myRetval;
}

