/***************************************************************************
    qgspanelwidget.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>
#include <QSettings>

#include "qgslogger.h"

#include "qgspanelwidgetstack.h"

#include "qgspanelwidget.h"

QgsPanelWidgetStack::QgsPanelWidgetStack( QWidget *parent )
    : QWidget( parent )
{
  setupUi( this );
  clear();

  connect( mBackButton, SIGNAL( pressed() ), this, SLOT( acceptCurrentPanel() ) );
}

void QgsPanelWidgetStack::addMainPanel( QgsPanelWidget *panel )
{
  // TODO Don't allow adding another main widget or else that would be strange for the user.
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( showPanel( QgsPanelWidget* ) ),
           // using unique connection because addMainPanel() may be called multiple times
           // for a panel, so showPanel() slot could be invoked more times from one signal
           Qt::UniqueConnection );
  mStackedWidget->insertWidget( 0, panel );
  mStackedWidget->setCurrentIndex( 0 );
}

QgsPanelWidget *QgsPanelWidgetStack::mainWidget()
{
  return qobject_cast<QgsPanelWidget*>( mStackedWidget->widget( 0 ) );
}

QgsPanelWidget *QgsPanelWidgetStack::takeMainWidget()
{
  QWidget* widget = mStackedWidget->widget( 0 );
  mStackedWidget->removeWidget( widget );
  return qobject_cast<QgsPanelWidget*>( widget );
}

void QgsPanelWidgetStack::clear()
{
  for ( int i = mStackedWidget->count(); i >= 0; i-- )
  {
    if ( QgsPanelWidget* panelWidget = qobject_cast<QgsPanelWidget*>( mStackedWidget->widget( i ) ) )
    {
      mStackedWidget->removeWidget( panelWidget );
      if ( panelWidget->autoDelete() )
      {
        panelWidget->deleteLater();
      }
    }
    else if ( QWidget* widget = mStackedWidget->widget( i ) )
    {
      mStackedWidget->removeWidget( widget );
      widget->deleteLater();
    }
  }
  mTitles.clear();
  mTitleText->hide();
  mBackButton->hide();
  this->updateBreadcrumb();
}

void QgsPanelWidgetStack::acceptCurrentPanel()
{
  // You can't accept the main panel.
  if ( mStackedWidget->currentIndex() == 0 )
    return;

  QgsPanelWidget* widget = qobject_cast<QgsPanelWidget*>( mStackedWidget->currentWidget() );
  widget->acceptPanel();
}

void QgsPanelWidgetStack::showPanel( QgsPanelWidget *panel )
{
  mTitles.push( panel->panelTitle() );

  connect( panel, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( closePanel( QgsPanelWidget* ) ) );
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( showPanel( QgsPanelWidget* ) ) );

  int index = mStackedWidget->addWidget( panel );
  mStackedWidget->setCurrentIndex( index );
  mBackButton->show();
  mTitleText->show();

  this->updateBreadcrumb();
}

void QgsPanelWidgetStack::closePanel( QgsPanelWidget *panel )
{
  mTitles.pop();
  mStackedWidget->setCurrentIndex( mStackedWidget->currentIndex() - 1 );
  mStackedWidget->removeWidget( panel );
  if ( panel->autoDelete() )
  {
    panel->deleteLater();
  }

  if ( mStackedWidget->currentIndex() == 0 )
  {
    mBackButton->hide();
    mTitleText->hide();
  }
  this->updateBreadcrumb();
}

void QgsPanelWidgetStack::updateBreadcrumb()
{
  QString breadcrumb;
  Q_FOREACH ( QString title, mTitles )
  {
    breadcrumb += QString( " %1 >" ).arg( title );
  }
  // Remove the last
  breadcrumb.chop( 1 );
  mTitleText->setText( breadcrumb );
}
