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

#include <stdio.h>

#include <QString>
#include <QApplication>
#include <QSettings>

#include "qgshelpviewer.h"
#include "qgsapplication.h"
#include "qgslogger.h"

QgsReaderThread::QgsReaderThread()
    : QThread()
{
}

void QgsReaderThread::run()
{
  QString help;

  char buffer[1024];
  while ( fgets( buffer, sizeof buffer, stdin ) )
  {
    if ( strcmp( buffer, "EOH\n" ) == 0 )
    {
      emit helpRead( help );
      help.clear();
    }
    else
    {
      help += QString::fromUtf8( buffer );
    }
  }
}

QgsHelpViewer::QgsHelpViewer( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  restorePosition();

  mThread = new QgsReaderThread();
  mThread->start();

  connect( mThread, SIGNAL( helpRead( QString ) ), this, SLOT( showHelp( QString ) ) );
}

QgsHelpViewer::~QgsHelpViewer()
{
  mThread->terminate();
}

void QgsHelpViewer::showHelp( QString help )
{
  // Set the browser text to the help contents
  QString myStyle = QgsApplication::reportStyleSheet();
  QString helpContents = "<head><style>" + myStyle + "</style></head><body>" + help + "</body>";
  webView->setHtml( helpContents );
  setWindowTitle( tr( "QGIS Help" ) );

#ifndef WIN32
  setWindowState( windowState() & ~Qt::WindowMinimized );
#endif
  raise();
  activateWindow();
  show();
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
  Q_UNUSED( event );
  saveWindowLocation();
}

void QgsHelpViewer::resizeEvent( QResizeEvent *event )
{
  Q_UNUSED( event );
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
