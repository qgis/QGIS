#include "qgsattributetablemodel.h"
#include "qgsfeaturemodel.h"
#include "qgsfeatureselectionmodel.h"
#include "qgsvectorlayer.h"
#include <qdebug.h>

QgsFeatureSelectionModel::QgsFeatureSelectionModel( QAbstractItemModel* model, QgsFeatureModel* featureModel, QgsVectorLayer* layer, QObject* parent )
    : QItemSelectionModel( model, parent )
    , mFeatureModel( featureModel )
    , mLayer( layer )
    , mSyncEnabled( true )
    , mClearAndSelectBuffer( false )
{
  connect( mLayer, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( layerSelectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );
}

void QgsFeatureSelectionModel::enableSync( bool enable )
{
  mSyncEnabled = enable;

  if ( mSyncEnabled )
  {
    if ( mClearAndSelectBuffer )
    {
      mLayer->setSelectedFeatures( mSelectedBuffer );
    }
    else
    {
      mLayer->select( mSelectedBuffer );
      mLayer->deselect( mDeselectedBuffer );
    }

    mSelectedBuffer.clear();
    mDeselectedBuffer.clear();
    mClearAndSelectBuffer = false;
  }
}

bool QgsFeatureSelectionModel::isSelected( QgsFeatureId fid )
{
  if ( mSelectedBuffer.contains( fid ) )
    return true;

  if ( mDeselectedBuffer.contains( fid ) )
    return false;

  if ( mLayer->selectedFeaturesIds().contains( fid ) )
    return true;

  return false;
}

bool QgsFeatureSelectionModel::isSelected( const QModelIndex &index )
{
  return isSelected( index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toInt() );
}

void QgsFeatureSelectionModel::selectFeatures( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command )
{
  QgsFeatureIds ids;

  foreach ( const QModelIndex index, selection.indexes() )
  {
    QgsFeatureId id = index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toInt();

    ids << id;
  }

  disconnect( mLayer, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( layerSelectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );

  if ( command.testFlag( QItemSelectionModel::ClearAndSelect ) )
  {
    if ( !mSyncEnabled )
    {
      mClearAndSelectBuffer = true;
      foreach ( QgsFeatureId id, ids )
      {
        if ( !mDeselectedBuffer.remove( id ) )
        {
          mSelectedBuffer.insert( id );
        }
      }
    }
    else
    {
      mLayer->setSelectedFeatures( ids );
    }
  }
  else if ( command.testFlag( QItemSelectionModel::Select ) )
  {
    if ( !mSyncEnabled )
    {
      foreach ( QgsFeatureId id, ids )
      {
        if ( !mDeselectedBuffer.remove( id ) )
        {
          mSelectedBuffer.insert( id );
        }
      }
    }
    else
    {
      mLayer->select( ids );
    }
  }
  else if ( command.testFlag( QItemSelectionModel::Deselect ) )
  {
    if ( !mSyncEnabled )
    {
      foreach ( QgsFeatureId id, ids )
      {
        if ( !mSelectedBuffer.remove( id ) )
        {
          mDeselectedBuffer.insert( id );
        }
      }
    }
    else
    {
      mLayer->deselect( ids );
    }
  }

  connect( mLayer, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( layerSelectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );

  QModelIndexList updatedIndexes;
  foreach ( QModelIndex idx, selection.indexes() )
  {
    updatedIndexes.append( expandIndexToRow( idx ) );
  }

  emit requestRepaint( updatedIndexes );
}

void QgsFeatureSelectionModel::layerSelectionChanged( QgsFeatureIds selected, QgsFeatureIds deselected, bool clearAndSelect )
{
  if ( clearAndSelect )
  {
    emit requestRepaint();
  }
  else
  {
    QModelIndexList updatedIndexes;
    foreach ( QgsFeatureId fid, selected )
    {
      updatedIndexes.append( expandIndexToRow( mFeatureModel->fidToIndex( fid ) ) );
    }

    foreach ( QgsFeatureId fid, deselected )
    {
      updatedIndexes.append( expandIndexToRow( mFeatureModel->fidToIndex( fid ) ) );
    }

    emit requestRepaint( updatedIndexes );
  }
}

QModelIndexList QgsFeatureSelectionModel::expandIndexToRow( const QModelIndex& index ) const
{
  QModelIndexList indexes;
  const QAbstractItemModel* model = index.model();
  int row = index.row();

  if ( !model )
    return indexes;

  for ( int column = 0; column < model->columnCount(); ++column )
  {
    indexes.append( model->index( row, column ) );
  }

  return indexes;
}
