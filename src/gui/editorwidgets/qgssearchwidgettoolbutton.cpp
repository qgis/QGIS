/***************************************************************************
     qgssearchwidgettoolbutton.cpp
     -----------------------------
    Date                 : May 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssearchwidgettoolbutton.h"
#include "qgsapplication.h"
#include <QMenu>

QgsSearchWidgetToolButton::QgsSearchWidgetToolButton( QWidget* parent )
    : QToolButton( parent )
    , mAvailableFilterFlags( QgsSearchWidgetWrapper::EqualTo | QgsSearchWidgetWrapper::NotEqualTo | QgsSearchWidgetWrapper::CaseInsensitive )
    , mDefaultFilterFlags( QgsSearchWidgetWrapper::EqualTo )
    , mFilterFlags( QgsSearchWidgetWrapper::EqualTo )
    , mMenu( nullptr )
{
  setFocusPolicy( Qt::StrongFocus );
  setPopupMode( QToolButton::InstantPopup );

  mMenu = new QMenu( this );
  connect( mMenu, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowMenu() ) );
  setMenu( mMenu );

  // sets initial appearance
  updateState();
}

void QgsSearchWidgetToolButton::setAvailableFlags( QgsSearchWidgetWrapper::FilterFlags flags )
{
  mFilterFlags &= flags;
  mAvailableFilterFlags = flags;
  mDefaultFilterFlags = mDefaultFilterFlags & flags;
  updateState();
}

void QgsSearchWidgetToolButton::setDefaultFlags( QgsSearchWidgetWrapper::FilterFlags flags )
{
  mDefaultFilterFlags = flags & mAvailableFilterFlags;
}

void QgsSearchWidgetToolButton::setActiveFlags( QgsSearchWidgetWrapper::FilterFlags flags )
{
  // sanitize list
  QgsSearchWidgetWrapper::FilterFlags newFlags;

  // only accept a single exclusive flag
  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::exclusiveFilterFlags() )
  {
    if ( !( mAvailableFilterFlags & flag ) )
    {
      //unsupported
      continue;
    }
    if ( flags & flag )
    {
      newFlags |= flag;
      break;
    }
  }

  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::nonExclusiveFilterFlags() )
  {
    if ( !( mAvailableFilterFlags & flag ) )
    {
      //unsupported
      continue;
    }

    if ( flags & flag )
      newFlags |= flag;
  }

  mFilterFlags = newFlags;

  updateState();
}

void QgsSearchWidgetToolButton::toggleFlag( QgsSearchWidgetWrapper::FilterFlag flag )
{
  if ( !( flag & mAvailableFilterFlags ) )
    return;

  if ( QgsSearchWidgetWrapper::nonExclusiveFilterFlags().contains( flag ) )
  {
    if ( flag & mFilterFlags )
      mFilterFlags &= ~flag;
    else
      mFilterFlags |= flag;
  }
  else
  {
    // clear other exclusive flags
    Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag exclusiveFlag, QgsSearchWidgetWrapper::exclusiveFilterFlags() )
    {
      mFilterFlags &= ~exclusiveFlag;
    }
    // and set new exclusive flag
    mFilterFlags |= flag;
  }

  updateState();
}

bool QgsSearchWidgetToolButton::isActive() const
{
  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::exclusiveFilterFlags() )
  {
    if ( mFilterFlags & flag )
      return true;
  }
  return false;
}

void QgsSearchWidgetToolButton::aboutToShowMenu()
{
  mMenu->clear();
  bool fieldActive = false;
  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::exclusiveFilterFlags() )
  {
    if ( !( mAvailableFilterFlags & flag ) )
    {
      //unsupported
      continue;
    }

    QAction* action = mMenu->addAction( QgsSearchWidgetWrapper::toString( flag ) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( actionSelected() ) );
    action->setData( flag );
    action->setCheckable( true );
    if ( mFilterFlags & flag )
    {
      fieldActive = true;
      action->setChecked( true );
    }
  }

  QAction* clearAction = mMenu->addAction( tr( "Exclude field" ) );
  connect( clearAction, SIGNAL( triggered( bool ) ), this, SLOT( setInactive() ) );
  clearAction->setCheckable( true );
  clearAction->setChecked( !fieldActive );
  if ( mMenu->actions().count() > 0 )
  {
    mMenu->insertAction( mMenu->actions().at( 0 ), clearAction );
    mMenu->insertSeparator( mMenu->actions().at( 1 ) );
  }
  else
    mMenu->addAction( clearAction );

  mMenu->addSeparator();

  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::nonExclusiveFilterFlags() )
  {
    if ( !( mAvailableFilterFlags & flag ) )
    {
      //unsupported
      continue;
    }

    QAction* action = mMenu->addAction( QgsSearchWidgetWrapper::toString( flag ) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( actionSelected() ) );
    action->setData( flag );
    action->setCheckable( true );
    if ( mFilterFlags & flag )
      action->setChecked( true );
  }
}

void QgsSearchWidgetToolButton::actionSelected()
{
  QgsSearchWidgetWrapper::FilterFlag flag = static_cast< QgsSearchWidgetWrapper::FilterFlag >( qobject_cast< QAction* >( sender() )->data().toInt() );
  toggleFlag( flag );
}

void QgsSearchWidgetToolButton::searchWidgetValueChanged()
{
  setActive();
}

void QgsSearchWidgetToolButton::setInactive()
{
  if ( !isActive() )
    return;

  QgsSearchWidgetWrapper::FilterFlags newFlags;
  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::nonExclusiveFilterFlags() )
  {
    if ( !( mAvailableFilterFlags & flag ) || !( mFilterFlags & flag ) )
      continue;
    newFlags |= flag;
  }
  mFilterFlags = newFlags;
  updateState();
}

void QgsSearchWidgetToolButton::setActive()
{
  if ( isActive() )
    return;

  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::exclusiveFilterFlags() )
  {
    if ( mDefaultFilterFlags & flag )
    {
      toggleFlag( flag );
      return;
    }
  }
}

void QgsSearchWidgetToolButton::updateState()
{
  bool active = false;
  QStringList toolTips;
  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::exclusiveFilterFlags() )
  {
    if ( mFilterFlags & flag )
    {
      toolTips << QgsSearchWidgetWrapper::toString( flag );
      active = true;
    }
  }
  Q_FOREACH ( QgsSearchWidgetWrapper::FilterFlag flag, QgsSearchWidgetWrapper::nonExclusiveFilterFlags() )
  {
    if ( mFilterFlags & flag )
    {
      toolTips << QgsSearchWidgetWrapper::toString( flag ).toLower();
    }
  }

  if ( active )
  {
    QString text = toolTips.join( ", " );
    setText( text );
    setToolTip( text );
  }
  else
  {
    setText( tr( "Exclude field" ) );
    setToolTip( QString() );
  }

  emit activeFlagsChanged( mFilterFlags );
}
