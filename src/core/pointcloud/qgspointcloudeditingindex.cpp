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

#include "qgscoordinatereferencesystem.h"
#include "qgscopcpointcloudindex.h"
#include "qgscopcupdate.h"
#include "qgslazdecoder.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayereditutils.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>

QgsPointCloudEditingIndex::QgsPointCloudEditingIndex( QgsPointCloudLayer *layer )
{
  if ( !layer ||
       !layer->dataProvider() ||
       !layer->dataProvider()->hasValidIndex() ||
       !( layer->dataProvider()->capabilities() & QgsPointCloudDataProvider::Capability::ChangeAttributeValues ) )
    return;

  mUri = layer->source();
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

void QgsPointCloudEditingIndex::load( const QString &, const QString & )
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

bool QgsPointCloudEditingIndex::setSubsetString( const QString &subset )
{
  return mIndex.setSubsetString( subset );
}

QString QgsPointCloudEditingIndex::subsetString() const
{
  return mIndex.subsetString();
}

std::unique_ptr< QgsPointCloudBlock > QgsPointCloudEditingIndex::nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  mEditedNodeDataMutex.lock(); // Unlocked in both branches!
  if ( mEditedNodeData.contains( n ) )
  {
    // we need to create a copy of the expression to pass to the decoder
    // as the same QgsPointCloudExpression object mighgt be concurrently
    // used on another thread, for example in a 3d view
    QgsPointCloudExpression filterExpression = QgsPointCloudExpression( request.ignoreIndexFilterEnabled() ? QString() : subsetString() );
    QgsPointCloudAttributeCollection requestAttributes = request.attributes();
    requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );

    QgsRectangle filterRect = request.filterRect();

    QByteArray rawBlockData = mEditedNodeData[n];
    mEditedNodeDataMutex.unlock();

    QgsCopcPointCloudIndex *copcIndex = static_cast<QgsCopcPointCloudIndex *>( mIndex.get() );

    int pointCount = copcIndex->mHierarchy.value( n );

    return QgsLazDecoder::decompressCopc( rawBlockData, *copcIndex->mLazInfo.get(), pointCount, requestAttributes, filterExpression, filterRect );
  }
  else
  {
    mEditedNodeDataMutex.unlock();
    return mIndex.nodeData( n, request );
  }
}

QgsPointCloudBlockRequest *QgsPointCloudEditingIndex::asyncNodeData( const QgsPointCloudNodeId &, const QgsPointCloudRequest & )
{
  Q_ASSERT( false );
  return nullptr;
}

QgsPointCloudIndex QgsPointCloudEditingIndex::backingIndex() const
{
  return mIndex;
}

const QByteArray QgsPointCloudEditingIndex::rawEditedNodeData( QgsPointCloudNodeId n ) const
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  return mEditedNodeData.value( n );
}

void QgsPointCloudEditingIndex::resetNodeEdits( QgsPointCloudNodeId n )
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  mEditedNodeData.remove( n );
}

bool QgsPointCloudEditingIndex::commitChanges( QString *errorMessage )
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  if ( mEditedNodeData.isEmpty() )
    return true;

  QHash<QgsPointCloudNodeId, QgsCopcUpdate::UpdatedChunk> updatedChunks;
  for ( auto it = mEditedNodeData.constBegin(); it != mEditedNodeData.constEnd(); ++it )
  {
    QgsPointCloudNodeId n = it.key();
    // right now we're assuming there's no change of point count
    qint32 nodePointCount = static_cast<qint32>( getNode( n ).pointCount() );
    updatedChunks[n] = QgsCopcUpdate::UpdatedChunk{ nodePointCount, it.value() };
  }

  QFileInfo fileInfo( mUri );
  const QString outputFilename = fileInfo.dir().filePath( fileInfo.baseName() + u"-update.copc.laz"_s );

  if ( !QgsCopcUpdate::writeUpdatedFile( mUri, outputFilename, updatedChunks, errorMessage ) )
  {
    return false;
  }

  // reset the underlying index - we will reload it at the end
  QgsCopcPointCloudIndex *copcIndex = static_cast<QgsCopcPointCloudIndex *>( mIndex.get() );
  copcIndex->reset();

  const QString originalFilename = fileInfo.dir().filePath( fileInfo.baseName() + u"-original.copc.laz"_s );
  if ( !QFile::rename( mUri, originalFilename ) )
  {
    if ( errorMessage )
      *errorMessage = u"Rename of the old COPC failed!"_s;
    QFile::remove( outputFilename );
    return false;
  }

  if ( !QFile::rename( outputFilename, mUri ) )
  {
    if ( errorMessage )
      *errorMessage = u"Rename of the new COPC failed!"_s;
    QFile::rename( originalFilename, mUri );
    QFile::remove( outputFilename );
    return false;
  }

  if ( !QFile::remove( originalFilename ) )
  {
    if ( errorMessage )
      *errorMessage = u"Removal of the old COPC failed!"_s;
    // TODO: cleanup here as well?
    return false;
  }

  mEditedNodeData.clear();

  // now let's reload
  copcIndex->load( mUri );

  return true;
}

bool QgsPointCloudEditingIndex::isModified() const
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  return !mEditedNodeData.isEmpty();
}

bool QgsPointCloudEditingIndex::isNodeModified( QgsPointCloudNodeId n ) const
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  return mEditedNodeData.contains( n );
}

QList<QgsPointCloudNodeId> QgsPointCloudEditingIndex::updatedNodes() const
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  return mEditedNodeData.keys();
}

bool QgsPointCloudEditingIndex::updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> &data )
{
  QMutexLocker locker( &mEditedNodeDataMutex );

  for ( auto it = data.constBegin(); it != data.constEnd(); ++it )
  {
    mEditedNodeData[it.key()] = it.value();
  }

  // get rid of cached keys that got modified
  {
    QMutexLocker locker( &sBlockCacheMutex );
    const QList<QgsPointCloudCacheKey> cacheKeys = sBlockCache.keys();
    for ( const QgsPointCloudCacheKey &cacheKey : cacheKeys )
    {
      if ( cacheKey.uri() == mUri && data.contains( cacheKey.node() ) )
        sBlockCache.remove( cacheKey );
    }
  }

  return true;
}
