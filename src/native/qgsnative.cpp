/***************************************************************************
    qgsnative.cpp - abstracted interface to native system calls
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

#include "qgsnative.h"
#include <QString>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

void QgsNative::cleanup()
{}

QgsNative::Capabilities QgsNative::capabilities() const
{
  return QgsNative::Capabilities();
}

void QgsNative::initializeMainWindow( QWindow *,
                                      const QString &,
                                      const QString &,
                                      const QString & )
{

}

void QgsNative::currentAppActivateIgnoringOtherApps()
{
}

void QgsNative::openFileExplorerAndSelectFile( const QString &path )
{
  const QFileInfo fi( path );
  const QString folder = fi.path();
  QDesktopServices::openUrl( QUrl::fromLocalFile( folder ) );
}

void QgsNative::showFileProperties( const QString & )
{

}

void QgsNative::showUndefinedApplicationProgress()
{

}

void QgsNative::setApplicationProgress( double )
{

}

void QgsNative::hideApplicationProgress()
{

}

void QgsNative::setApplicationBadgeCount( int )
{

}

bool QgsNative::hasDarkTheme()
{
  return false;
}

bool QgsNative::openTerminalAtPath( const QString & )
{
  return false;
}

QgsNative::NotificationResult QgsNative::showDesktopNotification( const QString &, const QString &, const NotificationSettings & )
{
  NotificationResult result;
  result.successful = false;
  return result;
}

void QgsNative::onRecentProjectsChanged( const std::vector<QgsNative::RecentProjectProperties> & )
{

}
