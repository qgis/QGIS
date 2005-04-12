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

//stdc++ includes
#include <iostream>
#include <cstdlib>

QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget* parent , const char* name , WFlags fl  )
    : QgsCustomProjectionDialogBase( parent, "Projection Designer", fl )
{
  QString myQGisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  // first we look for ~/.qgis/user_projections.db
  // if it doesnt exist we copy it in from the global resources dir
  QFileInfo myFileInfo;
  myFileInfo.setFile(myQGisSettingsDir+"user_projections.db");
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
    myMasterDatabaseFileName += "/resources/user_projections.db";
    //now copy the master file into the users .qgis dir
    std::ifstream myInputStream(myMasterDatabaseFileName.latin1() );

    if (! myInputStream)
    {
      std::cerr << "unable to open input file: "
          << myMasterDatabaseFileName << " --bailing out! \n";
      //XXX Do better error handling
      return ;
    }

    std::ofstream myOutputStream(QString(myQGisSettingsDir+"user_projections.db").latin1());

    if (! myOutputStream)
    {
      std::cerr << "cannot open " << QString(myQGisSettingsDir+"user_projections.db").latin1()  << "  for output\n";
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
  // Populate the projection combo
  // 
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
    //     database if it does not exist.
    assert(myResult == 0);
  }

  // Set up the query to retreive the projection information needed to populate the PROJECTION list
  QString mySql = "select * from tbl_projection order by name";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      cboProjectionFamily->insertItem((char *)sqlite3_column_text(myPreparedStatement,1));
    }
  }
  sqlite3_finalize(myPreparedStatement);
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  mySql = "select * from tbl_ellipsoid order by name";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      cboEllipsoid->insertItem((char *)sqlite3_column_text(myPreparedStatement,1));
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}

QgsCustomProjectionDialog::~QgsCustomProjectionDialog()
{
}


void QgsCustomProjectionDialog::pbnHelp_clicked()
{

}


void QgsCustomProjectionDialog::pbnOK_clicked()
{

}


void QgsCustomProjectionDialog::pbnApply_clicked()
{

}


void QgsCustomProjectionDialog::pbnCancel_clicked()
{

}



    
void QgsCustomProjectionDialog::cboProjectionFamily_textChanged( const QString & )
{

}


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
          (const char *)myTempFileInfo.fileName(),
          myBaseDir.path().latin1());

  return myBaseDir.mkdir(myTempFileInfo.fileName());
}
