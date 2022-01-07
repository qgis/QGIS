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
  if ( ! mLayers.contains( layer ) )
    mLayers.append( layer );
}

void QgsVectorLayerEditBufferGroup::clear()
{
  mLayers.clear();
}

QList<QgsVectorLayer *> QgsVectorLayerEditBufferGroup::layers() const
{
  return mLayers;
}

QList<QgsVectorLayer *> QgsVectorLayerEditBufferGroup::modifiedLayers() const
{
  QList<QgsVectorLayer *> modifiedLayers;

  const QList<QgsVectorLayer *> constLayers = mLayers;
  for ( QgsVectorLayer *layer : constLayers )
    if ( layer->isModified() )
      modifiedLayers.append( layer );

  return modifiedLayers;
}

bool QgsVectorLayerEditBufferGroup::startEditing()
{
  bool editingStarted = true;
  const QList<QgsVectorLayer *> constLayers = mLayers;
  for ( QgsVectorLayer *layer : constLayers )
  {
    if ( layer->startEditing()
         || layer->editBuffer() )
    {
      if ( layer->editBuffer() )
        layer->editBuffer()->setEditBufferGroup( this );
    }
    else
    {
      editingStarted = false;
    }
  }

  return editingStarted;
}

bool QgsVectorLayerEditBufferGroup::commitChanges( bool stopEditing, QStringList &commitErrors )
{
  bool success = true;

  const QList<QgsVectorLayer *> constModifiedLayers = modifiedLayers();
  if ( constModifiedLayers.isEmpty() )
    return success;

  QMap<QString, QList<QgsVectorLayer *> > connectionStringsLayers;
  for ( QgsVectorLayer *modifiedLayer : constModifiedLayers )
    connectionStringsLayers[QgsTransaction::connectionString( modifiedLayer->source() )].append( modifiedLayer );

  QList<QSharedPointer<QgsTransaction> > openTransactions;
  const QStringList connectionStrings = connectionStringsLayers.keys();
  for ( const QString &connectionString : connectionStrings )
  {
    const QString providerKey = connectionStringsLayers.value( connectionString ).first()->providerType();

    QSharedPointer<QgsTransaction> transaction;
    transaction.reset( QgsTransaction::create( connectionString, providerKey ) );

    QString errorMsg;
    if ( ! transaction->begin( errorMsg ) )
    {
      commitErrors << tr( "ERROR: could not start a transaction on data provider '%1', detailled error: '%2'." ).arg( providerKey, errorMsg );
      success = false;
      break;
    }

    const auto constLayers = connectionStringsLayers.value( connectionString );
    for ( QgsVectorLayer *layer : constLayers )
    {
      if ( ! transaction->addLayer( layer ) )
      {
        commitErrors << tr( "ERROR: could not add layer '%1' to transaction on data provider '%2'." ).arg( layer->name(), providerKey );
        success = false;
        break;
      }
    }

    openTransactions.append( transaction );
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

  // Update geometries
  if ( success )
  {
    for ( orderedLayersIterator = orderedLayers.constBegin(); orderedLayersIterator != orderedLayers.constEnd(); ++orderedLayersIterator )
    {
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesUpdateGeometry( commitErrors );
      if ( ! success )
        break;
    }
  }

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
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesDeleteFeatures( commitErrors );
      if ( ! success )
        break;
    }
  }

  // add all features, in forward dependency order (parents first)
  if ( success )
  {
    for ( orderedLayersIterator = orderedLayers.constBegin(); orderedLayersIterator != orderedLayers.constEnd(); ++orderedLayersIterator )
    {
      ( *orderedLayersIterator )->editBuffer()->commitChangesAddFeatures( commitErrors );
      if ( ! success )
        break;
    }
  }

  // change all attributes in reverse dependency order (children first)
  if ( success )
  {
    orderedLayersIterator = orderedLayers.constEnd();
    while ( orderedLayersIterator != orderedLayers.constBegin() )
    {
      --orderedLayersIterator;
      success = ( *orderedLayersIterator )->editBuffer()->commitChangesChangeAttributes( commitErrors );
      if ( ! success )
        break;
    }
  }

  // change all geometries in reverse dependency order (children first)

  // if everything went well, commit
  if ( success )
  {
    QList<QSharedPointer<QgsTransaction> >::iterator openTransactionsIterator = openTransactions.begin();
    while ( openTransactionsIterator != openTransactions.end() )
    {
      QString errorMsg;
      if ( !( *openTransactionsIterator )->commit( errorMsg ) )
      {
        success = false;
        commitErrors << tr( "ERROR: could not commit a transaction, detailled error: '%1'." ).arg( errorMsg );
        break;
      }

      openTransactionsIterator = openTransactions.erase( openTransactionsIterator );
    }
  }

  // Otherwise rollback
  if ( !success )
  {
    QString rollbackError;
    for ( const QSharedPointer<QgsTransaction> &transaction : openTransactions )
      transaction->rollback( rollbackError );
  }

  // Stop editing
  if ( success )
  {
    const QList<QgsVectorLayer *> constLayers = mLayers;
    for ( QgsVectorLayer *layer : constLayers )
    {
//      if ( !mDeletedFids.empty() )
//      {
//        emit featuresDeleted( mDeletedFids );
//        mDeletedFids.clear();
//      }

      if ( stopEditing )
      {
        layer->clearEditBuffer();
      }
      layer->undoStack()->clear();
      emit layer->afterCommitChanges();
      if ( stopEditing )
        emit layer->editingStopped();

      layer->updateFields();

//      mDataProvider->updateExtents();
//      mDataProvider->leaveUpdateMode();

//      // This second call is required because OGR provider with JSON
//      // driver might have changed fields order after the call to
//      // leaveUpdateMode
//      if ( mFields.names() != mDataProvider->fields().names() )
//      {
//        updateFields();
//      }

      layer->triggerRepaint();
    }
  }

  return success;
}

bool QgsVectorLayerEditBufferGroup::rollBack()
{
  const QList<QgsVectorLayer *> constLayers = mLayers;
  for ( QgsVectorLayer *layer : constLayers )
    layer->rollBack();
//    if( layer->editBuffer() )
//      layer->editBuffer()->rollBack();


  return false;
}

QList<QgsVectorLayer *> QgsVectorLayerEditBufferGroup::orderLayersParentsToChildren( QList<QgsVectorLayer *> layers )
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
          insertIndex = qMax( insertIndex, index + 1 );
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

  // Layers without relations
  {
    QSet<QgsVectorLayer *> layersWithoutRelations = layers.toSet() - referencedLayers;
    layersWithoutRelations -= referencingLayers;
    orderedLayers.append( layersWithoutRelations.values() );
  }

  return orderedLayers;
}
