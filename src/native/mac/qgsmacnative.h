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

class QgsMacAppKit
{
  public:
    virtual ~QgsMacAppKit();

    // NSRunningApplication interface
    virtual const char* currentAppLocalizedName() = 0;
    virtual void currentAppActivateIgnoringOtherApps() = 0;
};

#endif // QGSMACNATIVE_H
