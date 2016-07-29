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
#include "qgsvectorlayer.h"
#include <qdebug.h>

QgsFeatureSelectionModel::QgsFeatureSelectionModel( QAbstractItemModel* model, QgsFeatureModel* featureModel, QgsIFeatureSelectionManager* featureSelectionManager, QObject* parent )
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

  if ( !mClearAndSelectBuffer && mFeatureSelectionManager->selectedFeaturesIds().contains( fid ) )
    return true;

  return false;
}

bool QgsFeatureSelectionModel::isSelected( const QModelIndex &index )
{
  return isSelected( index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong() );
}

void QgsFeatureSelectionModel::selectFeatures( const QItemSelection &selection, const QItemSelectionModel::SelectionFlags& command )
{
  QgsFeatureIds ids;

  QgsDebugMsg( QString( "Index count: %1" ).arg( selection.indexes().size() ) );

  Q_FOREACH ( const QModelIndex& index, selection.indexes() )
  {
    QgsFeatureId id = index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong();

    ids << id;
  }

  disconnect( mFeatureSelectionManager, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( layerSelectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );

  if ( command.testFlag( QItemSelectionModel::ClearAndSelect ) )
  {
    if ( !mSyncEnabled )
    {
      mClearAndSelectBuffer = true;
      Q_FOREACH ( QgsFeatureId id, ids )
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
      Q_FOREACH ( QgsFeatureId id, ids )
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
      Q_FOREACH ( QgsFeatureId id, ids )
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

  connect( mFeatureSelectionManager, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( layerSelectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );

  QModelIndexList updatedIndexes;
  Q_FOREACH ( const QModelIndex& idx, selection.indexes() )
  {
    updatedIndexes.append( expandIndexToRow( idx ) );
  }

  emit requestRepaint( updatedIndexes );
}

void QgsFeatureSelectionModel::setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager )
{
  mFeatureSelectionManager = featureSelectionManager;

  connect( mFeatureSelectionManager, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( layerSelectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );
}

void QgsFeatureSelectionModel::layerSelectionChanged( const QgsFeatureIds& selected, const QgsFeatureIds& deselected, bool clearAndSelect )
{
  if ( clearAndSelect )
  {
    emit requestRepaint();
  }
  else
  {
    QModelIndexList updatedIndexes;
    Q_FOREACH ( QgsFeatureId fid, selected )
    {
      updatedIndexes.append( expandIndexToRow( mFeatureModel->fidToIndex( fid ) ) );
    }

    Q_FOREACH ( QgsFeatureId fid, deselected )
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

  int columns = model->columnCount();
  indexes.reserve( columns );
  for ( int column = 0; column < columns; ++column )
  {
    indexes.append( model->index( row, column ) );
  }

  return indexes;
}
