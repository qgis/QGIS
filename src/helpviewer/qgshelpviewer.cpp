/***************************************************************************
                             qgshelpviewer.cpp 
                             Simple help browser
                             -------------------
    begin                : 2005-07-02
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

#include <cassert>
#include <iostream>
#include <qstring.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <q3textbrowser.h>
#include <sqlite3.h>
#include <QTextCodec>
#include <QTextStream>
#include <QFile>
#include "qgshelpviewer.h"
QgsHelpViewer::QgsHelpViewer(const QString &contextId, QWidget *parent, 
    Qt::WFlags fl)
: QDialog(parent, fl)
{
  setupUi(this);
  loadContext(contextId);
}
QgsHelpViewer::~QgsHelpViewer()
{
}
void QgsHelpViewer::setContext(const QString &contextId)
{
#ifndef WIN32
  setWindowState(windowState() & ~Qt::WindowMinimized);
#endif
  raise();
  setActiveWindow();
  loadContext(contextId);
}
void QgsHelpViewer::fileExit()
{
  QApplication::exit();
}
/*
 * Read the help file and populate the viewer
 */
void QgsHelpViewer::loadContext(const QString &contextId)
{
  if(contextId != QString::null)
  {
    // set up the path to the help file
    QString helpFilesPath =
#ifdef Q_OS_MACX
      // remove bin/qgis_help.app/Contents/MacOS to get to share/qgis
      qApp->applicationDirPath() + "/../../../../share/qgis" +
#elif WIN32
      qApp->applicationDirPath() + "/share/qgis"
#else
      QString(PKGDATAPATH) +
#endif
      "/resources/context_help/";
    /* 
     * determine the locale and create the file name from
     * the context id
     */
    QString lang(QTextCodec::locale());
    QString fullHelpPath = helpFilesPath + contextId + "_" + lang;
    // get the help content and title from the localized file
    QString helpContents;
    QFile file(fullHelpPath);
    // check to see if the localized version exists
    if(!file.exists())
    {
      // change the file name to the en_US version (default)
      fullHelpPath = helpFilesPath + contextId + "_en_US";
      file.setFileName(fullHelpPath);
      // Check for some sort of english locale and if not found, include 
      // translate this for us message
      if(!lang.contains("en_"))
      {
        helpContents = "<i>This help file is not available in your language."
         " If you would like to translate it, please contact the QGIS  development team.</i><hr>";
      }
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      helpContents = tr("This help file does not exist for your language")
        +":<p><b>"
        + fullHelpPath 
        + "</b><p>"
        + tr("Feel free to translate it and submit it to the QGIS development team");
    }
    else
    { 
      QTextStream in(&file);
      while (!in.atEnd()) {
        QString line = in.readLine();
        helpContents += line;
      }
    }
    file.close();

    // Set the browser text to the help contents
    txtBrowser->setText(helpContents);
    setCaption(tr("Quantum GIS Help"));

        }
        }

void QgsHelpViewer::loadContextFromSqlite(const QString &contextId)
{
  if(contextId != QString::null)
  {
    // connect to the database
    QString helpDbPath =
#ifdef Q_OS_MACX
      // remove bin/qgis_help.app/Contents/MacOS to get to share/qgis
      qApp->applicationDirPath() + "/../../../../share/qgis" +
#elif WIN32
      qApp->applicationDirPath() + "/share/qgis"
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
      QString sql = "select content,title from context_helps where context_id = " 
        + contextId;
      rc = sqlite3_prepare(db, (const char *)sql, sql.length(), &ppStmt, &pzTail);
      if(rc == SQLITE_OK)
      {
        if(sqlite3_step(ppStmt) == SQLITE_ROW){
          // there should only be one row returned
          // Set the browser text to the record from the database
          txtBrowser->setText((char*)sqlite3_column_text(ppStmt, 0));
          setCaption(tr("Quantum GIS Help - ") +QString((char*)sqlite3_column_text(ppStmt, 1)));
        }
      }
      else
      {
        QMessageBox::critical(this, tr("Error"), 
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
