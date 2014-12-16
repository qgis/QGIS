/***************************************************************************
    qgsmacappkit.mm - interface to Mac objective-c AppKit.framework
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

#include "qgsmacappkit.h"

#include <Cocoa/Cocoa.h>

class QgsNSRunningApplication::Private
{
  public:
//    NSObject *obj;
};

QgsNSRunningApplication::QgsNSRunningApplication()
{
//  d = new Private;
//  d->obj = [NSObject someFunction];
}

QgsNSRunningApplication::~QgsNSRunningApplication()
{
//  [d->obj release];
//  delete d;
//  d = 0;
}

const char* QgsNSRunningApplication::currentAppLocalizedName()
{
  return [[[NSRunningApplication currentApplication] localizedName] UTF8String];
}

void QgsNSRunningApplication::currentAppActivateIgnoringOtherApps()
{
  // valid for Mac OS X >= 10.6
  [[NSRunningApplication currentApplication] activateWithOptions:
    (NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
}
