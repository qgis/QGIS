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

QgsNative::Capabilities QgsNative::capabilities() const
{
  return nullptr;
}

void QgsNative::initializeMainWindow( QWindow * )
{

}

void QgsNative::currentAppActivateIgnoringOtherApps()
{
}

void QgsNative::openFileExplorerAndSelectFile( const QString &path )
{
  QFileInfo fi( path );
  QString folder = fi.path();
  QDesktopServices::openUrl( QUrl::fromLocalFile( folder ) );
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

QgsNative::NotificationResult QgsNative::showDesktopNotification( const QString &, const QString &, const NotificationSettings & )
{
  NotificationResult result;
  result.successful = false;
  return result;
}

void QgsNative::onRecentProjectsChanged( const std::vector<QgsNative::RecentProjectProperties> & )
{

}
