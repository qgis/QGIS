/***************************************************************************
    qgsbrowserproxymodel.cpp
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbrowserproxymodel.h"
#include "qgsbrowsermodel.h"
#include "qgslayeritem.h"
#include "qgsdatacollectionitem.h"

#include <QRegularExpression>

QgsBrowserProxyModel::QgsBrowserProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  setSortRole( QgsBrowserModel::SortRole );
  setSortCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

void QgsBrowserProxyModel::setBrowserModel( QgsBrowserModel *model )
{
  mModel = model;
  setSourceModel( model );
}

QgsDataItem *QgsBrowserProxyModel::dataItem( const QModelIndex &index ) const
{
  const QModelIndex sourceIndex = mapToSource( index );
  return mModel ? mModel->dataItem( sourceIndex ) : nullptr;
}

void QgsBrowserProxyModel::setFilterSyntax( FilterSyntax syntax )
{
  if ( mPatternSyntax == syntax )
    return;
  mPatternSyntax = syntax;
  updateFilter();
}

QgsBrowserProxyModel::FilterSyntax QgsBrowserProxyModel::filterSyntax() const
{
  return mPatternSyntax;
}

void QgsBrowserProxyModel::setFilterString( const QString &filter )
{
  if ( mFilter == filter )
    return;
  mFilter = filter;
  updateFilter();
}

QString QgsBrowserProxyModel::filterString() const
{
  return mFilter;
}

void QgsBrowserProxyModel::setFilterCaseSensitivity( Qt::CaseSensitivity sensitivity )
{
  mCaseSensitivity = sensitivity;
  updateFilter();
}

Qt::CaseSensitivity QgsBrowserProxyModel::caseSensitivity() const
{
  return mCaseSensitivity;
}

void QgsBrowserProxyModel::updateFilter()
{
  mREList.clear();
  switch ( mPatternSyntax )
  {
    case Normal:
    {
      const QStringList filterParts = mFilter.split( '|' );
      for ( const QString &f : filterParts )
      {
        const QRegularExpression rx( QRegularExpression::wildcardToRegularExpression( QStringLiteral( "*%1*" ).arg( f.trimmed() ) ),
                                     mCaseSensitivity == Qt::CaseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption );
        mREList.append( rx );
      }
      break;
    }
    case Wildcards:
    {
      const QStringList filterParts = mFilter.split( '|' );
      for ( const QString &f : filterParts )
      {
        const QRegularExpression rx( QRegularExpression::wildcardToRegularExpression( f.trimmed() ),
                                     mCaseSensitivity == Qt::CaseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption );
        mREList.append( rx );
      }
      break;
    }
    case RegularExpression:
    {
      const QRegularExpression rx( mFilter.trimmed(), mCaseSensitivity == Qt::CaseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption );
      mREList.append( rx );
      break;
    }
  }
  invalidateFilter();
}

bool QgsBrowserProxyModel::filterAcceptsString( const QString &value ) const
{
  for ( const QRegularExpression &rx : mREList )
  {
    if ( rx.match( value ).hasMatch() )
      return true;
  }

  return false;
}

bool QgsBrowserProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( ( mFilter.isEmpty() && !mFilterByLayerType && mHiddenDataItemsKeys.empty() && mShownDataItemsKeys.empty() ) || !mModel )
    return true;

  const QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  if ( !filterAcceptsProviderKey( sourceIndex ) || !filterRootAcceptsProviderKey( sourceIndex ) )
    return false;

  if ( ! mShowLayers )
  {
    QgsDataItem *item = mModel->dataItem( sourceIndex );
    if ( qobject_cast< QgsLayerItem * >( item ) )
    {
      return false;
    }
  }

  return filterAcceptsItem( sourceIndex ) || filterAcceptsAncestor( sourceIndex ) || filterAcceptsDescendant( sourceIndex );
}

bool QgsBrowserProxyModel::showLayers() const
{
  return mShowLayers;
}

void QgsBrowserProxyModel::setShowLayers( bool showLayers )
{
  mShowLayers = showLayers;
}

QgsMapLayerType QgsBrowserProxyModel::layerType() const
{
  return mLayerType;
}

void QgsBrowserProxyModel::setLayerType( QgsMapLayerType type )
{
  mLayerType = type;
  invalidateFilter();
}

void QgsBrowserProxyModel::setFilterByLayerType( bool filterByLayerType )
{
  mFilterByLayerType = filterByLayerType;
  invalidateFilter();
}

bool QgsBrowserProxyModel::filterAcceptsAncestor( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  if ( mFilterByLayerType )
    return false;

  const QModelIndex sourceParentIndex = mModel->parent( sourceIndex );
  if ( !sourceParentIndex.isValid() )
    return false;
  if ( filterAcceptsItem( sourceParentIndex ) )
    return true;

  return filterAcceptsAncestor( sourceParentIndex );
}

bool QgsBrowserProxyModel::filterAcceptsDescendant( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  for ( int i = 0; i < mModel->rowCount( sourceIndex ); i++ )
  {
    const QModelIndex sourceChildIndex = mModel->index( i, 0, sourceIndex );
    if ( filterAcceptsItem( sourceChildIndex ) )
      return true;
    if ( filterAcceptsDescendant( sourceChildIndex ) )
      return true;
  }
  return false;
}

bool QgsBrowserProxyModel::filterAcceptsItem( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  if ( mFilterByLayerType )
  {
    QgsDataItem *item = mModel->dataItem( sourceIndex );
    if ( QgsLayerItem *layerItem = qobject_cast< QgsLayerItem * >( item ) )
    {
      if ( layerItem->mapLayerType() != mLayerType )
        return false;
    }
    else if ( !qobject_cast< QgsDataCollectionItem * >( item ) )
      return false;
  }

  if ( !mFilter.isEmpty() )
  {
    //accept item if either displayed text or comment role matches string
    const QString comment = mModel->data( sourceIndex, QgsBrowserModel::CommentRole ).toString();
    return ( filterAcceptsString( mModel->data( sourceIndex, Qt::DisplayRole ).toString() )
             || ( !comment.isEmpty() && filterAcceptsString( comment ) ) );
  }

  return true;
}

bool QgsBrowserProxyModel::filterAcceptsProviderKey( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  const QString providerKey = mModel->data( sourceIndex, QgsBrowserModel::ProviderKeyRole ).toString();
  if ( providerKey.isEmpty() )
    return true;

  return !mHiddenDataItemsKeys.contains( providerKey ) && ( mShownDataItemsKeys.isEmpty() || mShownDataItemsKeys.contains( providerKey ) );
}

bool QgsBrowserProxyModel::filterRootAcceptsProviderKey( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  const QModelIndex sourceParentIndex = mModel->parent( sourceIndex );
  if ( !sourceParentIndex.isValid() )
  {
    return filterAcceptsProviderKey( sourceIndex );
  }

  return filterRootAcceptsProviderKey( sourceParentIndex );
}

void QgsBrowserProxyModel::setHiddenDataItemProviderKeyFilter( const QStringList &filter )
{
  mHiddenDataItemsKeys = filter;
  invalidateFilter();
}

void QgsBrowserProxyModel::setShownDataItemProviderKeyFilter( const QStringList &filter )
{
  mShownDataItemsKeys = filter;
  invalidateFilter();
}


bool QgsBrowserProxyModel::hasChildren( const QModelIndex &parent ) const
{
  const bool isFertile { QSortFilterProxyModel::hasChildren( parent ) };
  if ( isFertile && parent.isValid() )
  {
    QgsDataItem *item = dataItem( parent );
    if ( ! mShowLayers )
    {
      return ! item->layerCollection();
    }
    // Hide everything below layers if filter is set
    else if ( mFilterByLayerType && qobject_cast< QgsLayerItem * >( item ) )
    {
      return false;
    }
  }
  return isFertile;
}
