/***************************************************************************
     QgsAttributeTableFilterModel.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QItemSelectionModel>

#include "qgis.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetablemodel.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayereditbuffer.h"
//////////////////
// Filter Model //
//////////////////

QgsAttributeTableFilterModel::QgsAttributeTableFilterModel( QgsMapCanvas* canvas, QgsAttributeTableModel* sourceModel, QObject* parent )
    : QSortFilterProxyModel( parent )
    , mCanvas( canvas )
    , mFilterMode( ShowAll )
    , mSelectedOnTop( false )
{
  setSourceModel( sourceModel );
  setDynamicSortFilter( true );
  setSortRole( QgsAttributeTableModel::SortRole );
  connect( layer(), SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) );
}

bool QgsAttributeTableFilterModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  if ( mSelectedOnTop )
  {
    bool leftSelected = layer()->selectedFeaturesIds().contains( masterModel()->rowToId( left.row() ) );
    bool rightSelected = layer()->selectedFeaturesIds().contains( masterModel()->rowToId( right.row() ) );

    if ( leftSelected && !rightSelected )
    {
      return sortOrder() == Qt::AscendingOrder;
    }
    else if ( rightSelected && !leftSelected )
    {
      return sortOrder() == Qt::DescendingOrder;
    }
  }

  return qgsVariantLessThan( left.data( QgsAttributeTableModel::SortRole ),
                             right.data( QgsAttributeTableModel::SortRole ) );
}

void QgsAttributeTableFilterModel::sort( int column, Qt::SortOrder order )
{
  int myColumn = mColumnMapping.at( column );
  masterModel()->prefetchColumnData( myColumn );
  QSortFilterProxyModel::sort( myColumn, order );
  emit sortColumnChanged( column, order );
}

QVariant QgsAttributeTableFilterModel::data( const QModelIndex& index, int role ) const
{
  if ( mapColumnToSource( index.column() ) == -1 ) // actions
  {
    if ( role == TypeRole )
      return ColumnTypeActionButton;
    else if ( role == QgsAttributeTableModel::FeatureIdRole )
    {
      QModelIndex fieldIndex = QSortFilterProxyModel::mapToSource( QSortFilterProxyModel::index( index.row(), 0, index.parent() ) );
      return sourceModel()->data( fieldIndex, QgsAttributeTableModel::FeatureIdRole );
    }
  }
  else if ( role == TypeRole )
    return ColumnTypeField;

  return QSortFilterProxyModel::data( index, role );
}

QVariant QgsAttributeTableFilterModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation ==  Qt::Horizontal )
  {
    if ( mColumnMapping.at( section ) == -1 && role == Qt::DisplayRole )
      return tr( "Actions" );
    else
      return QSortFilterProxyModel::headerData( section, orientation, role );
  }
  else
  {
    if ( role == Qt::DisplayRole )
      return section + 1;
    else
    {
      int sourceSection = mapToSource( index( section, ( !mColumnMapping.isEmpty() && mColumnMapping.at( 0 ) == -1 ) ? 1 : 0 ) ).row();
      return sourceModel()->headerData( sourceSection, orientation, role );
    }
  }
}

int QgsAttributeTableFilterModel::actionColumnIndex() const
{
  return mColumnMapping.indexOf( -1 );
}

int QgsAttributeTableFilterModel::columnCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );
  return mColumnMapping.count();
}

void QgsAttributeTableFilterModel::setAttributeTableConfig( const QgsAttributeTableConfig& config )
{
  mConfig = config;
  mConfig.update( layer()->fields() );

  QVector<int> newColumnMapping;

  Q_FOREACH ( const QgsAttributeTableConfig::ColumnConfig& columnConfig, mConfig.columns() )
  {
    // Hidden? Forget about this column
    if ( columnConfig.hidden )
      continue;

    // The new value for the mapping (field index or -1 for action column)
    int newValue = ( columnConfig.type == QgsAttributeTableConfig::Action ) ? -1 : layer()->fieldNameIndex( columnConfig.name );
    newColumnMapping << newValue;
  }

  if ( newColumnMapping != mColumnMapping )
  {
    bool requiresReset = false;
    int firstRemovedColumn = -1;
    int removedColumnCount = 0;

    // Check if there have a contiguous set of columns have been removed or if we require a full reset
    for ( int i = 0; i < qMin( newColumnMapping.size(), mColumnMapping.size() - removedColumnCount ); ++i )
    {
      if ( newColumnMapping.at( i ) == mColumnMapping.at( i + removedColumnCount ) )
        continue;

      if ( firstRemovedColumn == -1 )
      {
        firstRemovedColumn = i;

        while ( i < mColumnMapping.size() - removedColumnCount && mColumnMapping.at( i + removedColumnCount ) != newColumnMapping.at( i ) )
        {
          ++removedColumnCount;
        }
      }
      else
      {
        requiresReset = true;
        break;
      }
    }

    // No difference found so far
    if ( firstRemovedColumn == -1 )
    {
      if ( newColumnMapping.size() > mColumnMapping.size() )
      {
        // More columns: appended to the end
        beginInsertColumns( QModelIndex(), mColumnMapping.size(), newColumnMapping.size() - 1 );
        mColumnMapping = newColumnMapping;
        endInsertColumns();
      }
      else
      {
        // Less columns: removed from the end
        beginRemoveColumns( QModelIndex(), newColumnMapping.size(), mColumnMapping.size() - 1 );
        mColumnMapping = newColumnMapping;
        endRemoveColumns();
      }
    }
    else
    {
      if ( newColumnMapping.size() == mColumnMapping.size() - removedColumnCount )
      {
        beginRemoveColumns( QModelIndex(), firstRemovedColumn, firstRemovedColumn + removedColumnCount );
        mColumnMapping = newColumnMapping;
        endRemoveColumns();
      }
      else
      {
        requiresReset = true;
      }
    }

    if ( requiresReset )
    {
      beginResetModel();
      mColumnMapping = newColumnMapping;
      endResetModel();
    }
  }

  sort( config.sortExpression(), config.sortOrder() );
}

void QgsAttributeTableFilterModel::sort( QString expression, Qt::SortOrder order )
{
  QSortFilterProxyModel::sort( -1 );
  masterModel()->prefetchSortData( expression );
  QSortFilterProxyModel::sort( 0, order ) ;
}

QString QgsAttributeTableFilterModel::sortExpression() const
{
  return masterModel()->sortCacheExpression();
}

void QgsAttributeTableFilterModel::setSelectedOnTop( bool selectedOnTop )
{
  if ( mSelectedOnTop != selectedOnTop )
  {
    mSelectedOnTop = selectedOnTop;

    if ( sortColumn() == -1 )
    {
      sort( 0 );
    }
    invalidate();
  }
}

void QgsAttributeTableFilterModel::setSourceModel( QgsAttributeTableModel* sourceModel )
{
  mTableModel = sourceModel;

  for ( int i = 0; i < mTableModel->columnCount() - mTableModel->extraColumns(); ++i )
  {
    mColumnMapping.append( i );
  }

  QSortFilterProxyModel::setSourceModel( sourceModel );

  // Disconnect any code to update columns in the parent, we handle this manually
  disconnect( sourceModel, SIGNAL( columnsAboutToBeInserted( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsAboutToBeInserted( QModelIndex, int, int ) ) );
  disconnect( sourceModel, SIGNAL( columnsInserted( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsInserted( QModelIndex, int, int ) ) );
  disconnect( sourceModel, SIGNAL( columnsAboutToBeRemoved( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsAboutToBeRemoved( QModelIndex, int, int ) ) );
  disconnect( sourceModel, SIGNAL( columnsRemoved( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsRemoved( QModelIndex, int, int ) ) );

  connect( mTableModel, SIGNAL( columnsAboutToBeInserted( QModelIndex, int, int ) ), this, SLOT( onColumnsChanged() ) );
  connect( mTableModel, SIGNAL( columnsAboutToBeRemoved( QModelIndex, int, int ) ), this, SLOT( onColumnsChanged() ) );
}

bool QgsAttributeTableFilterModel::selectedOnTop()
{
  return mSelectedOnTop;
}

void QgsAttributeTableFilterModel::setFilteredFeatures( const QgsFeatureIds& ids )
{
  mFilteredFeatures = ids;
  setFilterMode( ShowFilteredList );
  invalidateFilter();
}

QgsFeatureIds QgsAttributeTableFilterModel::filteredFeatures()
{
  QgsFeatureIds ids;
  for ( int i = 0; i < rowCount(); ++i )
  {
    QModelIndex row = index( i, 0 );
    ids << rowToId( row );
  }
  return ids;
}

void QgsAttributeTableFilterModel::setFilterMode( FilterMode filterMode )
{
  if ( filterMode != mFilterMode )
  {
    if ( filterMode == ShowVisible )
    {
      connect( mCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentsChanged() ) );
      generateListOfVisibleFeatures();
    }
    else
    {
      disconnect( mCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentsChanged() ) );
    }

    if ( filterMode == ShowSelected )
    {
      generateListOfVisibleFeatures();
    }

    mFilterMode = filterMode;
    invalidateFilter();
  }
}

bool QgsAttributeTableFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  Q_UNUSED( sourceParent );
  switch ( mFilterMode )
  {
    case ShowAll:
      return true;

    case ShowFilteredList:
      return mFilteredFeatures.contains( masterModel()->rowToId( sourceRow ) );

    case ShowSelected:
      return layer()->selectedFeaturesIds().isEmpty() || layer()->selectedFeaturesIds().contains( masterModel()->rowToId( sourceRow ) );

    case ShowVisible:
      return mFilteredFeatures.contains( masterModel()->rowToId( sourceRow ) );

    case ShowEdited:
    {
      QgsVectorLayerEditBuffer* editBuffer = layer()->editBuffer();
      if ( editBuffer )
      {
        const QList<QgsFeatureId> addedFeatures = editBuffer->addedFeatures().keys();
        const QList<QgsFeatureId> changedFeatures = editBuffer->changedAttributeValues().keys();
        const QList<QgsFeatureId> changedGeometries = editBuffer->changedGeometries().keys();
        const QgsFeatureId fid = masterModel()->rowToId( sourceRow );
        return addedFeatures.contains( fid ) || changedFeatures.contains( fid ) || changedGeometries.contains( fid );
      }
      return false;
    }

    default:
      Q_ASSERT( false ); // In debug mode complain
      return true; // In release mode accept row
  }
  // returns are handled in their respective case statement above
}

void QgsAttributeTableFilterModel::extentsChanged()
{
  generateListOfVisibleFeatures();
  invalidateFilter();
}

void QgsAttributeTableFilterModel::selectionChanged()
{
  if ( ShowSelected == mFilterMode )
  {
    generateListOfVisibleFeatures();
    invalidateFilter();
  }
  else if ( mSelectedOnTop )
  {
    sort( sortColumn(), sortOrder() );
    invalidate();
  }
}

void QgsAttributeTableFilterModel::onColumnsChanged()
{
  setAttributeTableConfig( mConfig );
}

int QgsAttributeTableFilterModel::mapColumnToSource( int column ) const
{
  if ( mColumnMapping.isEmpty() )
    return column;
  if ( column < 0 || column >= mColumnMapping.size() )
    return -1;
  else
    return mColumnMapping.at( column );
}

void QgsAttributeTableFilterModel::generateListOfVisibleFeatures()
{
  if ( !layer() )
    return;

  bool filter = false;
  QgsRectangle rect = mCanvas->mapSettings().mapToLayerCoordinates( layer(), mCanvas->extent() );
  QgsRenderContext renderContext;
  renderContext.expressionContext() << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( layer() );
  QgsFeatureRendererV2* renderer = layer()->rendererV2();

  mFilteredFeatures.clear();

  if ( !renderer )
  {
    QgsDebugMsg( "Cannot get renderer" );
    return;
  }

  const QgsMapSettings& ms = mCanvas->mapSettings();
  if ( !layer()->isInScaleRange( ms.scale() ) )
  {
    QgsDebugMsg( "Out of scale limits" );
  }
  else
  {
    if ( renderer && renderer->capabilities() & QgsFeatureRendererV2::ScaleDependent )
    {
      // setup scale
      // mapRenderer()->renderContext()->scale is not automaticaly updated when
      // render extent changes (because it's scale is used to identify if changed
      // since last render) -> use local context
      renderContext.setExtent( ms.visibleExtent() );
      renderContext.setMapToPixel( ms.mapToPixel() );
      renderContext.setRendererScale( ms.scale() );
    }

    filter = renderer && renderer->capabilities() & QgsFeatureRendererV2::Filter;
  }

  renderer->startRender( renderContext, layer()->fields() );

  QgsFeatureRequest r( masterModel()->request() );
  if ( !r.filterRect().isNull() )
  {
    r.setFilterRect( r.filterRect().intersect( &rect ) );
  }
  else
  {
    r.setFilterRect( rect );
  }
  QgsFeatureIterator features = masterModel()->layerCache()->getFeatures( r );

  QgsFeature f;

  while ( features.nextFeature( f ) )
  {
    renderContext.expressionContext().setFeature( f );
    if ( !filter || renderer->willRenderFeature( f, renderContext ) )
    {
      mFilteredFeatures << f.id();
    }
#if 0
    if ( t.elapsed() > 5000 )
    {
      bool cancel = false;
      emit progress( i, cancel );
      if ( cancel )
        break;

      t.restart();
    }
#endif
  }

  features.close();

  if ( renderer && renderer->capabilities() & QgsFeatureRendererV2::ScaleDependent )
  {
    renderer->stopRender( renderContext );
  }
}

QgsFeatureId QgsAttributeTableFilterModel::rowToId( const QModelIndex& row )
{
  return masterModel()->rowToId( mapToSource( row ).row() );
}

QModelIndex QgsAttributeTableFilterModel::fidToIndex( QgsFeatureId fid )
{
  return mapFromMaster( masterModel()->idToIndex( fid ) );
}

QModelIndexList QgsAttributeTableFilterModel::fidToIndexList( QgsFeatureId fid )
{
  QModelIndexList indexes;
  Q_FOREACH ( const QModelIndex& idx, masterModel()->idToIndexList( fid ) )
  {
    indexes.append( mapFromMaster( idx ) );
  }

  return indexes;
}

QModelIndex QgsAttributeTableFilterModel::mapToSource( const QModelIndex& proxyIndex ) const
{
  if ( !proxyIndex.isValid() )
    return QModelIndex();

  int sourceColumn = mapColumnToSource( proxyIndex.column() );

  // For the action column there is no matching column in the source model, just return the first one
  // so we are still able to query for the feature id, the feature...
  if ( sourceColumn == -1 )
    sourceColumn = 0;

  return QSortFilterProxyModel::mapToSource( index( proxyIndex.row(), sourceColumn, proxyIndex.parent() ) );
}

QModelIndex QgsAttributeTableFilterModel::mapFromSource( const QModelIndex& sourceIndex ) const
{
  QModelIndex proxyIndex = QSortFilterProxyModel::mapFromSource( sourceIndex );

  if ( proxyIndex.column() < 0 )
    return QModelIndex();

  int col = mapColumnToSource( proxyIndex.column() );
  if ( col == -1 )
    col = 0;

  return index( proxyIndex.row(), col , proxyIndex.parent() );
}

Qt::ItemFlags QgsAttributeTableFilterModel::flags( const QModelIndex& index ) const
{
  // Handle the action column flags here, the master model doesn't know it
  if ( mapColumnToSource( index.column() ) == -1 )
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  QModelIndex source_index = mapToSource( index );
  return masterModel()->flags( source_index );
}

