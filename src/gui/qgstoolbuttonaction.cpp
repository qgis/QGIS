/***************************************************************************
                         qgstoolbuttonaction.cpp
                         ---------------------
    begin                : October 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstoolbuttonaction.h"

#include <QMenu>
#include <QToolBar>

#include "moc_qgstoolbuttonaction.cpp"

QgsToolButtonAction::QgsToolButtonAction( QObject *parent )
  : QWidgetAction( parent )
{
  // keep icon/tooltip/enabled state in sync on all live buttons
  connect( this, &QAction::changed, this, &QgsToolButtonAction::updateButtons );
}

void QgsToolButtonAction::setAutoRaise( bool autoRaise )
{
  mAutoRaise = autoRaise;

  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      button->setAutoRaise( autoRaise );
    }
  }
}

void QgsToolButtonAction::setPopupMode( QToolButton::ToolButtonPopupMode mode )
{
  mPopupMode = mode;

  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      button->setPopupMode( mode );
    }
  }
}

void QgsToolButtonAction::setToolButtonStyle( Qt::ToolButtonStyle style )
{
  mToolButtonStyle = style;

  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      button->setToolButtonStyle( style );
    }
  }
}

void QgsToolButtonAction::setMenu( QMenu *menu )
{
  mMenu = menu;

  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      button->setMenu( menu );
    }
  }
}

void QgsToolButtonAction::addActions( const QList< QAction * > &actions )
{
  for ( QAction *action : std::as_const( actions ) )
  {
    mButtonActions.append( action );
  }

  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      button->addActions( actions );
    }
  }
}

void QgsToolButtonAction::setDefaultAction( QAction *action )
{
  if ( !action )
  {
    return;
  }

  mDefaultAction = action;

  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      button->setDefaultAction( action );
    }
  }
}

QAction *QgsToolButtonAction::defaultAction() const
{
  return mDefaultAction;
}

QWidget *QgsToolButtonAction::createWidget( QWidget *parent )
{
  QToolButton *button = new QToolButton( parent );
  button->setAutoRaise( mAutoRaise );
  button->setPopupMode( mPopupMode );
  button->setToolButtonStyle( mToolButtonStyle );

  bool isInExtensionPopup = false;
  for ( QWidget *w = parent; w; w = w->parentWidget() )
  {
    if ( qobject_cast< QMenu * >( w ) )
    {
      isInExtensionPopup = true;
      break;
    }
  }

  if ( isInExtensionPopup )
  {
    button->setToolButtonStyle( Qt::ToolButtonStyle::ToolButtonTextBesideIcon );
  }
  else
  {
    button->setToolButtonStyle( mToolButtonStyle );
  }

  if ( QToolBar *toolBar = qobject_cast< QToolBar * >( parent ) )
  {
    button->setIconSize( toolBar->iconSize() );
    connect( toolBar, &QToolBar::iconSizeChanged, button, &QToolButton::setIconSize );

    if ( !isInExtensionPopup )
    {
      button->setToolButtonStyle( toolBar->toolButtonStyle() );
      connect( toolBar, &QToolBar::toolButtonStyleChanged, button, &QToolButton::setToolButtonStyle );
    }
  }

  if ( mMenu )
  {
    button->setMenu( mMenu );
  }

  if ( !mButtonActions.isEmpty() )
  {
    button->addActions( mButtonActions );
  }

  if ( mDefaultAction )
  {
    button->setDefaultAction( mDefaultAction );
  }
  else
  {
    applyState( button );
    connect( button, &QToolButton::clicked, this, &QAction::trigger );
  }

  return button;
}

void QgsToolButtonAction::applyState( QToolButton *button ) const
{
  button->setEnabled( isEnabled() );

  if ( !mDefaultAction )
  {
    button->setIcon( icon() );
    button->setText( text() );
    button->setToolTip( toolTip() );
  }
}

void QgsToolButtonAction::updateButtons()
{
  const QList< QWidget * > widgets( createdWidgets() );
  for ( QWidget *widget : std::as_const( widgets ) )
  {
    if ( auto *button = qobject_cast<QToolButton *>( widget ) )
    {
      applyState( button );
    }
  }
}
