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
#include "qgsapplication.h"

#include <QShortcut>
#include <QRegularExpression>
#include <QWidgetAction>
#include <QPalette>

QgsShortcutsManager::QgsShortcutsManager( QObject *parent, const QString &settingsRoot )
  : QObject( parent )
  , mSettingsPath( settingsRoot )
{
  // Register common actions
  auto registerCommonAction = [this]( CommonAction commonAction, const QIcon &icon, const QString &text, const QString &toolTip, const QString &sequence, const QString &objectName, const QString &section ) {
    QAction *action = new QAction( icon, text, this );
    action->setToolTip( toolTip );
    setObjectName( objectName );
    // We do not want these actions to be enabled, they are just there to be able to change
    // the shortcuts in the Shortcuts Manager.
    action->setEnabled( false );
    action->setProperty( "commonAction", static_cast< int >( commonAction ) );
    registerAction( action, sequence, section );
    mCommonActions.insert( static_cast< int >( commonAction ), action );
  };
  registerCommonAction( CommonAction::CodeToggleComment, QgsApplication::getThemeIcon( QStringLiteral( "console/iconCommentEditorConsole.svg" ), QgsApplication::palette().color( QPalette::ColorRole::WindowText ) ), tr( "Toggle Comment" ), tr( "Toggle comment" ), QStringLiteral( "Ctrl+/" ), QStringLiteral( "mEditorToggleComment" ), QStringLiteral( "Editor" ) );
  registerCommonAction( CommonAction::CodeReformat, QgsApplication::getThemeIcon( QStringLiteral( "console/iconFormatCode.svg" ) ), tr( "Reformat Code" ), tr( "Reformat code" ), QStringLiteral( "Ctrl+Alt+F" ), QStringLiteral( "mEditorReformatCode" ), QStringLiteral( "Editor" ) );
  registerCommonAction( CommonAction::CodeRunScript, QgsApplication::getThemeIcon( QStringLiteral( "mActionStart.svg" ) ), tr( "Run Script" ), tr( "Run entire script" ), QStringLiteral( "Ctrl+Shift+E" ), QStringLiteral( "mEditorRunScript" ), QStringLiteral( "Editor" ) );
  registerCommonAction( CommonAction::CodeRunSelection, QgsApplication::getThemeIcon( QStringLiteral( "mActionRunSelected.svg" ) ), tr( "Run Selection" ), tr( "Run selected part of script" ), QStringLiteral( "Ctrl+E" ), QStringLiteral( "mEditorRunSelection" ), QStringLiteral( "Editor" ) );
}

QgsShortcutsManager::~QgsShortcutsManager()
{
  // delete all common actions BEFORE this object is destroyed -- they have a lambda connection which
  // we do NOT want to be triggered during the qt child object cleanup which will occur after this destructor
  const QHash< int, QAction * > commonActionsToCleanup = std::move( mCommonActions );
  for ( auto it = commonActionsToCleanup.constBegin(); it != commonActionsToCleanup.constEnd(); ++it )
  {
    delete it.value();
  }
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

  const QString settingKey = mSettingsPath + ( section.isEmpty() || section.endsWith( QLatin1Char( '/' ) ) ? section : section + QStringLiteral( "/" ) ) + key;

  mActions.insert( action, { defaultSequence, settingKey } );
  connect( action, &QObject::destroyed, this, [action, this]() { actionDestroyed( action ); } );

  // load overridden value from settings
  const QgsSettings settings;
  const QString sequence = settings.value( settingKey, defaultSequence ).toString();

  action->setShortcut( sequence );
  if ( !action->toolTip().isEmpty() )
  {
    action->setToolTip( formatActionToolTip( action->toolTip() ) );
    updateActionToolTip( action, sequence );
  }

  return true;
}

void QgsShortcutsManager::initializeCommonAction( QAction *action, CommonAction commonAction )
{
  const auto it = mCommonActions.constFind( static_cast< int >( commonAction ) );
  if ( it == mCommonActions.constEnd() )
    return;

  // copy properties from common action
  action->setText( it.value()->text() );
  action->setToolTip( it.value()->toolTip() );
  action->setShortcut( it.value()->shortcut() );

  mLinkedCommonActions.insert( action, commonAction );
  connect( action, &QObject::destroyed, this, [action, this]() { actionDestroyed( action ); } );
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

  const QString settingKey = mSettingsPath + ( section.isEmpty() || section.endsWith( QLatin1Char( '/' ) ) ? section : section + QStringLiteral( "/" ) ) + shortcut->objectName();

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

  if ( action->property( "commonAction" ).isValid() )
  {
    // if the key sequence for a common action is changed, update all QActions currently linked
    // to that common action
    const CommonAction commonAction = static_cast< CommonAction >( action->property( "commonAction" ).toInt() );
    for ( auto it = mLinkedCommonActions.constBegin(); it != mLinkedCommonActions.constEnd(); ++it )
    {
      if ( it.value() == commonAction )
      {
        it.key()->setShortcut( action->shortcut() );
        it.key()->setToolTip( action->toolTip() );
      }
    }
  }

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

QKeySequence QgsShortcutsManager::sequenceForCommonAction( CommonAction action ) const
{
  const auto it = mCommonActions.constFind( static_cast< int >( action ) );
  if ( it == mCommonActions.constEnd() )
    return QKeySequence();

  return it.value()->shortcut();
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
  mLinkedCommonActions.remove( action );
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

QString QgsShortcutsManager::formatActionToolTip( const QString &toolTip )
{
  if ( toolTip.isEmpty() )
    return QString();

  const QStringList parts = toolTip.split( '\n' );
  QString formatted = QStringLiteral( "<b>%1</b>" ).arg( parts.at( 0 ) );
  if ( parts.count() > 1 )
  {
    for ( int i = 1; i < parts.count(); ++i )
      formatted += QStringLiteral( "<p>%1</p>" ).arg( parts.at( i ) );
  }

  return formatted;
}

void QgsShortcutsManager::updateActionToolTip( QAction *action, const QString &sequence )
{
  QString current = action->toolTip();
  const thread_local QRegularExpression rx( QStringLiteral( "\\s*\\((.*)\\)" ) );
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
