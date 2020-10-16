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

#include "qgspointclouddecoder.h"

IndexedPointCloudNode::IndexedPointCloudNode(): d( -1 ), x( 0 ), y( 0 ), z( 0 ) {}

IndexedPointCloudNode::IndexedPointCloudNode( int _d, int _x, int _y, int _z ): d( _d ), x( _x ), y( _y ), z( _z ) {}

bool IndexedPointCloudNode::operator==( const IndexedPointCloudNode &other ) const { return d == other.d && x == other.x && y == other.y && z == other.z; }

IndexedPointCloudNode IndexedPointCloudNode::fromString( const QString &str )
{
  QStringList lst = str.split( '-' );
  if ( lst.count() != 4 )
    return IndexedPointCloudNode();
  return IndexedPointCloudNode( lst[0].toInt(), lst[1].toInt(), lst[2].toInt(), lst[3].toInt() );
}

QString IndexedPointCloudNode::toString() const
{
  return QString( "%1-%2-%3-%4" ).arg( d ).arg( x ).arg( y ).arg( z );
}

uint qHash( const IndexedPointCloudNode &id )
{
  return id.d + id.x + id.y + id.z;
}

QgsPointCloudDataBounds::QgsPointCloudDataBounds() = default;

QgsPointCloudDataBounds::QgsPointCloudDataBounds( qint32 xmin, qint32 ymin, qint32 xmax, qint32 ymax, qint32 zmin, qint32 zmax )
  : mXMin( xmin )
  , mYMin( ymin )
  , mXMax( xmax )
  , mYMax( ymax )
  , mZMin( zmin )
  , mZMax( zmax )
{

}

QgsPointCloudDataBounds::QgsPointCloudDataBounds( const QgsPointCloudDataBounds &obj )
  : mXMin( obj.xMin() )
  , mYMin( obj.yMin() )
  , mXMax( obj.xMax() )
  , mYMax( obj.yMax() )
  , mZMin( obj.zMin() )
  , mZMax( obj.zMax() )
{
}

qint32 QgsPointCloudDataBounds::xMin() const
{
  return mXMin;
}

qint32 QgsPointCloudDataBounds::yMin() const
{
  return mYMin;
}

qint32 QgsPointCloudDataBounds::xMax() const
{
  return mXMax;
}

qint32 QgsPointCloudDataBounds::yMax() const
{
  return mYMax;
}

qint32 QgsPointCloudDataBounds::zMin() const
{
  return mZMin;
}

qint32 QgsPointCloudDataBounds::zMax() const
{
  return mZMax;
}

QgsPointCloudIndex::QgsPointCloudIndex() = default;

QgsPointCloudIndex::~QgsPointCloudIndex() = default;

bool QgsPointCloudIndex::load( const QString &fileName )
{
  // mDirectory = directory;
  QFile f( fileName );
  if ( !f.open( QIODevice::ReadOnly ) )
    return false;

  const QDir directory = QFileInfo( fileName ).absoluteDir();
  mDirectory = directory.absolutePath();

  QByteArray dataJson = f.readAll();
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson( dataJson, &err );
  if ( err.error != QJsonParseError::NoError )
    return false;

  mDataType = doc["dataType"].toString();  // "binary" or "laszip"
  if ( mDataType != "laszip" && mDataType != "binary" && mDataType != "zstandard" )
    return false;

  QString hierarchyType = doc["hierarchyType"].toString();  // "json" or "gzip"
  if ( hierarchyType != "json" )
    return false;

  mSpan = doc["span"].toInt();

  // WKT
  QJsonObject srs = doc["srs"].toObject();
  mWkt = srs["wkt"].toString();

  // rectangular
  QJsonArray bounds = doc["bounds"].toArray();
  if ( bounds.size() != 6 )
    return false;

  QJsonArray bounds_conforming = doc["boundsConforming"].toArray();
  if ( bounds.size() != 6 )
    return false;

  QJsonArray schemaArray = doc["schema"].toArray();

  for ( QJsonValue schemaItem : schemaArray )
  {
    QJsonObject schemaObj = schemaItem.toObject();
    QString name = schemaObj["name"].toString();
    QString type = schemaObj["type"].toString();
    int size = schemaObj["size"].toInt();

    float scale = 1.f;
    if ( schemaObj.contains( "scale" ) )
      scale = schemaObj["scale"].toDouble();

    float offset = 0.f;
    if ( schemaObj.contains( "offset" ) )
      offset = schemaObj["offset"].toDouble();

    if ( name == "X" )
    {
      mOffset.set( offset, mOffset.y(), mOffset.z() );
      mScale.set( scale, mScale.y(), mScale.z() );
    }
    else if ( name == "Y" )
    {
      mOffset.set( mOffset.x(), offset, mOffset.z() );
      mScale.set( mScale.x(), scale, mScale.z() );
    }
    else if ( name == "Z" )
    {
      mOffset.set( mOffset.x(), mOffset.y(), offset );
      mScale.set( mScale.x(), mScale.y(), scale );
    }

    // TODO: can parse also stats: "count", "minimum", "maximum", "mean", "stddev", "variance"
  }

  // save mRootBounds

  // bounds (cube - octree volume)
  double xmin = bounds[0].toDouble();
  double ymin = bounds[1].toDouble();
  double zmin = bounds[2].toDouble();
  double xmax = bounds[3].toDouble();
  double ymax = bounds[4].toDouble();
  double zmax = bounds[5].toDouble();

  mRootBounds = QgsPointCloudDataBounds(
                  ( xmin - mOffset.x() ) / mScale.x(),
                  ( xmax - mOffset.x() ) / mScale.x(),
                  ( ymin - mOffset.y() ) / mScale.y(),
                  ( ymax - mOffset.y() ) / mScale.y(),
                  ( zmin - mOffset.z() ) / mScale.z(),
                  ( zmax - mOffset.z() ) / mScale.z()
                );


  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  qDebug() << "lvl0 node size in CRS units:" << dx << dy << dz;   // all dims should be the same
  qDebug() << "res at lvl0" << dx / mSpan;
  qDebug() << "res at lvl1" << dx / mSpan / 2;
  qDebug() << "res at lvl2" << dx / mSpan / 4 << "with node size" << dx / 4;

  // load hierarchy

  QFile fH( QStringLiteral( "%1/ept-hierarchy/0-0-0-0.json" ).arg( mDirectory ) );
  if ( !fH.open( QIODevice::ReadOnly ) )
    return false;

  QByteArray dataJsonH = fH.readAll();
  QJsonParseError errH;
  QJsonDocument docH = QJsonDocument::fromJson( dataJsonH, &errH );
  if ( errH.error != QJsonParseError::NoError )
    return false;

  QJsonObject rootHObj = docH.object();
  for ( auto it = rootHObj.constBegin(); it != rootHObj.constEnd(); ++it )
  {
    QString nodeIdStr = it.key();
    int nodePointCount = it.value().toInt();
    IndexedPointCloudNode nodeId = IndexedPointCloudNode::fromString( nodeIdStr );
    mHierarchy[nodeId] = nodePointCount;
  }

  return true;
}

QList<IndexedPointCloudNode> QgsPointCloudIndex::children( const IndexedPointCloudNode &n )
{
  Q_ASSERT( mHierarchy.contains( n ) );
  QList<IndexedPointCloudNode> lst;
  int d = n.d + 1;
  int x = n.x * 2;
  int y = n.y * 2;
  int z = n.z * 2;

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    IndexedPointCloudNode n2( d, x + dx, y + dy, z + dz );
    if ( mHierarchy.contains( n2 ) )
      lst.append( n2 );
  }
  return lst;
}

QVector<qint32> QgsPointCloudIndex::nodePositionDataAsInt32( const IndexedPointCloudNode &n )
{
  Q_ASSERT( mHierarchy.contains( n ) );
  // int count = mHierarchy[n];

  if ( mDataType == "binary" )
  {
    QString filename = QString( "%1/ept-data/%2.bin" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsPointCloudDecoder::decompressBinary( filename );
  }
  else if ( mDataType == "zstandard" )
  {
    QString filename = QString( "%1/ept-data/%2.zst" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsPointCloudDecoder::decompressBinary( filename );
  }
  else     // if ( mDataType == "laz" )
  {
    QString filename = QString( "%1/ept-data/%2.laz" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsPointCloudDecoder::decompressBinary( filename );
  }
}

QgsPointCloudDataBounds QgsPointCloudIndex::nodeBounds( const IndexedPointCloudNode &n )
{
  qint32 xMin = -999999999, yMin = -999999999, zMin = -999999999;
  qint32 xMax = 999999999, yMax = 999999999, zMax = 999999999;

  int d = mRootBounds.xMax() - mRootBounds.xMin();
  double dLevel = ( double )d / pow( 2, n.d );

  xMin = round( mRootBounds.xMin() + dLevel * n.x );
  xMax = round( mRootBounds.xMin() + dLevel * ( n.x + 1 ) );
  yMin = round( mRootBounds.yMin() + dLevel * n.y );
  yMax = round( mRootBounds.yMin() + dLevel * ( n.y + 1 ) );
  zMin = round( mRootBounds.zMin() + dLevel * n.z );
  zMax = round( mRootBounds.zMin() + dLevel * ( n.z + 1 ) );

  QgsPointCloudDataBounds db( xMin, xMax, yMin, yMax, zMin, zMax );
  return db;
}

QString QgsPointCloudIndex::wkt() const
{
  return mWkt;
}

QgsVector3D QgsPointCloudIndex::scale() const
{
  return mScale;
}

QgsVector3D QgsPointCloudIndex::offset() const
{
  return mOffset;
}
