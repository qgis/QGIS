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

#include <QList>

class QMenu;
class QAction;
class QgisApp;
class QgsProcessingAlgorithm;
class QMenuBar;

class QgsAppMenuUtils
{
  public:
    /**
     * Find a top level menu with matching name.
     *
     * Name matching is done in a case-insensitive way, ignoring special characters like '&'.
     *
     * If no matching menu exists, a new one will be created.
     */
    static QMenu *getMenu( QMenuBar *menuBar, const QString &menuName );

    /**
     * Find the QMenu with the given name within the a parent menu (ie the user visible text on the menu item).
     *
     * Name matching is done in a case-insensitive way, ignoring special characters like '&'
     *
     * If no matching menu exists, a new one will be created.
     */
    static QMenu *getSubMenu( QMenu *parentMenu, const QString &menuName );

    static QString normalizedMenuName( const QString &name );

    static void insertActionAlphabeticallyToMenu( QMenu *menu, QAction *action, bool insertAfterSubMenus = false );
    static void insertSubmenuAlphabeticallyToMenu( QMenu *menu, QMenu *subMenu, bool insertMenusOnTop = false );
};

#endif // QGSAPPMENUUTILS_H
