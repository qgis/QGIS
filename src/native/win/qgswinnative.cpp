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
#include <QCoreApplication>
#include <QString>
#include <QDir>
#include <QWindow>
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>
#include <QtWinExtras/QWinJumpList>
#include <QtWinExtras/QWinJumpListItem>
#include <QtWinExtras/QWinJumpListCategory>

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

void QgsWinNative::onRecentProjectsChanged( const std::vector<QgsNative::RecentProjectProperties> &recentProjects )
{
  QWinJumpList jumplist;
  jumplist.recent()->clear();
  for ( const RecentProjectProperties &recentProject : recentProjects )
  {
    QString name = recentProject.title != recentProject.path ? recentProject.title : QFileInfo( recentProject.path ).baseName();
    QWinJumpListItem *newProject = new QWinJumpListItem( QWinJumpListItem::Link );
    newProject->setTitle( name );
    newProject->setFilePath( QDir::toNativeSeparators( QCoreApplication::applicationFilePath() ) );
    newProject->setArguments( QStringList( recentProject.path ) );
    jumplist.recent()->addItem( newProject );
  }
}
