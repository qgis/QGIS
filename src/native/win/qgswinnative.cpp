/***************************************************************************
    qgswinnative.cpp - abstracted interface to native system calls
                             -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswinnative.h"
#include <QString>
#include <QDir>
#include <QWindow>
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>

void QgsWinNative::initializeMainWindow( QWindow *window )
{
  if ( mTaskButton )
    return; // already initialized!

  mTaskButton = new QWinTaskbarButton( window );
  mTaskButton->setWindow( window );
  mTaskProgress = mTaskButton->progress();
  mTaskProgress->setVisible( false );
}

void QgsWinNative::openFileExplorerAndSelectFile( const QString &path )
{
  const QString nativePath = QDir::toNativeSeparators( path );
  ITEMIDLIST *pidl = ILCreateFromPath( nativePath.toUtf8().constData() );
  if ( pidl )
  {
    SHOpenFolderAndSelectItems( pidl, 0, 0, 0 );
    ILFree( pidl );
  }
}

void QgsWinNative::showUndefinedApplicationProgress()
{
  mTaskProgress->setMaximum( 0 );
  mTaskProgress->show();
}

void QgsWinNative::setApplicationProgress( double progress )
{
  mTaskProgress->setMaximum( 100 );
  mTaskProgress->show();
  mTaskProgress->setValue( progress );
}

void QgsWinNative::hideApplicationProgress()
{
  mTaskProgress->hide();
}
