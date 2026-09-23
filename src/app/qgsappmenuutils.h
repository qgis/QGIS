/***************************************************************************
                             qgsappmenuutils.h
                             ------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAPPMENUUTILS_H
#define QGSAPPMENUUTILS_H

#include "qgis.h"

class QMenu;

class QgsAppMenuUtils
{
  public:
    //! Find the QMenu with the given name within the a parent menu (ie the user visible text on the menu item)
    static QMenu *getSubMenu( QMenu *parentMenu, const QString &menuName );

    static QString normalizedMenuName( const QString &name );
};

#endif // QGSAPPMENUUTILS_H
