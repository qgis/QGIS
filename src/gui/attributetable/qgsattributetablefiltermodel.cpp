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
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsfeature.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgsrenderer.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsapplication.h"
#include "qgsvectorlayercache.h"
#include "qgsrendercontext.h"
#include "qgsmapcanvasutils.h"

//////////////////
// Filter Model //
//////////////////

QgsAttributeTableFilterModel::QgsAttributeTableFilterModel( QgsMapCanvas *canvas, QgsAttributeTableModel *sourceModel, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mCanvas( canvas )
{
  setSourceModel( sourceModel );
  setDynamicSortFilter( true );
  setSortRole( QgsAttributeTableModel::SortRole );
  connect( layer(), &QgsVectorLayer::selectionChanged, this, &QgsAttributeTableFilterModel::selectionChanged );

  mReloadVisibleTimer.setSingleShot( true );
  connect( &mReloadVisibleTimer, &QTimer::timeout, this, &QgsAttributeTableFilterModel::reloadVisible );
  mFilterFeaturesTimer.setSingleShot( true );
  connect( &mFilterFeaturesTimer, &QTimer::timeout, this, &QgsAttributeTableFilterModel::filterFeatures );
}

bool QgsAttributeTableFilterModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  if ( mSelectedOnTop )
  {
    const bool leftSelected = layer()->selectedFeatureIds().contains( masterModel()->rowToId( left.row() ) );
    const bool rightSelected = layer()->selectedFeatureIds().contains( masterModel()->rowToId( right.row() ) );

    if ( leftSelected && !rightSelected )
    {
      return sortOrder() == Qt::AscendingOrder;
    }
    else if ( rightSelected && !leftSelected )
    {
      return sortOrder() == Qt::DescendingOrder;
    }
  }

  if ( mTableModel->sortCacheExpression().isEmpty() )
  {
    //shortcut when no sort order set
    return false;
  }

  return qgsVariantLessThan( left.data( QgsAttributeTableModel::SortRole ),
                             right.data( QgsAttributeTableModel::SortRole ) );
}

void QgsAttributeTableFilterModel::sort( int column, Qt::SortOrder order )
{
  if ( order != Qt::AscendingOrder && order != Qt::DescendingOrder )
    order = Qt::AscendingOrder;
  if ( column < 0 || column >= mColumnMapping.size() )
  {
    sort( QString() );
  }
  else
  {
    const int myColumn = mColumnMapping.at( column );
    masterModel()->prefetchColumnData( myColumn );
    QSortFilterProxyModel::sort( myColumn, order );
  }
  emit sortColumnChanged( column, order );
}

QVariant QgsAttributeTableFilterModel::data( const QModelIndex &index, int role ) const
{
  if ( mapColumnToSource( index.column() ) == -1 ) // actions
  {
    if ( role == TypeRole )
      return ColumnTypeActionButton;
    else if ( role == QgsAttributeTableModel::FeatureIdRole )
    {
      const QModelIndex fieldIndex = QSortFilterProxyModel::mapToSource( QSortFilterProxyModel::index( index.row(), 0, index.parent() ) );
      return sourceModel()->data( fieldIndex, QgsAttributeTableModel::FeatureIdRole );
    }
  }
  else if ( role == TypeRole )
    return ColumnTypeField;

  return QSortFilterProxyModel::data( index, role );
}

QVariant QgsAttributeTableFilterModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
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
      const int sourceSection = mapToSource( index( section, ( !mColumnMapping.isEmpty() && mColumnMapping.at( 0 ) == -1 ) ? 1 : 0 ) ).row();
      return sourceModel()->headerData( sourceSection, orientation, role );
    }
  }
}

int QgsAttributeTableFilterModel::actionColumnIndex() const
{
  return mColumnMapping.indexOf( -1 );
}

int QgsAttributeTableFilterModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mColumnMapping.count();
}

void QgsAttributeTableFilterModel::setAttributeTableConfig( const QgsAttributeTableConfig &config, bool force )
{
  const QgsAttributeTableConfig oldConfig = mConfig;
  mConfig = config;
  mConfig.update( layer()->fields() );

  if ( !force && mConfig.hasSameColumns( oldConfig ) )
  {
    return;
  }

  QVector<int> newColumnMapping;
  const auto constColumns = mConfig.columns();
  for ( const QgsAttributeTableConfig::ColumnConfig &columnConfig : constColumns )
  {
    // Hidden? Forget about this column
    if ( columnConfig.hidden )
      continue;

    // The new value for the mapping (field index or -1 for action column)
    const int newValue = ( columnConfig.type == QgsAttributeTableConfig::Action ) ? -1 : layer()->fields().lookupField( columnConfig.name );
    newColumnMapping << newValue;
  }

  if ( newColumnMapping != mColumnMapping )
  {
    bool requiresReset = false;
    int firstRemovedColumn = -1;
    int removedColumnCount = 0;

    // Check if there have a contiguous set of columns have been removed or if we require a full reset
    for ( int i = 0; i < std::min( newColumnMapping.size(), mColumnMapping.size() - removedColumnCount ); ++i )
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
        //the amount of removed column in the model need to be equal removedColumnCount
        beginRemoveColumns( QModelIndex(), firstRemovedColumn, firstRemovedColumn + removedColumnCount - 1 );
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

  if ( !config.sortExpression().isEmpty() )
    sort( config.sortExpression(), config.sortOrder() );
}

void QgsAttributeTableFilterModel::setFilterExpression( const QgsExpression &expression, const QgsExpressionContext &context )
{
  mFilterExpression = expression;
  mFilterExpressionContext = context;
}

void QgsAttributeTableFilterModel::sort( const QString &expression, Qt::SortOrder order )
{
  if ( order != Qt::AscendingOrder && order != Qt::DescendingOrder )
    order = Qt::AscendingOrder;

  QSortFilterProxyModel::sort( -1 );
  masterModel()->prefetchSortData( expression );
  QSortFilterProxyModel::sort( 0, order );
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
    int column = sortColumn();
    Qt::SortOrder order = sortOrder();

    // set default sort values if they are not correctly set
    if ( column < 0 )
      column = 0;

    if ( order != Qt::AscendingOrder && order != Qt::DescendingOrder )
      order = Qt::AscendingOrder;

    sort( 0, Qt::AscendingOrder );
    invalidate();
  }
}

void QgsAttributeTableFilterModel::setSourceModel( QgsAttributeTableModel *sourceModel )
{
  mTableModel = sourceModel;

  for ( int i = 0; i < mTableModel->columnCount() - mTableModel->extraColumns(); ++i )
  {
    mColumnMapping.append( i );
  }

  QSortFilterProxyModel::setSourceModel( sourceModel );

  // Disconnect any code to update columns in the parent, we handle this manually
  disconnect( mTableModel, SIGNAL( columnsAboutToBeInserted( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsAboutToBeInserted( QModelIndex, int, int ) ) );
  disconnect( mTableModel, SIGNAL( columnsInserted( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsInserted( QModelIndex, int, int ) ) );
  disconnect( mTableModel, SIGNAL( columnsAboutToBeRemoved( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsAboutToBeRemoved( QModelIndex, int, int ) ) );
  disconnect( mTableModel, SIGNAL( columnsRemoved( QModelIndex, int, int ) ), this, SLOT( _q_sourceColumnsRemoved( QModelIndex, int, int ) ) );
  // The following connections are needed in order to keep the filter model in sync, see: regression #15974
  connect( mTableModel, &QAbstractItemModel::columnsAboutToBeInserted, this, &QgsAttributeTableFilterModel::onColumnsChanged );
  connect( mTableModel, &QAbstractItemModel::columnsAboutToBeRemoved, this, &QgsAttributeTableFilterModel::onColumnsChanged );

}

bool QgsAttributeTableFilterModel::selectedOnTop()
{
  return mSelectedOnTop;
}

void QgsAttributeTableFilterModel::setFilteredFeatures( const QgsFeatureIds &ids )
{
  mFilteredFeatures = ids;
  setFilterMode( ShowFilteredList );
  invalidateFilter();
}

QgsFeatureIds QgsAttributeTableFilterModel::filteredFeatures()
{
  QgsFeatureIds ids;
  ids.reserve( rowCount() );
  for ( int i = 0; i < rowCount(); ++i )
  {
    const QModelIndex row = index( i, 0 );
    ids << rowToId( row );
  }
  return ids;
}

void QgsAttributeTableFilterModel::setFilterMode( FilterMode filterMode )
{
  if ( filterMode != mFilterMode )
  {
    disconnectFilterModeConnections();
    connectFilterModeConnections( filterMode );
    mFilterMode = filterMode;
    invalidate();
  }
}

void QgsAttributeTableFilterModel::disconnectFilterModeConnections()
{
  // cleanup existing connections
  switch ( mFilterMode )
  {
    case ShowVisible:
      disconnect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      disconnect( mCanvas, &QgsMapCanvas::temporalRangeChanged, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      disconnect( layer(), &QgsVectorLayer::featureAdded, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      disconnect( layer(), &QgsVectorLayer::geometryChanged, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      break;
    case ShowAll:
    case ShowEdited:
    case ShowSelected:
      break;
    case ShowFilteredList:
      disconnect( layer(), &QgsVectorLayer::featureAdded, this, &QgsAttributeTableFilterModel::startTimedFilterFeatures );
      disconnect( layer(), &QgsVectorLayer::attributeValueChanged, this, &QgsAttributeTableFilterModel::onAttributeValueChanged );
      disconnect( layer(), &QgsVectorLayer::geometryChanged, this, &QgsAttributeTableFilterModel::onGeometryChanged );
      break;
  }
}

void QgsAttributeTableFilterModel::connectFilterModeConnections( QgsAttributeTableFilterModel::FilterMode filterMode )
{
  // setup new connections
  switch ( filterMode )
  {
    case ShowVisible:
      connect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      connect( mCanvas, &QgsMapCanvas::temporalRangeChanged, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      connect( layer(), &QgsVectorLayer::featureAdded, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      connect( layer(), &QgsVectorLayer::geometryChanged, this, &QgsAttributeTableFilterModel::startTimedReloadVisible );
      generateListOfVisibleFeatures();
      break;
    case ShowAll:
    case ShowEdited:
    case ShowSelected:
      break;
    case ShowFilteredList:
      connect( layer(), &QgsVectorLayer::featureAdded, this, &QgsAttributeTableFilterModel::startTimedFilterFeatures );
      connect( layer(), &QgsVectorLayer::attributeValueChanged, this, &QgsAttributeTableFilterModel::onAttributeValueChanged );
      connect( layer(), &QgsVectorLayer::geometryChanged, this, &QgsAttributeTableFilterModel::onGeometryChanged );
      break;
  }
}

bool QgsAttributeTableFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  Q_UNUSED( sourceParent )
  switch ( mFilterMode )
  {
    case ShowAll:
      return true;

    case ShowFilteredList:
      return mFilteredFeatures.contains( masterModel()->rowToId( sourceRow ) );

    case ShowSelected:
      return layer()->selectedFeatureIds().contains( masterModel()->rowToId( sourceRow ) );

    case ShowVisible:
      return mFilteredFeatures.contains( masterModel()->rowToId( sourceRow ) );

    case ShowEdited:
    {
      QgsVectorLayerEditBuffer *editBuffer = layer()->editBuffer();
      if ( editBuffer )
      {
        const QgsFeatureId fid = masterModel()->rowToId( sourceRow );

        if ( editBuffer->isFeatureAdded( fid ) )
          return true;

        if ( editBuffer->isFeatureAttributesChanged( fid ) )
          return true;

        if ( editBuffer->isFeatureGeometryChanged( fid ) )
          return true;

        return false;
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
  reloadVisible();
}

void QgsAttributeTableFilterModel::reloadVisible()
{
  generateListOfVisibleFeatures();
  invalidateFilter();
  emit visibleReloaded();
}

void QgsAttributeTableFilterModel::onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  Q_UNUSED( fid );
  Q_UNUSED( value );

  if ( mFilterExpression.referencedAttributeIndexes( layer()->fields() ).contains( idx ) )
  {
    startTimedFilterFeatures();
  }
}

void QgsAttributeTableFilterModel::onGeometryChanged()
{
  if ( mFilterExpression.needsGeometry() )
  {
    startTimedFilterFeatures();
  }
}

void QgsAttributeTableFilterModel::startTimedReloadVisible()
{
  mReloadVisibleTimer.start( 10 );
}

void QgsAttributeTableFilterModel::startTimedFilterFeatures()
{
  mFilterFeaturesTimer.start( 10 );
}

void QgsAttributeTableFilterModel::filterFeatures()
{
  if ( !mFilterExpression.isValid() )
    return;

  QgsFeatureIds filteredFeatures;
  QgsDistanceArea distanceArea;

  distanceArea.setSourceCrs( mTableModel->layer()->crs(), QgsProject::instance()->transformContext() );
  distanceArea.setEllipsoid( QgsProject::instance()->ellipsoid() );

  const bool fetchGeom = mFilterExpression.needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  mFilterExpression.setGeomCalculator( &distanceArea );
  mFilterExpression.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  mFilterExpression.setAreaUnits( QgsProject::instance()->areaUnits() );
  QgsFeatureRequest request( mTableModel->request() );
  request.setSubsetOfAttributes( mFilterExpression.referencedColumns(), mTableModel->layer()->fields() );
  if ( !fetchGeom )
  {
    request.setFlags( QgsFeatureRequest::NoGeometry );
  }
  else
  {
    // force geometry extraction if the filter requests it
    request.setFlags( request.flags() & ~QgsFeatureRequest::NoGeometry );
  }
  QgsFeatureIterator featIt = mTableModel->layer()->getFeatures( request );

  QgsFeature f;

  // Record the first evaluation error
  QString error;

  while ( featIt.nextFeature( f ) )
  {
    mFilterExpressionContext.setFeature( f );
    if ( mFilterExpression.evaluate( &mFilterExpressionContext ).toInt() != 0 )
      filteredFeatures << f.id();

    // check if there were errors during evaluating
    if ( mFilterExpression.hasEvalError() && error.isEmpty() )
    {
      error = mFilterExpression.evalErrorString( );
    }
  }

  featIt.close();

  setFilteredFeatures( filteredFeatures );

  QApplication::restoreOverrideCursor();

  emit featuresFiltered();

  if ( ! error.isEmpty() )
  {
    emit filterError( error );
  }

}


void QgsAttributeTableFilterModel::selectionChanged()
{
  if ( ShowSelected == mFilterMode )
  {
    invalidateFilter();
  }
  else if ( mSelectedOnTop )
  {
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

int QgsAttributeTableFilterModel::mapColumnFromSource( int column ) const
{
  if ( mColumnMapping.isEmpty() )
    return column;
  else
    return mColumnMapping.indexOf( column );
}

void QgsAttributeTableFilterModel::generateListOfVisibleFeatures()
{
  if ( !layer() )
    return;

  bool filter = false;
  const QgsRectangle rect = mCanvas->mapSettings().mapToLayerCoordinates( layer(), mCanvas->extent() );
  QgsRenderContext renderContext;
  renderContext.expressionContext().appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer() ) );

  mFilteredFeatures.clear();
  if ( !layer()->renderer() )
  {
    QgsDebugMsg( QStringLiteral( "Cannot get renderer" ) );
    return;
  }

  std::unique_ptr< QgsFeatureRenderer > renderer( layer()->renderer()->clone() );

  const QgsMapSettings &ms = mCanvas->mapSettings();
  if ( !layer()->isInScaleRange( ms.scale() ) )
  {
    QgsDebugMsg( QStringLiteral( "Out of scale limits" ) );
  }
  else
  {
    if ( renderer->capabilities() & QgsFeatureRenderer::ScaleDependent )
    {
      // setup scale
      // mapRenderer()->renderContext()->scale is not automatically updated when
      // render extent changes (because it's scale is used to identify if changed
      // since last render) -> use local context
      renderContext.setExtent( ms.visibleExtent() );
      renderContext.setMapToPixel( ms.mapToPixel() );
      renderContext.setRendererScale( ms.scale() );
    }

    filter = renderer->capabilities() & QgsFeatureRenderer::Filter;
  }

  renderer->startRender( renderContext, layer()->fields() );

  QgsFeatureRequest r( masterModel()->request() );
  if ( r.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox )
  {
    r.setFilterRect( r.filterRect().intersect( rect ) );
  }
  else
  {
    r.setFilterRect( rect );
  }

  const QString canvasFilter = QgsMapCanvasUtils::filterForLayer( mCanvas, layer() );
  if ( canvasFilter == QLatin1String( "FALSE" ) )
    return;
  if ( !canvasFilter.isEmpty() )
    r.setFilterExpression( canvasFilter );

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

  if ( renderer->capabilities() & QgsFeatureRenderer::ScaleDependent )
  {
    renderer->stopRender( renderContext );
  }
}

QgsFeatureId QgsAttributeTableFilterModel::rowToId( const QModelIndex &row )
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
  const auto constIdToIndexList = masterModel()->idToIndexList( fid );
  for ( const QModelIndex &idx : constIdToIndexList )
  {
    indexes.append( mapFromMaster( idx ) );
  }

  return indexes;
}

QModelIndex QgsAttributeTableFilterModel::mapToSource( const QModelIndex &proxyIndex ) const
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

QModelIndex QgsAttributeTableFilterModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
  const QModelIndex proxyIndex = QSortFilterProxyModel::mapFromSource( sourceIndex );

  if ( proxyIndex.column() < 0 )
    return QModelIndex();

  int col = mapColumnFromSource( proxyIndex.column() );

  if ( col == -1 )
    col = 0;

  return index( proxyIndex.row(), col, proxyIndex.parent() );
}

Qt::ItemFlags QgsAttributeTableFilterModel::flags( const QModelIndex &index ) const
{
  // Handle the action column flags here, the master model doesn't know it
  if ( mapColumnToSource( index.column() ) == -1 )
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  const QModelIndex source_index = mapToSource( index );
  return masterModel()->flags( source_index );
}
