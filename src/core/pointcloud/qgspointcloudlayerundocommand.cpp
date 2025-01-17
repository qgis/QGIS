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
#include "qgspointcloudlayereditutils.h"


QgsPointCloudLayerUndoCommand::QgsPointCloudLayerUndoCommand( QgsPointCloudIndex index )
  : mIndex( index )
{}

QgsPointCloudLayerUndoCommandChangeAttribute::QgsPointCloudLayerUndoCommandChangeAttribute( QgsPointCloudIndex index, const QgsPointCloudNodeId &n, const QVector<int> &points, const QgsPointCloudAttribute &attribute, double value )
  : QgsPointCloudLayerUndoCommand( index )
  , mNode( n )
  , mAttribute( attribute )
  , mNewValue( value )
{
  const QgsPointCloudAttributeCollection allAttributes = mIndex.attributes();
  QgsPointCloudRequest req;
  req.setAttributes( allAttributes );
  std::unique_ptr<QgsPointCloudBlock> block = mIndex.nodeData( n, req );
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
  QgsPointCloudEditingIndex *editIndex = static_cast<QgsPointCloudEditingIndex *>( mIndex.get() );
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
  if ( isUndo )
  {
    data = QgsPointCloudLayerEditUtils::updateChunkValues( copcIndex, chunkData, mAttribute, mNode, mPointValues );
  }
  else
  {
    data = QgsPointCloudLayerEditUtils::updateChunkValues( copcIndex, chunkData, mAttribute, mNode, mPointValues, mNewValue );
  }

  mIndex.updateNodeData( {{mNode, data}} );
}
