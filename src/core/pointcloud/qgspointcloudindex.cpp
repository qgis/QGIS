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

#include "qgspointcloudindex.h"
#include "moc_qgspointcloudindex.cpp"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>
#include <qglobal.h>

#include "qgsbox3d.h"
#include "qgstiledownloadmanager.h"
#include "qgspointcloudstatistics.h"
#include "qgslogger.h"

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
  const QgsBox3D rootBounds = mIndex.rootNodeBounds();
  const double d = rootBounds.xMaximum() - rootBounds.xMinimum();
  const double dLevel = ( double )d / pow( 2, mId.d() );

  const double xMin = rootBounds.xMinimum() + dLevel * mId.x();
  const double xMax = rootBounds.xMinimum() + dLevel * ( mId.x() + 1 );
  const double yMin = rootBounds.yMinimum() + dLevel * mId.y();
  const double yMax = rootBounds.yMinimum() + dLevel * ( mId.y() + 1 );
  const double zMin = rootBounds.zMinimum() + dLevel * mId.z();
  const double zMax = rootBounds.zMinimum() + dLevel * ( mId.z() + 1 );

  return QgsBox3D( xMin, yMin, zMin, xMax, yMax, zMax );
}

float QgsPointCloudNode::error() const
{
  return bounds().width() / mIndex.span();
}

///@endcond

//
// QgsPointCloudIndex
//

QMutex QgsPointCloudIndex::sBlockCacheMutex;
QCache<QgsPointCloudCacheKey, QgsPointCloudBlock> QgsPointCloudIndex::sBlockCache( 200'000'000 ); // 200MB of cached points

QgsPointCloudIndex::QgsPointCloudIndex() = default;

QgsPointCloudIndex::~QgsPointCloudIndex() = default;

bool QgsPointCloudIndex::hasNode( const QgsPointCloudNodeId &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );
  return mHierarchy.contains( n );
}

QgsPointCloudNode QgsPointCloudIndex::getNode( const QgsPointCloudNodeId &id ) const
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

  return QgsPointCloudNode( *this, id, pointCount, children );
}

QgsPointCloudAttributeCollection QgsPointCloudIndex::attributes() const
{
  return mAttributes;
}

QgsVector3D QgsPointCloudIndex::scale() const
{
  return mScale;
}

QgsVector3D QgsPointCloudIndex::offset() const
{
  return mOffset;
}

void QgsPointCloudIndex::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
}

int QgsPointCloudIndex::span() const
{
  return mSpan;
}

bool QgsPointCloudIndex::setSubsetString( const QString &subset )
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

QString QgsPointCloudIndex::subsetString() const
{
  return mFilterExpression;
}

QgsPointCloudStatistics QgsPointCloudIndex::metadataStatistics() const
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

void QgsPointCloudIndex::copyCommonProperties( QgsPointCloudIndex *destination ) const
{
  // Base QgsPointCloudIndex fields
  destination->mUri = mUri;
  destination->mExtent = mExtent;
  destination->mZMin = mZMin;
  destination->mZMax = mZMax;
  destination->mHierarchy = mHierarchy;
  destination->mRootBounds = mRootBounds;
  destination->mAttributes = mAttributes;
  destination->mSpan = mSpan;
  destination->mFilterExpression = mFilterExpression;
}

QgsPointCloudBlock *QgsPointCloudIndex::getNodeDataFromCache( const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request )
{
  QgsPointCloudCacheKey key( node, request, mFilterExpression, mUri );

  QMutexLocker l( &sBlockCacheMutex );
  QgsPointCloudBlock *cached = sBlockCache.object( key );
  return cached ? cached->clone() : nullptr;
}

void QgsPointCloudIndex::storeNodeDataToCache( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request )
{
  storeNodeDataToCacheStatic( data, node, request, mFilterExpression, mUri );
}

void QgsPointCloudIndex::storeNodeDataToCacheStatic( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri )
{
  if ( !data )
    return;

  QgsPointCloudCacheKey key( node, request, expression, uri );

  const int cost = data->pointCount() * data->pointRecordSize();

  QMutexLocker l( &sBlockCacheMutex );
  QgsDebugMsgLevel( QStringLiteral( "(%1/%2): Caching node %3 of %4" ).arg( sBlockCache.totalCost() ).arg( sBlockCache.maxCost() ).arg( key.node().toString() ).arg( key.uri() ), 4 );
  sBlockCache.insert( key, data->clone(), cost );
}
