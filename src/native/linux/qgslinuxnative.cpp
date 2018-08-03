/***************************************************************************
    qgslinuxnative.h
                             -------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinuxnative.h"

#include <QUrl>
#include <QString>
#include <QtDBus/QtDBus>
#include <QtDebug>

void QgsLinuxNative::openFileExplorerAndSelectFile( const QString &path )
{
  if ( !QDBusConnection::sessionBus().isConnected() )
  {
    QgsNative::openFileExplorerAndSelectFile( path );
    return;
  }

  QDBusInterface iface( QStringLiteral( "org.freedesktop.FileManager1" ),
                        QStringLiteral( "/org/freedesktop/FileManager1" ),
                        QStringLiteral( "org.freedesktop.FileManager1" ),
                        QDBusConnection::sessionBus() );

  iface.call( QDBus::NoBlock, QStringLiteral( "ShowItems" ), QStringList( QUrl::fromLocalFile( path ).toString() ), QStringLiteral( "QGIS" ) );
  if ( iface.lastError().type() != QDBusError::NoError )
  {
    QgsNative::openFileExplorerAndSelectFile( path );
  }
}
