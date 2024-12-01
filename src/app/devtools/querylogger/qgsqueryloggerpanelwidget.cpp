/***************************************************************************
    qgsqueryloggerpanelwidget.cpp
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
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
#include "qgsqueryloggerpanelwidget.h"
#include "moc_qgsqueryloggerpanelwidget.cpp"
#include "qgsdatabasequeryloggernode.h"
#include "qgsappquerylogger.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QToolButton>
#include <QCheckBox>
#include <QTextStream>
#include <QHeaderView>

#include <nlohmann/json.hpp>

//
// QgsDatabaseQueryLoggerTreeView
//

QgsDatabaseQueryLoggerTreeView::QgsDatabaseQueryLoggerTreeView( QgsAppQueryLogger *logger, QWidget *parent )
  : QTreeView( parent )
  , mLogger( logger )
{
  connect( this, &QTreeView::expanded, this, &QgsDatabaseQueryLoggerTreeView::itemExpanded );

  setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );

  mProxyModel = new QgsDatabaseQueryLoggerProxyModel( mLogger, this );
  mProxyModel->setSortRole( QgsDevToolsModelNode::RoleSort );
  setModel( mProxyModel );

  connect( mProxyModel, &QAbstractItemModel::rowsInserted, this, [this]( const QModelIndex &parent, int first, int last ) {
    // we want all second level items to be spanned
    for ( int row = first; row <= last; ++row )
    {
      setFirstColumnSpanned( row, parent, parent.isValid() );

      const QModelIndex childIndex = mProxyModel->index( row, 0, parent );
      for ( int childRow = 0; childRow < mProxyModel->rowCount( childIndex ); ++childRow )
      {
        setFirstColumnSpanned( childRow, childIndex, true );
      }
    }
  } );

  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, &QgsDatabaseQueryLoggerTreeView::customContextMenuRequested, this, &QgsDatabaseQueryLoggerTreeView::contextMenu );

  connect( verticalScrollBar(), &QAbstractSlider::sliderMoved, this, [this]( int value ) {
    if ( value == verticalScrollBar()->maximum() )
      mAutoScroll = true;
    else
      mAutoScroll = false;
  } );

  connect( mLogger, &QAbstractItemModel::rowsInserted, this, [=] {
    if ( mLogger->rowCount() > ( QgsAppQueryLogger::MAX_LOGGED_REQUESTS * 1.2 ) ) // 20 % more as buffer
    {
      // never trim expanded nodes
      const int toTrim = mLogger->rowCount() - QgsAppQueryLogger::MAX_LOGGED_REQUESTS;
      int trimmed = 0;
      QList<int> rowsToTrim;
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

    if ( mAutoScroll && isVisible() )
      scrollToBottom();
  } );

  mMenu = new QMenu( this );
}

void QgsDatabaseQueryLoggerTreeView::setFilterString( const QString &string )
{
  mProxyModel->setFilterString( string );
}

void QgsDatabaseQueryLoggerTreeView::itemExpanded( const QModelIndex &index )
{
  // if the item is a QgsNetworkLoggerRequestGroup item, open all children (show ALL info of it)
  // we want to scroll to last request

  // only expand all children on QgsNetworkLoggerRequestGroup nodes (which don't have a valid parent!)
  if ( !index.parent().isValid() )
    expandChildren( index );

  // make ALL request information visible by scrolling view to it
  scrollTo( index );
}

void QgsDatabaseQueryLoggerTreeView::contextMenu( QPoint point )
{
  const QModelIndex viewModelIndex = indexAt( point );
  const QModelIndex modelIndex = mProxyModel->mapToSource( viewModelIndex );

  if ( modelIndex.isValid() )
  {
    mMenu->clear();

    const QList<QAction *> actions = mLogger->actions( modelIndex, mMenu );
    mMenu->addActions( actions );
    if ( !mMenu->actions().empty() )
    {
      mMenu->exec( viewport()->mapToGlobal( point ) );
    }
  }
}

void QgsDatabaseQueryLoggerTreeView::expandChildren( const QModelIndex &index )
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
// QgsDatabaseQueryLoggerPanelWidget
//

QgsDatabaseQueryLoggerPanelWidget::QgsDatabaseQueryLoggerPanelWidget( QgsAppQueryLogger *logger, QWidget *parent )
  : QgsDevToolWidget( parent )
  , mLogger( logger )
{
  setupUi( this );

  mTreeView = new QgsDatabaseQueryLoggerTreeView( mLogger );
  mTreeView->setItemDelegateForColumn( 1, new QueryCostDelegate( QgsDevToolsModelNode::RoleElapsedTime, QgsDevToolsModelNode::RoleMaximumTime, mTreeView ) );
  mTreeView->setSortingEnabled( true );
  mTreeView->sortByColumn( 0, Qt::SortOrder::AscendingOrder );

  verticalLayout->addWidget( mTreeView );
  mToolbar->setIconSize( QgsGuiUtils::iconSize( true ) );

  mFilterLineEdit->setShowClearButton( true );
  mFilterLineEdit->setShowSearchIcon( true );
  mFilterLineEdit->setPlaceholderText( tr( "Filter queries" ) );

  mActionRecord->setChecked( QgsApplication::databaseQueryLog()->enabled() );

  connect( mFilterLineEdit, &QgsFilterLineEdit::textChanged, mTreeView, &QgsDatabaseQueryLoggerTreeView::setFilterString );
  connect( mActionClear, &QAction::triggered, mLogger, &QgsAppQueryLogger::clear );
  connect( mActionRecord, &QAction::toggled, this, [=]( bool enabled ) {
    QgsSettings().setValue( QStringLiteral( "logDatabaseQueries" ), enabled, QgsSettings::App );
    QgsApplication::databaseQueryLog()->setEnabled( enabled );
  } );
  connect( mActionSaveLog, &QAction::triggered, this, [=]() {
    if ( QMessageBox::warning( this, tr( "Save Database Query Log" ), tr( "Security warning: query logs may contain sensitive data including usernames or passwords. Treat this log as confidential and be careful who you share it with. Continue?" ), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
      return;

    const QString saveFilePath = QFileDialog::getSaveFileName( this, tr( "Save Query Log" ), QDir::homePath(), tr( "Log files" ) + " (*.json)" );
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

  QgsSettings settings;
  mTreeView->header()->restoreState( settings.value( QStringLiteral( "UI/queryLogger/treeState" ), QByteArray(), QgsSettings::Gui ).toByteArray() );
}

QgsDatabaseQueryLoggerPanelWidget::~QgsDatabaseQueryLoggerPanelWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/queryLogger/treeState" ), mTreeView->header()->saveState(), QgsSettings::Gui );
}
