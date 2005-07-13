#include <cassert>
#include <iostream>
#include <qstring.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qtextbrowser.h>
#include <sqlite3.h>
#include "qgshelpviewer.h"
QgsHelpViewer::QgsHelpViewer(const QString &contextId, QWidget *parent, 
    const char *name)
{
  loadContext(contextId);
}
QgsHelpViewer::~QgsHelpViewer()
{
}
void QgsHelpViewer::setContext(const QString &contextId)
{
  setWindowState(windowState() & ~WindowMinimized);
  raise();
  setActiveWindow();
  loadContext(contextId);
}
void QgsHelpViewer::fileExit()
{
  QApplication::exit();
}
void QgsHelpViewer::loadContext(const QString &contextId)
{
  if(contextId != QString::null)
  {
    // connect to the database
    QString helpDbPath =
#ifdef Q_OS_MACX
      // remove bin/qgis_help.app/Contents/MacOS to get to share/qgis
      qApp->applicationDirPath() + "/../../../../share/qgis" +
#else
      QString(PKGDATAPATH) +
#endif
      "/resources/qgis_help.db";
    int rc = connectDb(helpDbPath);
    // get the help content and title from the database

    if(rc == SQLITE_OK)
    {
      sqlite3_stmt *ppStmt;
      const char *pzTail;
      // build the sql statement
      QString sql = "select content,title from tbl_help where context_id = " 
        + contextId;
      rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
      if(rc == SQLITE_OK)
      {
        if(sqlite3_step(ppStmt) == SQLITE_ROW){
          // there should only be one row returned
          // Set the browser text to the record from the database
          txtBrowser->setText((char*)sqlite3_column_text(ppStmt, 0));
          setCaption(tr("Quantum GIS Help - ") +(char*)sqlite3_column_text(ppStmt, 1));
        }
      }
      else
      {
        QMessageBox::critical(this, "Error", 
            tr("Failed to get the help text from the database") + QString(":\n   ")
            + sqlite3_errmsg(db));  
      }
      // close the statement
      sqlite3_finalize(ppStmt);
      // close the database
      sqlite3_close(db);
    }   
  }
}

int QgsHelpViewer::connectDb(const QString &helpDbPath)
{
  // Check to see if the database exists on the path since opening
  // a sqlite3 database always succeeds 
  int result;
  if(QFileInfo(helpDbPath).exists()){
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(helpDbPath, &db);
    result = rc;
  }
  else
  {
    QMessageBox::critical(this, tr("Error"),  
        tr("The QGIS help database is not installed"));
    result = SQLITE_ERROR;

  }
  return result;
}
