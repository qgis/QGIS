/***************************************************************************
    qgspointcloudlayerundocommand.cpp
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerundocommand.h"
#include "qgspointcloudeditingindex.h"
#include "qgscopcpointcloudindex.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayereditutils.h"


QgsPointCloudLayerUndoCommand::QgsPointCloudLayerUndoCommand( QgsPointCloudLayer *layer )
  : mLayer( layer )
{}

QgsPointCloudLayerUndoCommandChangeAttribute::QgsPointCloudLayerUndoCommandChangeAttribute( QgsPointCloudLayer *layer, const QgsPointCloudNodeId &n, const QVector<int> &points, const QgsPointCloudAttribute &attribute, double value )
  : QgsPointCloudLayerUndoCommand( layer )
  , mNode( n )
  , mAttribute( attribute )
  , mNewValue( value )
{
  QgsPointCloudIndex index = mLayer->index();
  QgsPointCloudEditingIndex *editIndex = static_cast<QgsPointCloudEditingIndex *>( index.get() );

  if ( editIndex->mEditedNodeData.contains( n ) )
  {
    const QgsPointCloudAttributeCollection allAttributes = index.attributes();
    QgsPointCloudRequest req;
    req.setAttributes( allAttributes );
    // we want to iterate all points so we have the correct point indexes within the node
    req.setIgnoreIndexFilterEnabled( true );
    std::unique_ptr<QgsPointCloudBlock> block = index.nodeData( n, req );
    const char *ptr = block->data();
    block->attributes().find( attribute.name(), mAttributeOffset );
    const int size = block->pointRecordSize();
    for ( const int point : points )
    {
      const int offset = point * size + mAttributeOffset;
      const double oldValue = attribute.convertValueToDouble( ptr + offset );
      mPointValues[point] = oldValue;
    }
  }
  else
  {
    // If this is the first time this node is edited, we don't need the previous values, we will just discard the node from the edit index when undoing
    mFirstEditForNode = true;
    for ( const int point : points )
    {
      mPointValues[point] = std::numeric_limits<double>::quiet_NaN();
    }
  }
}

void QgsPointCloudLayerUndoCommandChangeAttribute::undo()
{
  undoRedoPrivate( true );
}

void QgsPointCloudLayerUndoCommandChangeAttribute::redo()
{
  undoRedoPrivate( false );
}

void QgsPointCloudLayerUndoCommandChangeAttribute::undoRedoPrivate( bool isUndo )
{
  QgsPointCloudEditingIndex *editIndex = static_cast<QgsPointCloudEditingIndex *>( mLayer->index().get() );
  QgsCopcPointCloudIndex *copcIndex = static_cast<QgsCopcPointCloudIndex *>( editIndex->mIndex.get() );

  QByteArray chunkData;
  if ( editIndex->mEditedNodeData.contains( mNode ) )
  {
    chunkData = editIndex->mEditedNodeData[mNode];
  }
  else
  {
    QPair<uint64_t, int32_t> offsetSizePair = copcIndex->mHierarchyNodePos[mNode];
    chunkData = copcIndex->readRange( offsetSizePair.first, offsetSizePair.second );
  }

  QByteArray data;
  if ( isUndo && mFirstEditForNode )
  {
    editIndex->mEditedNodeData.remove( mNode );
  }
  else if ( isUndo )
  {
    data = QgsPointCloudLayerEditUtils::updateChunkValues( copcIndex, chunkData, mAttribute, mNode, mPointValues );
    mLayer->index().updateNodeData( {{mNode, data}} );
  }
  else
  {
    data = QgsPointCloudLayerEditUtils::updateChunkValues( copcIndex, chunkData, mAttribute, mNode, mPointValues, mNewValue );
    mLayer->index().updateNodeData( {{mNode, data}} );
  }

  emit mLayer->chunkAttributeValuesChanged( mNode );
}
