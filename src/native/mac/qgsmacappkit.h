/***************************************************************************
    qgsmacappkit.h - interface to Mac objective-c AppKit.framework
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

#ifndef QGSMACAPPKIT_H
#define QGSMACAPPKIT_H

#include "qgsmacnative.h"

class QgsNSRunningApplication : public QgsMacAppKit
{
  public:
    QgsNSRunningApplication();
    ~QgsNSRunningApplication();

    const char* currentAppLocalizedName();
    void currentAppActivateIgnoringOtherApps();

  private:
    class Private;
    Private* d;
};

#endif // QGSMACAPPKIT_H
