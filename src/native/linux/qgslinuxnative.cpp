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

#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QFile>
#include <QImage>
#include <QProcess>
#include <QString>
#include <QUrl>
#include <QtDebug>

using namespace Qt::StringLiterals;

#include "moc_qgslinuxnative.cpp"

QgsNative::Capabilities QgsLinuxNative::capabilities() const
{
  return NativeDesktopNotifications | NativeFilePropertiesDialog | NativeOpenTerminalAtPath;
}

void QgsLinuxNative::initializeMainWindow( QWindow *, const QString &, const QString &, const QString & )
{
  // Hardcoded desktop file value matching our official .deb packages
  mDesktopFile = u"org.qgis.qgis.desktop"_s;
}

void QgsLinuxNative::openFileExplorerAndSelectFile( const QString &path )
{
  if ( !QDBusConnection::sessionBus().isConnected() )
  {
    QgsNative::openFileExplorerAndSelectFile( path );
    return;
  }

  QDBusInterface iface( u"org.freedesktop.FileManager1"_s, u"/org/freedesktop/FileManager1"_s, u"org.freedesktop.FileManager1"_s, QDBusConnection::sessionBus() );

  iface.call( QDBus::NoBlock, u"ShowItems"_s, QStringList( QUrl::fromLocalFile( path ).toString() ), u"QGIS"_s );
  if ( iface.lastError().type() != QDBusError::NoError )
  {
    QgsNative::openFileExplorerAndSelectFile( path );
  }
}

void QgsLinuxNative::showFileProperties( const QString &path )
{
  if ( !QDBusConnection::sessionBus().isConnected() )
  {
    QgsNative::showFileProperties( path );
    return;
  }

  QDBusInterface iface( u"org.freedesktop.FileManager1"_s, u"/org/freedesktop/FileManager1"_s, u"org.freedesktop.FileManager1"_s, QDBusConnection::sessionBus() );

  iface.call( QDBus::NoBlock, u"ShowItemProperties"_s, QStringList( QUrl::fromLocalFile( path ).toString() ), u"QGIS"_s );
  if ( iface.lastError().type() != QDBusError::NoError )
  {
    QgsNative::showFileProperties( path );
  }
}

void QgsLinuxNative::showUndefinedApplicationProgress()
{
  const QVariantMap properties {
    { u"progress-visible"_s, true },
    { u"progress"_s, 0.0 }
  };

  QDBusMessage message = QDBusMessage::createSignal( u"/org/qgis/UnityLauncher"_s, u"com.canonical.Unity.LauncherEntry"_s, u"Update"_s );
  message.setArguments( { mDesktopFile, properties } );
  QDBusConnection::sessionBus().send( message );
}

void QgsLinuxNative::setApplicationProgress( double progress )
{
  const QVariantMap properties {
    { u"progress-visible"_s, true },
    { u"progress"_s, progress / 100.0 }
  };

  QDBusMessage message = QDBusMessage::createSignal( u"/org/qgis/UnityLauncher"_s, u"com.canonical.Unity.LauncherEntry"_s, u"Update"_s );
  message.setArguments( { mDesktopFile, properties } );
  QDBusConnection::sessionBus().send( message );
}

void QgsLinuxNative::hideApplicationProgress()
{
  const QVariantMap properties {
    { u"progress-visible"_s, false },
  };

  QDBusMessage message = QDBusMessage::createSignal( u"/org/qgis/UnityLauncher"_s, u"com.canonical.Unity.LauncherEntry"_s, u"Update"_s );
  message.setArguments( { mDesktopFile, properties } );
  QDBusConnection::sessionBus().send( message );
}

void QgsLinuxNative::setApplicationBadgeCount( int count )
{
  // the badge will only be shown when the count is greater than one
  const QVariantMap properties {
    { u"count-visible"_s, count > 1 },
    { u"count"_s, static_cast<long long>( count ) }
  };

  QDBusMessage message = QDBusMessage::createSignal( u"/org/qgis/UnityLauncher"_s, u"com.canonical.Unity.LauncherEntry"_s, u"Update"_s );
  message.setArguments( { mDesktopFile, properties } );
  QDBusConnection::sessionBus().send( message );
}

bool QgsLinuxNative::openTerminalAtPath( const QString &path )
{
  // logic adapted from https://askubuntu.com/a/227669,
  // https://github.com/Microsoft/vscode/blob/fec1775aa52e2124d3f09c7b2ac8f69c57309549/src/vs/workbench/parts/execution/electron-browser/terminal.ts
  QString term = u"xterm"_s;
  const QString desktopSession = qgetenv( "DESKTOP_SESSION" );
  const QString currentDesktop = qgetenv( "XDG_CURRENT_DESKTOP" );
  const QString gdmSession = qgetenv( "GDMSESSION" );
  const bool isDebian = QFile::exists( u"/etc/debian_version"_s );
  if ( isDebian )
  {
    term = u"x-terminal-emulator"_s;
  }
  else if ( desktopSession.contains( "gnome"_L1, Qt::CaseInsensitive ) || currentDesktop.contains( "gnome"_L1, Qt::CaseInsensitive ) || currentDesktop.contains( "unity"_L1, Qt::CaseInsensitive ) )
  {
    term = u"gnome-terminal"_s;
  }
  else if ( desktopSession.contains( "kde"_L1, Qt::CaseInsensitive ) || currentDesktop.contains( "kde"_L1, Qt::CaseInsensitive ) || gdmSession.contains( "kde"_L1, Qt::CaseInsensitive ) )
  {
    term = u"konsole"_s;
  }

  QStringList arguments;
  arguments << u"--working-directory"_s
            << path;
  return QProcess::startDetached( term, QStringList(), path );
}

/**
 * Automatic marshaling of a QImage for org.freedesktop.Notifications.Notify
 *
 * This function is from the Clementine project (see
 * http://www.clementine-player.org) and licensed under the GNU General Public
 * License, version 3 or later.
 *
 * Copyright 2010, David Sansome <me@davidsansome.com>
 */
QDBusArgument &operator<<( QDBusArgument &arg, const QImage &image )
{
  if ( image.isNull() )
  {
    arg.beginStructure();
    arg << 0 << 0 << 0 << false << 0 << 0 << QByteArray();
    arg.endStructure();
    return arg;
  }

  QImage scaled = image.scaledToHeight( 100, Qt::SmoothTransformation );
  scaled = scaled.convertToFormat( QImage::Format_ARGB32 );

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
  // ABGR -> ARGB
  QImage i = scaled.rgbSwapped();
#else
  // ABGR -> GBAR
  QImage i( scaled.size(), scaled.format() );
  for ( int y = 0; y < i.height(); ++y )
  {
    QRgb *p = ( QRgb * ) scaled.scanLine( y );
    QRgb *q = ( QRgb * ) i.scanLine( y );
    QRgb *end = p + scaled.width();
    while ( p < end )
    {
      *q = qRgba( qGreen( *p ), qBlue( *p ), qAlpha( *p ), qRed( *p ) );
      p++;
      q++;
    }
  }
#endif

  arg.beginStructure();
  arg << i.width();
  arg << i.height();
  arg << i.bytesPerLine();
  arg << i.hasAlphaChannel();
  const int channels = i.isGrayscale() ? 1 : ( i.hasAlphaChannel() ? 4 : 3 );
  arg << i.depth() / channels;
  arg << channels;
  arg << QByteArray( reinterpret_cast<const char *>( i.bits() ), i.sizeInBytes() );
  arg.endStructure();
  return arg;
}

const QDBusArgument &operator>>( const QDBusArgument &arg, QImage & )
{
  // This is needed to link but shouldn't be called.
  Q_ASSERT( 0 );
  return arg;
}

QgsNative::NotificationResult QgsLinuxNative::showDesktopNotification( const QString &summary, const QString &body, const NotificationSettings &settings )
{
  NotificationResult result;
  result.successful = false;

  if ( !QDBusConnection::sessionBus().isConnected() )
  {
    return result;
  }

  qDBusRegisterMetaType<QImage>();

  QDBusInterface iface( u"org.freedesktop.Notifications"_s, u"/org/freedesktop/Notifications"_s, u"org.freedesktop.Notifications"_s, QDBusConnection::sessionBus() );

  QVariantMap hints;
  hints[u"transient"_s] = settings.transient;
  if ( !settings.image.isNull() )
    hints[u"image_data"_s] = settings.image;

  QVariantList argumentList;
  argumentList << "qgis"; //app_name
  // replace_id
  if ( settings.messageId.isValid() )
    argumentList << static_cast<uint>( settings.messageId.toInt() );
  else
    argumentList << static_cast<uint>( 0 );
  // app_icon
  if ( !settings.svgAppIconPath.isEmpty() )
    argumentList << settings.svgAppIconPath;
  else
    argumentList << "";
  argumentList << summary;       // summary
  argumentList << body;          // body
  argumentList << QStringList(); // actions
  argumentList << hints;         // hints
  argumentList << -1;            // timeout in ms "If -1, the notification's expiration time is dependent on the notification server's settings, and may vary for the type of notification."

  const QDBusMessage reply = iface.callWithArgumentList( QDBus::AutoDetect, u"Notify"_s, argumentList );
  if ( reply.type() == QDBusMessage::ErrorMessage )
  {
    qDebug() << "D-Bus Error:" << reply.errorMessage();
    return result;
  }

  result.successful = true;
  result.messageId = reply.arguments().value( 0 );
  return result;
}
