/***************************************************************************
    qgspointcloudeditingindex.cpp
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudeditingindex.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayereditutils.h"
#include "qgscoordinatereferencesystem.h"


QgsPointCloudEditingIndex::QgsPointCloudEditingIndex( QgsPointCloudLayer *layer )
{
  if ( !layer ||
       !layer->dataProvider() ||
       !layer->dataProvider()->hasValidIndex() ||
       !( layer->dataProvider()->capabilities() & QgsPointCloudDataProvider::Capability::ChangeAttributeValues ) )
    return;

  mIndex = layer->dataProvider()->index();

  mAttributes = mIndex.attributes();
  mScale = mIndex.scale();
  mOffset = mIndex.offset();
  mExtent = mIndex.extent();
  mZMin = mIndex.zMin();
  mZMax = mIndex.zMax();
  mRootBounds = mIndex.rootNodeBounds();
  mSpan = mIndex.span();
  mIsValid = true;
}

std::unique_ptr<QgsAbstractPointCloudIndex> QgsPointCloudEditingIndex::clone() const
{
  return nullptr;
}

void QgsPointCloudEditingIndex::load( const QString & )
{
  return;
}

bool QgsPointCloudEditingIndex::isValid() const
{
  return mIsValid && mIndex.isValid();
}

Qgis::PointCloudAccessType QgsPointCloudEditingIndex::accessType() const
{
  return mIndex.accessType();
}

QgsCoordinateReferenceSystem QgsPointCloudEditingIndex::crs() const
{
  return mIndex.crs();
}

qint64 QgsPointCloudEditingIndex::pointCount() const
{
  return mIndex.pointCount();
}

QVariantMap QgsPointCloudEditingIndex::originalMetadata() const
{
  return mIndex.originalMetadata();
}

bool QgsPointCloudEditingIndex::hasNode( const QgsPointCloudNodeId &n ) const
{
  return mIndex.hasNode( n );
}

QgsPointCloudNode QgsPointCloudEditingIndex::getNode( const QgsPointCloudNodeId &id ) const
{
  return mIndex.getNode( id );
}

std::unique_ptr< QgsPointCloudBlock > QgsPointCloudEditingIndex::nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  if ( mEditedNodeData.contains( n ) )
  {
    const QByteArray data = mEditedNodeData.value( n );
    int nPoints = data.size() / mIndex.attributes().pointRecordSize();

    const QByteArray requestedData = QgsPointCloudLayerEditUtils::dataForAttributes( mIndex.attributes(), data, request );

    std::unique_ptr<QgsPointCloudBlock> block = std::make_unique< QgsPointCloudBlock >(
          nPoints,
          request.attributes(),
          requestedData,
          mIndex.scale(),
          mIndex.offset() );
    return block;
  }
  else
  {
    return mIndex.nodeData( n, request );
  }
}

QgsPointCloudBlockRequest *QgsPointCloudEditingIndex::asyncNodeData( const QgsPointCloudNodeId &, const QgsPointCloudRequest & )
{
  Q_ASSERT( false );
  return nullptr;
}

bool QgsPointCloudEditingIndex::commitChanges()
{
  if ( !isModified() )
    return true;

  if ( !mIndex.updateNodeData( mEditedNodeData ) )
    return false;

  mEditedNodeData.clear();
  return true;
}

bool QgsPointCloudEditingIndex::isModified() const
{
  return !mEditedNodeData.isEmpty();
}

bool QgsPointCloudEditingIndex::updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> &data )
{
  for ( auto it = data.constBegin(); it != data.constEnd(); ++it )
  {
    mEditedNodeData[it.key()] = it.value();
  }

  return true;
}
