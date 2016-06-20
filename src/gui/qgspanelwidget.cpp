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

#include "qgspanelwidget.h"
#include "qgslogger.h"

QgsPanelWidget::QgsPanelWidget( QWidget *parent )
    : QWidget( parent )
    , mAutoDelete( true )
    , mDockMode( false )
{
}

void QgsPanelWidget::connectChildPanels( QList<QgsPanelWidget *> panels )
{
  Q_FOREACH ( QgsPanelWidget* widget, panels )
  {
    connectChildPanel( widget );
  }
}

void QgsPanelWidget::connectChildPanel( QgsPanelWidget *panel )
{
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( openPanel( QgsPanelWidget* ) ) );
  connect( panel, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
}

void QgsPanelWidget::setDockMode( bool dockMode )
{
  mDockMode = dockMode;
}

void QgsPanelWidget::openPanel( QgsPanelWidget* panel )
{
  if ( mDockMode )
  {
    emit showPanel( panel );
  }
  else
  {
    // Show the dialog version if no one is connected
    QDialog* dlg = new QDialog();
    QString key =  QString( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
    QSettings settings;
    dlg->restoreGeometry( settings.value( key ).toByteArray() );
    dlg->setWindowTitle( panel->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( panel );
    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
    dlg->layout()->addWidget( buttonBox );
    dlg->exec();
    settings.setValue( key, dlg->saveGeometry() );
    emit panelAccepted( panel );
  }
}

void QgsPanelWidget::acceptPanel()
{
  emit panelAccepted( this );
}

void QgsPanelWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    acceptPanel();
  }
}

QgsPanelWidgetStack::QgsPanelWidgetStack( QWidget *parent )
    : QWidget( parent )
{
  setupUi( this );
  mBackButton->hide();
  mTitleText->hide();

  connect( mBackButton, SIGNAL( pressed() ), this, SLOT( acceptCurrentPanel() ) );
}

void QgsPanelWidgetStack::addMainPanel( QgsPanelWidget *panel )
{
  // TODO Don't allow adding another main widget or else that would be strange for the user.
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( showPanel( QgsPanelWidget* ) ) );
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
  // TODO Remove all widgets;
  for ( int i = mStackedWidget->count(); i >= 0; i-- )
  {
    QgsPanelWidget* widget = qobject_cast<QgsPanelWidget*>( mStackedWidget->widget( i ) );
    if ( widget )
    {
      mStackedWidget->removeWidget( widget );
      if ( widget->autoDelete() )
      {
        widget->deleteLater();
      }
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

QgsPanelWidgetWrapper::QgsPanelWidgetWrapper( QWidget *widget, QWidget *parent )
    : QgsPanelWidget( parent )
    , mWidget( widget )
{
  this->setLayout( new QVBoxLayout() );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );
  this->layout()->addWidget( widget );
}
