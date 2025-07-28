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
#include "qgslayout.h"
#include "qgsproject.h"
#include "qgslayoutundostack.h"
#include "qgsprintlayout.h"
#include "qgsreport.h"
#include "qgscompositionconverter.h"
#include "qgslayoutmanager.h"
#include <QMessageBox>


//
// QgsLayoutManagerModel
//

QgsLayoutManagerModel::QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent )
  : QAbstractListModel( parent )
  , mLayoutManager( manager )
{
  connect( mLayoutManager, &QgsLayoutManager::layoutAboutToBeAdded, this, &QgsLayoutManagerModel::layoutAboutToBeAdded );
  connect( mLayoutManager, &QgsLayoutManager::layoutAdded, this, &QgsLayoutManagerModel::layoutAdded );
  connect( mLayoutManager, &QgsLayoutManager::layoutAboutToBeRemoved, this, &QgsLayoutManagerModel::layoutAboutToBeRemoved );
  connect( mLayoutManager, &QgsLayoutManager::layoutRemoved, this, &QgsLayoutManagerModel::layoutRemoved );
  connect( mLayoutManager, &QgsLayoutManager::layoutRenamed, this, &QgsLayoutManagerModel::layoutRenamed );
}

int QgsLayoutManagerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return ( mLayoutManager ? mLayoutManager->layouts().count() : 0 ) + ( mAllowEmpty ? 1 : 0 );
}

QVariant QgsLayoutManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const bool isEmpty = index.row() == 0 && mAllowEmpty;
  const int layoutRow = mAllowEmpty ? index.row() - 1 : index.row();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
      return !isEmpty && mLayoutManager ? mLayoutManager->layouts().at( layoutRow )->name() : QVariant();

    case static_cast< int >( CustomRole::Layout ):
    {
      if ( isEmpty || !mLayoutManager )
        return QVariant();
      else if ( QgsLayout *l = dynamic_cast< QgsLayout * >( mLayoutManager->layouts().at( layoutRow ) ) )
        return QVariant::fromValue( l );
      else if ( QgsReport *r = dynamic_cast< QgsReport * >( mLayoutManager->layouts().at( layoutRow ) ) )
        return QVariant::fromValue( r );
      else
        return QVariant();
    }

    case Qt::DecorationRole:
    {
      return isEmpty || !mLayoutManager ? QIcon() : mLayoutManager->layouts().at( layoutRow )->icon();
    }

    default:
      return QVariant();
  }
}

bool QgsLayoutManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.row() >= mLayoutManager->layouts().count() )
  {
    return false;
  }

  if ( index.row() == 0 && mAllowEmpty )
    return false;

  if ( value.toString().isEmpty() )
    return false;

  QgsMasterLayoutInterface *layout = layoutFromIndex( index );
  if ( !layout )
    return false;

  //has name changed?
  bool changed = layout->name() != value.toString();
  if ( !changed )
    return true;

  //check if name already exists
  QStringList layoutNames;
  const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts(); // skip-keyword-check
  for ( QgsMasterLayoutInterface *l : layouts )
  {
    layoutNames << l->name();
  }
  if ( layoutNames.contains( value.toString() ) )
  {
    //name exists!
    QMessageBox::warning( nullptr, tr( "Rename Layout" ), tr( "There is already a layout named “%1”." ).arg( value.toString() ) );
    return false;
  }

  layout->setName( value.toString() );
  return true;
}

Qt::ItemFlags QgsLayoutManagerModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractListModel::flags( index );
#if 0 // double-click is now used for opening the layout
  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
#endif
  return flags;
}

QgsMasterLayoutInterface *QgsLayoutManagerModel::layoutFromIndex( const QModelIndex &index ) const
{
  if ( index.row() == 0 && mAllowEmpty )
    return nullptr;

  if ( QgsPrintLayout *l = qobject_cast< QgsPrintLayout * >( qvariant_cast<QObject *>( data( index, static_cast< int >( CustomRole::Layout ) ) ) ) )
    return l;
  else if ( QgsReport *r = qobject_cast< QgsReport * >( qvariant_cast<QObject *>( data( index, static_cast< int >( CustomRole::Layout ) ) ) ) )
    return r;
  else
    return nullptr;
}

QModelIndex QgsLayoutManagerModel::indexFromLayout( QgsMasterLayoutInterface *layout ) const
{
  if ( !mLayoutManager )
  {
    return QModelIndex();
  }

  const int r = mLayoutManager->layouts().indexOf( layout );
  if ( r < 0 )
    return QModelIndex();

  QModelIndex idx = index( mAllowEmpty ? r + 1 : r, 0, QModelIndex() );
  if ( idx.isValid() )
  {
    return idx;
  }

  return QModelIndex();
}

void QgsLayoutManagerModel::setAllowEmptyLayout( bool allowEmpty )
{
  if ( allowEmpty == mAllowEmpty )
    return;

  if ( allowEmpty )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mAllowEmpty = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), 0, 0 );
    mAllowEmpty = false;
    endRemoveRows();
  }
}

void QgsLayoutManagerModel::layoutAboutToBeAdded( const QString & )
{
  int row = mLayoutManager->layouts().count() + ( mAllowEmpty ? 1 : 0 );
  beginInsertRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::layoutAboutToBeRemoved( const QString &name )
{
  QgsMasterLayoutInterface *l = mLayoutManager->layoutByName( name );
  int row = mLayoutManager->layouts().indexOf( l ) + ( mAllowEmpty ? 1 : 0 );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::layoutAdded( const QString & )
{
  endInsertRows();
}

void QgsLayoutManagerModel::layoutRemoved( const QString & )
{
  endRemoveRows();
}

void QgsLayoutManagerModel::layoutRenamed( QgsMasterLayoutInterface *layout, const QString & )
{
  int row = mLayoutManager->layouts().indexOf( layout ) + ( mAllowEmpty ? 1 : 0 );
  QModelIndex index = createIndex( row, 0 );
  emit dataChanged( index, index, QVector<int>() << Qt::DisplayRole );
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
