/***************************************************************************
  qgslateralpanelwidget.cpp
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslateralpanelwidget.h"

#include <QTabBar>

#include "moc_qgslateralpanelwidget.cpp"

QgsLateralPanelWidget::QgsLateralPanelWidget( QAction *toggleAction, QWidget *parent )
  : QTabWidget( parent )
{
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

  mToggleAction = toggleAction;
  mToggleAction->setCheckable( true );
  mToggleAction->setChecked( true );
  QObject::connect( mToggleAction, &QAction::toggled, this, [this]( const bool visibility ) { setVisible( visibility ); } );
}

int QgsLateralPanelWidget::addWidget( QWidget *widget, const QString &name )
{
  return addTab( widget, name );
}

void QgsLateralPanelWidget::hideWidget( int index )
{
  if ( index < count() && index >= 0 )
  {
    setTabVisible( index, false );
  }
}

void QgsLateralPanelWidget::hideWidget( const QString &name )
{
  for ( int i = 0; i < count(); ++i )
  {
    if ( tabBar()->tabText( i ) == name )
    {
      hideWidget( i );
      break;
    }
  }
}

void QgsLateralPanelWidget::showWidget( int index )
{
  if ( index < count() && index >= 0 )
  {
    setTabVisible( index, true );
    setCurrentIndex( index );
    mToggleAction->setChecked( true );
  }
}

void QgsLateralPanelWidget::showWidget( const QString &name )
{
  for ( int i = 0; i < count(); ++i )
  {
    if ( tabBar()->tabText( i ) == name )
    {
      showWidget( i );
      break;
    }
  }
}

void QgsLateralPanelWidget::removeWidget( int index )
{
  if ( index < count() && index >= 0 )
  {
    removeTab( index );
  }
}

void QgsLateralPanelWidget::removeWidget( const QString &name )
{
  for ( int i = 0; i < count(); ++i )
  {
    if ( tabBar()->tabText( i ) == name )
    {
      removeWidget( i );
      break;
    }
  }
}

QAction *QgsLateralPanelWidget::toggleAction()
{
  return mToggleAction;
}

void QgsLateralPanelWidget::hide()
{
  mToggleAction->setChecked( false );
}
