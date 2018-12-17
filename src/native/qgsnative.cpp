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



#ifdef Q_OS_MACX
#include "qgsmacnative.h"
#elif defined (Q_OS_WIN)
#ifndef __MINGW32__
#include "qgswinnative.h"
#else
#include "qgsnative.h"
#endif
#elif defined (Q_OS_LINUX)
#include "qgslinuxnative.h"
#endif


QgsNative *QgsNative::platformInterface( const QString &iconPath )
{
#ifdef Q_OS_MAC
  QgsMacNative *interface = new QgsMacNative();
  interface->setIconPath( iconPath );
#elif defined (Q_OS_WIN)
#ifndef __MINGW32__
  QgsWinNative *interface = new QgsWinNative();
#else
  QgsNative *interface = new QgsNative();
#endif
#elif defined(Q_OS_LINUX)
  QgsLinuxNative *interface = new QgsLinuxNative();
#else
  QgsNative *interface = new QgsNative();
#endif
  return interface;
}

void QgsNative::cleanup()
{}

QgsNative::Capabilities QgsNative::capabilities() const
{
  return nullptr;
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
  QFileInfo fi( path );
  QString folder = fi.path();
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
