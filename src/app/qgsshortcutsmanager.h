/***************************************************************************
    qgsshortcutsmanager.h
    ---------------------
    begin                : May 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSHORTCUTSMANAGER_H
#define QGSSHORTCUTSMANAGER_H

#include <QHash>
#include <QList>
#include <QAction>

/**
  Shortcuts manager is a singleton class that contains a list of actions from main window
  that have been registered and their shortcut can be changed.
  */
class QgsShortcutsManager
{
  public:

    //! return instance of the manager
    static QgsShortcutsManager* instance();

    //! register all actions which are children of the passed object
    void registerAllChildrenActions( QObject* object );

    //! add action to the manager so the shortcut can be changed in GUI
    bool registerAction( QAction* action, QString defaultShortcut = QString() );

    //! remove action from the manager
    bool unregisterAction( QAction* action );

    //! get list of actions in the manager
    QList<QAction*> listActions();

    //! return default shortcut for action. Empty string means no shortcut
    QString actionDefaultShortcut( QAction* action );

    //! modify action's shortcut
    bool setActionShortcut( QAction* action, QString shortcut );

    //! return action which is associated for the shortcut, NULL if no action is associated
    QAction* actionForShortcut( QKeySequence s );

    // return action by it's name. NULL if nothing found
    QAction* actionByName( QString name );

  protected:
    QgsShortcutsManager();

    typedef QHash<QAction*, QString> ActionsHash;

    ActionsHash mActions;
    static QgsShortcutsManager* mInstance;
};

#endif // QGSSHORTCUTSMANAGER_H
