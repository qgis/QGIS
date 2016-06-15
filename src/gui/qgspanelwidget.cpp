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

#include "qgspanelwidget.h"
#include "qgslogger.h"

QgsPanelWidget::QgsPanelWidget( QWidget *parent )
    : QWidget( parent )
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
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SIGNAL( showPanel( QgsPanelWidget* ) ) );
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
    dlg->setWindowTitle( panel->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( panel );
    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
    dlg->layout()->addWidget( buttonBox );
    dlg->exec();
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

QgsPanelWidgetPage::QgsPanelWidgetPage( QgsPanelWidget *widget, QWidget *parent )
    : QgsPanelWidget( parent )
    , mWidget( widget )
{
  setupUi( this );
  mWidgetLayout->addWidget( widget );
  mWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mTitleText->setText( widget->panelTitle() );

  connect( mBackButton, SIGNAL( pressed() ), this, SLOT( acceptPanel() ) );
  connect( widget, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( acceptPanel() ) );
  connect( widget, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SIGNAL( showPanel( QgsPanelWidget* ) ) );
}

QgsPanelWidgetPage::~QgsPanelWidgetPage()
{
}

void QgsPanelWidgetPage::setTitle( QString title )
{
  mTitleText->setText( title );
}

QgsPanelWidgetStackWidget::QgsPanelWidgetStackWidget( QWidget *parent )
    : QStackedWidget( parent )
{

}

void QgsPanelWidgetStackWidget::connectPanels( QList<QgsPanelWidget *> panels )
{
  Q_FOREACH ( QgsPanelWidget* widget, panels )
  {
    connectPanel( widget );
  }
}

void QgsPanelWidgetStackWidget::connectPanel( QgsPanelWidget *panel )
{
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( showPanel( QgsPanelWidget* ) ) );
}

void QgsPanelWidgetStackWidget::addMainPanel( QgsPanelWidget *panel )
{
  // TODO Don't allow adding another main widget or else that would be strange for the user.
  connect( panel, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( showPanel( QgsPanelWidget* ) ) );
  this->insertWidget( 0, panel );
  this->setCurrentIndex( 0 );
}

void QgsPanelWidgetStackWidget::showPanel( QgsPanelWidget *panel )
{
  mTitles.push( panel->panelTitle() );
  QString breadcrumb;
  Q_FOREACH ( QString title, mTitles )
  {
    breadcrumb += QString( " %1 >" ).arg( title );
  }
  breadcrumb.chop( 1 );

  QgsPanelWidgetPage* page = new QgsPanelWidgetPage( panel, this );
  page->setTitle( breadcrumb );

  connect( page, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( closePanel( QgsPanelWidget* ) ) );
  connect( page, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( showPanel( QgsPanelWidget* ) ) );

  int index = this->addWidget( page );
  this->setCurrentIndex( index );
}

void QgsPanelWidgetStackWidget::closePanel( QgsPanelWidget *panel )
{
  this->setCurrentIndex( this->currentIndex() - 1 );
  this->removeWidget( panel );
  mTitles.pop();
  panel->deleteLater();
}
