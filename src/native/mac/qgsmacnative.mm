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
  mQgsUserNotificationCenter->_qgisIcon = [[NSImage alloc] initWithCGImage:QPixmap( iconPath ).toImage().toCGImage() size:NSZeroSize];
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
    image = [[NSImage alloc] initWithCGImage:px.toImage().toCGImage() size:NSZeroSize];
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
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101400
  // Version comparison needs to be numeric, in case __MAC_10_10_4 is not defined, e.g. some pre-10.14 SDKs
  // See: https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/cross_development/Using/using.html
  //      Section "Conditionally Compiling for Different SDKs"
  if ( [NSApp respondsToSelector:@selector( effectiveAppearance )] )
  {
    // compiled on macos 10.14+ AND running on macos 10.14+
    // check the settings of effective appearance of the user
    NSAppearanceName appearanceName = [NSApp.effectiveAppearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameAqua, NSAppearanceNameDarkAqua]];
    return ( [appearanceName isEqualToString:NSAppearanceNameDarkAqua] );
  }
  else
  {
    // compiled on macos 10.14+ BUT running on macos 10.13-
    // DarkTheme was introduced in MacOS 10.14, fallback to light theme
    return false;
  }
#endif
#endif
  // compiled on macos 10.13-
  // NSAppearanceNameDarkAqua is not in SDK headers
  // fallback to light theme
  return false;
}
