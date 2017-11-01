/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsshortcutsmanager.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsShortcutsManager : QObject
{
%Docstring
 Shortcuts manager is a class that contains a list of QActions and QShortcuts
 that have been registered and their shortcuts can be changed.

 QgsShortcutsManager is not usually directly created, but rather accessed through
 QgsGui.shortcutsManager().
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsshortcutsmanager.h"
%End
  public:

    QgsShortcutsManager( QObject *parent /TransferThis/ = 0, const QString &settingsRoot = "/shortcuts/" );
%Docstring
 Constructor for QgsShortcutsManager.
 \param parent parent object
 \param settingsRoot root QgsSettings path for storing settings, e.g., "/myplugin/shortcuts". Leave
 as the default value to store settings alongside built in QGIS shortcuts, but care must be
 taken to not register actions which conflict with the built in QGIS actions.
%End

    void registerAllChildren( QObject *object, bool recursive = false );
%Docstring
 Automatically registers all QActions and QShortcuts which are children of the
 passed object.
 \param object parent object containing actions and shortcuts to register
 \param recursive set to true to recursively add child actions and shortcuts
.. seealso:: registerAllChildActions()
.. seealso:: registerAllChildShortcuts()
%End

    void registerAllChildActions( QObject *object, bool recursive = false );
%Docstring
 Automatically registers all QActions which are children of the passed object.
 \param object parent object containing actions to register
 \param recursive set to true to recursively add child actions
.. seealso:: registerAction()
.. seealso:: registerAllChildren()
.. seealso:: registerAllChildShortcuts()
%End

    void registerAllChildShortcuts( QObject *object, bool recursive = false );
%Docstring
 Automatically registers all QShortcuts which are children of the passed object.
 \param object parent object containing shortcuts to register
 \param recursive set to true to recursively add child shortcuts
.. seealso:: registerShortcut()
.. seealso:: registerAllChildren()
.. seealso:: registerAllChildActions()
%End

    bool registerAction( QAction *action, const QString &defaultShortcut = QString() );
%Docstring
 Registers an action with the manager so the shortcut can be configured in GUI.
 \param action action to register. The action must have a unique text string for
 identification.
 \param defaultShortcut default key sequence for action
 :return: true if action was successfully registered
.. seealso:: registerShortcut()
.. seealso:: unregisterAction()
.. seealso:: registerAllChildActions()
 :rtype: bool
%End

    bool registerShortcut( QShortcut *shortcut, const QString &defaultSequence = QString() );
%Docstring
 Registers a QShortcut with the manager so the shortcut can be configured in GUI.
 \param shortcut QShortcut to register. The shortcut must have a unique QObject.objectName() for
 identification.
 \param defaultSequence default key sequence for shortcut
 :return: true if shortcut was successfully registered
.. seealso:: registerAction()
.. seealso:: registerAllChildShortcuts()
 :rtype: bool
%End

    bool unregisterAction( QAction *action );
%Docstring
 Removes an action from the manager.
 \param action action to remove
 :return: true if action was previously registered in manager and has been removed, or
 false if action was not previously registered in manager
.. seealso:: registerAction()
.. seealso:: unregisterShortcut()
 :rtype: bool
%End

    bool unregisterShortcut( QShortcut *shortcut );
%Docstring
 Removes a shortcut from the manager.
 \param shortcut shortcut to remove
 :return: true if shortcut was previously registered in manager and has been removed, or
 false if shortcut was not previously registered in manager
.. seealso:: registerShortcut()
.. seealso:: unregisterAction()
 :rtype: bool
%End

    QList<QAction *> listActions() const;
%Docstring
 Returns a list of all actions in the manager.
.. seealso:: listShortcuts()
.. seealso:: listAll()
 :rtype: list of QAction
%End

    QList<QShortcut *> listShortcuts() const;
%Docstring
 Returns a list of shortcuts in the manager.
.. seealso:: listActions()
.. seealso:: listAll()
 :rtype: list of QShortcut
%End

    QList<QObject *> listAll() const;
%Docstring
 Returns a list of both actions and shortcuts in the manager.
.. seealso:: listAction()
.. seealso:: listShortcuts()
 :rtype: list of QObject
%End

    QString objectDefaultKeySequence( QObject *object ) const;
%Docstring
 Returns the default sequence for an object (either a QAction or QShortcut).
 An empty return string indicates no shortcut.
 \param object QAction or QShortcut to return default key sequence for
.. seealso:: defaultKeySequence()
 :rtype: str
%End

    QString defaultKeySequence( QAction *action ) const;
%Docstring
 Returns the default sequence for an action. An empty return string indicates
 no default sequence.
 \param action action to return default key sequence for
.. seealso:: objectDefaultKeySequence()
 :rtype: str
%End

    QString defaultKeySequence( QShortcut *shortcut ) const;
%Docstring
 Returns the default sequence for a shortcut. An empty return string indicates
 no default sequence.
 \param shortcut shortcut to return default key sequence for
.. seealso:: objectDefaultKeySequence()
 :rtype: str
%End

    bool setKeySequence( const QString &name, const QString &sequence );
%Docstring
 Modifies an action or shortcut's key sequence.
 \param name name of action or shortcut to modify. Must match the action's QAction.text() or the
 shortcut's QObject.objectName()
 \param sequence new shortcut key sequence
.. seealso:: setObjectKeySequence()
 :rtype: bool
%End

    bool setObjectKeySequence( QObject *object, const QString &sequence );
%Docstring
 Modifies an object's (either a QAction or a QShortcut) key sequence.
 \param object QAction or QShortcut to modify
 \param sequence new shortcut key sequence
.. seealso:: setKeySequence()
 :rtype: bool
%End

    bool setKeySequence( QAction *action, const QString &sequence );
%Docstring
 Modifies an action's key sequence.
 \param action action to modify
 \param sequence new shortcut key sequence
.. seealso:: setObjectKeySequence()
 :rtype: bool
%End

    bool setKeySequence( QShortcut *shortcut, const QString &sequence );
%Docstring
 Modifies a shortcuts's key sequence.
 \param shortcut QShortcut to modify
 \param sequence new shortcut key sequence
.. seealso:: setObjectKeySequence()
 :rtype: bool
%End

    QObject *objectForSequence( const QKeySequence &sequence ) const;
%Docstring
 Returns the object (QAction or QShortcut) matching the specified key sequence,
 \param sequence key sequence to find
 :return: object with matching sequence, or None if not found
.. seealso:: actionForSequence()
.. seealso:: shortcutForSequence()
 :rtype: QObject
%End

    QAction *actionForSequence( const QKeySequence &sequence ) const;
%Docstring
 Returns the action which is associated for a shortcut sequence, or None if no action is associated.
 \param sequence shortcut key sequence
.. seealso:: objectForSequence()
.. seealso:: shortcutForSequence()
 :rtype: QAction
%End

    QShortcut *shortcutForSequence( const QKeySequence &sequence ) const;
%Docstring
 Returns the shortcut which is associated for a key sequence, or None if no shortcut is associated.
 \param sequence shortcut key sequence
.. seealso:: objectForSequence()
.. seealso:: actionForSequence()
 :rtype: QShortcut
%End

    QAction *actionByName( const QString &name ) const;
%Docstring
 Returns an action by its name, or None if nothing found.
 \param name action name. Must match QAction's text.
.. seealso:: shortcutByName()
 :rtype: QAction
%End

    QShortcut *shortcutByName( const QString &name ) const;
%Docstring
 Returns a shortcut by its name, or None if nothing found
 \param name shortcut name. Must match QShortcut's QObject.objectName() property.
.. seealso:: actionByName()
 :rtype: QShortcut
%End

    QString settingsPath() const;
%Docstring
Returns the root settings path used to store shortcut customisation.
 :rtype: str
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsshortcutsmanager.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
