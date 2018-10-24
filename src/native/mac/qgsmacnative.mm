/***************************************************************************
    qgsmacnative.cpp - abstracted interface to native Mac objective-c
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

#include "qgsmacnative.h"

#include <Cocoa/Cocoa.h>
#include <QtMacExtras/QtMac>

#include <QString>
#include <QPixmap>


@interface QgsUserNotificationCenterDelegate : NSObject <NSUserNotificationCenterDelegate>
@end

@implementation QgsUserNotificationCenterDelegate

- ( BOOL )userNotificationCenter:( NSUserNotificationCenter * )center shouldPresentNotification:( NSUserNotification * )notification
{
#pragma unused (notification)
#pragma unused (center)
  return YES;
}

@end

class QgsMacNative::QgsUserNotificationCenter
{
  public:
    QgsUserNotificationCenterDelegate *_qgsUserNotificationCenter;
    NSImage *_qgisIcon;
};

QgsMacNative::QgsMacNative()
  : mQgsUserNotificationCenter( new QgsMacNative::QgsUserNotificationCenter() )
{
  mQgsUserNotificationCenter->_qgsUserNotificationCenter = [[QgsUserNotificationCenterDelegate alloc] init];
  [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate: mQgsUserNotificationCenter->_qgsUserNotificationCenter];
}

QgsMacNative::~QgsMacNative()
{
  [mQgsUserNotificationCenter->_qgsUserNotificationCenter dealloc];
  delete mQgsUserNotificationCenter;
}

void QgsMacNative::setIconPath( const QString &iconPath )
{
  mQgsUserNotificationCenter->_qgisIcon = QtMac::toNSImage( QPixmap( iconPath ) );
}

const char *QgsMacNative::currentAppLocalizedName()
{
  return [[[NSRunningApplication currentApplication] localizedName] UTF8String];
}

void QgsMacNative::currentAppActivateIgnoringOtherApps()
{
  [[NSRunningApplication currentApplication] activateWithOptions:
                                             ( NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps )];
}

void QgsMacNative::openFileExplorerAndSelectFile( const QString &path )
{
  NSString *pathStr = [[NSString alloc] initWithUTF8String:path.toUtf8().constData()];
  NSArray *fileURLs = [NSArray arrayWithObjects:[NSURL fileURLWithPath:pathStr], nil];
  [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
}

QgsNative::Capabilities QgsMacNative::capabilities() const
{
  return NativeDesktopNotifications;
}

QgsNative::NotificationResult QgsMacNative::showDesktopNotification( const QString &summary,
    const QString &body,
    const QgsNative::NotificationSettings &settings )
{
  NSUserNotification *notification = [[NSUserNotification alloc] init];
  notification.title = summary.toNSString();
  notification.informativeText = body.toNSString();
  notification.soundName = NSUserNotificationDefaultSoundName;   //Will play a default sound
  const QPixmap px = QPixmap::fromImage( settings.image );
  NSImage *image = nil;
  if ( settings.image.isNull() )
  {
    // image application (qgis.icns) seems not to be set for now, although present in the plist
    // whenever fixed, try following line (and remove corresponding code in QgsMacNative::QgsUserNotificationCenter)
    // image = [[NSImage imageNamed:@"NSApplicationIcon"] retain]
    image = mQgsUserNotificationCenter->_qgisIcon;
  }
  else
  {
    image = QtMac::toNSImage( px );
  }
  notification.contentImage = image;

  [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification: notification];
  [notification autorelease];

  //[userCenterDelegate dealloc];

  NotificationResult result;
  result.successful = true;
  return result;
}

bool QgsMacNative::hasDarkTheme()
{
  return ( NSApp.effectiveAppearance.name != NSAppearanceNameAqua );
}
