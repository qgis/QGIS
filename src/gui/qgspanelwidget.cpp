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
#include <QVBoxLayout>

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

QgsPanelWidgetWrapper::QgsPanelWidgetWrapper( QWidget *widget, QWidget *parent )
    : QgsPanelWidget( parent )
    , mWidget( widget )
{
  this->setLayout( new QVBoxLayout() );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );
  this->layout()->addWidget( widget );
}
