/***************************************************************************
    qgsdevtoolspanelwidget.cpp
    ---------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdevtoolspanelwidget.h"
#include "moc_qgsdevtoolspanelwidget.cpp"
#include "qgisapp.h"
#include "qgsdevtoolwidgetfactory.h"
#include "qgsdevtoolwidget.h"
#include "qgspanelwidgetstack.h"
#include "qgssettingsentryimpl.h"

const QgsSettingsEntryString *QgsDevToolsPanelWidget::settingLastActiveTab = new QgsSettingsEntryString( QStringLiteral( "last-active-tab" ), QgsDevToolsPanelWidget::sTreeDevTools, QString(), QStringLiteral( "Last visible tab in developer tools panel" ) );


QgsDevToolsPanelWidget::QgsDevToolsPanelWidget( const QList<QgsDevToolWidgetFactory *> &factories, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mOptionsListWidget->setIconSize( QgisApp::instance()->iconSize( false ) );
  mOptionsListWidget->setMaximumWidth( static_cast<int>( mOptionsListWidget->iconSize().width() * 1.18 ) );

  for ( QgsDevToolWidgetFactory *factory : factories )
    addToolFactory( factory );

  connect( mOptionsListWidget, &QListWidget::currentRowChanged, this, [=]( int row ) {
    setCurrentTool( row );
    settingLastActiveTab->setValue( mOptionsListWidget->currentItem()->data( Qt::UserRole ).toString() );
  } );
}

QgsDevToolsPanelWidget::~QgsDevToolsPanelWidget() = default;

void QgsDevToolsPanelWidget::addToolFactory( QgsDevToolWidgetFactory *factory )
{
  if ( QgsDevToolWidget *toolWidget = factory->createWidget( this ) )
  {
    QgsPanelWidgetStack *toolStack = new QgsPanelWidgetStack();
    toolStack->setMainPanel( toolWidget );
    mStackedWidget->addWidget( toolStack );

    QListWidgetItem *item = new QListWidgetItem( factory->icon(), QString() );
    item->setToolTip( factory->title() );
    item->setData( Qt::UserRole, factory->title() );

    mOptionsListWidget->addItem( item );
    const int row = mOptionsListWidget->row( item );
    mFactoryPages[factory] = row;

    if ( mOptionsListWidget->count() == 1 )
    {
      setCurrentTool( 0 );
    }
  }
}

void QgsDevToolsPanelWidget::removeToolFactory( QgsDevToolWidgetFactory *factory )
{
  if ( mFactoryPages.contains( factory ) )
  {
    const int currentRow = mStackedWidget->currentIndex();
    const int row = mFactoryPages.value( factory );
    if ( QWidget *widget = mStackedWidget->widget( row ) )
    {
      mStackedWidget->removeWidget( widget );
    }
    mOptionsListWidget->removeItemWidget( mOptionsListWidget->item( row ) );
    mFactoryPages.remove( factory );
    if ( currentRow == row )
      setCurrentTool( 0 );
  }
}

void QgsDevToolsPanelWidget::setActiveTab( const QString &title )
{
  if ( !title.isEmpty() )
  {
    for ( int row = 0; row < mOptionsListWidget->count(); ++row )
    {
      if ( mOptionsListWidget->item( row )->data( Qt::UserRole ).toString() == title )
      {
        setCurrentTool( row );
        break;
      }
    }
  }
}

void QgsDevToolsPanelWidget::setCurrentTool( int row )
{
  whileBlocking( mOptionsListWidget )->setCurrentRow( row );
  mStackedWidget->setCurrentIndex( row );
}
