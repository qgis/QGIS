/***************************************************************************
    qgsshortcutsmanager.cpp
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

#include "qgsshortcutsmanager.h"
#include "qgslogger.h"
#include <QSettings>
#include <QShortcut>

QgsShortcutsManager* QgsShortcutsManager::mInstance = nullptr;


QgsShortcutsManager* QgsShortcutsManager::instance()
{
  if ( !mInstance )
    mInstance = new QgsShortcutsManager( nullptr );
  return mInstance;
}

QgsShortcutsManager::QgsShortcutsManager( QObject *parent, const QString& settingsRoot )
    : QObject( parent )
    , mSettingsPath( settingsRoot )
{
}

void QgsShortcutsManager::registerAllChildren( QObject* object, bool recursive )
{
  registerAllChildActions( object, recursive );
  registerAllChildShortcuts( object, recursive );
}

void QgsShortcutsManager::registerAllChildActions( QObject* object, bool recursive )
{
  if ( recursive )
  {
    QList< QAction* > actions = object->findChildren< QAction* >();
    Q_FOREACH ( QAction* a, actions )
    {
      registerAction( a, a->shortcut() );
    }
  }
  else
  {
    Q_FOREACH ( QObject* child, object->children() )
    {
      if ( QAction* a = qobject_cast<QAction*>( child ) )
      {
        registerAction( a, a->shortcut() );
      }
    }
  }
}

void QgsShortcutsManager::registerAllChildShortcuts( QObject* object, bool recursive )
{
  if ( recursive )
  {
    QList< QShortcut* > shortcuts = object->findChildren< QShortcut* >();
    Q_FOREACH ( QShortcut* s, shortcuts )
    {
      registerShortcut( s, s->key() );
    }
  }
  else
  {
    Q_FOREACH ( QObject* child, object->children() )
    {
      if ( QShortcut* s = qobject_cast<QShortcut*>( child ) )
      {
        registerShortcut( s, s->key() );
      }
    }
  }
}

bool QgsShortcutsManager::registerAction( QAction* action, const QString& defaultSequence )
{
#ifdef QGISDEBUG
  // if using a debug build, warn on duplicate actions
  if ( actionByName( action->text() ) || shortcutByName( action->text() ) )
    QgsLogger::warning( QString( "Duplicate shortcut registered: %1" ).arg( action->text() ) );
#endif

  mActions.insert( action, defaultSequence );
  connect( action, SIGNAL( destroyed() ), this, SLOT( actionDestroyed() ) );

  QString actionText = action->text();
  actionText.remove( '&' ); // remove the accelerator

  // load overridden value from settings
  QSettings settings;
  QString sequence = settings.value( mSettingsPath + actionText, defaultSequence ).toString();

  action->setShortcut( sequence );

  return true;
}

bool QgsShortcutsManager::registerShortcut( QShortcut* shortcut, const QString& defaultSequence )
{
#ifdef QGISDEBUG
  // if using a debug build, warn on duplicate actions
  if ( actionByName( shortcut->objectName() ) || shortcutByName( shortcut->objectName() ) )
    QgsLogger::warning( QString( "Duplicate shortcut registered: %1" ).arg( shortcut->objectName() ) );
#endif

  mShortcuts.insert( shortcut, defaultSequence );
  connect( shortcut, SIGNAL( destroyed() ), this, SLOT( shortcutDestroyed() ) );

  QString shortcutName = shortcut->objectName();

  // load overridden value from settings
  QSettings settings;
  QString keySequence = settings.value( mSettingsPath + shortcutName, defaultSequence ).toString();

  shortcut->setKey( keySequence );

  return true;
}

bool QgsShortcutsManager::unregisterAction( QAction* action )
{
  if ( !mActions.contains( action ) )
    return false;

  mActions.remove( action );
  return true;
}

bool QgsShortcutsManager::unregisterShortcut( QShortcut* shortcut )
{
  if ( !mShortcuts.contains( shortcut ) )
    return false;

  mShortcuts.remove( shortcut );
  return true;
}

QList<QAction*> QgsShortcutsManager::listActions() const
{
  return mActions.keys();
}

QList<QShortcut*> QgsShortcutsManager::listShortcuts() const
{
  return mShortcuts.keys();
}

QList<QObject*> QgsShortcutsManager::listAll() const
{
  QList< QObject* > list;
  ActionsHash::const_iterator actionIt = mActions.constBegin();
  for ( ; actionIt != mActions.constEnd(); ++actionIt )
  {
    list << actionIt.key();
  }
  ShortcutsHash::const_iterator shortcutIt = mShortcuts.constBegin();
  for ( ; shortcutIt != mShortcuts.constEnd(); ++shortcutIt )
  {
    list << shortcutIt.key();
  }
  return list;
}

QString QgsShortcutsManager::objectDefaultKeySequence( QObject* object ) const
{
  if ( QAction* action = qobject_cast< QAction* >( object ) )
    return defaultKeySequence( action );
  else if ( QShortcut* shortcut = qobject_cast< QShortcut* >( object ) )
    return defaultKeySequence( shortcut );
  else
    return QString();
}

QString QgsShortcutsManager::defaultKeySequence( QAction* action ) const
{
  return mActions.value( action, QString() );
}

QString QgsShortcutsManager::defaultKeySequence( QShortcut* shortcut ) const
{
  return mShortcuts.value( shortcut, QString() );
}

bool QgsShortcutsManager::setKeySequence( const QString& name, const QString& sequence )
{
  if ( QAction* action = actionByName( name ) )
    return setKeySequence( action, sequence );
  else if ( QShortcut* shortcut = shortcutByName( name ) )
    return setKeySequence( shortcut, sequence );
  else
    return false;
}

bool QgsShortcutsManager::setObjectKeySequence( QObject* object, const QString& sequence )
{
  if ( QAction* action = qobject_cast< QAction* >( object ) )
    return setKeySequence( action, sequence );
  else if ( QShortcut* shortcut = qobject_cast< QShortcut* >( object ) )
    return setKeySequence( shortcut, sequence );
  else
    return false;
}

bool QgsShortcutsManager::setKeySequence( QAction* action, const QString& sequence )
{
  action->setShortcut( sequence );

  QString actionText = action->text();
  actionText.remove( '&' ); // remove the accelerator

  // save to settings
  QSettings settings;
  settings.setValue( mSettingsPath + actionText, sequence );
  return true;
}

bool QgsShortcutsManager::setKeySequence( QShortcut* shortcut, const QString& sequence )
{
  shortcut->setKey( sequence );

  QString shortcutText = shortcut->objectName();

  // save to settings
  QSettings settings;
  settings.setValue( mSettingsPath + shortcutText, sequence );
  return true;
}

QObject* QgsShortcutsManager::objectForSequence( const QKeySequence& sequence ) const
{
  if ( QAction* action = actionForSequence( sequence ) )
    return action;
  else if ( QShortcut* shortcut = shortcutForSequence( sequence ) )
    return shortcut;
  else
    return nullptr;
}

QAction* QgsShortcutsManager::actionForSequence( const QKeySequence& sequence ) const
{
  if ( sequence.isEmpty() )
    return nullptr;

  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    if ( it.key()->shortcut() == sequence )
      return it.key();
  }

  return nullptr;
}

QShortcut* QgsShortcutsManager::shortcutForSequence( const QKeySequence& sequence ) const
{
  if ( sequence.isEmpty() )
    return nullptr;

  for ( ShortcutsHash::const_iterator it = mShortcuts.constBegin(); it != mShortcuts.constEnd(); ++it )
  {
    if ( it.key()->key() == sequence )
      return it.key();
  }

  return nullptr;
}

QAction* QgsShortcutsManager::actionByName( const QString& name ) const
{
  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    if ( it.key()->text() == name )
      return it.key();
  }

  return nullptr;
}

QShortcut* QgsShortcutsManager::shortcutByName( const QString& name ) const
{
  for ( ShortcutsHash::const_iterator it = mShortcuts.constBegin(); it != mShortcuts.constEnd(); ++it )
  {
    if ( it.key()->objectName() == name )
      return it.key();
  }

  return nullptr;
}

void QgsShortcutsManager::actionDestroyed()
{
  mActions.remove( qobject_cast<QAction*>( sender() ) );
}

void QgsShortcutsManager::shortcutDestroyed()
{
  mShortcuts.remove( qobject_cast<QShortcut*>( sender() ) );
}
