/***************************************************************************
                             qgshistorywidget.cpp
                             ------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshistorywidget.h"
#include "moc_qgshistorywidget.cpp"
#include "qgsgui.h"
#include "qgshistoryentrymodel.h"
#include "qgshistoryentrynode.h"
#include "qgssettings.h"
#include "qgsnative.h"

#include <QTextBrowser>
#include <QtGlobal>
#include <QMenu>
#include <QFileInfo>
#include <QDesktopServices>

QgsHistoryWidget::QgsHistoryWidget( const QString &providerId, Qgis::HistoryProviderBackends backends, QgsHistoryProviderRegistry *registry, const QgsHistoryWidgetContext &context, QWidget *parent )
  : QgsPanelWidget( parent )
  , mContext( context )
{
  setupUi( this );

  mModel = new QgsHistoryEntryModel( providerId, backends, registry, mContext, this );
  mProxyModel = new QgsHistoryEntryProxyModel( this );
  mProxyModel->setSourceModel( mModel );

  mTreeView->setModel( mProxyModel );

  mFilterEdit->setShowClearButton( true );
  mFilterEdit->setShowSearchIcon( true );
  connect( mFilterEdit, &QLineEdit::textChanged, mProxyModel, &QgsHistoryEntryProxyModel::setFilter );
  connect( mTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsHistoryWidget::currentItemChanged );
  connect( mTreeView, &QTreeView::doubleClicked, this, &QgsHistoryWidget::nodeDoubleClicked );
  mTreeView->setExpandsOnDoubleClick( false );

  mTreeView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTreeView, &QWidget::customContextMenuRequested, this, &QgsHistoryWidget::showNodeContextMenu );

  // expand first group (usually most recent date group)
  const QModelIndex firstGroup = mProxyModel->index( 0, 0, QModelIndex() );
  mTreeView->expand( firstGroup );

  QgsSettings settings;
  mSplitter->restoreState( settings.value( QStringLiteral( "history/splitterState%1" ).arg( providerId ) ).toByteArray() );

  connect( mSplitter, &QSplitter::splitterMoved, this, [providerId, this] {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "history/splitterState%1" ).arg( providerId ), mSplitter->saveState() );
  } );
}

void QgsHistoryWidget::currentItemChanged( const QModelIndex &selected, const QModelIndex & )
{
  QWidget *newWidget = nullptr;
  if ( QgsHistoryEntryNode *node = mModel->index2node( mProxyModel->mapToSource( selected ) ) )
  {
    newWidget = node->createWidget( mContext );
    if ( !newWidget )
    {
      const QString html = node->html( mContext );
      if ( !html.isEmpty() )
      {
        QTextBrowser *htmlBrowser = new QTextBrowser();
        htmlBrowser->setOpenLinks( false );
        htmlBrowser->setHtml( html );
        connect( htmlBrowser, &QTextBrowser::anchorClicked, this, &QgsHistoryWidget::urlClicked );

        newWidget = htmlBrowser;
      }
    }
    if ( newWidget )
    {
      mContainerStackedWidget->addWidget( newWidget );
      mContainerStackedWidget->setCurrentWidget( newWidget );
    }
  }

  if ( !newWidget )
  {
    //remove current widget, if any
    if ( mContainerStackedWidget->count() > 1 )
    {
      mContainerStackedWidget->removeWidget( mContainerStackedWidget->widget( 1 ) );
      mContainerStackedWidget->setCurrentIndex( 0 );
    }
  }
}

void QgsHistoryWidget::nodeDoubleClicked( const QModelIndex &index )
{
  if ( QgsHistoryEntryNode *node = mModel->index2node( mProxyModel->mapToSource( index ) ) )
  {
    if ( node->doubleClicked( mContext ) )
      return; // double-click handled
  }

  // otherwise double-clicks expands/collapses the node
  if ( mTreeView->isExpanded( index ) )
    mTreeView->collapse( index );
  else
    mTreeView->expand( index );
}

void QgsHistoryWidget::showNodeContextMenu( const QPoint &pos )
{
  if ( QgsHistoryEntryNode *node = mModel->index2node( mProxyModel->mapToSource( mTreeView->currentIndex() ) ) )
  {
    QMenu *menu = new QMenu();

    node->populateContextMenu( menu, mContext );
    if ( !menu->isEmpty() )
    {
      menu->exec( mTreeView->mapToGlobal( pos ) );
    }
    delete menu;
  }
}

void QgsHistoryWidget::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

//
// QgsHistoryEntryProxyModel
//

///@cond PRIVATE
QgsHistoryEntryProxyModel::QgsHistoryEntryProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  setRecursiveFilteringEnabled( true );
}

void QgsHistoryEntryProxyModel::setFilter( const QString &filter )
{
  if ( filter == mFilter )
    return;

  mFilter = filter;
  invalidateFilter();
}

bool QgsHistoryEntryProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilter.isEmpty() )
    return true;

  const QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );
  if ( QgsHistoryEntryNode *node = qobject_cast<QgsHistoryEntryModel *>( sourceModel() )->index2node( sourceIndex ) )
  {
    if ( !node->matchesString( mFilter ) )
    {
      return false;
    }
  }
  return true;
}
///@endcond PRIVATE
