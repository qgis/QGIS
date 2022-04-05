/***************************************************************************
                         qgsremoteeptpointcloudindex.cpp
                         --------------------
    begin                : March 2021
    copyright            : (C) 2021 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsremotecopcpointcloudindex.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>
#include <QQueue>
#include <QTimer>

#include "qgseptdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"

#include "qgstiledownloadmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslazdecoder.h"
#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "qgscopcpointcloudblockrequest.h"
#include "qgspointcloudexpression.h"
#include "qgsnetworkaccessmanager.h"

///@cond PRIVATE

QgsRemoteCopcPointCloudIndex::QgsRemoteCopcPointCloudIndex() : QgsCopcPointCloudIndex()
{

}

QgsRemoteCopcPointCloudIndex::~QgsRemoteCopcPointCloudIndex() = default;

QList<IndexedPointCloudNode> QgsRemoteCopcPointCloudIndex::nodeChildren( const IndexedPointCloudNode &n ) const
{
  fetchNodeHierarchy( n );

  mHierarchyMutex.lock();
  Q_ASSERT( mHierarchy.contains( n ) );
  QList<IndexedPointCloudNode> lst;
  const int d = n.d() + 1;
  const int x = n.x() * 2;
  const int y = n.y() * 2;
  const int z = n.z() * 2;
  mHierarchyMutex.unlock();

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const IndexedPointCloudNode n2( d, x + dx, y + dy, z + dz );
    if ( fetchNodeHierarchy( n2 ) && mHierarchy[n] > 0 )
      lst.append( n2 );
  }
  return lst;
}

void QgsRemoteCopcPointCloudIndex::load( const QString &url )
{
  mUrl = QUrl( url );

  mIsValid = loadHeader();
  if ( !mIsValid )
    return;

  fetchHierarchyPage( mCopcInfoVlr.root_hier_offset, mCopcInfoVlr.root_hier_size );
}


bool QgsRemoteCopcPointCloudIndex::loadHeader()
{
  // Request header data
  {
    QNetworkRequest nr( mUrl );
    nr.setAttribute( QNetworkRequest::HttpPipeliningAllowedAttribute, true );
    nr.setRawHeader( "Range", "bytes=0-548" );
    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsg( QStringLiteral( "Request failed: " ) + mUrl.toString() );
      return false;
    }
    const QgsNetworkReplyContent reply = req.reply();
    mCopcHeaderData = reply.content();
    {
      std::istringstream file( mCopcHeaderData.toStdString() );
      mCopcHeader = lazperf::header14::create( file );
    }

    mCopcInfoVlr.fill( mCopcHeaderData.data() + 375 + 54, 160 );

    // Read VLR data (other than the already read COPC info)
    QByteArray vlrRequestRange = QStringLiteral( "bytes=%1-%2" ).arg( 375 ).arg( mCopcHeader.point_offset - 1 ).toLocal8Bit();
    nr.setRawHeader( "Range", vlrRequestRange );
    errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsg( QStringLiteral( "Request failed: " ) + mUrl.toString() );
      return false;
    }
    mVlrData = req.reply().content();
  }

  mPointCount = mCopcHeader.point_count;

  mScale = QgsVector3D( mCopcHeader.scale.x, mCopcHeader.scale.y, mCopcHeader.scale.z );
  mOffset = QgsVector3D( mCopcHeader.offset.x, mCopcHeader.offset.y, mCopcHeader.offset.z );

  mOriginalMetadata[ QStringLiteral( "creation_year" ) ] = mCopcHeader.creation.year;
  mOriginalMetadata[ QStringLiteral( "creation_day" ) ] = mCopcHeader.creation.day;
  mOriginalMetadata[ QStringLiteral( "major_version" ) ] = mCopcHeader.version.major;
  mOriginalMetadata[ QStringLiteral( "minor_version" ) ] = mCopcHeader.version.minor;
  mOriginalMetadata[ QStringLiteral( "dataformat_id" ) ] = mCopcHeader.point_format_id;
  mOriginalMetadata[ QStringLiteral( "scale_x" ) ] = mScale.x();
  mOriginalMetadata[ QStringLiteral( "scale_y" ) ] = mScale.y();
  mOriginalMetadata[ QStringLiteral( "scale_z" ) ] = mScale.z();
  mOriginalMetadata[ QStringLiteral( "offset_x" ) ] = mOffset.x();
  mOriginalMetadata[ QStringLiteral( "offset_y" ) ] = mOffset.y();
  mOriginalMetadata[ QStringLiteral( "offset_z" ) ] = mOffset.z();
  mOriginalMetadata[ QStringLiteral( "project_id" ) ] = QString( QByteArray( mCopcHeader.guid, 16 ).toHex() );
  mOriginalMetadata[ QStringLiteral( "system_id" ) ] = QString::fromLocal8Bit( mCopcHeader.system_identifier, 32 );
  mOriginalMetadata[ QStringLiteral( "software_id" ) ] = QString::fromLocal8Bit( mCopcHeader.generating_software, 32 );

  // The COPC format only uses PDRF 6, 7 or 8. So there should be a OGC Coordinate System WKT record
  mWkt = QString();

  lazperf::vlr_header vlrHeader;
  int offset = 0;
  while ( offset < mVlrData.size() )
  {
    char *data = mVlrData.data() + offset;
    vlrHeader.fill( data, 54 );
    if ( vlrHeader.user_id == "LASF_Projection" && vlrHeader.record_id == 2112 )
    {
      mWktVlr.fill( data + 54, vlrHeader.data_length );
    }
    else if ( vlrHeader.user_id == "LASF_Spec" && vlrHeader.record_id == 4 )
    {
      mExtraBytesVlr.fill( data + 54, vlrHeader.data_length );
      mExtraBytesData = QByteArray( data + 54, vlrHeader.data_length );
    }
    offset += 54 + vlrHeader.data_length;
  }

  if ( !mWktVlr.wkt.empty() )
  {
    mWkt = QString::fromStdString( mWktVlr.wkt );
  }
  else
  {
    qDebug() << "Failed to load Wkt data";
  }

  mExtent.set( mCopcHeader.minx, mCopcHeader.miny, mCopcHeader.maxx, mCopcHeader.maxy );

  mZMin = mCopcHeader.minz;
  mZMax = mCopcHeader.maxz;


  // Attributes for COPC format
  // COPC supports only PDRF 6, 7 and 8

  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( "X", QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( "Y", QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( "Z", QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( "Intensity", QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( "ReturnNumber", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "NumberOfReturns", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "ScanDirectionFlag", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "EdgeOfFlightLine", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "Classification", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "ScanAngleRank", QgsPointCloudAttribute::Short ) );
  attributes.push_back( QgsPointCloudAttribute( "UserData", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "PointSourceId", QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( "GpsTime", QgsPointCloudAttribute::Double ) );

  switch ( mCopcHeader.pointFormat() )
  {
    case 6:
      break;
    case 7:
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
      break;
    case 8:
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "NIR" ), QgsPointCloudAttribute::UShort ) );
      break;
    default:
      return false;
  }

  QVector<QgsLazDecoder::ExtraBytesAttributeDetails> extrabyteAttributes = QgsLazDecoder::readExtraByteAttributesFromVlr( mExtraBytesVlr, mCopcHeader.point_record_length );
  for ( QgsLazDecoder::ExtraBytesAttributeDetails attr : extrabyteAttributes )
  {
    attributes.push_back( QgsPointCloudAttribute( attr.attribute, attr.type ) );
  }

  setAttributes( attributes );

  const double xmin = mCopcInfoVlr.center_x - mCopcInfoVlr.halfsize;
  const double ymin = mCopcInfoVlr.center_y - mCopcInfoVlr.halfsize;
  const double zmin = mCopcInfoVlr.center_z - mCopcInfoVlr.halfsize;
  const double xmax = mCopcInfoVlr.center_x + mCopcInfoVlr.halfsize;
  const double ymax = mCopcInfoVlr.center_y + mCopcInfoVlr.halfsize;
  const double zmax = mCopcInfoVlr.center_z + mCopcInfoVlr.halfsize;

  mRootBounds = QgsPointCloudDataBounds(
                  ( xmin - mOffset.x() ) / mScale.x(),
                  ( ymin - mOffset.y() ) / mScale.y(),
                  ( zmin - mOffset.z() ) / mScale.z(),
                  ( xmax - mOffset.x() ) / mScale.x(),
                  ( ymax - mOffset.y() ) / mScale.y(),
                  ( zmax - mOffset.z() ) / mScale.z()
                );

  double calculatedSpan = nodeMapExtent( root() ).width() / mCopcInfoVlr.spacing;
  mSpan = calculatedSpan;

#ifdef QGIS_DEBUG
  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  QgsDebugMsgLevel( QStringLiteral( "lvl0 node size in CRS units: %1 %2 %3" ).arg( dx ).arg( dy ).arg( dz ), 2 );    // all dims should be the same
  QgsDebugMsgLevel( QStringLiteral( "res at lvl0 %1" ).arg( dx / mSpan ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl1 %1" ).arg( dx / mSpan / 2 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl2 %1 with node size %2" ).arg( dx / mSpan / 4 ).arg( dx / 4 ), 2 );
#endif
  return true;
}

QgsPointCloudBlock *QgsRemoteCopcPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  std::unique_ptr<QgsPointCloudBlockRequest> blockRequest( asyncNodeData( n, request ) );
  if ( !blockRequest )
    return nullptr;

  QEventLoop loop;
  connect( blockRequest.get(), &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
  loop.exec();

  if ( !blockRequest->block() )
  {
    QgsDebugMsg( QStringLiteral( "Error downloading node %1 data, error : %2 " ).arg( n.toString(), blockRequest->errorStr() ) );
  }

  return blockRequest->block();
}

QgsPointCloudBlockRequest *QgsRemoteCopcPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( !fetchNodeHierarchy( n ) )
    return nullptr;
  QMutexLocker locker( &mHierarchyMutex );

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object might be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
  auto [ blockOffset, blockSize ] = mHierarchyNodePos[n];
  int pointCount = mHierarchy[n];

  QgsCopcPointCloudBlockRequest *req = new QgsCopcPointCloudBlockRequest( n, mUrl.toString(), attributes(), requestAttributes,
      scale(), offset(), filterExpression,
      blockOffset, blockSize, pointCount, mCopcHeaderData, mExtraBytesData );
  req->startRequest();
  return req;
}

bool QgsRemoteCopcPointCloudIndex::hasNode( const IndexedPointCloudNode &n ) const
{
  return fetchNodeHierarchy( n );
}

bool QgsRemoteCopcPointCloudIndex::fetchNodeHierarchy( const IndexedPointCloudNode &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );

  QVector<IndexedPointCloudNode> ancestors;
  IndexedPointCloudNode foundRoot = n;
  while ( !mHierarchy.contains( foundRoot ) )
  {
    ancestors.push_front( foundRoot );
    foundRoot = foundRoot.parentNode();
  }
  ancestors.push_front( foundRoot );
  for ( IndexedPointCloudNode n : ancestors )
  {
    if ( !mHierarchy.contains( n ) )
      return false;
    int nodesCount = mHierarchy[n];
    if ( nodesCount < 0 )
    {
      fetchHierarchyPage( mHierarchyNodePos[n].first, mHierarchyNodePos[n].second );
    }
  }
  return mHierarchy.contains( n );
}

bool QgsRemoteCopcPointCloudIndex::isValid() const
{
  return mIsValid;
}

void QgsRemoteCopcPointCloudIndex::fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const
{
  QNetworkRequest nr( mUrl );
  nr.setAttribute( QNetworkRequest::HttpPipeliningAllowedAttribute, true );
  QByteArray queryRange = QStringLiteral( "bytes=%1-%2" ).arg( offset ).arg( offset + byteSize - 1 ).toLocal8Bit();
  nr.setRawHeader( "Range", queryRange );
  QgsBlockingNetworkRequest req;
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + mUrl.toString() );
  }
  const QgsNetworkReplyContent reply = req.reply();
  QByteArray data = reply.content();

  struct CopcVoxelKey
  {
    int32_t level;
    int32_t x;
    int32_t y;
    int32_t z;
  };

  struct CopcEntry
  {
    CopcVoxelKey key;
    uint64_t offset;
    int32_t byteSize;
    int32_t pointCount;
  };

  for ( uint64_t i = 0; i < byteSize; i += sizeof( CopcEntry ) )
  {
    CopcEntry *entry = reinterpret_cast<CopcEntry *>( data.data() + i );
    const IndexedPointCloudNode nodeId( entry->key.level, entry->key.x, entry->key.y, entry->key.z );
    mHierarchy[nodeId] = entry->pointCount;
    mHierarchyNodePos.insert( nodeId, QPair<uint64_t, int32_t>( entry->offset, entry->byteSize ) );
  }
}


///@endcond
