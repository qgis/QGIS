/***************************************************************************
    qgsfeatureselectionmodel.cpp
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributetablemodel.h"
#include "qgsfeaturemodel.h"
#include "qgsifeatureselectionmanager.h"
#include "qgsfeatureselectionmodel.h"
#include "moc_qgsfeatureselectionmodel.cpp"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

QgsFeatureSelectionModel::QgsFeatureSelectionModel( QAbstractItemModel *model, QgsFeatureModel *featureModel, QgsIFeatureSelectionManager *featureSelectionManager, QObject *parent )
  : QItemSelectionModel( model, parent )
  , mFeatureModel( featureModel )
  , mSyncEnabled( true )
  , mClearAndSelectBuffer( false )
{
  setFeatureSelectionManager( featureSelectionManager );
}

void QgsFeatureSelectionModel::enableSync( bool enable )
{
  mSyncEnabled = enable;

  if ( mSyncEnabled )
  {
    if ( mClearAndSelectBuffer )
    {
      mFeatureSelectionManager->setSelectedFeatures( mSelectedBuffer );
    }
    else
    {
      mFeatureSelectionManager->select( mSelectedBuffer );
      mFeatureSelectionManager->deselect( mDeselectedBuffer );
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

  if ( !mClearAndSelectBuffer && mFeatureSelectionManager->selectedFeatureIds().contains( fid ) )
    return true;

  return false;
}

bool QgsFeatureSelectionModel::isSelected( const QModelIndex &index )
{
  return isSelected( index.model()->data( index, static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ).toLongLong() );
}

void QgsFeatureSelectionModel::selectFeatures( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command )
{
  QgsFeatureIds ids;

  QgsDebugMsgLevel( QStringLiteral( "Index count: %1" ).arg( selection.indexes().size() ), 2 );

  const auto constIndexes = selection.indexes();
  for ( const QModelIndex &index : constIndexes )
  {
    const QgsFeatureId id = index.model()->data( index, static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ).toLongLong();

    ids << id;
  }

  disconnect( mFeatureSelectionManager, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsFeatureSelectionModel::layerSelectionChanged );

  if ( command.testFlag( QItemSelectionModel::ClearAndSelect ) )
  {
    if ( !mSyncEnabled )
    {
      mClearAndSelectBuffer = true;
      const auto constIds = ids;
      for ( const QgsFeatureId id : constIds )
      {
        if ( !mDeselectedBuffer.remove( id ) )
        {
          mSelectedBuffer.insert( id );
        }
      }
    }
    else
    {
      mFeatureSelectionManager->setSelectedFeatures( ids );
    }
  }
  else if ( command.testFlag( QItemSelectionModel::Select ) )
  {
    if ( !mSyncEnabled )
    {
      const auto constIds = ids;
      for ( const QgsFeatureId id : constIds )
      {
        if ( !mDeselectedBuffer.remove( id ) )
        {
          mSelectedBuffer.insert( id );
        }
      }
    }
    else
    {
      mFeatureSelectionManager->select( ids );
    }
  }
  else if ( command.testFlag( QItemSelectionModel::Deselect ) )
  {
    if ( !mSyncEnabled )
    {
      const auto constIds = ids;
      for ( const QgsFeatureId id : constIds )
      {
        if ( !mSelectedBuffer.remove( id ) )
        {
          mDeselectedBuffer.insert( id );
        }
      }
    }
    else
    {
      mFeatureSelectionManager->deselect( ids );
    }
  }

  connect( mFeatureSelectionManager, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsFeatureSelectionModel::layerSelectionChanged );

  QModelIndexList updatedIndexes;
  const auto indexes = selection.indexes();
  for ( const QModelIndex &idx : indexes )
  {
    updatedIndexes.append( expandIndexToRow( idx ) );
  }

  emit requestRepaint( updatedIndexes );
}

void QgsFeatureSelectionModel::setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager )
{
  if ( mFeatureSelectionManager )
    disconnect( mFeatureSelectionManager, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsFeatureSelectionModel::layerSelectionChanged );

  mFeatureSelectionManager = featureSelectionManager;

  connect( mFeatureSelectionManager, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsFeatureSelectionModel::layerSelectionChanged );
}

void QgsFeatureSelectionModel::layerSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect )
{
  if ( clearAndSelect )
  {
    emit requestRepaint();
  }
  else
  {
    QModelIndexList updatedIndexes;
    const auto constSelected = selected;
    for ( const QgsFeatureId fid : constSelected )
    {
      updatedIndexes.append( expandIndexToRow( mFeatureModel->fidToIndex( fid ) ) );
    }

    const auto constDeselected = deselected;
    for ( const QgsFeatureId fid : constDeselected )
    {
      updatedIndexes.append( expandIndexToRow( mFeatureModel->fidToIndex( fid ) ) );
    }

    emit requestRepaint( updatedIndexes );
  }
}

QModelIndexList QgsFeatureSelectionModel::expandIndexToRow( const QModelIndex &index ) const
{
  QModelIndexList indexes;
  const QAbstractItemModel *model = index.model();
  const int row = index.row();

  if ( !model )
    return indexes;

  const int columns = model->columnCount();
  indexes.reserve( columns );
  for ( int column = 0; column < columns; ++column )
  {
    indexes.append( model->index( row, column ) );
  }

  return indexes;
}
