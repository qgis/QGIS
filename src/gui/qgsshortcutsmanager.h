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

class QShortcut;

/** \ingroup gui
 * \class QgsShortcutsManager
 * Shortcuts manager is a class that contains a list of QActions and QShortcuts
  that have been registered and their shortcuts can be changed.
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsShortcutsManager : public QObject
{
    Q_OBJECT

  public:

    //! Return the singleton instance of the manager.
    static QgsShortcutsManager* instance();

    /** Constructor for QgsShortcutsManager.
     * @param parent parent object
     * @param settingsRoot root QSettings path for storing settings, eg "/myplugin/shortcuts". Leave
     * as the default value to store settings alongside built in QGIS shortcuts, but care must be
     * taken to not register actions which conflict with the built in QGIS actions.
     */
    QgsShortcutsManager( QObject *parent = nullptr, const QString& settingsRoot = "/shortcuts/" );

    /** Automatically registers all QActions and QShortcuts which are children of the
     * passed object.
     * @param object parent object containing actions and shortcuts to register
     * @param recursive set to true to recursively add child actions and shortcuts
     * @see registerAllChildActions()
     * @see registerAllChildShortcuts()
     */
    void registerAllChildren( QObject* object, bool recursive = false );

    /** Automatically registers all QActions which are children of the passed object.
     * @param object parent object containing actions to register
     * @param recursive set to true to recursively add child actions
     * @see registerAction()
     * @see registerAllChildren()
     * @see registerAllChildShortcuts()
     */
    void registerAllChildActions( QObject* object, bool recursive = false );

    /** Automatically registers all QShortcuts which are children of the passed object.
     * @param object parent object containing shortcuts to register
     * @param recursive set to true to recursively add child shortcuts
     * @see registerShortcut()
     * @see registerAllChildren()
     * @see registerAllChildActions()
     */
    void registerAllChildShortcuts( QObject* object, bool recursive = false );

    /** Registers an action with the manager so the shortcut can be configured in GUI.
     * @param action action to register. The action must have a unique text string for
     * identification.
     * @param defaultShortcut default key sequence for action
     * @returns true if action was successfully registered
     * @see registerShortcut()
     * @see unregisterAction()
     * @see registerAllChildActions()
     */
    bool registerAction( QAction *action, const QString &defaultShortcut = QString() );

    /** Registers a QShortcut with the manager so the shortcut can be configured in GUI.
    * @param shortcut QShortcut to register. The shortcut must have a unique QObject::objectName() for
    * identification.
    * @param defaultSequence default key sequence for shortcut
    * @returns true if shortcut was successfully registered
    * @see registerAction()
    * @see registerAllChildShortcuts()
    */
    bool registerShortcut( QShortcut* shortcut, const QString& defaultSequence = QString() );

    /** Removes an action from the manager.
     * @param action action to remove
     * @returns true if action was previously registered in manager and has been removed, or
     * false if action was not previously registered in manager
     * @see registerAction()
     * @see unregisterShortcut()
     */
    bool unregisterAction( QAction* action );

    /** Removes a shortcut from the manager.
     * @param shortcut shortcut to remove
     * @returns true if shortcut was previously registered in manager and has been removed, or
     * false if shortcut was not previously registered in manager
     * @see registerShortcut()
     * @see unregisterAction()
     */
    bool unregisterShortcut( QShortcut* shortcut );

    /** Returns a list of all actions in the manager.
     * @see listShortcuts()
     * @see listAll()
     */
    QList<QAction*> listActions() const;

    /** Returns a list of shortcuts in the manager.
     * @see listActions()
     * @see listAll()
     */
    QList<QShortcut*> listShortcuts() const;

    /** Returns a list of both actions and shortcuts in the manager.
     * @see listAction()
     * @see listShortcuts()
     */
    QList<QObject*> listAll() const;

    /** Returns the default sequence for an object (either a QAction or QShortcut).
     * An empty return string indicates no shortcut.
     * @param object QAction or QShortcut to return default key sequence for
     * @see defaultKeySequence()
     */
    QString objectDefaultKeySequence( QObject* object ) const;

    /** Returns the default sequence for an action. An empty return string indicates
     * no default sequence.
     * @param action action to return default key sequence for
     * @see objectDefaultKeySequence()
     */
    QString defaultKeySequence( QAction* action ) const;

    /** Returns the default sequence for a shortcut. An empty return string indicates
     * no default sequence.
     * @param shortcut shortcut to return default key sequence for
     * @see objectDefaultKeySequence()
     */
    QString defaultKeySequence( QShortcut* shortcut ) const;

    /** Modifies an action or shortcut's key sequence.
     * @param name name of action or shortcut to modify. Must match the action's QAction::text() or the
     * shortcut's QObject::objectName()
     * @param sequence new shortcut key sequence
     * @see setObjectKeySequence()
     */
    bool setKeySequence( const QString& name, const QString& sequence );

    /** Modifies an object's (either a QAction or a QShortcut) key sequence.
     * @param object QAction or QShortcut to modify
     * @param sequence new shortcut key sequence
     * @see setKeySequence()
     */
    bool setObjectKeySequence( QObject* object, const QString& sequence );

    /** Modifies an action's key sequence.
     * @param action action to modify
     * @param sequence new shortcut key sequence
     * @see setObjectKeySequence()
     */
    bool setKeySequence( QAction* action, const QString& sequence );

    /** Modifies a shortcuts's key sequence.
     * @param shortcut QShortcut to modify
     * @param sequence new shortcut key sequence
     * @see setObjectKeySequence()
     */
    bool setKeySequence( QShortcut* shortcut, const QString& sequence );

    /** Returns the object (QAction or QShortcut) matching the specified key sequence,
     * @param sequence key sequence to find
     * @returns object with matching sequence, or nullptr if not found
     * @see actionForSequence()
     * @see shortcutForSequence()
     */
    QObject* objectForSequence( const QKeySequence& sequence ) const;

    /** Returns the action which is associated for a shortcut sequence, or nullptr if no action is associated.
     * @param sequence shortcut key sequence
     * @see objectForSequence()
     * @see shortcutForSequence()
     */
    QAction* actionForSequence( const QKeySequence& sequence ) const;

    /** Returns the shortcut which is associated for a key sequence, or nullptr if no shortcut is associated.
     * @param sequence shortcut key sequence
     * @see objectForSequence()
     * @see actionForSequence()
     */
    QShortcut* shortcutForSequence( const QKeySequence& sequence ) const;

    /** Returns an action by its name, or nullptr if nothing found.
     * @param name action name. Must match QAction's text.
     * @see shortcutByName()
     */
    QAction* actionByName( const QString& name ) const;

    /** Returns a shortcut by its name, or nullptr if nothing found
     * @param name shortcut name. Must match QShortcut's QObject::objectName() property.
     * @see actionByName()
     */
    QShortcut* shortcutByName( const QString& name ) const;

    //! Returns the root settings path used to store shortcut customisation.
    QString settingsPath() const { return mSettingsPath; }

  private slots:

    void actionDestroyed();
    void shortcutDestroyed();

  private:

    typedef QHash< QAction*, QString > ActionsHash;
    typedef QHash< QShortcut*, QString > ShortcutsHash;

    ActionsHash mActions;
    ShortcutsHash mShortcuts;
    QString mSettingsPath;
    static QgsShortcutsManager* mInstance;
};

#endif // QGSSHORTCUTSMANAGER_H
