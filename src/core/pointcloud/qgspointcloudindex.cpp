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
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>

#include "qgstiledownloadmanager.h"
#include "qgspointcloudstatistics.h"
#include "qgslogger.h"

IndexedPointCloudNode::IndexedPointCloudNode():
  mD( -1 ),
  mX( 0 ),
  mY( 0 ),
  mZ( 0 )
{}

IndexedPointCloudNode::IndexedPointCloudNode( int _d, int _x, int _y, int _z ):
  mD( _d ),
  mX( _x ),
  mY( _y ),
  mZ( _z )
{}

IndexedPointCloudNode IndexedPointCloudNode::parentNode() const
{
  return IndexedPointCloudNode( mD - 1, mX / 2, mY / 2, mZ / 2 );
}

IndexedPointCloudNode IndexedPointCloudNode::fromString( const QString &str )
{
  QStringList lst = str.split( '-' );
  if ( lst.count() != 4 )
    return IndexedPointCloudNode();
  return IndexedPointCloudNode( lst[0].toInt(), lst[1].toInt(), lst[2].toInt(), lst[3].toInt() );
}

QString IndexedPointCloudNode::toString() const
{
  return QStringLiteral( "%1-%2-%3-%4" ).arg( mD ).arg( mX ).arg( mY ).arg( mZ );
}

int IndexedPointCloudNode::d() const
{
  return mD;
}

int IndexedPointCloudNode::x() const
{
  return mX;
}

int IndexedPointCloudNode::y() const
{
  return mY;
}

int IndexedPointCloudNode::z() const
{
  return mZ;
}

uint qHash( IndexedPointCloudNode id )
{
  return id.d() + id.x() + id.y() + id.z();
}

///@cond PRIVATE

//
// QgsPointCloudCacheKey
//

QgsPointCloudCacheKey::QgsPointCloudCacheKey( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri )
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
// QgsPointCloudDataBounds
//

QgsPointCloudDataBounds::QgsPointCloudDataBounds() = default;

QgsPointCloudDataBounds::QgsPointCloudDataBounds( qint64 xmin, qint64 ymin, qint64 zmin, qint64 xmax, qint64 ymax, qint64 zmax )
  : mXMin( xmin )
  , mYMin( ymin )
  , mZMin( zmin )
  , mXMax( xmax )
  , mYMax( ymax )
  , mZMax( zmax )
{

}

qint64 QgsPointCloudDataBounds::xMin() const
{
  return mXMin;
}

qint64 QgsPointCloudDataBounds::yMin() const
{
  return mYMin;
}

qint64 QgsPointCloudDataBounds::xMax() const
{
  return mXMax;
}

qint64 QgsPointCloudDataBounds::yMax() const
{
  return mYMax;
}

qint64 QgsPointCloudDataBounds::zMin() const
{
  return mZMin;
}

qint64 QgsPointCloudDataBounds::zMax() const
{
  return mZMax;
}

QgsRectangle QgsPointCloudDataBounds::mapExtent( const QgsVector3D &offset, const QgsVector3D &scale ) const
{
  return QgsRectangle(
           mXMin * scale.x() + offset.x(), mYMin * scale.y() + offset.y(),
           mXMax * scale.x() + offset.x(), mYMax * scale.y() + offset.y()
         );
}

QgsDoubleRange QgsPointCloudDataBounds::zRange( const QgsVector3D &offset, const QgsVector3D &scale ) const
{
  return QgsDoubleRange( mZMin * scale.z() + offset.z(), mZMax * scale.z() + offset.z() );
}

///@endcond

//
// QgsPointCloudIndex
//

QMutex QgsPointCloudIndex::sBlockCacheMutex;
QCache<QgsPointCloudCacheKey, QgsPointCloudBlock> QgsPointCloudIndex::sBlockCache( 200'000'000 ); // 200MB of cached points

QgsPointCloudIndex::QgsPointCloudIndex() = default;

QgsPointCloudIndex::~QgsPointCloudIndex() = default;

bool QgsPointCloudIndex::hasNode( const IndexedPointCloudNode &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );
  return mHierarchy.contains( n );
}

qint64 QgsPointCloudIndex::nodePointCount( const IndexedPointCloudNode &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );
  return mHierarchy.value( n, -1 );
}

QList<IndexedPointCloudNode> QgsPointCloudIndex::nodeChildren( const IndexedPointCloudNode &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );
  Q_ASSERT( mHierarchy.contains( n ) );
  QList<IndexedPointCloudNode> lst;
  const int d = n.d() + 1;
  const int x = n.x() * 2;
  const int y = n.y() * 2;
  const int z = n.z() * 2;

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const IndexedPointCloudNode n2( d, x + dx, y + dy, z + dz );
    if ( mHierarchy.contains( n2 ) )
      lst.append( n2 );
  }
  return lst;
}

QgsPointCloudAttributeCollection QgsPointCloudIndex::attributes() const
{
  return mAttributes;
}

QgsPointCloudDataBounds QgsPointCloudIndex::nodeBounds( const IndexedPointCloudNode &n ) const
{
  qint64 xMin, yMin, zMin, xMax, yMax, zMax;

  const qint64 d = mRootBounds.xMax() - mRootBounds.xMin();
  const double dLevel = ( double )d / pow( 2, n.d() );

  xMin = round( mRootBounds.xMin() + dLevel * n.x() );
  xMax = round( mRootBounds.xMin() + dLevel * ( n.x() + 1 ) );
  yMin = round( mRootBounds.yMin() + dLevel * n.y() );
  yMax = round( mRootBounds.yMin() + dLevel * ( n.y() + 1 ) );
  zMin = round( mRootBounds.zMin() + dLevel * n.z() );
  zMax = round( mRootBounds.zMin() + dLevel * ( n.z() + 1 ) );

  QgsPointCloudDataBounds db( xMin, yMin, zMin, xMax, yMax, zMax );
  return db;
}

QgsRectangle QgsPointCloudIndex::nodeMapExtent( const IndexedPointCloudNode &node ) const
{
  return nodeBounds( node ).mapExtent( mOffset, mScale );
}

QgsDoubleRange QgsPointCloudIndex::nodeZRange( const IndexedPointCloudNode &node ) const
{
  return nodeBounds( node ).zRange( mOffset, mScale );
}

float QgsPointCloudIndex::nodeError( const IndexedPointCloudNode &n ) const
{
  const double w = nodeMapExtent( n ).width();
  return w / mSpan;
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

int QgsPointCloudIndex::nodePointCount( const IndexedPointCloudNode &n )
{
  mHierarchyMutex.lock();
  const int count = mHierarchy.value( n, -1 );
  mHierarchyMutex.unlock();
  return count;
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

QVariant QgsPointCloudIndex::metadataStatistic( const QString &attribute, Qgis::Statistic statistic ) const
{
  if ( attribute == QLatin1String( "X" ) && statistic == Qgis::Statistic::Min )
    return mExtent.xMinimum();
  if ( attribute == QLatin1String( "X" ) && statistic == Qgis::Statistic::Max )
    return mExtent.xMaximum();

  if ( attribute == QLatin1String( "Y" ) && statistic == Qgis::Statistic::Min )
    return mExtent.yMinimum();
  if ( attribute == QLatin1String( "Y" ) && statistic == Qgis::Statistic::Max )
    return mExtent.yMaximum();

  if ( attribute == QLatin1String( "Z" ) && statistic == Qgis::Statistic::Min )
    return mZMin;
  if ( attribute == QLatin1String( "Z" ) && statistic == Qgis::Statistic::Max )
    return mZMax;

  return QVariant();
}

QVariantList QgsPointCloudIndex::metadataClasses( const QString &attribute ) const
{
  Q_UNUSED( attribute );
  return QVariantList();
}

QVariant QgsPointCloudIndex::metadataClassStatistic( const QString &attribute, const QVariant &value, Qgis::Statistic statistic ) const
{
  Q_UNUSED( attribute );
  Q_UNUSED( value );
  Q_UNUSED( statistic );
  return QVariant();
}

QgsPointCloudStatistics QgsPointCloudIndex::metadataStatistics() const
{
  QMap<QString, QgsPointCloudAttributeStatistics> statsMap;
  for ( QgsPointCloudAttribute attribute : attributes().attributes() )
  {
    QString name = attribute.name();
    QgsPointCloudAttributeStatistics s;
    QVariant min = metadataStatistic( name, Qgis::Statistic::Min );
    QVariant max = metadataStatistic( name, Qgis::Statistic::Max );
    QVariant mean = metadataStatistic( name, Qgis::Statistic::Mean );
    QVariant stDev = metadataStatistic( name, Qgis::Statistic::StDev );
    if ( !min.isValid() )
      continue;

    s.minimum = min.toDouble();
    s.maximum = max.toDouble();
    s.mean = mean.toDouble();
    s.stDev = stDev.toDouble();
    s.count = metadataStatistic( name, Qgis::Statistic::Count ).toInt();
    QVariantList classes = metadataClasses( name );
    for ( QVariant c : classes )
    {
      s.classCount[ c.toInt() ] = metadataClassStatistic( name, c, Qgis::Statistic::Count ).toInt();
    }
    statsMap[ name ] = s;
  }
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
  destination->mScale = mScale;
  destination->mOffset = mOffset;
  destination->mRootBounds = mRootBounds;
  destination->mAttributes = mAttributes;
  destination->mSpan = mSpan;
  destination->mFilterExpression = mFilterExpression;
}

QgsPointCloudBlock *QgsPointCloudIndex::getNodeDataFromCache( const IndexedPointCloudNode &node, const QgsPointCloudRequest &request )
{
  QgsPointCloudCacheKey key( node, request, mFilterExpression, mUri );

  QMutexLocker l( &sBlockCacheMutex );
  QgsPointCloudBlock *cached = sBlockCache.object( key );
  return cached ? cached->clone() : nullptr;
}

void QgsPointCloudIndex::storeNodeDataToCache( QgsPointCloudBlock *data, const IndexedPointCloudNode &node, const QgsPointCloudRequest &request )
{
  storeNodeDataToCacheStatic( data, node, request, mFilterExpression, mUri );
}

void QgsPointCloudIndex::storeNodeDataToCacheStatic( QgsPointCloudBlock *data, const IndexedPointCloudNode &node, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri )
{
  if ( !data )
    return;

  QgsPointCloudCacheKey key( node, request, expression, uri );

  const int cost = data->pointCount() * data->pointRecordSize();

  QMutexLocker l( &sBlockCacheMutex );
  QgsDebugMsgLevel( QStringLiteral( "(%1/%2): Caching node %3 of %4" ).arg( sBlockCache.totalCost() ).arg( sBlockCache.maxCost() ).arg( key.node().toString() ).arg( key.uri() ), 4 );
  sBlockCache.insert( key, data->clone(), cost );
}
