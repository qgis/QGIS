/***************************************************************************
    qgswinnative.h - abstracted interface to native Mac objective-c
                             -------------------
    begin                : January 2014
    copyright            : (C) 2014 by Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWINNATIVE_H
#define QGSWINNATIVE_H

#include <ShlObj.h>
#include <Windows.h>

#include "qgsnative.h"

#include <QAbstractNativeEventFilter>

#pragma comment( lib, "Shell32.lib" )

class QWindow;


class QgsWinNativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
  public:
    bool nativeEventFilter( const QByteArray &eventType, void *message, qintptr *result ) override;

  signals:

    void usbStorageNotification( const QString &path, bool inserted );

  private:
    quintptr mLastMessageHash = 0;
};


class NATIVE_EXPORT QgsWinNative : public QgsNative
{
  public:
    Capabilities capabilities() const override;
    void initializeMainWindow( QWindow *window, const QString &applicationName, const QString &organizationName, const QString &version ) override;
    void cleanup() override;
    void openFileExplorerAndSelectFile( const QString &path ) override;
    void showFileProperties( const QString &path ) override;
    NotificationResult showDesktopNotification( const QString &summary, const QString &body, const NotificationSettings &settings = NotificationSettings() ) override;
    bool openTerminalAtPath( const QString &path ) override;

  private:
    QWindow *mWindow = nullptr;
    Capabilities mCapabilities = NativeFilePropertiesDialog | NativeOpenTerminalAtPath;
    bool mWinToastInitialized = false;
    QgsWinNativeEventFilter *mNativeEventFilter = nullptr;
};

#endif // QGSWINNATIVE_H
