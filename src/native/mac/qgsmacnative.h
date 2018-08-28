/***************************************************************************
    qgsmacnative.h - abstracted interface to native Mac objective-c
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

#ifndef QGSMACNATIVE_H
#define QGSMACNATIVE_H

#include "qgsnative.h"

class QString;


class NATIVE_EXPORT QgsMacNative : public QgsNative
{
  public:
    explicit QgsMacNative( const QString &iconPath = QString() );
    ~QgsMacNative() override;

    virtual const char *currentAppLocalizedName();
    void currentAppActivateIgnoringOtherApps() override;
    void openFileExplorerAndSelectFile( const QString &path ) override;
    QgsNative::Capabilities capabilities() const override;
    QgsNative::NotificationResult showDesktopNotification( const QString &summary, const QString &body, const NotificationSettings &settings ) override;

  private:
    class QgsUserNotificationCenter;
    QgsUserNotificationCenter *mQgsUserNotificationCenter = nullptr;

};


#endif // QGSMACNATIVE_H
