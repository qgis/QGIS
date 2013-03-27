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
  mMasterSelection = new QItemSelectionModel( this, this );
  setSourceModel( sourceModel );
  setDynamicSortFilter( true );
  setSortRole( QgsAttributeTableModel::SortRole );
  connect( layer(), SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) );
}

bool QgsAttributeTableFilterModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  if ( mSelectedOnTop )
  {
    // TODO: does the model index (left/right) need to be converted to first?
    bool leftSelected = layer()->selectedFeaturesIds().contains( masterModel()->rowToId( left.row() ) );
    bool rightSelected = layer()->selectedFeaturesIds().contains( masterModel()->rowToId( right.row() ) );

    if ( leftSelected && !rightSelected )
    {
      return true;
    }
    else if ( rightSelected && !leftSelected )
    {
      return false;
    }
  }

  return QSortFilterProxyModel::lessThan( left, right );
}

void QgsAttributeTableFilterModel::setSelectedOnTop( bool selectedOnTop )
{
  mSelectedOnTop = selectedOnTop;
  if ( sortColumn() == -1 )
  {
    sort( 0, sortOrder() );
  }
  invalidate();
}


void QgsAttributeTableFilterModel::setSourceModel( QgsAttributeTableModel* sourceModel )
{
  mTableModel = sourceModel;
  delete mMasterSelection;
  mMasterSelection = new QItemSelectionModel( sourceModel, this );

  QSortFilterProxyModel::setSourceModel( sourceModel );

  connect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( masterSelectionChanged( QItemSelection, QItemSelection ) ) );
  // Issue a selectionChanged, so the selection gets synchronized to the map canvas selection
  selectionChanged();
}

bool QgsAttributeTableFilterModel::selectedOnTop()
{
  return mSelectedOnTop;
}

void QgsAttributeTableFilterModel::setFilteredFeatures( QgsFeatureIds ids )
{
  mFilteredFeatures = ids;
  mFilterMode = ShowFilteredList;
  announcedInvalidateFilter();
}

void QgsAttributeTableFilterModel::setFilterMode( FilterMode filterMode )
{
  if ( filterMode != mFilterMode )
  {
    if ( filterMode == ShowVisible )
    {
      connect( mCanvas, SIGNAL( extentsChanged() ), SLOT( extentsChanged() ) );
      generateListOfVisibleFeatures();
    }
    else
    {
      disconnect( SLOT( extentsChanged() ) );
    }

    mFilterMode = filterMode;
    announcedInvalidateFilter();
  }
}

void QgsAttributeTableFilterModel::selectionChanged()
{
  disconnect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( masterSelectionChanged( QItemSelection, QItemSelection ) ) );
  mMasterSelection->clear();

  QItemSelection selection;

  foreach ( QgsFeatureId fid, layer()->selectedFeaturesIds() )
  {
    selection.append( QItemSelectionRange( mTableModel->idToIndex( fid ) ) );
  }

  mMasterSelection->select( selection, QItemSelectionModel::ClearAndSelect );

  if ( mFilterMode == ShowSelected )
  {
    announcedInvalidateFilter();
  }

  if ( mSelectedOnTop )
  {
    invalidate();
    sort( sortColumn(), sortOrder() );
  }

  connect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( masterSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsAttributeTableFilterModel::masterSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  disconnect( layer(), SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );

  // Filter duplicate items (multiple columns)
  QgsFeatureIds selectedFeatureIds;
  QgsFeatureIds deselectedFeatureIds;

  foreach ( QItemSelectionRange selectedRange, selected )
  {
    foreach ( QModelIndex selectedModelIdx, selectedRange.indexes() )
    {
      selectedFeatureIds << masterModel()->rowToId( selectedModelIdx.row() );
    }
  }

  foreach ( QItemSelectionRange deselectedRange, deselected )
  {
    foreach ( QModelIndex deselectedModelIdx, deselectedRange.indexes() )
    {
      deselectedFeatureIds << masterModel()->rowToId( deselectedModelIdx.row() );
    }
  }

  // Change selection without emitting a signal
  foreach ( QgsFeatureId seletedFid, selectedFeatureIds )
  {
    layer()->select( seletedFid, false );
  }
  foreach ( QgsFeatureId deselectedFid, deselectedFeatureIds )
  {
    layer()->deselect( deselectedFid, false );
  }

  // Now emit the signal
  layer()->setSelectedFeatures( layer()->selectedFeaturesIds() );

  connect( layer(), SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );
}

bool QgsAttributeTableFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  Q_UNUSED( sourceParent );
  switch ( mFilterMode )
  {
    case ShowAll:
      return true;
      break;

    case ShowFilteredList:
      return mFilteredFeatures.contains( masterModel()->rowToId( sourceRow ) );
      break;

    case ShowSelected:
      return layer()->selectedFeaturesIds().contains( masterModel()->rowToId( sourceRow ) );
      break;

    case ShowVisible:
      return mFilteredFeatures.contains( masterModel()->rowToId( sourceRow ) );
      break;

    case ShowEdited:
    {
      QgsVectorLayerEditBuffer* editBuffer = layer()->editBuffer();
      if ( editBuffer )
      {
        const QList<QgsFeatureId> addedFeatures = editBuffer->addedFeatures().keys();
        const QList<QgsFeatureId> changedFeatures = editBuffer->changedAttributeValues().keys();
        const QgsFeatureId fid = masterModel()->rowToId( sourceRow );
        return addedFeatures.contains( fid ) || changedFeatures.contains( fid );
      }
      return false;
      break;
    }

    default:
      Q_ASSERT( false ); // In debug mode complain
      return true; // In release mode accept row
      break;
  }
  // returns are handled in their respective case statement above
}

void QgsAttributeTableFilterModel::extentsChanged()
{
  generateListOfVisibleFeatures();
  announcedInvalidateFilter();
}

void QgsAttributeTableFilterModel::announcedInvalidateFilter()
{
  emit filterAboutToBeInvalidated();
  invalidateFilter();
  emit filterInvalidated();
}

void QgsAttributeTableFilterModel::generateListOfVisibleFeatures()
{
  if ( !layer() )
    return;

  bool filter = false;
  QgsRectangle rect = mCanvas->mapRenderer()->mapToLayerCoordinates( layer(), mCanvas->extent() );
  QgsRenderContext renderContext;
  QgsFeatureRendererV2* renderer = layer()->rendererV2();

  mFilteredFeatures.clear();

  if ( !renderer )
  {
    QgsDebugMsg( "Cannot get renderer" );
  }

  if ( layer()->hasScaleBasedVisibility() &&
       ( layer()->minimumScale() > mCanvas->mapRenderer()->scale() ||
         layer()->maximumScale() <= mCanvas->mapRenderer()->scale() ) )
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
      renderContext.setExtent( mCanvas->mapRenderer()->rendererContext()->extent() );
      renderContext.setMapToPixel( mCanvas->mapRenderer()->rendererContext()->mapToPixel() );
      renderContext.setRendererScale( mCanvas->mapRenderer()->scale() );
      renderer->startRender( renderContext, layer() );
    }

    filter = renderer && renderer->capabilities() & QgsFeatureRendererV2::Filter;
  }

  QgsFeatureIterator features = masterModel()->layerCache()->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setFilterRect( rect ) );

  QgsFeature f;

  while ( features.nextFeature( f ) )
  {
    if ( !filter || renderer->willRenderFeature( f ) )
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

QItemSelectionModel* QgsAttributeTableFilterModel::masterSelection()
{
  return mMasterSelection;
}

QModelIndex QgsAttributeTableFilterModel::mapToMaster( const QModelIndex &proxyIndex ) const
{
  // Master is source
  return mapToSource( proxyIndex );
}

QModelIndex QgsAttributeTableFilterModel::mapFromMaster( const QModelIndex &sourceIndex ) const
{
  // Master is source
  return mapFromSource( sourceIndex );
}

QItemSelection QgsAttributeTableFilterModel::mapSelectionFromMaster( const QItemSelection& sourceSelection ) const
{
  // Master is source
  return mapSelectionFromMaster( sourceSelection );
}
