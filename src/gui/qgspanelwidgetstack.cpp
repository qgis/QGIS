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

  connect( mBackButton, &QAbstractButton::pressed, this, &QgsPanelWidgetStack::acceptCurrentPanel );
}

void QgsPanelWidgetStack::setMainPanel( QgsPanelWidget *panel )
{
  // TODO Don't allow adding another main widget or else that would be strange for the user.
  connect( panel, &QgsPanelWidget::showPanel, this, &QgsPanelWidgetStack::showPanel,
           // using unique connection because addMainPanel() may be called multiple times
           // for a panel, so showPanel() slot could be invoked more times from one signal
           Qt::UniqueConnection );
  mStackedWidget->insertWidget( 0, panel );
  mStackedWidget->setCurrentIndex( 0 );
}

QgsPanelWidget *QgsPanelWidgetStack::mainPanel()
{
  return qobject_cast<QgsPanelWidget *>( mStackedWidget->widget( 0 ) );
}

QgsPanelWidget *QgsPanelWidgetStack::takeMainPanel()
{
  // clear out the current stack
  acceptAllPanels();

  QWidget *widget = mStackedWidget->widget( 0 );
  mStackedWidget->removeWidget( widget );
  return qobject_cast<QgsPanelWidget *>( widget );
}

void QgsPanelWidgetStack::clear()
{
  for ( int i = mStackedWidget->count() - 1; i >= 0; i-- )
  {
    if ( QgsPanelWidget *panelWidget = qobject_cast<QgsPanelWidget *>( mStackedWidget->widget( i ) ) )
    {
      mStackedWidget->removeWidget( panelWidget );
      if ( panelWidget->autoDelete() )
      {
        panelWidget->deleteLater();
      }
    }
    else if ( QWidget *widget = mStackedWidget->widget( i ) )
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

QgsPanelWidget *QgsPanelWidgetStack::currentPanel()
{
  return qobject_cast<QgsPanelWidget *>( mStackedWidget->currentWidget() );
}

void QgsPanelWidgetStack::acceptCurrentPanel()
{
  // You can't accept the main panel.
  if ( mStackedWidget->currentIndex() <= 0 )
    return;

  QgsPanelWidget *widget = currentPanel();
  widget->acceptPanel();
}

void QgsPanelWidgetStack::acceptAllPanels()
{
  //avoid messy multiple redraws
  setUpdatesEnabled( false );
  mStackedWidget->setUpdatesEnabled( false );

  for ( int i = mStackedWidget->count() - 1; i > 0; --i )
  {
    if ( QgsPanelWidget *panelWidget = qobject_cast<QgsPanelWidget *>( mStackedWidget->widget( i ) ) )
    {
      panelWidget->acceptPanel();
    }
  }
  setUpdatesEnabled( true );
  mStackedWidget->setUpdatesEnabled( true );
}

void QgsPanelWidgetStack::showPanel( QgsPanelWidget *panel )
{
  mTitles.push( panel->panelTitle() );

  connect( panel, &QgsPanelWidget::panelAccepted, this, &QgsPanelWidgetStack::closePanel );
  connect( panel, &QgsPanelWidget::showPanel, this, &QgsPanelWidgetStack::showPanel );

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

void QgsPanelWidgetStack::mouseReleaseEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::BackButton )
  {
    acceptCurrentPanel();
  }
}

void QgsPanelWidgetStack::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    acceptCurrentPanel();
  }
}

void QgsPanelWidgetStack::updateBreadcrumb()
{
  QString breadcrumb;
  const auto constMTitles = mTitles;
  for ( QString title : constMTitles )
  {
    breadcrumb += QStringLiteral( " %1 >" ).arg( title );
  }
  // Remove the last
  breadcrumb.chop( 1 );
  mTitleText->setText( breadcrumb );
}
