/***************************************************************************
                         qgspointcloudindex.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudindex.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>
#include <qglobal.h>
#include <qstringliteral.h>

#include "qgsbox3d.h"
#include "qgstiledownloadmanager.h"
#include "qgspointcloudstatistics.h"
#include "qgslogger.h"
#include "qgspointcloudeditingindex.h"

QgsPointCloudNodeId::QgsPointCloudNodeId():
  mD( -1 ),
  mX( 0 ),
  mY( 0 ),
  mZ( 0 )
{}

QgsPointCloudNodeId::QgsPointCloudNodeId( int _d, int _x, int _y, int _z ):
  mD( _d ),
  mX( _x ),
  mY( _y ),
  mZ( _z )
{}

QgsPointCloudNodeId QgsPointCloudNodeId::parentNode() const
{
  return QgsPointCloudNodeId( mD - 1, mX / 2, mY / 2, mZ / 2 );
}

QgsPointCloudNodeId QgsPointCloudNodeId::fromString( const QString &str )
{
  QStringList lst = str.split( '-' );
  if ( lst.count() != 4 )
    return QgsPointCloudNodeId();
  return QgsPointCloudNodeId( lst[0].toInt(), lst[1].toInt(), lst[2].toInt(), lst[3].toInt() );
}

QString QgsPointCloudNodeId::toString() const
{
  return QStringLiteral( "%1-%2-%3-%4" ).arg( mD ).arg( mX ).arg( mY ).arg( mZ );
}

int QgsPointCloudNodeId::d() const
{
  return mD;
}

int QgsPointCloudNodeId::x() const
{
  return mX;
}

int QgsPointCloudNodeId::y() const
{
  return mY;
}

int QgsPointCloudNodeId::z() const
{
  return mZ;
}

uint qHash( QgsPointCloudNodeId id )
{
  return id.d() + id.x() + id.y() + id.z();
}

///@cond PRIVATE

//
// QgsPointCloudCacheKey
//

QgsPointCloudCacheKey::QgsPointCloudCacheKey( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri )
  : mNode( n )
  , mUri( uri )
  , mRequest( request )
  , mFilterExpression( expression )
{
}

bool QgsPointCloudCacheKey::operator==( const QgsPointCloudCacheKey &other ) const
{
  return mNode == other.mNode &&
         mUri == other.mUri &&
         mRequest == other.mRequest &&
         mFilterExpression == other.mFilterExpression;
}

uint qHash( const QgsPointCloudCacheKey &key )
{
  return qHash( key.node() ) ^ qHash( key.request() ) ^ qHash( key.uri() ) ^ qHash( key.filterExpression() );
}

//
// QgsPointCloudNode
//

QgsBox3D QgsPointCloudNode::bounds() const
{
  return mBounds;
}

QgsBox3D QgsPointCloudNode::bounds( QgsBox3D rootBounds, QgsPointCloudNodeId id )
{
  const double d = rootBounds.xMaximum() - rootBounds.xMinimum();
  const double dLevel = d / pow( 2, id.d() );

  const double xMin = rootBounds.xMinimum() + dLevel * id.x();
  const double xMax = rootBounds.xMinimum() + dLevel * ( id.x() + 1 );
  const double yMin = rootBounds.yMinimum() + dLevel * id.y();
  const double yMax = rootBounds.yMinimum() + dLevel * ( id.y() + 1 );
  const double zMin = rootBounds.zMinimum() + dLevel * id.z();
  const double zMax = rootBounds.zMinimum() + dLevel * ( id.z() + 1 );

  return QgsBox3D( xMin, yMin, zMin, xMax, yMax, zMax );
}

float QgsPointCloudNode::error() const
{
  return mError;
}

///@endcond

//
// QgsAbstractPointCloudIndex
//

QMutex QgsAbstractPointCloudIndex::sBlockCacheMutex;
QCache<QgsPointCloudCacheKey, QgsPointCloudBlock> QgsAbstractPointCloudIndex::sBlockCache( 200'000'000 ); // 200MB of cached points

QgsAbstractPointCloudIndex::QgsAbstractPointCloudIndex() = default;

QgsAbstractPointCloudIndex::~QgsAbstractPointCloudIndex() = default;

bool QgsAbstractPointCloudIndex::hasNode( const QgsPointCloudNodeId &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );
  return mHierarchy.contains( n );
}

QgsPointCloudNode QgsAbstractPointCloudIndex::getNode( const QgsPointCloudNodeId &id ) const
{
  Q_ASSERT( hasNode( id ) );

  qint64 pointCount;
  {
    QMutexLocker locker( &mHierarchyMutex );
    pointCount = mHierarchy.value( id, -1 );
  }

  QList<QgsPointCloudNodeId> children;
  {
    const int d = id.d() + 1;
    const int x = id.x() * 2;
    const int y = id.y() * 2;
    const int z = id.z() * 2;

    for ( int i = 0; i < 8; ++i )
    {
      int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
      const QgsPointCloudNodeId n2( d, x + dx, y + dy, z + dz );
      if ( hasNode( n2 ) )
        children.append( n2 );
    }
  }

  QgsBox3D bounds = QgsPointCloudNode::bounds( mRootBounds, id );
  return QgsPointCloudNode( id, pointCount, children, bounds.width() / mSpan, bounds );
}

bool QgsAbstractPointCloudIndex::updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> & )
{
  return false;
}

QgsPointCloudAttributeCollection QgsAbstractPointCloudIndex::attributes() const
{
  return mAttributes;
}

QgsVector3D QgsAbstractPointCloudIndex::scale() const
{
  return mScale;
}

QgsVector3D QgsAbstractPointCloudIndex::offset() const
{
  return mOffset;
}

void QgsAbstractPointCloudIndex::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
}

int QgsAbstractPointCloudIndex::span() const
{
  return mSpan;
}

bool QgsAbstractPointCloudIndex::setSubsetString( const QString &subset )
{
  const QString lastExpression = mFilterExpression;
  mFilterExpression.setExpression( subset );
  if ( mFilterExpression.hasParserError() && !subset.isEmpty() )
  {
    mFilterExpression.setExpression( lastExpression );
    return false;
  }

  // fail if expression references unknown attributes
  int offset;
  const QSet<QString> attributes = mFilterExpression.referencedAttributes();
  for ( const QString &attribute : attributes )
  {
    if ( !mAttributes.find( attribute, offset ) )
    {
      mFilterExpression.setExpression( lastExpression );
      return false;
    }
  }
  return true;
}

QString QgsAbstractPointCloudIndex::subsetString() const
{
  return mFilterExpression;
}

QgsPointCloudStatistics QgsAbstractPointCloudIndex::metadataStatistics() const
{
  QMap<QString, QgsPointCloudAttributeStatistics> statsMap;
  statsMap[ "X" ].minimum = mExtent.xMinimum();
  statsMap[ "X" ].maximum = mExtent.xMaximum();
  statsMap[ "Y" ].minimum = mExtent.yMinimum();
  statsMap[ "Y" ].maximum = mExtent.yMinimum();
  statsMap[ "Z" ].minimum = mZMin;
  statsMap[ "Z" ].maximum = mZMax;

  return QgsPointCloudStatistics( pointCount(), statsMap );
}

bool QgsAbstractPointCloudIndex::writeStatistics( QgsPointCloudStatistics &stats )
{
  Q_UNUSED( stats );
  return false;
}

void QgsAbstractPointCloudIndex::copyCommonProperties( QgsAbstractPointCloudIndex *destination ) const
{
  // Base QgsAbstractPointCloudIndex fields
  destination->mUri = mUri;
  destination->mExtent = mExtent;
  destination->mZMin = mZMin;
  destination->mZMax = mZMax;
  destination->mHierarchy = mHierarchy;
  destination->mScale = mScale;
  destination->mOffset = mOffset;
  destination->mRootBounds = mRootBounds;
  destination->mAttributes = mAttributes;
  destination->mSpan = mSpan;
  destination->mFilterExpression = mFilterExpression;
}

QgsPointCloudBlock *QgsAbstractPointCloudIndex::getNodeDataFromCache( const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request )
{
  QgsPointCloudCacheKey key( node, request, mFilterExpression, mUri );

  QMutexLocker l( &sBlockCacheMutex );
  QgsPointCloudBlock *cached = sBlockCache.object( key );
  return cached ? cached->clone() : nullptr;
}

void QgsAbstractPointCloudIndex::storeNodeDataToCache( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request ) const
{
  storeNodeDataToCacheStatic( data, node, request, mFilterExpression, mUri );
}

void QgsAbstractPointCloudIndex::storeNodeDataToCacheStatic( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri )
{
  if ( !data )
    return;

  QgsPointCloudCacheKey key( node, request, expression, uri );

  const int cost = data->pointCount() * data->pointRecordSize();

  QMutexLocker l( &sBlockCacheMutex );
  QgsDebugMsgLevel( QStringLiteral( "(%1/%2): Caching node %3 of %4" ).arg( sBlockCache.totalCost() ).arg( sBlockCache.maxCost() ).arg( key.node().toString() ).arg( key.uri() ), 4 );
  sBlockCache.insert( key, data->clone(), cost );
}

QVariantMap QgsAbstractPointCloudIndex::extraMetadata() const
{
  return {};
}

//
// QgsPointCloudIndex
//
//

QgsPointCloudIndex::QgsPointCloudIndex( QgsAbstractPointCloudIndex *index )
{
  mIndex.reset( index );
}

QgsPointCloudIndex::operator bool() const
{
  return mIndex != nullptr;
}

void QgsPointCloudIndex::load( const QString &fileName )
{
  Q_ASSERT( mIndex );
  mIndex->load( fileName );
}

bool QgsPointCloudIndex::isValid() const
{
  return mIndex && mIndex->isValid();
}

QString QgsPointCloudIndex::error() const
{
  return mIndex ? mIndex->error() : QStringLiteral( "Index is NULL" );
}

Qgis::PointCloudAccessType QgsPointCloudIndex::accessType() const
{
  Q_ASSERT( mIndex );
  return mIndex->accessType();
}

QgsCoordinateReferenceSystem QgsPointCloudIndex::crs() const
{
  Q_ASSERT( mIndex );
  return mIndex->crs();
}

qint64 QgsPointCloudIndex::pointCount() const
{
  Q_ASSERT( mIndex );
  return mIndex->pointCount();
}

QVariantMap QgsPointCloudIndex::originalMetadata() const
{
  Q_ASSERT( mIndex );
  return mIndex->originalMetadata();
}

QgsPointCloudStatistics QgsPointCloudIndex::metadataStatistics() const
{
  Q_ASSERT( mIndex );
  return mIndex->metadataStatistics();
}

bool QgsPointCloudIndex::writeStatistics( QgsPointCloudStatistics &stats )
{
  Q_ASSERT( mIndex );
  return mIndex->writeStatistics( stats );
}

QgsPointCloudNodeId QgsPointCloudIndex::root() const
{
  Q_ASSERT( mIndex );
  return mIndex->root();
}

bool QgsPointCloudIndex::hasNode( const QgsPointCloudNodeId &id ) const
{
  Q_ASSERT( mIndex );
  return mIndex->hasNode( id );
}

QgsPointCloudNode QgsPointCloudIndex::getNode( const QgsPointCloudNodeId &id ) const
{
  Q_ASSERT( mIndex );
  return mIndex->getNode( id );
}

QgsPointCloudAttributeCollection QgsPointCloudIndex::attributes() const
{
  Q_ASSERT( mIndex );
  return mIndex->attributes();
}

std::unique_ptr<QgsPointCloudBlock> QgsPointCloudIndex::nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  Q_ASSERT( mIndex );
  return mIndex->nodeData( n, request );
}

QgsPointCloudBlockRequest *QgsPointCloudIndex::asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  Q_ASSERT( mIndex );
  return mIndex->asyncNodeData( n, request );
}

bool QgsPointCloudIndex::updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> &data )
{
  Q_ASSERT( mIndex );
  return mIndex->updateNodeData( data );
}

QgsRectangle QgsPointCloudIndex::extent() const
{
  Q_ASSERT( mIndex );
  return mIndex->extent();
}

double QgsPointCloudIndex::zMin() const
{
  Q_ASSERT( mIndex );
  return mIndex->zMin();
}
double QgsPointCloudIndex::zMax() const
{
  Q_ASSERT( mIndex );
  return mIndex->zMax();
}

QgsBox3D QgsPointCloudIndex::rootNodeBounds() const
{
  Q_ASSERT( mIndex );
  return mIndex->rootNodeBounds();
}

QgsVector3D QgsPointCloudIndex::scale() const
{
  Q_ASSERT( mIndex );
  return mIndex->scale();
}

QgsVector3D QgsPointCloudIndex::offset() const
{
  Q_ASSERT( mIndex );
  return mIndex->offset();
}

int QgsPointCloudIndex::span() const
{
  Q_ASSERT( mIndex );
  return mIndex->span();
}


bool QgsPointCloudIndex::setSubsetString( const QString &subset )
{
  Q_ASSERT( mIndex );
  return mIndex->setSubsetString( subset );
}

QString QgsPointCloudIndex::subsetString() const
{
  Q_ASSERT( mIndex );
  return mIndex->subsetString();
}

QgsPointCloudBlock *QgsPointCloudIndex::getNodeDataFromCache( const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request )
{
  Q_ASSERT( mIndex );
  return mIndex->getNodeDataFromCache( node, request );
}

void QgsPointCloudIndex::storeNodeDataToCache( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request )
{
  Q_ASSERT( mIndex );
  mIndex->storeNodeDataToCache( data, node, request );
}

QVariantMap QgsPointCloudIndex::extraMetadata() const
{
  Q_ASSERT( mIndex );
  return mIndex->extraMetadata();
}

bool QgsPointCloudIndex::commitChanges( QString *errorMessage )
{
  Q_ASSERT( mIndex );
  if ( QgsPointCloudEditingIndex *index = dynamic_cast<QgsPointCloudEditingIndex *>( mIndex.get() ) )
    return index->commitChanges( errorMessage );

  return false;
}

bool QgsPointCloudIndex::isModified() const
{
  if ( QgsPointCloudEditingIndex *index = dynamic_cast<QgsPointCloudEditingIndex *>( mIndex.get() ) )
    return index->isModified();

  return false;
}

