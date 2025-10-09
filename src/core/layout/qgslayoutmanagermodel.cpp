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
  : QgsProjectStoredObjectManagerProxyModel( parent )
{
}

bool QgsLayoutManagerProxyModel::filterAcceptsRowInternal( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( !QgsProjectStoredObjectManagerProxyModel::filterAcceptsRowInternal( sourceRow, sourceParent ) )
    return false;

  QgsLayoutManagerModel *model = qobject_cast< QgsLayoutManagerModel * >( sourceModel() );
  if ( !model )
    return false;

  QgsMasterLayoutInterface *layout = model->layoutFromIndex( model->index( sourceRow, 0, sourceParent ) );
  if ( !layout )
    return model->allowEmptyLayout();

  // clang-tidy false positive
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( layout->layoutType() )
  {
    case QgsMasterLayoutInterface::PrintLayout:
      return mFilters & FilterPrintLayouts;
    case QgsMasterLayoutInterface::Report:
      return mFilters & FilterReports;
  }
  // NOLINTEND(bugprone-branch-clone)
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
