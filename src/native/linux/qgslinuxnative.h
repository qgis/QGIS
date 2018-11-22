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

#ifndef QGSLINUXNATIVE_H
#define QGSLINUXNATIVE_H

#include "qgsnative.h"

/**
 * Native implementation for linux platforms.
 *
 * Implements the native platform interface for Linux based platforms. This is
 * intended to expose desktop-agnostic implementations of the QgsNative methods,
 * so that they work without issue across the wide range of Linux desktop environments
 * (e.g. Gnome/KDE).
 *
 * Typically, this means implementing methods using DBUS calls to freedesktop standards.
 */
class NATIVE_EXPORT QgsLinuxNative : public QgsNative
{
    Q_OBJECT

  public:
    QgsNative::Capabilities capabilities() const override;
    void initializeMainWindow( QWindow *window,
                               const QString &applicationName,
                               const QString &organizationName,
                               const QString &version ) override;
    void openFileExplorerAndSelectFile( const QString &path ) override;
    void showFileProperties( const QString &path ) override;
    void showUndefinedApplicationProgress() override;
    void setApplicationProgress( double progress ) override;
    void hideApplicationProgress() override;
    void setApplicationBadgeCount( int count ) override;
    bool openTerminalAtPath( const QString &path ) override;
    NotificationResult showDesktopNotification( const QString &summary, const QString &body, const NotificationSettings &settings = NotificationSettings() ) override;
  private:
    QString mDesktopFile;
};

#endif // QGSLINUXNATIVE_H
