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
#include "moc_qgsshortcutsmanager.cpp"
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

void QgsShortcutsManager::registerAllChildren( QObject *object, bool recursive, const QString &section )
{
  registerAllChildActions( object, recursive, section );
  registerAllChildShortcuts( object, recursive, section );
}

void QgsShortcutsManager::registerAllChildActions( QObject *object, bool recursive, const QString &section )
{
  const QList<QObject *> children = object->children();
  for ( QObject *child : children )
  {
    if ( QAction *a = qobject_cast<QAction *>( child ) )
    {
      registerAction( a, a->shortcut().toString( QKeySequence::NativeText ), section );
    }
    else if ( recursive )
    {
      const QString newSection = child->objectName().isEmpty() ? section : section + child->objectName() + "/";
      registerAllChildActions( child, recursive, newSection );
    }
  }
}

void QgsShortcutsManager::registerAllChildShortcuts( QObject *object, bool recursive, const QString &section )
{
  const QList<QObject *> children = object->children();
  for ( QObject *child : children )
  {
    if ( QShortcut *s = qobject_cast<QShortcut *>( child ) )
    {
      registerShortcut( s, s->key().toString( QKeySequence::NativeText ), section );
    }
    else if ( recursive )
    {
      const QString newSection = child->objectName().isEmpty() ? section : section + child->objectName() + "/";
      registerAllChildShortcuts( child, recursive, newSection );
    }
  }
}

bool QgsShortcutsManager::registerAction( QAction *action, const QString &defaultSequence, const QString &section )
{
  if ( qobject_cast<QWidgetAction *>( action ) )
    return false;

  if ( mActions.contains( action ) )
    return false; // already registered

  // if using a debug build, warn on duplicate or non-compliant actions
  if ( action->text().isEmpty() && action->objectName().isEmpty() )
  {
#ifdef QGISDEBUG
    QgsLogger::warning( QStringLiteral( "Action has no text set: %1" ).arg( action->objectName() ) );
#endif
    return false;
  }

  QString key = action->objectName().isEmpty() ? action->text() : action->objectName();
  key.remove( '&' ); // remove the accelerator

#ifdef QGISDEBUG
  if ( actionByName( key ) || shortcutByName( key ) )
    QgsLogger::warning( QStringLiteral( "Duplicate shortcut registered: %1" ).arg( key ) );
#endif

  const QString settingKey = mSettingsPath + section + key;

  mActions.insert( action, { defaultSequence, settingKey } );
  connect( action, &QObject::destroyed, this, [action, this]() { actionDestroyed( action ); } );

  // load overridden value from settings
  const QgsSettings settings;
  const QString sequence = settings.value( settingKey, defaultSequence ).toString();

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

bool QgsShortcutsManager::registerShortcut( QShortcut *shortcut, const QString &defaultSequence, const QString &section )
{
#ifdef QGISDEBUG
  // if using a debug build, warn on duplicate or non-compliant actions
  if ( shortcut->objectName().isEmpty() )
    QgsLogger::warning( QStringLiteral( "Shortcut has no object name set: %1" ).arg( shortcut->key().toString() ) );
  else if ( actionByName( shortcut->objectName() ) || shortcutByName( shortcut->objectName() ) )
    QgsLogger::warning( QStringLiteral( "Duplicate shortcut registered: %1" ).arg( shortcut->objectName() ) );
#endif

  const QString settingKey = mSettingsPath + section + shortcut->objectName();

  mShortcuts.insert( shortcut, { defaultSequence, settingKey } );
  connect( shortcut, &QObject::destroyed, this, [shortcut, this]() { shortcutDestroyed( shortcut ); } );

  // load overridden value from settings
  const QgsSettings settings;
  const QString keySequence = settings.value( settingKey, defaultSequence ).toString();

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
  QList<QObject *> list;
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
  if ( QAction *action = qobject_cast<QAction *>( object ) )
    return defaultKeySequence( action );
  else if ( QShortcut *shortcut = qobject_cast<QShortcut *>( object ) )
    return defaultKeySequence( shortcut );
  else
    return QString();
}

QString QgsShortcutsManager::defaultKeySequence( QAction *action ) const
{
  return mActions.value( action ).first;
}

QString QgsShortcutsManager::defaultKeySequence( QShortcut *shortcut ) const
{
  return mShortcuts.value( shortcut ).first;
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
  if ( QAction *action = qobject_cast<QAction *>( object ) )
    return setKeySequence( action, sequence );
  else if ( QShortcut *shortcut = qobject_cast<QShortcut *>( object ) )
    return setKeySequence( shortcut, sequence );
  else
    return false;
}

bool QgsShortcutsManager::setKeySequence( QAction *action, const QString &sequence )
{
  if ( !mActions.contains( action ) )
  {
    return false;
  }
  action->setShortcut( sequence );
  this->updateActionToolTip( action, sequence );

  const QString settingKey = mActions[action].second;

  // save to settings
  QgsSettings settings;
  settings.setValue( settingKey, sequence );
  return true;
}

bool QgsShortcutsManager::setKeySequence( QShortcut *shortcut, const QString &sequence )
{
  if ( !mShortcuts.contains( shortcut ) )
  {
    return false;
  }
  shortcut->setKey( sequence );

  const QString settingKey = mShortcuts[shortcut].second;

  // save to settings
  QgsSettings settings;
  settings.setValue( settingKey, sequence );
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
    if ( it.key()->objectName() == name )
      return it.key();
  }
  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    QString key = it.key()->text();
    key.remove( '&' ); // remove the accelerator
    if ( key == name )
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

void QgsShortcutsManager::actionDestroyed( QAction *action )
{
  mActions.remove( action );
}

QString QgsShortcutsManager::objectSettingKey( QObject *object ) const
{
  if ( auto action = qobject_cast<QAction *>( object ) )
  {
    return mActions.value( action ).second;
  }
  else if ( auto shortcut = qobject_cast<QShortcut *>( object ) )
  {
    return mShortcuts.value( shortcut ).second;
  }
  return QString();
}

QObject *QgsShortcutsManager::objectForSettingKey( const QString &settingKey ) const
{
  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    if ( it.value().second == settingKey )
      return it.key();
  }
  for ( ShortcutsHash::const_iterator it = mShortcuts.constBegin(); it != mShortcuts.constEnd(); ++it )
  {
    if ( it.value().second == settingKey )
      return it.key();
  }
  return nullptr;
}

void QgsShortcutsManager::shortcutDestroyed( QShortcut *shortcut )
{
  mShortcuts.remove( shortcut );
}

void QgsShortcutsManager::updateActionToolTip( QAction *action, const QString &sequence )
{
  QString current = action->toolTip();
  const thread_local QRegularExpression rx( QStringLiteral( "\\((.*)\\)" ) );
  // Look for the last occurrence of text inside parentheses
  QRegularExpressionMatch match;
  if ( current.lastIndexOf( rx, -1, &match ) != -1 )
  {
    // Check if it is a valid QKeySequence
    const QStringList parts = QKeySequence( match.captured( 1 ) ).toString().split( "," );
    if ( std::all_of( parts.constBegin(), parts.constEnd(), []( const QString &part ) { return !part.trimmed().isEmpty(); } ) )
    {
      current = current.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    }
  }

  if ( !sequence.isEmpty() )
  {
    action->setToolTip( current + " (" + sequence + ")" );
  }
  else
  {
    action->setToolTip( current );
  }
}
