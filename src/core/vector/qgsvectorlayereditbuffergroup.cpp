/***************************************************************************
  qgsvectorlayereditbuffergroup.cpp - QgsVectorLayerEditBufferGroup

 ---------------------
 begin                : 22.12.2021
 copyright            : (C) 2021 by Damiano Lombardi
 email                : damiano@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayereditbuffergroup.h"

#include "qgsproject.h"
#include "qgstransaction.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayer.h"

#include <QQueue>

QgsVectorLayerEditBufferGroup::QgsVectorLayerEditBufferGroup( QObject *parent )
  : QObject( parent )
{

}

void QgsVectorLayerEditBufferGroup::addLayer( QgsVectorLayer *layer )
{
  mLayers.insert( layer );
}

void QgsVectorLayerEditBufferGroup::clear()
{
  mLayers.clear();
}

QSet<QgsVectorLayer *> QgsVectorLayerEditBufferGroup::layers() const
{
  return mLayers;
}

QSet<QgsVectorLayer *> QgsVectorLayerEditBufferGroup::modifiedLayers() const
{
  QSet<QgsVectorLayer *> modifiedLayers;

  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
    if ( layer->isModified() )
      modifiedLayers.insert( layer );

  return modifiedLayers;
}

bool QgsVectorLayerEditBufferGroup::startEditing()
{
  if ( mIsEditing )
    return true;

  bool editingStarted = true;
  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
  {
    if ( !layer->isValid() )
    {
      editingStarted = false;
      QgsLogger::debug( tr( "Can't start editing invalid layer '%1'." ).arg( layer->name() ) );
      break;
    }

    if ( !layer->dataProvider() )
    {
      editingStarted = false;
      QgsLogger::debug( tr( "Can't start editing layer '%1' with invalid data provider." ).arg( layer->name() ) );
      break;
    }

    // allow editing if provider supports any of the capabilities
    if ( !layer->supportsEditing() )
    {
      editingStarted = false;
      QgsLogger::debug( tr( "Can't start editing. Layer '%1' doesn't support editing." ).arg( layer->name() ) );
      break;
    }

    if ( layer->editBuffer() )
    {
      // editing already underway
      layer->editBuffer()->setEditBufferGroup( this );
      continue;
    }


    emit layer->beforeEditingStarted();
    layer->dataProvider()->enterUpdateMode();
    layer->createEditBuffer();
    layer->editBuffer()->setEditBufferGroup( this );
    layer->updateFields();
    emit layer->editingStarted();
  }

  if ( ! editingStarted )
  {
    QStringList rollbackErrors;
    if ( ! rollBack( rollbackErrors, true ) )
      QgsLogger::debug( tr( "Can't rollback after start editing failure. Roll back detailed errors: %1" ).arg( rollbackErrors.join( " / " ) ) );
  }

  mIsEditing = editingStarted;
  return mIsEditing;
}

bool QgsVectorLayerEditBufferGroup::commitChanges( QStringList &commitErrors, bool stopEditing )
{
  bool success = true;

  const QSet<QgsVectorLayer *> constModifiedLayers = modifiedLayers();
  if ( constModifiedLayers.isEmpty() )
  {
    editingFinished( stopEditing );
    mIsEditing = !stopEditing;
    return success;
  }

  QMap<QString, QSet<QgsVectorLayer *> > connectionStringsLayers;
  for ( QgsVectorLayer *modifiedLayer : constModifiedLayers )
    if ( QgsTransaction::supportsTransaction( modifiedLayer ) )
      connectionStringsLayers[QgsTransaction::connectionString( modifiedLayer->source() )].insert( modifiedLayer );

  QList<QgsVectorLayer *> transactionLayers;
  QList<std::shared_ptr<QgsTransaction> > openTransactions;
  const QStringList connectionStrings = connectionStringsLayers.keys();
  for ( const QString &connectionString : connectionStrings )
  {
    const QString providerKey = ( *connectionStringsLayers.value( connectionString ).begin() )->providerType();

    std::shared_ptr<QgsTransaction> transaction;
    transaction.reset( QgsTransaction::create( connectionString, providerKey ) );

    QString errorMsg;
    if ( ! transaction->begin( errorMsg ) )
    {
      commitErrors << tr( "ERROR: could not start a transaction on data provider '%1', detailed error: '%2'." ).arg( providerKey, errorMsg );
      success = false;
      break;
    }

    const auto constLayers = connectionStringsLayers.value( connectionString );
    for ( QgsVectorLayer *layer : constLayers )
    {
      if ( ! transaction->addLayer( layer, true ) )
      {
        commitErrors << tr( "ERROR: could not add layer '%1' to transaction on data provider '%2'." ).arg( layer->name(), providerKey );
        success = false;
        break;
      }

      transactionLayers.append( layer );
    }

    openTransactions.append( transaction );

    if ( !success )
      break;
  }

  // Order layers childrens to parents
  const QList<QgsVectorLayer *> orderedLayers = orderLayersParentsToChildren( constModifiedLayers );
  QList<QgsVectorLayer *>::const_iterator orderedLayersIterator;

  // Check geometry types
  if ( success )
  {
    for ( orderedLayersIterator = orderedLayers.constBegin(); orderedLayersIterator != orderedLayers.constEnd(); ++orderedLayersIterator )
    {
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesCheckGeometryTypeCompatibility( commitErrors );
      if ( ! success )
        break;
    }
  }

  QSet<QgsVectorLayer *> modifiedLayersOnProviderSide;

  // Change fields (add new fields, delete fields)
  if ( success )
  {
    for ( orderedLayersIterator = orderedLayers.constBegin(); orderedLayersIterator != orderedLayers.constEnd(); ++orderedLayersIterator )
    {
      QgsFields oldFields = ( *orderedLayersIterator )->fields();

      bool attributesDeleted = false;
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesDeleteAttributes( attributesDeleted, commitErrors );
      if ( ! success )
        break;

      bool attributesRenamed = false;
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesRenameAttributes( attributesRenamed, commitErrors );
      if ( ! success )
        break;

      bool attributesAdded = false;
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesAddAttributes( attributesAdded, commitErrors );
      if ( ! success )
        break;

      if ( attributesDeleted || attributesRenamed || attributesAdded )
      {
        if ( ! transactionLayers.contains( ( *orderedLayersIterator ) ) )
          modifiedLayersOnProviderSide.insert( ( *orderedLayersIterator ) );

        success = ( *orderedLayersIterator )->editBuffer()->commitChangesCheckAttributesModifications( oldFields, commitErrors );
        if ( ! success )
          break;
      }
    }
  }

  // delete all features, in reverse dependency order (children first)
  if ( success )
  {
    orderedLayersIterator = orderedLayers.constEnd();
    while ( orderedLayersIterator != orderedLayers.constBegin() )
    {
      --orderedLayersIterator;
      bool featuresDeleted;
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesDeleteFeatures( featuresDeleted, commitErrors );
      if ( ! success )
        break;

      if ( featuresDeleted && transactionLayers.contains( ( *orderedLayersIterator ) ) )
        modifiedLayersOnProviderSide.insert( ( *orderedLayersIterator ) );
    }
  }

  // add all features, in forward dependency order (parents first)
  if ( success )
  {
    for ( orderedLayersIterator = orderedLayers.constBegin(); orderedLayersIterator != orderedLayers.constEnd(); ++orderedLayersIterator )
    {
      bool featuresAdded;
      ( *orderedLayersIterator )->editBuffer()->commitChangesAddFeatures( featuresAdded, commitErrors );
      if ( ! success )
        break;

      if ( featuresAdded && transactionLayers.contains( ( *orderedLayersIterator ) ) )
        modifiedLayersOnProviderSide.insert( ( *orderedLayersIterator ) );
    }
  }

  // change all attributes and geometries in reverse dependency order (children first)
  if ( success )
  {
    orderedLayersIterator = orderedLayers.constEnd();
    while ( orderedLayersIterator != orderedLayers.constBegin() )
    {
      --orderedLayersIterator;

      bool attributesChanged;
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesChangeAttributes( attributesChanged, commitErrors );
      if ( ! success )
        break;

      if ( attributesChanged && transactionLayers.contains( ( *orderedLayersIterator ) ) )
        modifiedLayersOnProviderSide.insert( ( *orderedLayersIterator ) );
    }
  }

  // if everything went well, commit
  if ( success )
  {
    QList<std::shared_ptr<QgsTransaction> >::iterator openTransactionsIterator = openTransactions.begin();
    while ( openTransactionsIterator != openTransactions.end() )
    {
      QString errorMsg;
      if ( !( *openTransactionsIterator )->commit( errorMsg ) )
      {
        success = false;
        commitErrors << tr( "ERROR: could not commit a transaction, detailed error: '%1'." ).arg( errorMsg );
        break;
      }

      modifiedLayersOnProviderSide += connectionStringsLayers.value( ( *openTransactionsIterator )->connectionString() );
      openTransactionsIterator = openTransactions.erase( openTransactionsIterator );
    }
  }

  // Otherwise rollback
  if ( !success )
  {
    // Append additional information about layer which can't be rollbacked
    if ( ! modifiedLayersOnProviderSide.isEmpty() )
    {
      if ( modifiedLayersOnProviderSide.size() == 1 )
        commitErrors << tr( "WARNING: changes to layer '%1' where already sent to data provider and cannot be rolled back." ).arg( ( *modifiedLayersOnProviderSide.begin() )->name() );
      else
      {
        commitErrors << tr( "WARNING: changes to following layers where already sent to data provider and cannot be rolled back:" );
        for ( QgsVectorLayer *layer : std::as_const( modifiedLayersOnProviderSide ) )
          commitErrors << tr( "- '%1'" ).arg( layer->name() );
      }
    }

    QString rollbackError;
    for ( const std::shared_ptr<QgsTransaction> &transaction : openTransactions )
      transaction->rollback( rollbackError );
  }

  // Stop editing
  if ( success )
    editingFinished( stopEditing );

  if ( success && stopEditing )
    mIsEditing = false;

  return success;
}

bool QgsVectorLayerEditBufferGroup::rollBack( QStringList &rollbackErrors, bool stopEditing )
{
  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
  {
    if ( ! layer->editBuffer() )
      continue;

    if ( !layer->dataProvider() )
    {
      rollbackErrors << tr( "Layer '%1' doesn't have a valid data provider" ).arg( layer->name() );
      return false;
    }

    bool rollbackExtent = !layer->editBuffer()->deletedFeatureIds().isEmpty() ||
                          !layer->editBuffer()->addedFeatures().isEmpty() ||
                          !layer->editBuffer()->changedGeometries().isEmpty();

    emit layer->beforeRollBack();

    layer->editBuffer()->rollBack();

    emit layer->afterRollBack();

    if ( layer->isModified() )
    {
      // new undo stack roll back method
      // old method of calling every undo could cause many canvas refreshes
      layer->undoStack()->setIndex( 0 );
    }

    layer->updateFields();

    if ( stopEditing )
    {
      layer->clearEditBuffer();
      layer->undoStack()->clear();
    }
    emit layer->editingStopped();

    if ( rollbackExtent )
      layer->updateExtents();

    layer->dataProvider()->leaveUpdateMode();

    layer->triggerRepaint();
  }

  mIsEditing = ! stopEditing;
  return true;
}

bool QgsVectorLayerEditBufferGroup::isEditing() const
{
  return mIsEditing;
}

QList<QgsVectorLayer *> QgsVectorLayerEditBufferGroup::orderLayersParentsToChildren( QSet<QgsVectorLayer *> layers )
{
  QSet<QgsVectorLayer *> referencingLayers;
  QSet<QgsVectorLayer *> referencedLayers;

  {
    const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->relations().values();
    for ( const QgsRelation &relation : relations )
    {
      referencingLayers.insert( relation.referencingLayer() );
      referencedLayers.insert( relation.referencedLayer() );
    }
  }

  QList<QgsVectorLayer *> orderedLayers;

  // Layers that are only parents
  {
    QSet<QgsVectorLayer *> onlyParents = referencedLayers - referencingLayers;
    orderedLayers.append( onlyParents.values() );
  }

  // Other related layers
  {
    QSet<QgsVectorLayer *> intersection = referencedLayers;
    intersection.intersect( referencingLayers );

    QQueue<QgsVectorLayer *> otherLayersQueue;
    otherLayersQueue.append( intersection.values() );
    while ( ! otherLayersQueue.isEmpty() )
    {
      QgsVectorLayer *layer = otherLayersQueue.dequeue();

      int insertIndex = -1;
      const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencingRelations( layer );
      for ( const QgsRelation &relation : relations )
      {
        QgsVectorLayer *referencedLayer = relation.referencedLayer();
        int index = orderedLayers.indexOf( referencedLayer );
        if ( index >= 0 )
        {
          insertIndex = std::max( insertIndex, index + 1 );
        }
        else
        {
          // Check if there is a circular relation
          bool circularRelation = false;
          const QList<QgsRelation> backRelations = QgsProject::instance()->relationManager()->referencingRelations( referencedLayer );
          for ( const QgsRelation &backRelation : backRelations )
          {
            if ( backRelation.referencedLayer() == layer )
            {
              QgsLogger::warning( tr( "Circular relation between layers '%1' and '%2'. Correct saving order of layers can't be guaranteed" ).arg( layer->name(), referencedLayer->name() ) );
              insertIndex = orderedLayers.size();
              circularRelation = true;
              break;
            }
          }

          if ( !circularRelation )
          {
            insertIndex = -1;
            break;
          }
        }
      }

      // No place found this cycle
      if ( insertIndex == -1 )
      {
        otherLayersQueue.enqueue( layer );
        continue;
      }

      orderedLayers.insert( insertIndex, layer );
    }
  }

  // Layers that are only children
  {
    QSet<QgsVectorLayer *> onlyChildren = referencingLayers - referencedLayers;
    orderedLayers.append( onlyChildren.values() );
  }

  // Layers without relations (all other layers)
  {
    QSet<QgsVectorLayer *> layersWithoutRelations = layers - referencedLayers;
    layersWithoutRelations -= referencingLayers;
    orderedLayers.append( layersWithoutRelations.values() );
  }

  return orderedLayers;
}

void QgsVectorLayerEditBufferGroup::editingFinished( bool stopEditing )
{
  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
  {
    if ( !layer->mDeletedFids.empty() )
    {
      emit layer->featuresDeleted( layer->mDeletedFids );
      layer->mDeletedFids.clear();
    }

    if ( stopEditing )
      layer->clearEditBuffer();

    layer->undoStack()->clear();
    emit layer->afterCommitChanges();
    if ( stopEditing )
      emit layer->editingStopped();

    layer->updateFields();

    layer->dataProvider()->updateExtents();
    layer->dataProvider()->leaveUpdateMode();

    // This second call is required because OGR provider with JSON
    // driver might have changed fields order after the call to
    // leaveUpdateMode
    if ( layer->fields().names() != layer->dataProvider()->fields().names() )
    {
      layer->updateFields();
    }

    layer->triggerRepaint();
  }
}
