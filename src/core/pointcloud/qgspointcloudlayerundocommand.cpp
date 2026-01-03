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

#include "qgscopcpointcloudindex.h"
#include "qgseventtracing.h"
#include "qgspointcloudeditingindex.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayereditutils.h"

#include <QtConcurrentMap>

QgsPointCloudLayerUndoCommand::QgsPointCloudLayerUndoCommand( QgsPointCloudLayer *layer )
  : mLayer( layer )
{}

QgsPointCloudLayerUndoCommandChangeAttribute::QgsPointCloudLayerUndoCommandChangeAttribute( QgsPointCloudLayer *layer, const QHash<QgsPointCloudNodeId, QVector<int>> &nodesAndPoints, const QgsPointCloudAttribute &attribute, double value )
  : QgsPointCloudLayerUndoCommand( layer )
  , mAttribute( attribute )
  , mNewValue( value )
{
  QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"QgsPointCloudLayerUndoCommand constructor"_s );

  QgsPointCloudIndex index = mLayer->index();
  QgsPointCloudEditingIndex *editIndex = static_cast<QgsPointCloudEditingIndex *>( index.get() );

  std::function mapFn = [editIndex, attribute, index = std::move( index )]( std::pair<const QgsPointCloudNodeId &, const QVector<int> &> pair )
  {
    QgsPointCloudNodeId n = pair.first;
    auto &points = pair.second;
    QgsPointCloudIndex index2 = index; // Copy to remove const

    PerNodeData perNodeData;

    if ( editIndex->isNodeModified( n ) )
    {
      const QgsPointCloudAttributeCollection allAttributes = index.attributes();
      QgsPointCloudRequest req;
      req.setAttributes( allAttributes );
      // we want to iterate all points so we have the correct point indexes within the node
      req.setIgnoreIndexFilterEnabled( true );
      std::unique_ptr<QgsPointCloudBlock> block = index2.nodeData( n, req );
      const char *ptr = block->data();
      block->attributes().find( attribute.name(), perNodeData.attributeOffset );
      const int size = block->pointRecordSize();
      for ( const int point : points )
      {
        const int offset = point * size + perNodeData.attributeOffset;
        const double oldValue = attribute.convertValueToDouble( ptr + offset );
        perNodeData.oldPointValues[point] = oldValue;
      }
    }
    else
    {
      // If this is the first time this node is edited, we don't need the previous values, we will just discard the node from the edit index when undoing
      // we still need the keys in mPointValues though as they are the points to be modified in the Redo stage, so we populate them with some NaNs
      perNodeData.firstEdit = true;
      for ( const int point : points )
      {
        perNodeData.oldPointValues[point] = std::numeric_limits<double>::quiet_NaN();
      }
    }

    return std::pair { n, perNodeData };
  };

  std::function reduceFn = []( QHash<QgsPointCloudNodeId, PerNodeData> &res, const std::pair<QgsPointCloudNodeId, PerNodeData> &pair )
  {
    res[pair.first] = pair.second;
  };

  mPerNodeData = QtConcurrent::blockingMappedReduced<QHash<QgsPointCloudNodeId, PerNodeData>>(
                   nodesAndPoints.keyValueBegin(), nodesAndPoints.keyValueEnd(),
                   std::move( mapFn ), std::move( reduceFn )
                 );
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
  QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"QgsPointCloudLayerUndoCommand::undoRedoPrivate"_s );
  QgsPointCloudEditingIndex *editIndex = dynamic_cast<QgsPointCloudEditingIndex *>( mLayer->index().get() );
  QgsCopcPointCloudIndex *copcIndex = dynamic_cast<QgsCopcPointCloudIndex *>( editIndex->backingIndex().get() );
  QgsPointCloudAttribute attribute = mAttribute;

  QtConcurrent::blockingMap(
    mPerNodeData.keyValueBegin(),
    mPerNodeData.keyValueEnd(),
    [editIndex, copcIndex, isUndo, attribute = mAttribute, newValue = mNewValue](
      std::pair<const QgsPointCloudNodeId &, PerNodeData &> pair
    )
  {
    QgsPointCloudNodeId node = pair.first;
    PerNodeData &perNodeData = pair.second;

    QByteArray chunkData = editIndex->rawEditedNodeData( node );
    if ( chunkData.isEmpty() ) // Not edited yet
      chunkData = copcIndex->rawNodeData( node );

    QByteArray data;
    if ( isUndo && perNodeData.firstEdit )
    {
      editIndex->resetNodeEdits( node );
    }
    else if ( isUndo )
    {
      data = QgsPointCloudLayerEditUtils::updateChunkValues( copcIndex, chunkData, attribute, node, perNodeData.oldPointValues );
      editIndex->updateNodeData( { { node, data } } );
    }
    else
    {
      data = QgsPointCloudLayerEditUtils::updateChunkValues( copcIndex, chunkData, attribute, node, perNodeData.oldPointValues, newValue );
      editIndex->updateNodeData( { { node, data } } );
    }
  }
  );

  for ( auto it = mPerNodeData.constBegin(); it != mPerNodeData.constEnd(); it++ )
  {
    emit mLayer->chunkAttributeValuesChanged( it.key() );
  }
}
