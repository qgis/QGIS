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

#include <QString>
#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QFile>

#include <sqlite3.h>

#include "qgshelpviewer.h"

#include "qgsapplication.h"

QgsHelpViewer::QgsHelpViewer( const QString &contextId, QWidget *parent,
                              Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  restorePosition();
  loadContext( contextId );
}
QgsHelpViewer::~QgsHelpViewer()
{
}
void QgsHelpViewer::setContext( const QString &contextId )
{
#ifndef WIN32
  setWindowState( windowState() & ~Qt::WindowMinimized );
#endif
  raise();
  activateWindow();
  loadContext( contextId );
}
void QgsHelpViewer::fileExit()
{
  QApplication::exit();
}

/*
 * Window geometry is saved during move and resize events rather then when
 * the window is closed because HelpViewer is a subprocess which could be
 * closed by the parent process invoking QProcess::terminate(). When this
 * happens, the HelpViewer process receives the signal WM_CLOSE on Windows
 * and SIGTERM on Mac and Unix. There is no way to catch these using Qt;
 * OS specific code must be written. To avoid OS specific code, the window
 * geometry is saved as it changes.
 */
void QgsHelpViewer::moveEvent( QMoveEvent *event )
{
  saveWindowLocation();
}

void QgsHelpViewer::resizeEvent( QResizeEvent *event )
{
  saveWindowLocation();
}

void QgsHelpViewer::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/HelpViewer/geometry" ).toByteArray() );
}

void QgsHelpViewer::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/HelpViewer/geometry", saveGeometry() );
}

/*
 * Read the help file and populate the viewer
 */
void QgsHelpViewer::loadContext( const QString &contextId )
{
  if ( contextId != QString::null )
  {
    // set up the path to the help file
    QString helpFilesPath = QgsApplication::pkgDataPath() + "/resources/context_help/";
    /*
     * determine the locale and create the file name from
     * the context id
     */
    QString lang = QLocale::system().name();

    QSettings settings;
    if( settings.value( "locale/overrideFlag", false ).toBool() )
    {
       QLocale l( settings.value( "locale/userLocale", "en_US" ).toString() );
       lang = l.name();
    }
    /*
     * If the language isn't set on the system, assume en_US,
     * otherwise we get the banner at the top of the help file
     * saying it isn't available in "your" language. Some systems
     * may be installed without the LANG environment being set.
     */
    if ( lang.length() == 0 || lang == "C" )
    {
      lang = "en_US";
    }
    QString fullHelpPath = helpFilesPath + contextId + "-" + lang;
    // get the help content and title from the localized file
    QString helpContents;
    QFile file( fullHelpPath );
    // check to see if the localized version exists
    if ( !file.exists() )
    {
      // change the file name to the en_US version (default)
      fullHelpPath = helpFilesPath + contextId + "-en_US";
      file.setFileName( fullHelpPath );
      // Check for some sort of english locale and if not found, include
      // translate this for us message
      if ( !lang.contains( "en_" ) )
      {
        helpContents = "<i>" + tr( "This help file is not available in your language %1. If you would like to translate it, please contact the QGIS  development team." ).arg( lang ) + "</i><hr />";
      }
    }
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      helpContents = tr( "This help file does not exist for your language:<p><b>%1</b><p>If you would like to create it, contact the QGIS development team" )
                     .arg( fullHelpPath );
    }
    else
    {
      QTextStream in( &file );
      in.setCodec( "UTF-8" ); // Help files must be in Utf-8
      while ( !in.atEnd() )
      {
        QString line = in.readLine();
        helpContents += line;
      }
    }
    file.close();

    // Set the browser text to the help contents
    QString myStyle = QgsApplication::reportStyleSheet();
    helpContents = "<head><style>" + myStyle + "</style></head><body>" + helpContents + "</body>";
    webView->setHtml( helpContents );
    setWindowTitle( tr( "Quantum GIS Help" ) );

  }
}

void QgsHelpViewer::loadContextFromSqlite( const QString &contextId )
{
  if ( contextId != QString::null )
  {
    // connect to the database
    QString helpDbPath = QgsApplication::pkgDataPath() + "/resources/qgis_help.db";
    int rc = connectDb( helpDbPath );
    // get the help content and title from the database

    if ( rc == SQLITE_OK )
    {
      sqlite3_stmt *ppStmt;
      const char *pzTail;
      // build the sql statement
      QString sql = "select content,title from context_helps where context = '" + contextId + "'";
      rc = sqlite3_prepare( db, sql.toUtf8(), sql.toUtf8().length(), &ppStmt, &pzTail );
      if ( rc == SQLITE_OK )
      {
        if ( sqlite3_step( ppStmt ) == SQLITE_ROW )
        {
          // there should only be one row returned
          // Set the browser text to the record from the database
          webView->setHtml(( char* )sqlite3_column_text( ppStmt, 0 ) );
          setWindowTitle( tr( "Quantum GIS Help - %1" ).arg(( char* )sqlite3_column_text( ppStmt, 1 ) ) );
        }
      }
      else
      {
        QMessageBox::critical( this, tr( "Error" ),
                               tr( "Failed to get the help text from the database:\n  %1" )
                               .arg( sqlite3_errmsg( db ) ) );
      }
      // close the statement
      sqlite3_finalize( ppStmt );
      // close the database
      sqlite3_close( db );
    }
  }
}

int QgsHelpViewer::connectDb( const QString &helpDbPath )
{
  // Check to see if the database exists on the path since opening
  // a sqlite3 database always succeeds
  int result;
  if ( QFileInfo( helpDbPath ).exists() )
  {
    int rc;
    rc = sqlite3_open( helpDbPath.toUtf8().data(), &db );
    result = rc;
  }
  else
  {
    QMessageBox::critical( this, tr( "Error" ),
                           tr( "The QGIS help database is not installed" ) );
    result = SQLITE_ERROR;
  }
  return result;
}
