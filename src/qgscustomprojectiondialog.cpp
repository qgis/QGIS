//
// C++ Implementation: qgscustomprojectiondialog
//
// Description: 
//
//
// Author: Tim Sutton tim@linfiniti.com, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "qgscustomprojectiondialog.h"

//standard includes
#include <iostream>
#include <cassert>
#include <sqlite3.h>
#include <fstream>

//qgis includes
#include "qgis.h" //<--magick numbers
#include "qgscsexception.h"
#include "qgsconfig.h"

//qt includes
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qstring.h>
#include <qurloperator.h>
#include <qcombobox.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qregexp.h>

//stdc++ includes
#include <cstdlib>
#include <string.h>

//proj4 includes
extern "C"{
#include <proj_api.h>
}


QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget* parent , const char* name , Qt::WFlags fl  )
#ifdef Q_OS_MACX
  // Mac modeless dialog dosn't have correct window type if parent is specified
  : QgsCustomProjectionDialogBase( NULL, name, false, fl)
#else
  // Specifying parent suppresses separate taskbar entry for dialog
  : QgsCustomProjectionDialogBase( parent, name, false, fl)
#endif
{
  mQGisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  // first we look for ~/.qgis/qgis.db
  // if it doesnt exist we copy it in from the global resources dir
  QFileInfo myFileInfo;
  myFileInfo.setFile(mQGisSettingsDir+"qgis.db");
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
    std::ifstream myInputStream(myMasterDatabaseFileName.local8Bit() );

    if (! myInputStream)
    {
      std::cerr << "unable to open input file: "
          << myMasterDatabaseFileName.local8Bit() << " --bailing out! \n";
      //XXX Do better error handling
      return ;
    }

    std::ofstream myOutputStream(QString(mQGisSettingsDir+"qgis.db").local8Bit());

    if (! myOutputStream)
    {
      std::cerr << "cannot open " << QString(mQGisSettingsDir+"qgis.db").local8Bit()  << "  for output\n";
      //XXX Do better error handling
      return ;
    }

    char myChar;
    while (myInputStream.get(myChar))
    {
      myOutputStream.put(myChar);
    }

  }

  //
  // Setup member vars
  //
  mCurrentRecordId="";

  //
  // Set up databound controls
  //

  // deprecated methods
  //getProjList();
  //getEllipsoidList();
  mRecordCountLong=getRecordCount();
  pbnFirst_clicked();
}

QgsCustomProjectionDialog::~QgsCustomProjectionDialog()
{
  
}
/*
 * These two methods will be deprecated
 * 
void QgsCustomProjectionDialog::getProjList ()
{
  // 
  // Populate the projection combo
  // 
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  // Set up the query to retreive the projection information needed to populate the PROJECTION list
  QString mySql = "select * from tbl_projection order by name";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      cboProjectionFamily->insertItem(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1)));
    }
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}

void QgsCustomProjectionDialog::getEllipsoidList()
{

  // 
  // Populate the ellipsoid combo
  // 
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select * from tbl_ellipsoid order by name";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      cboEllipsoid->insertItem(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1)));
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}
*/
void QgsCustomProjectionDialog::pbnHelp_clicked()
{

}

void QgsCustomProjectionDialog::pbnDelete_clicked()
{

  if (QMessageBox::Yes!=QMessageBox::warning(
        this,
        tr("Delete Projection Definition?"),
        tr("Deleting a projection definition is not reversable. Do you want to delete it?") ,
        QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton ) )
  {
    return ;
  }

  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "delete from tbl_srs where srs_id='" + mCurrentRecordId + "'";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
#ifdef QGISDEBUG
    std::cout << "Query to delete current:" << mySql.local8Bit() << std::endl;
#endif
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  //move to an appropriate rec now this one is gone
  --mRecordCountLong;
  if (mRecordCountLong < 1)
  {
    pbnNew_clicked();
  }
  else if (mCurrentRecordLong==1)
  {
    pbnFirst_clicked();
  }
  else if (mCurrentRecordLong>mRecordCountLong)
  {
    pbnLast_clicked();
  }
  else
  {
    mCurrentRecordLong=mCurrentRecordLong-2;
    pbnNext_clicked();
  }
  return ;
}

void QgsCustomProjectionDialog::pbnClose_clicked()
{
 close(); 
}




long QgsCustomProjectionDialog::getRecordCount()
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  long          myRecordCount=0;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select count(*) from tbl_srs";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      QString myRecordCountString = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      myRecordCount=myRecordCountString.toLong();
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myRecordCount;

}

QString QgsCustomProjectionDialog::getProjectionFamilyName(QString theProjectionFamilyAcronym)
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select name from tbl_projection where acronym='" + theProjectionFamilyAcronym + "'";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      myName = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}
QString QgsCustomProjectionDialog::getEllipsoidName(QString theEllipsoidAcronym)
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select name from tbl_ellipsoid where acronym='" + theEllipsoidAcronym + "'";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      myName = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}
QString QgsCustomProjectionDialog::getProjectionFamilyAcronym(QString theProjectionFamilyName)
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select acronym from tbl_projection where name='" + theProjectionFamilyName + "'";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      myName = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}
QString QgsCustomProjectionDialog::getEllipsoidAcronym(QString theEllipsoidName)
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select acronym from tbl_ellipsoid where name='" + theEllipsoidName + "'";
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      myName = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}

void QgsCustomProjectionDialog::pbnFirst_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsCustomProjectionDialog::pbnFirst_clicked()" << std::endl;
#endif
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  QString mySql = "select * from tbl_srs order by srs_id limit 1";
#ifdef QGISDEBUG
    std::cout << "Query to move first:" << mySql.local8Bit() << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      mCurrentRecordId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      leName->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1)));
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,4)));
      mCurrentRecordLong=1; 
      lblRecordNo->setText(QString::number(mCurrentRecordLong) + " of " + QString::number(mRecordCountLong));
  }
  else
  {
#ifdef QGISDEBUG
  std::cout << "pbnFirst query failed: " << mySql.local8Bit() << std::endl;
#endif
    
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  
  //enable nav buttons as appropriate
  pbnFirst->setEnabled(false);
  pbnPrevious->setEnabled(false);
  if (mCurrentRecordLong==mRecordCountLong)
  {
    pbnNext->setEnabled(false);
    pbnLast->setEnabled(false);
  }
  else
  {
    pbnNext->setEnabled(true);
    pbnLast->setEnabled(true);
  }
}


void QgsCustomProjectionDialog::pbnPrevious_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsCustomProjectionDialog::pbnPrevious_clicked()" << std::endl;
#endif
  if (mCurrentRecordLong <= 1) 
  {
    return;
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  QString mySql = "select * from tbl_srs where srs_id < " + mCurrentRecordId + " order by srs_id desc limit 1";
#ifdef QGISDEBUG
    std::cout << "Query to move previous:" << mySql.local8Bit() << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      mCurrentRecordId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      leName->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1)));
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,4))),
      --mCurrentRecordLong;
      lblRecordNo->setText(QString::number(mCurrentRecordLong) + " of " + QString::number(mRecordCountLong));
  }
  else
  {
#ifdef QGISDEBUG
  std::cout << "pbnPrevious query failed: " << mySql.local8Bit() << std::endl;
#endif
    
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);

  //enable nav buttons as appropriate
  if (mCurrentRecordLong<= 1)
  {
    pbnFirst->setEnabled(false);
    pbnPrevious->setEnabled(false);
  }
  else
  {
    pbnFirst->setEnabled(true);
    pbnPrevious->setEnabled(true);
  }
  if (mCurrentRecordLong==mRecordCountLong)
  {
    pbnNext->setEnabled(false);
    pbnLast->setEnabled(false);
  }
  else
  {
    pbnNext->setEnabled(true);
    pbnLast->setEnabled(true);
  }

}


void QgsCustomProjectionDialog::pbnNext_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsCustomProjectionDialog::pbnNext_clicked()" << std::endl;
#endif
  if (mCurrentRecordLong >= mRecordCountLong)
  {
    return;
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }


  QString mySql = "select * from tbl_srs where srs_id > " + mCurrentRecordId + " order by srs_id asc limit 1";
#ifdef QGISDEBUG
    std::cout << "Query to move next:" << mySql.local8Bit() << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      mCurrentRecordId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      leName->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1)));
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      //leParameters->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,4)));
      ++mCurrentRecordLong;
      lblRecordNo->setText(QString::number(mCurrentRecordLong) + " of " + QString::number(mRecordCountLong));
  }
  else
  {
#ifdef QGISDEBUG
  std::cout << "pbnNext query failed: " << mySql.local8Bit() << std::endl;
#endif
    
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);

  //enable nav buttons as appropriate
  if (mCurrentRecordLong==mRecordCountLong)
  {
    pbnNext->setEnabled(false);
    pbnLast->setEnabled(false);
  }
  else
  {
    pbnNext->setEnabled(true);
    pbnLast->setEnabled(true);
  }
  if (mRecordCountLong <= 1)
  {
    pbnFirst->setEnabled(false);
    pbnPrevious->setEnabled(false);
  }
  else
  {
    pbnFirst->setEnabled(true);
    pbnPrevious->setEnabled(true);
  }

}


void QgsCustomProjectionDialog::pbnLast_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsCustomProjectionDialog::pbnLast_clicked()" << std::endl;
#endif
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  QString mySql = "select * from tbl_srs order by srs_id desc limit 1";
#ifdef QGISDEBUG
    std::cout << "Query to move last:" << mySql.local8Bit() << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
      sqlite3_step(myPreparedStatement) == SQLITE_ROW;
      mCurrentRecordId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      leName->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1)));
      //QString myProjectionFamilyId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
      //cboProjectionFamily->setCurrentText(getProjectionFamilyName(myProjectionFamilyId));
      //QString myEllipsoidId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
      //cboEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
      leParameters->setText(QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,4)));
      mCurrentRecordLong =mRecordCountLong;
      lblRecordNo->setText(QString::number(mCurrentRecordLong) + " of " + QString::number(mRecordCountLong));
  }
  else
  {
#ifdef QGISDEBUG
  std::cout << "pbnLast query failed: " << mySql.local8Bit() << std::endl;
#endif
    
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  
  //enable nav buttons as appropriate
  pbnNext->setEnabled(false);
  pbnLast->setEnabled(false);
  if (mRecordCountLong <= 1)
  {
    pbnFirst->setEnabled(false);
    pbnPrevious->setEnabled(false);
  }
  else
  {
    pbnFirst->setEnabled(true);
    pbnPrevious->setEnabled(true);
  }
}


void QgsCustomProjectionDialog::pbnNew_clicked()
{
#ifdef QGISDEBUG
  if (pbnNew->text()==tr("Abort")) 
  {
    std::cout << "QgsCustomProjectionDialog::pbnNew_clicked() - abort requested" << std::endl;
  }
  else
  {
   std::cout << "QgsCustomProjectionDialog::pbnNew_clicked() - new requested" << std::endl;
  }
#endif
  if (pbnNew->text()==tr("Abort")) 
  {
    //if we get here, user has aborted add record
    pbnNew->setText(tr("New"));
    //get back to the last used record before insert was pressed
   if (mCurrentRecordId.isEmpty())
   {
      pbnFirst_clicked();
    }
    else
    {
      mCurrentRecordLong=mLastRecordLong;
      pbnNext_clicked();
    }
  }
  else
  {
    //if we get here user has elected to add new record
    pbnFirst->setEnabled(false);
    pbnPrevious->setEnabled(false);
    pbnNext->setEnabled(false);
    pbnLast->setEnabled(false);
    pbnNew->setText(tr("Abort"));
    //clear the controls
    leName->setText("");
    leParameters->setText("");
    //cboProjectionFamily->setCurrentItem(0);
    //cboEllipsoid->setCurrentItem(0);
    lblRecordNo->setText("* of " + QString::number(mRecordCountLong));
    //remember the rec we are on in case the user aborts
    mLastRecordLong=mCurrentRecordLong;
    mCurrentRecordId="";
  }

}


void QgsCustomProjectionDialog::pbnSave_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsCustomProjectionDialog::pbnSave_clicked()" << std::endl;
#endif
 
  QString myName = leName->text();
  QString myParameters = leParameters->text();
  if (myName.isEmpty())
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("This proj4 projection definition is not valid. Please give the projection a name before pressing save.") );
    return;
  }
  if (myParameters.isEmpty())
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("This proj4 projection definition is not valid. Please add the parameters before pressing save.") );
    return;
  }

  
  //
  // Now make sure parameters have proj and ellipse 
  //
  
  QString myProjectionAcronym  =  getProjFromParameters();
  QString myEllipsoidAcronym   =  getEllipseFromParameters();
  
  if ( myProjectionAcronym == NULL ) 
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("This proj4 projection definition is not valid. Please add a proj= clause before pressing save.") );
    return;
  }
  
  if ( myEllipsoidAcronym == NULL ) 
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.") );
    return;
  }
  
  
  //
  // We must check the prj def is valid!
  // NOTE :  the test below may be bogus as the processes abpve emsired there
  // is always at least a projection and ellpsoid - which proj will parse as acceptible
  //

  projPJ myProj = pj_init_plus( leParameters->text().local8Bit() );

  if ( myProj == NULL ) 
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("This proj4 projection definition is not valid. Please correct before pressing save.") );
    pj_free(myProj);
    return;
    
  }
  pj_free(myProj);


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
  if (pbnNew->text()==tr("Abort")) 
  {
    //if this is the first record we need to ensure that its srs_id is 10000. For 
    //any rec after that sqlite3 will take care of the autonumering
    //this was done to support sqlite 3.0 as it does not yet support
    //the autoinc related system tables.
    if (getRecordCount() == 0)
    {
      mySql=QString("insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo) ") 
        + " values ("+ QString::number(USER_PROJECTION_START_ID) + ",'" 
        + stringSQLSafe(myName) + "','" + myProjectionAcronym  
        + "','" + myEllipsoidAcronym  + "','" + stringSQLSafe(myParameters)
        + "',0)"; // <-- is_geo shamelessly hard coded for now
    }
    else
    {
      mySql="insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo) values ('" 
        + stringSQLSafe(myName) + "','" + myProjectionAcronym  
        + "','" + myEllipsoidAcronym  + "','" + stringSQLSafe(myParameters )
        + "',0)"; // <-- is_geo shamelessly hard coded for now
    }
  }
  else //user is updating an existing record
  {
    mySql="update tbl_srs set description='" + stringSQLSafe(myName)  
        + "',projection_acronym='" + myProjectionAcronym 
        + "',ellipsoid_acronym='" + myEllipsoidAcronym 
        + "',parameters='" + stringSQLSafe(myParameters) + "' "
        + ",is_geo=0" // <--shamelessly hard coded for now
        + " where srs_id='" + mCurrentRecordId + "'"
        ;
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult!=SQLITE_OK) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << 
        " /n please notify  QGIS developers of this error \n " << 
        QString(mQGisSettingsDir+"qgis.db").local8Bit() << " (file name) "
        << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
#ifdef QGISDEBUG
  std::cout << "Update or insert sql \n" << mySql.local8Bit() << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  sqlite3_step(myPreparedStatement);
  // XXX Need to free memory from the error msg if one is set
  if(myResult != SQLITE_OK)
  {
#ifdef QGISDEBUG
    std::cout << "Update or insert failed in custom projection dialog " << std::endl;
#endif
  }
  //reinstate button if we were doing an insert
  else if (pbnNew->text()==tr("Abort")) 
  {
    pbnNew->setText(tr("New"));
    //get to the newly inserted record 
    ++mRecordCountLong;
    mCurrentRecordLong=mRecordCountLong-1;
    pbnLast_clicked();
  }

  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}

void QgsCustomProjectionDialog::pbnCalculate_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsCustomProjectionDialog::pbnCalculate_clicked()" << std::endl;
#endif


  //
  // We must check the prj def is valid!
  //

  projPJ myProj = pj_init_plus( leTestParameters->text().local8Bit() );

  std::cout << "My proj: " << leTestParameters->text().local8Bit() << std::endl;

  if ( myProj == NULL ) 
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("This proj4 projection definition is not valid.") );
    projectedX->setText("");
    projectedY->setText("");
    pj_free(myProj);
    return;
    
  }
  // Get the WGS84 coordinates
  bool okN, okE;
  double northing = northWGS84->text().toDouble(&okN) * DEG_TO_RAD;  
  double easthing = eastWGS84->text().toDouble(&okE)  * DEG_TO_RAD;  

  if ( !okN || !okE )
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("Northing and Easthing must be in decimal form.") );
    projectedX->setText("");
    projectedY->setText("");
    pj_free(myProj);
    return;    
  }  

  projPJ wgs84Proj = pj_init_plus( GEOPROJ4.local8Bit() ); //defined in qgis.h

  if ( wgs84Proj == NULL ) 
  {
    QMessageBox::information( this, tr("QGIS Custom Projection"),
            tr("Internal Error (source projection invalid?") );
    projectedX->setText("");
    projectedY->setText("");
    pj_free(myProj);
    return;
  }

  double z = 0.0;

  int projResult = pj_transform(wgs84Proj, myProj, 1, 0, &northing, &easthing, &z);
  if ( projResult != 0 )
  {
    projectedX->setText("Error");
    projectedY->setText("Error");
    std::cout << pj_strerrno(projResult) << std::endl;
  }
  else 
  {
    QString tmp;

    tmp = tmp.setNum(northing, 'f', 4);
    projectedX->setText(tmp);
    tmp = tmp.setNum(easthing, 'f', 4);
    projectedY->setText(tmp);
  }

  //
  pj_free(myProj);
  pj_free(wgs84Proj);

}


/* This is deprecated - to be deleted
void QgsCustomProjectionDialog::cboProjectionFamily_highlighted( const QString & theText)
{
#ifdef QGISDEBUG
    std::cout << "Projection selected from combo" << std::endl;
#endif
  //search the sqlite user projections db for the projection entry 
  //and display its parameters
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").local8Bit(), &myDatabase);
  if(myResult!=SQLITE_OK) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  // Set up the query to retreive the projection information needed to populate the PROJECTION list
  QString mySql = "select parameters from tbl_projection name where name='"+theText+"'";
#ifdef QGISDEBUG
    std::cout << "Query to get proj params:" << mySql.local8Bit() << std::endl;
#endif
  myResult = sqlite3_prepare(myDatabase, mySql.utf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
    QString myParametersString = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
#ifdef QGISDEBUG
    std::cout << "Setting parameters text box to: " << myParametersString.local8Bit() << std::endl;
#endif
    txtExpectedParameters->setReadOnly(false);
    txtExpectedParameters->setText(myParametersString);
    txtExpectedParameters->setReadOnly(true);
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}
*/

//a recursive function to make a directory and its ancestors
bool QgsCustomProjectionDialog::makeDir(QDir &theQDir)
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
          (const char *)myTempFileInfo.fileName().local8Bit(),
          (const char *)myBaseDir.path().local8Bit());

  return myBaseDir.mkdir(myTempFileInfo.fileName());
}


QString QgsCustomProjectionDialog::getProjFromParameters()
{
  std::cout << "QgsCustomProjectionDialog::getProjFromParameters()" << std::endl;
  QString myProj4String = leParameters->text();
  QRegExp myProjRegExp( "\\+proj=[a-zA-Z]*" );    
  int myStart= 0;
  myStart = myProjRegExp.search(myProj4String, myStart);
  if (myStart==-1)
  {
    qDebug ("proj string supplied has no +proj argument!");
    return NULL;
  }
  else
  {
    int myLength = myProjRegExp.matchedLength();
    QString myProjectionAcronym = myProj4String.mid(myStart+(PROJ_PREFIX_LEN),myLength-(PROJ_PREFIX_LEN));//+1 for space
    return myProjectionAcronym;
  }
}

QString QgsCustomProjectionDialog::getEllipseFromParameters()
{
  std::cout << "QgsCustomProjectionDialog::getEllipseFromParameters()" << std::endl;
  QString myProj4String = leParameters->text();
  QRegExp myEllipseRegExp( "\\+ellps=[a-zA-Z0-9\\-_]*" );    
  int myStart= 0;
  myStart = myEllipseRegExp.search(myProj4String, myStart);
  if (myStart==-1)
  {
    std::cout << "proj string supplied has no +ellps!" << std::endl;
    return NULL;
  }
  else //match was found
  {
    int myLength = myEllipseRegExp.matchedLength();
    QString myEllipsoidAcronym = myProj4String.mid(myStart+(ELLPS_PREFIX_LEN),myLength-(ELLPS_PREFIX_LEN));
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
const QString QgsCustomProjectionDialog::stringSQLSafe(const QString theSQL)
{

    QString myRetval;
    QChar *it = (QChar *)theSQL.unicode();
    for (int i = 0; i < theSQL.length(); i++) {
        if (*it == '\"') {
            myRetval += "\\\"";
        } else if (*it == '\'') {
            myRetval += "\\'";
        } else if (*it == '\\') {
            myRetval += "\\\\";
        } else if (*it == '%')  {
            myRetval += "\\%";
        } else {
            myRetval += *it;
        }
        it++;
    }
    return myRetval;
}
  




