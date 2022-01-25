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
#include "qgssettings.h"

#include <QShortcut>
#include <QRegularExpression>
#include <QWidgetAction>

QgsShortcutsManager::QgsShortcutsManager( QObject *parent, const QString &settingsRoot )
  : QObject( parent )
  , mSettingsPath( settingsRoot )
{
}

void QgsShortcutsManager::registerAllChildren( QObject *object, bool recursive )
{
  registerAllChildActions( object, recursive );
  registerAllChildShortcuts( object, recursive );
}

void QgsShortcutsManager::registerAllChildActions( QObject *object, bool recursive )
{
  if ( recursive )
  {
    const QList< QAction * > actions = object->findChildren< QAction * >();
    for ( QAction *a : actions )
    {
      registerAction( a, a->shortcut().toString( QKeySequence::NativeText ) );
    }
  }
  else
  {
    const QList< QObject *> children = object->children();
    for ( QObject *child : children )
    {
      if ( QAction *a = qobject_cast<QAction *>( child ) )
      {
        registerAction( a, a->shortcut().toString( QKeySequence::NativeText ) );
      }
    }
  }
}

void QgsShortcutsManager::registerAllChildShortcuts( QObject *object, bool recursive )
{
  if ( recursive )
  {
    const QList< QShortcut * > shortcuts = object->findChildren< QShortcut * >();
    const auto constShortcuts = shortcuts;
    for ( QShortcut *s : constShortcuts )
    {
      registerShortcut( s, s->key().toString( QKeySequence::NativeText ) );
    }
  }
  else
  {
    const auto constChildren = object->children();
    for ( QObject *child : constChildren )
    {
      if ( QShortcut *s = qobject_cast<QShortcut *>( child ) )
      {
        registerShortcut( s, s->key().toString( QKeySequence::NativeText ) );
      }
    }
  }
}

bool QgsShortcutsManager::registerAction( QAction *action, const QString &defaultSequence )
{
  if ( qobject_cast< QWidgetAction * >( action ) )
    return false;

  if ( mActions.contains( action ) )
    return false; // already registered

#ifdef QGISDEBUG
  // if using a debug build, warn on duplicate or non-compliant actions
  if ( action->text().isEmpty() )
  {
    QgsLogger::warning( QStringLiteral( "Action has no text set: %1" ).arg( action->objectName() ) );
    return false;
  }
  else if ( actionByName( action->text() ) || shortcutByName( action->text() ) )
    QgsLogger::warning( QStringLiteral( "Duplicate shortcut registered: %1" ).arg( action->text() ) );
#endif

  mActions.insert( action, defaultSequence );
  connect( action, &QObject::destroyed, this, &QgsShortcutsManager::actionDestroyed );

  QString actionText = action->text();
  actionText.remove( '&' ); // remove the accelerator

  // load overridden value from settings
  const QgsSettings settings;
  const QString sequence = settings.value( mSettingsPath + actionText, defaultSequence ).toString();

  action->setShortcut( sequence );
  if ( !action->toolTip().isEmpty() )
  {
    const QStringList parts = action->toolTip().split( '\n' );
    QString formatted = QStringLiteral( "<b>%1</b>" ).arg( parts.at( 0 ) );
    if ( parts.count() > 1 )
    {
      for ( int i = 1; i < parts.count(); ++i )
        formatted += QStringLiteral( "<p>%1</p>" ).arg( parts.at( i ) );
    }

    action->setToolTip( formatted );
    updateActionToolTip( action, sequence );
  }

  return true;
}

bool QgsShortcutsManager::registerShortcut( QShortcut *shortcut, const QString &defaultSequence )
{
#ifdef QGISDEBUG
  // if using a debug build, warn on duplicate or non-compliant actions
  if ( shortcut->objectName().isEmpty() )
    QgsLogger::warning( QStringLiteral( "Shortcut has no object name set: %1" ).arg( shortcut->key().toString() ) );
  else if ( actionByName( shortcut->objectName() ) || shortcutByName( shortcut->objectName() ) )
    QgsLogger::warning( QStringLiteral( "Duplicate shortcut registered: %1" ).arg( shortcut->objectName() ) );
#endif

  mShortcuts.insert( shortcut, defaultSequence );
  connect( shortcut, &QObject::destroyed, this, &QgsShortcutsManager::shortcutDestroyed );

  const QString shortcutName = shortcut->objectName();

  // load overridden value from settings
  const QgsSettings settings;
  const QString keySequence = settings.value( mSettingsPath + shortcutName, defaultSequence ).toString();

  shortcut->setKey( keySequence );

  return true;
}

bool QgsShortcutsManager::unregisterAction( QAction *action )
{
  if ( !mActions.contains( action ) )
    return false;

  mActions.remove( action );
  return true;
}

bool QgsShortcutsManager::unregisterShortcut( QShortcut *shortcut )
{
  if ( !mShortcuts.contains( shortcut ) )
    return false;

  mShortcuts.remove( shortcut );
  return true;
}

QList<QAction *> QgsShortcutsManager::listActions() const
{
  return mActions.keys();
}

QList<QShortcut *> QgsShortcutsManager::listShortcuts() const
{
  return mShortcuts.keys();
}

QList<QObject *> QgsShortcutsManager::listAll() const
{
  QList< QObject * > list;
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

QString QgsShortcutsManager::objectDefaultKeySequence( QObject *object ) const
{
  if ( QAction *action = qobject_cast< QAction * >( object ) )
    return defaultKeySequence( action );
  else if ( QShortcut *shortcut = qobject_cast< QShortcut * >( object ) )
    return defaultKeySequence( shortcut );
  else
    return QString();
}

QString QgsShortcutsManager::defaultKeySequence( QAction *action ) const
{
  return mActions.value( action, QString() );
}

QString QgsShortcutsManager::defaultKeySequence( QShortcut *shortcut ) const
{
  return mShortcuts.value( shortcut, QString() );
}

bool QgsShortcutsManager::setKeySequence( const QString &name, const QString &sequence )
{
  if ( QAction *action = actionByName( name ) )
    return setKeySequence( action, sequence );
  else if ( QShortcut *shortcut = shortcutByName( name ) )
    return setKeySequence( shortcut, sequence );
  else
    return false;
}

bool QgsShortcutsManager::setObjectKeySequence( QObject *object, const QString &sequence )
{
  if ( QAction *action = qobject_cast< QAction * >( object ) )
    return setKeySequence( action, sequence );
  else if ( QShortcut *shortcut = qobject_cast< QShortcut * >( object ) )
    return setKeySequence( shortcut, sequence );
  else
    return false;
}

bool QgsShortcutsManager::setKeySequence( QAction *action, const QString &sequence )
{
  action->setShortcut( sequence );
  this->updateActionToolTip( action, sequence );

  QString actionText = action->text();
  actionText.remove( '&' ); // remove the accelerator

  // save to settings
  QgsSettings settings;
  settings.setValue( mSettingsPath + actionText, sequence );
  return true;
}

bool QgsShortcutsManager::setKeySequence( QShortcut *shortcut, const QString &sequence )
{
  shortcut->setKey( sequence );

  const QString shortcutText = shortcut->objectName();

  // save to settings
  QgsSettings settings;
  settings.setValue( mSettingsPath + shortcutText, sequence );
  return true;
}

QObject *QgsShortcutsManager::objectForSequence( const QKeySequence &sequence ) const
{
  if ( QAction *action = actionForSequence( sequence ) )
    return action;
  else if ( QShortcut *shortcut = shortcutForSequence( sequence ) )
    return shortcut;
  else
    return nullptr;
}

QAction *QgsShortcutsManager::actionForSequence( const QKeySequence &sequence ) const
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

QShortcut *QgsShortcutsManager::shortcutForSequence( const QKeySequence &sequence ) const
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

QAction *QgsShortcutsManager::actionByName( const QString &name ) const
{
  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    if ( it.key()->text() == name )
      return it.key();
  }

  return nullptr;
}

QShortcut *QgsShortcutsManager::shortcutByName( const QString &name ) const
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
  mActions.remove( qobject_cast<QAction *>( sender() ) );
}

void QgsShortcutsManager::shortcutDestroyed()
{
  mShortcuts.remove( qobject_cast<QShortcut *>( sender() ) );
}

void QgsShortcutsManager::updateActionToolTip( QAction *action, const QString &sequence )
{
  QString current = action->toolTip();
  // Remove the old shortcut.
  const QRegularExpression rx( QStringLiteral( "\\(.*\\)" ) );
  current.replace( rx, QString() );

  if ( !sequence.isEmpty() )
  {
    action->setToolTip( current + " (" + sequence + ")" );
  }
  else
  {
    action->setToolTip( current );
  }
}
