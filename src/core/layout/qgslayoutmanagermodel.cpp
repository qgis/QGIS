/***************************************************************************
    qgslayoutmanagermodel.cpp
    --------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmanagermodel.h"
#include "moc_qgslayoutmanagermodel.cpp"
#include "qgsproject.h"
#include "qgslayoutmanager.h"
#include <QMessageBox>

//
// QgsLayoutManagerModel
//

QgsLayoutManagerModel::QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent )
  : QgsProjectStoredObjectManagerModel( manager, parent )
{
  connect( manager, &QgsLayoutManager::layoutRenamed, this, &QgsLayoutManagerModel::objectRenamedInternal );
}

QgsMasterLayoutInterface *QgsLayoutManagerModel::layoutFromIndex( const QModelIndex &index ) const
{
  return objectFromIndex( index );
}

QModelIndex QgsLayoutManagerModel::indexFromLayout( QgsMasterLayoutInterface *layout ) const
{
  return indexFromObject( layout );
}

void QgsLayoutManagerModel::setAllowEmptyLayout( bool allowEmpty )
{
  setAllowEmptyObject( allowEmpty );
}


//
// QgsLayoutManagerProxyModel
//

QgsLayoutManagerProxyModel::QgsLayoutManagerProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  sort( 0 );
  setSortCaseSensitivity( Qt::CaseInsensitive );
}

bool QgsLayoutManagerProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const QString leftText = sourceModel()->data( left, Qt::DisplayRole ).toString();
  const QString rightText = sourceModel()->data( right, Qt::DisplayRole ).toString();
  if ( leftText.isEmpty() )
    return true;
  if ( rightText.isEmpty() )
    return false;

  return QString::localeAwareCompare( leftText, rightText ) < 0;
}

bool QgsLayoutManagerProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsLayoutManagerModel *model = qobject_cast< QgsLayoutManagerModel * >( sourceModel() );
  if ( !model )
    return false;

  QgsMasterLayoutInterface *layout = model->layoutFromIndex( model->index( sourceRow, 0, sourceParent ) );
  if ( !layout )
    return model->allowEmptyLayout();

  if ( !mFilterString.trimmed().isEmpty() )
  {
    if ( !layout->name().contains( mFilterString, Qt::CaseInsensitive ) )
      return false;
  }

  switch ( layout->layoutType() )
  {
    case QgsMasterLayoutInterface::PrintLayout:
      return mFilters & FilterPrintLayouts;
    case QgsMasterLayoutInterface::Report:
      return mFilters & FilterReports;
  }
  return false;
}

QgsLayoutManagerProxyModel::Filters QgsLayoutManagerProxyModel::filters() const
{
  return mFilters;
}

void QgsLayoutManagerProxyModel::setFilters( Filters filters )
{
  mFilters = filters;
  invalidateFilter();
}

void QgsLayoutManagerProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}
