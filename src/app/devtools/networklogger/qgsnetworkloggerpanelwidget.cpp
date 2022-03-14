/***************************************************************************
    qgsnetworkloggerpanelwidget.cpp
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgsjsonutils.h"
#include "qgsnetworkloggerpanelwidget.h"
#include "qgsnetworkloggernode.h"
#include "qgsnetworklogger.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QToolButton>
#include <QCheckBox>
#include <QTextStream>

#include <nlohmann/json.hpp>

//
// QgsNetworkLoggerTreeView
//

QgsNetworkLoggerTreeView::QgsNetworkLoggerTreeView( QgsNetworkLogger *logger, QWidget *parent )
  : QTreeView( parent )
  , mLogger( logger )
{
  connect( this, &QTreeView::expanded, this, &QgsNetworkLoggerTreeView::itemExpanded );

  setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );

  mProxyModel = new QgsNetworkLoggerProxyModel( mLogger, this );
  setModel( mProxyModel );

  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, &QgsNetworkLoggerTreeView::customContextMenuRequested, this, &QgsNetworkLoggerTreeView::contextMenu );

  connect( verticalScrollBar(), &QAbstractSlider::sliderMoved, this, [this]( int value )
  {
    if ( value == verticalScrollBar()->maximum() )
      mAutoScroll = true;
    else
      mAutoScroll = false;
  } );

  connect( mLogger, &QAbstractItemModel::rowsInserted, this, [ = ]
  {
    if ( mLogger->rowCount() > ( QgsNetworkLogger::MAX_LOGGED_REQUESTS * 1.2 ) ) // 20 % more as buffer
    {
      // never trim expanded nodes
      const int toTrim = mLogger->rowCount() - QgsNetworkLogger::MAX_LOGGED_REQUESTS;
      int trimmed = 0;
      QList< int > rowsToTrim;
      rowsToTrim.reserve( toTrim );
      for ( int i = 0; i < mLogger->rowCount(); ++i )
      {
        const QModelIndex proxyIndex = mProxyModel->mapFromSource( mLogger->index( i, 0 ) );
        if ( !proxyIndex.isValid() || !isExpanded( proxyIndex ) )
        {
          rowsToTrim << i;
          trimmed++;
        }
        if ( trimmed == toTrim )
          break;
      }

      mLogger->removeRequestRows( rowsToTrim );
    }

    if ( mAutoScroll )
      scrollToBottom();
  } );

  mMenu = new QMenu( this );
}

void QgsNetworkLoggerTreeView::setFilterString( const QString &string )
{
  mProxyModel->setFilterString( string );
}

void QgsNetworkLoggerTreeView::setShowSuccessful( bool show )
{
  mProxyModel->setShowSuccessful( show );
}

void QgsNetworkLoggerTreeView::setShowTimeouts( bool show )
{
  mProxyModel->setShowTimeouts( show );
}

void QgsNetworkLoggerTreeView::setShowCached( bool show )
{
  mProxyModel->setShowCached( show );
}

void QgsNetworkLoggerTreeView::itemExpanded( const QModelIndex &index )
{
  // if the item is a QgsNetworkLoggerRequestGroup item, open all children (show ALL info of it)
  // we want to scroll to last request

  // only expand all children on QgsNetworkLoggerRequestGroup nodes (which don't have a valid parent!)
  if ( !index.parent().isValid() )
    expandChildren( index );

  // make ALL request information visible by scrolling view to it
  scrollTo( index );
}

void QgsNetworkLoggerTreeView::contextMenu( QPoint point )
{
  const QModelIndex viewModelIndex = indexAt( point );
  const QModelIndex modelIndex = mProxyModel->mapToSource( viewModelIndex );

  if ( modelIndex.isValid() )
  {
    mMenu->clear();

    const QList< QAction * > actions = mLogger->actions( modelIndex, mMenu );
    mMenu->addActions( actions );
    if ( !mMenu->actions().empty() )
    {
      mMenu->exec( viewport()->mapToGlobal( point ) );
    }
  }
}

void QgsNetworkLoggerTreeView::expandChildren( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const int count = model()->rowCount( index );
  for ( int i = 0; i < count; ++i )
  {
    const QModelIndex childIndex = model()->index( i, 0, index );
    expandChildren( childIndex );
  }
  if ( !isExpanded( index ) )
    expand( index );
}


//
// QgsNetworkLoggerPanelWidget
//

QgsNetworkLoggerPanelWidget::QgsNetworkLoggerPanelWidget( QgsNetworkLogger *logger, QWidget *parent )
  : QgsDevToolWidget( parent )
  , mLogger( logger )
{
  setupUi( this );

  mTreeView = new QgsNetworkLoggerTreeView( mLogger );
  verticalLayout->addWidget( mTreeView );
  mToolbar->setIconSize( QgsGuiUtils::iconSize( true ) );

  mFilterLineEdit->setShowClearButton( true );
  mFilterLineEdit->setShowSearchIcon( true );
  mFilterLineEdit->setPlaceholderText( tr( "Filter requests" ) );

  mActionShowTimeouts->setChecked( true );
  mActionShowSuccessful->setChecked( true );
  mActionShowCached->setChecked( true );
  mActionRecord->setChecked( mLogger->isLogging() );

  connect( mFilterLineEdit, &QgsFilterLineEdit::textChanged, mTreeView, &QgsNetworkLoggerTreeView::setFilterString );
  connect( mActionShowTimeouts, &QAction::toggled, mTreeView, &QgsNetworkLoggerTreeView::setShowTimeouts );
  connect( mActionShowSuccessful, &QAction::toggled, mTreeView, &QgsNetworkLoggerTreeView::setShowSuccessful );
  connect( mActionShowCached, &QAction::toggled, mTreeView, &QgsNetworkLoggerTreeView::setShowCached );
  connect( mActionClear, &QAction::triggered, mLogger, &QgsNetworkLogger::clear );
  connect( mActionRecord, &QAction::toggled, this, [ = ]( bool enabled )
  {
    QgsSettings().setValue( QStringLiteral( "logNetworkRequests" ), enabled, QgsSettings::App );
    mLogger->enableLogging( enabled );
  } );
  connect( mActionSaveLog, &QAction::triggered, this, [ = ]()
  {
    if ( QMessageBox::warning( this, tr( "Save Network Log" ),
                               tr( "Security warning: network logs may contain sensitive data including usernames or passwords. Treat this log as confidential and be careful who you share it with. Continue?" ), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
      return;

    const QString saveFilePath = QFileDialog::getSaveFileName( this, tr( "Save Network Log" ), QDir::homePath(), tr( "Log files" ) + " (*.json)" );
    if ( saveFilePath.isEmpty() )
    {
      return;
    }

    QFile exportFile( saveFilePath );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      return;
    }
    QTextStream fout( &exportFile );

    const QVariant value = mLogger->rootGroup()->toVariant();
    const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( value ).dump( 2 ) );

    fout << json;
  } );


  QMenu *settingsMenu = new QMenu( this );
  QToolButton *settingsButton = new QToolButton();
  settingsButton->setAutoRaise( true );
  settingsButton->setToolTip( tr( "Settings" ) );
  settingsButton->setMenu( settingsMenu );
  settingsButton->setPopupMode( QToolButton::InstantPopup );
  settingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  mToolbar->addWidget( settingsButton );

  settingsMenu->addAction( mActionShowSuccessful );
  settingsMenu->addAction( mActionShowTimeouts );
  settingsMenu->addAction( mActionShowCached );

  mToolbar->addSeparator();
  QCheckBox *disableCacheCheck = new QCheckBox( tr( "Disable cache" ) );
  connect( disableCacheCheck, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    // note -- we deliberately do NOT store this as a permanent setting in QSettings
    // as it is designed to be a temporary debugging tool only and we don't want
    // users to accidentally leave this enabled and cause unnecessary server load...
    QgsNetworkAccessManager::instance()->setCacheDisabled( checked );
  } );

  mToolbar->addWidget( disableCacheCheck );
}
