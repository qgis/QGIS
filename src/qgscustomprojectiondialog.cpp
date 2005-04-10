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

//stdc++ includes
#include <iostream>
#include <cstdlib>

QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget* parent , const char* name , WFlags fl  )
    : QgsCustomProjectionDialogBase( parent, "Projection Designer", fl )
{
  QString myUserProjectionDb = QDir::homeDirPath () + "/.qgis/user_projections.db";
  // first we look for ~/.qgis/user_projections.db
  // if it doesnt exist we copy it in from the global resources dir
  QFileInfo myFileInfo;
  myFileInfo.setFile(myUserProjectionDb);
  if ( !myFileInfo.exists( ) )
  {
    // make sure the ~/.qgis dir exists first
    QDir myUserQGisDir;
    QString myPath = QDir::homeDirPath();
    myPath += "/.qgis";
    myUserQGisDir.setPath(myPath);
    makeDir(myUserQGisDir);
    // Get the package data path and set the full path name to the sqlite3 spatial reference
    // database.
#if defined(Q_OS_MACX) || defined(WIN32)
    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
    QString masterDatabaseFileName = PKGDATAPATH;
    masterDatabaseFileName += "/resources/home/user_projections.db";
    //now make sure the users .qgis dir exists 
    //XXX make windows friendly too!!!
    QUrlOperator *myUrlOperator = new QUrlOperator();
    myUrlOperator->copy ("file:/"+masterDatabaseFileName,
            "file:/"+myUserProjectionDb,
            false, false);
    delete myUrlOperator;

  }
  // 
  // Populate the projection combo
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
