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

#include <QString>
#include <QtConcurrentMap>

using namespace Qt::StringLiterals;

QgsPointCloudLayerUndoCommand::QgsPointCloudLayerUndoCommand( QgsPointCloudLayer *layer )
  : mLayer( layer )
{}

QgsPointCloudLayerUndoCommandChangeAttribute::QgsPointCloudLayerUndoCommandChangeAttribute( QgsPointCloudLayer *layer, const QHash<int, QHash<QgsPointCloudNodeId, QVector<int>>> &mappedPoints, const QgsPointCloudAttribute &attribute, double value )
  : QgsPointCloudLayerUndoCommand( layer )
  , mAttribute( attribute )
  , mNewValue( value )
{
  QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"QgsPointCloudLayerUndoCommand constructor"_s );

  QList<NodeProcessData> processData;
  for ( auto it = mappedPoints.constBegin(); it != mappedPoints.constEnd(); ++it )
  {
    const int position = it.key();
    QgsPointCloudIndex index;
    if ( position < 0 )
      index = mLayer->index();
    else
      index = mLayer->subIndexes().at( position ).index();
    const auto &nodes = it.value();
    for ( auto nodeIt = nodes.constBegin(); nodeIt != nodes.constEnd(); ++nodeIt )
    {
      processData.append( { position, index, nodeIt.key(), nodeIt.value() } );
    }
  }

  std::function mapFn = [attribute]( const NodeProcessData & data )
  {
    QgsPointCloudIndex index = data.index;
    QgsPointCloudEditingIndex *editIndex = static_cast<QgsPointCloudEditingIndex *>( index.get() );
    QgsPointCloudNodeId n = data.nodeId;
    const QVector<int> &points = data.points;

    PerNodeData perNodeData;

    if ( editIndex->isNodeModified( n ) )
    {
      const QgsPointCloudAttributeCollection allAttributes = index.attributes();
      QgsPointCloudRequest req;
      req.setAttributes( allAttributes );
      // we want to iterate all points so we have the correct point indexes within the node
      req.setIgnoreIndexFilterEnabled( true );
      std::unique_ptr<QgsPointCloudBlock> block = index.nodeData( n, req );
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

    return std::pair { data.position, std::pair { n, perNodeData } };
  };

  std::function reduceFn = []( QMap<int, QHash<QgsPointCloudNodeId, PerNodeData>> &res, const std::pair<int, std::pair<QgsPointCloudNodeId, PerNodeData>> &pair )
  {
    res[pair.first][pair.second.first] = pair.second.second;
  };

  mPerNodeData = QtConcurrent::blockingMappedReduced<QMap<int, QHash<QgsPointCloudNodeId, PerNodeData>>>(
                   processData,
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

  QgsPointCloudAttribute attribute = mAttribute;
  double newValue = mNewValue;

  for ( auto it = mPerNodeData.begin(); it != mPerNodeData.end(); ++it )
  {
    const int position = it.key();
    QHash<QgsPointCloudNodeId, PerNodeData> &nodesData = it.value();

    QgsPointCloudIndex index;
    if ( position < 0 )
      index = mLayer->index();
    else
      index = mLayer->subIndexes().at( position ).index();
    QgsPointCloudEditingIndex *editIndex = dynamic_cast<QgsPointCloudEditingIndex *>( index.get() );
    QgsCopcPointCloudIndex *copcIndex = dynamic_cast<QgsCopcPointCloudIndex *>( editIndex->backingIndex().get() );

    QtConcurrent::blockingMap(
      nodesData.keyValueBegin(),
      nodesData.keyValueEnd(),
      [editIndex, copcIndex, isUndo, attribute, newValue](
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

    for ( auto itNode = nodesData.constBegin(); itNode != nodesData.constEnd(); itNode++ )
    {
      emit mLayer->chunkAttributeValuesChanged( itNode.key(), position );
    }
  }
}
