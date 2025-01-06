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

#include "qgseptpointcloudindex.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>
#include <QQueue>
#include <QNetworkRequest>

#include "qgsapplication.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscachedpointcloudblockrequest.h"
#include "qgseptdecoder.h"
#include "qgseptpointcloudblockrequest.h"
#include "qgslazdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudblockrequest.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgspointcloudexpression.h"
#include "qgssetrequestinitiator_p.h"
#include "qgspointcloudstatistics.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "ept" )
#define PROVIDER_DESCRIPTION QStringLiteral( "EPT point cloud provider" )

QgsEptPointCloudIndex::QgsEptPointCloudIndex()
{
  mHierarchyNodes.insert( QgsPointCloudNodeId( 0, 0, 0, 0 ) );
}

QgsEptPointCloudIndex::~QgsEptPointCloudIndex() = default;

std::unique_ptr<QgsAbstractPointCloudIndex> QgsEptPointCloudIndex::clone() const
{
  QgsEptPointCloudIndex *clone = new QgsEptPointCloudIndex;
  QMutexLocker locker( &mHierarchyMutex );
  copyCommonProperties( clone );
  return std::unique_ptr<QgsAbstractPointCloudIndex>( clone );
}

void QgsEptPointCloudIndex::load( const QString &urlString )
{
  QUrl url = urlString;
  // Treat non-URLs as local files
  if ( url.isValid() && ( url.scheme() == "http" || url.scheme() == "https" ) )
    mAccessType = Qgis::PointCloudAccessType::Remote;
  else
    mAccessType = Qgis::PointCloudAccessType::Local;
  mUri = urlString;

  QStringList splitUrl = mUri.split( '/' );
  splitUrl.pop_back();
  mUrlDirectoryPart = splitUrl.join( '/' );

  QByteArray content;
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    QNetworkRequest nr = QNetworkRequest( QUrl( mUri ) );
    QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsEptPointCloudIndex" ) );

    QgsBlockingNetworkRequest req;
    if ( req.get( nr ) != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( QStringLiteral( "Request failed: " ) + mUri );
      mIsValid = false;
      mError = req.errorMessage();
      return;
    }
    content = req.reply().content();
  }
  else
  {
    QFile f( mUri );
    if ( !f.open( QIODevice::ReadOnly ) )
    {
      mError = QObject::tr( "Unable to open %1 for reading" ).arg( mUri );
      mIsValid = false;
      return;
    }
    content = f.readAll();
  }

  bool success = loadSchema( content );
  if ( success )
  {
    // try to import the metadata too!
    const QString manifestPath = mUrlDirectoryPart + QStringLiteral( "/ept-sources/manifest.json" );
    QByteArray manifestJson;
    if ( mAccessType == Qgis::PointCloudAccessType::Remote )
    {
      QUrl manifestUrl( manifestPath );

      QNetworkRequest nr = QNetworkRequest( QUrl( manifestUrl ) );
      QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsEptPointCloudIndex" ) );
      QgsBlockingNetworkRequest req;
      if ( req.get( nr ) == QgsBlockingNetworkRequest::NoError )
        manifestJson = req.reply().content();
    }
    else
    {
      QFile manifestFile( manifestPath );
      if ( manifestFile.open( QIODevice::ReadOnly ) )
        manifestJson = manifestFile.readAll();
    }

    if ( !manifestJson.isEmpty() )
      loadManifest( manifestJson );
  }

  if ( !loadNodeHierarchy( QgsPointCloudNodeId( 0, 0, 0, 0 ) ) )
  {
    QgsDebugError( QStringLiteral( "Failed to load root EPT node" ) );
    success = false;
  }

  mIsValid = success;
}

void QgsEptPointCloudIndex::loadManifest( const QByteArray &manifestJson )
{
  QJsonParseError err;
  // try to import the metadata too!
  const QJsonDocument manifestDoc = QJsonDocument::fromJson( manifestJson, &err );
  if ( err.error != QJsonParseError::NoError )
    return;

  const QJsonArray manifestArray = manifestDoc.array();
  if ( manifestArray.empty() )
    return;

  // TODO how to handle multiple?
  const QJsonObject sourceObject = manifestArray.at( 0 ).toObject();
  const QString metadataPath = sourceObject.value( QStringLiteral( "metadataPath" ) ).toString();
  const QString fullMetadataPath = mUrlDirectoryPart + QStringLiteral( "/ept-sources/" ) + metadataPath;

  QByteArray metadataJson;
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    QUrl metadataUrl( fullMetadataPath );
    QNetworkRequest nr = QNetworkRequest( QUrl( metadataUrl ) );
    QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsEptPointCloudIndex" ) );
    QgsBlockingNetworkRequest req;
    if ( req.get( nr ) != QgsBlockingNetworkRequest::NoError )
      return;
    metadataJson = req.reply().content();
  }
  else
  {
    QFile metadataFile( fullMetadataPath );
    if ( ! metadataFile.open( QIODevice::ReadOnly ) )
      return;
    metadataJson = metadataFile.readAll();
  }

  const QJsonDocument metadataDoc = QJsonDocument::fromJson( metadataJson, &err );
  if ( err.error != QJsonParseError::NoError )
    return;

  const QJsonObject metadataObject = metadataDoc.object().value( QStringLiteral( "metadata" ) ).toObject();
  if ( metadataObject.empty() )
    return;

  const QJsonObject sourceMetadata = metadataObject.constBegin().value().toObject();
  mOriginalMetadata = sourceMetadata.toVariantMap();
}

bool QgsEptPointCloudIndex::loadSchema( const QByteArray &dataJson )
{
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( dataJson, &err );
  if ( err.error != QJsonParseError::NoError )
    return false;
  const QJsonObject result = doc.object();
  mDataType = result.value( QLatin1String( "dataType" ) ).toString();  // "binary" or "laszip"
  if ( mDataType != QLatin1String( "laszip" ) && mDataType != QLatin1String( "binary" ) && mDataType != QLatin1String( "zstandard" ) )
    return false;

  const QString hierarchyType = result.value( QLatin1String( "hierarchyType" ) ).toString();  // "json" or "gzip"
  if ( hierarchyType != QLatin1String( "json" ) )
    return false;

  mSpan = result.value( QLatin1String( "span" ) ).toInt();
  mPointCount = result.value( QLatin1String( "points" ) ).toDouble();

  // WKT
  const QJsonObject srs = result.value( QLatin1String( "srs" ) ).toObject();
  mWkt = srs.value( QLatin1String( "wkt" ) ).toString();

  // rectangular
  const QJsonArray bounds = result.value( QLatin1String( "bounds" ) ).toArray();
  if ( bounds.size() != 6 )
    return false;

  const QJsonArray boundsConforming = result.value( QLatin1String( "boundsConforming" ) ).toArray();
  if ( boundsConforming.size() != 6 )
    return false;
  mExtent.set( boundsConforming[0].toDouble(), boundsConforming[1].toDouble(),
               boundsConforming[3].toDouble(), boundsConforming[4].toDouble() );
  mZMin = boundsConforming[2].toDouble();
  mZMax = boundsConforming[5].toDouble();

  const QJsonArray schemaArray = result.value( QLatin1String( "schema" ) ).toArray();
  QgsPointCloudAttributeCollection attributes;

  for ( const QJsonValue &schemaItem : schemaArray )
  {
    const QJsonObject schemaObj = schemaItem.toObject();
    const QString name = schemaObj.value( QLatin1String( "name" ) ).toString();
    const QString type = schemaObj.value( QLatin1String( "type" ) ).toString();

    const int size = schemaObj.value( QLatin1String( "size" ) ).toInt();

    if ( name == QLatin1String( "ClassFlags" ) && size == 1 )
    {
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Synthetic" ), QgsPointCloudAttribute::UChar ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "KeyPoint" ), QgsPointCloudAttribute::UChar ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Withheld" ), QgsPointCloudAttribute::UChar ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Overlap" ), QgsPointCloudAttribute::UChar ) );
    }
    else if ( type == QLatin1String( "float" ) && ( size == 4 ) )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Float ) );
    }
    else if ( type == QLatin1String( "float" ) && ( size == 8 ) )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Double ) );
    }
    else if ( size == 1 )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Char ) );
    }
    else if ( type == QLatin1String( "unsigned" ) && size == 2 )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::UShort ) );
    }
    else if ( size == 2 )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Short ) );
    }
    else if ( size == 4 )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Int32 ) );
    }
    else
    {
      // unknown attribute type
      return false;
    }

    double scale = 1.f;
    if ( schemaObj.contains( QLatin1String( "scale" ) ) )
      scale = schemaObj.value( QLatin1String( "scale" ) ).toDouble();

    double offset = 0.f;
    if ( schemaObj.contains( QLatin1String( "offset" ) ) )
      offset = schemaObj.value( QLatin1String( "offset" ) ).toDouble();

    if ( name == QLatin1String( "X" ) )
    {
      mOffset.set( offset, mOffset.y(), mOffset.z() );
      mScale.set( scale, mScale.y(), mScale.z() );
    }
    else if ( name == QLatin1String( "Y" ) )
    {
      mOffset.set( mOffset.x(), offset, mOffset.z() );
      mScale.set( mScale.x(), scale, mScale.z() );
    }
    else if ( name == QLatin1String( "Z" ) )
    {
      mOffset.set( mOffset.x(), mOffset.y(), offset );
      mScale.set( mScale.x(), mScale.y(), scale );
    }

    // store any metadata stats which are present for the attribute
    AttributeStatistics stats;
    bool foundStats = false;
    if ( schemaObj.contains( QLatin1String( "count" ) ) )
    {
      stats.count = schemaObj.value( QLatin1String( "count" ) ).toInt();
      foundStats = true;
    }
    if ( schemaObj.contains( QLatin1String( "minimum" ) ) )
    {
      stats.minimum = schemaObj.value( QLatin1String( "minimum" ) ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( QLatin1String( "maximum" ) ) )
    {
      stats.maximum = schemaObj.value( QLatin1String( "maximum" ) ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( QLatin1String( "count" ) ) )
    {
      stats.mean = schemaObj.value( QLatin1String( "mean" ) ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( QLatin1String( "stddev" ) ) )
    {
      stats.stDev = schemaObj.value( QLatin1String( "stddev" ) ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( QLatin1String( "variance" ) ) )
    {
      stats.variance = schemaObj.value( QLatin1String( "variance" ) ).toDouble();
      foundStats = true;
    }
    if ( foundStats )
      mMetadataStats.insert( name, stats );

    if ( schemaObj.contains( QLatin1String( "counts" ) ) )
    {
      QMap< int, int >  classCounts;
      const QJsonArray counts = schemaObj.value( QLatin1String( "counts" ) ).toArray();
      for ( const QJsonValue &count : counts )
      {
        const QJsonObject countObj = count.toObject();
        classCounts.insert( countObj.value( QLatin1String( "value" ) ).toInt(), countObj.value( QLatin1String( "count" ) ).toInt() );
      }
      mAttributeClasses.insert( name, classCounts );
    }
  }
  setAttributes( attributes );

  // save mRootBounds

  // bounds (cube - octree volume)
  const double xmin = bounds[0].toDouble();
  const double ymin = bounds[1].toDouble();
  const double zmin = bounds[2].toDouble();
  const double xmax = bounds[3].toDouble();
  const double ymax = bounds[4].toDouble();
  const double zmax = bounds[5].toDouble();

  mRootBounds = QgsBox3D( xmin, ymin, zmin, xmax, ymax, zmax );

#ifdef QGIS_DEBUG
  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  QgsDebugMsgLevel( QStringLiteral( "lvl0 node size in CRS units: %1 %2 %3" ).arg( dx ).arg( dy ).arg( dz ), 2 );    // all dims should be the same
  QgsDebugMsgLevel( QStringLiteral( "res at lvl0 %1" ).arg( dx / mSpan ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl1 %1" ).arg( dx / mSpan / 2 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl2 %1 with node size %2" ).arg( dx / mSpan / 4 ).arg( dx / 4 ), 2 );
#endif

  return true;
}

std::unique_ptr<QgsPointCloudBlock> QgsEptPointCloudIndex::nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  if ( QgsPointCloudBlock *cached = getNodeDataFromCache( n, request ) )
  {
    return std::unique_ptr<QgsPointCloudBlock>( cached );
  }

  std::unique_ptr<QgsPointCloudBlock> block;
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    std::unique_ptr<QgsPointCloudBlockRequest> blockRequest( asyncNodeData( n, request ) );
    if ( !blockRequest )
      return nullptr;

    QEventLoop loop;
    QObject::connect( blockRequest.get(), &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
    loop.exec();

    block = blockRequest->takeBlock();
    if ( !block )
    {
      QgsDebugError( QStringLiteral( "Error downloading node %1 data, error : %2 " ).arg( n.toString(), blockRequest->errorStr() ) );
    }
  }
  else
  {
    // we need to create a copy of the expression to pass to the decoder
    // as the same QgsPointCloudExpression object mighgt be concurrently
    // used on another thread, for example in a 3d view
    QgsPointCloudExpression filterExpression = mFilterExpression;
    QgsPointCloudAttributeCollection requestAttributes = request.attributes();
    requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
    QgsRectangle filterRect = request.filterRect();

    if ( mDataType == QLatin1String( "binary" ) )
    {
      const QString filename = QStringLiteral( "%1/ept-data/%2.bin" ).arg( mUrlDirectoryPart, n.toString() );
      block = QgsEptDecoder::decompressBinary( filename, attributes(), requestAttributes, scale(), offset(), filterExpression, filterRect );
    }
    else if ( mDataType == QLatin1String( "zstandard" ) )
    {
      const QString filename = QStringLiteral( "%1/ept-data/%2.zst" ).arg( mUrlDirectoryPart, n.toString() );
      block = QgsEptDecoder::decompressZStandard( filename, attributes(), request.attributes(), scale(), offset(), filterExpression, filterRect );
    }
    else if ( mDataType == QLatin1String( "laszip" ) )
    {
      const QString filename = QStringLiteral( "%1/ept-data/%2.laz" ).arg( mUrlDirectoryPart, n.toString() );
      block = QgsLazDecoder::decompressLaz( filename, requestAttributes, filterExpression, filterRect );
    }
  }

  storeNodeDataToCache( block.get(), n, request );
  return block;
}

QgsPointCloudBlockRequest *QgsEptPointCloudIndex::asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  if ( QgsPointCloudBlock *cached = getNodeDataFromCache( n, request ) )
  {
    return new QgsCachedPointCloudBlockRequest( cached,  n, mUri, attributes(), request.attributes(),
           scale(), offset(), mFilterExpression, request.filterRect() );
  }

  if ( mAccessType != Qgis::PointCloudAccessType::Remote )
    return nullptr;

  if ( !loadNodeHierarchy( n ) )
    return nullptr;

  QString fileUrl;
  if ( mDataType == QLatin1String( "binary" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.bin" ).arg( mUrlDirectoryPart, n.toString() );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.zst" ).arg( mUrlDirectoryPart, n.toString() );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.laz" ).arg( mUrlDirectoryPart, n.toString() );
  }
  else
  {
    return nullptr;
  }

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object might be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
  return new QgsEptPointCloudBlockRequest( n, fileUrl, mDataType, attributes(), requestAttributes, scale(), offset(), filterExpression, request.filterRect() );
}

bool QgsEptPointCloudIndex::hasNode( const QgsPointCloudNodeId &n ) const
{
  return loadNodeHierarchy( n );
}

QgsCoordinateReferenceSystem QgsEptPointCloudIndex::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt( mWkt );
}

qint64 QgsEptPointCloudIndex::pointCount() const
{
  return mPointCount;
}

QgsPointCloudNode QgsEptPointCloudIndex::getNode( const QgsPointCloudNodeId &id ) const
{
  QgsPointCloudNode node = QgsAbstractPointCloudIndex::getNode( id );

  // First try cached value
  if ( node.pointCount() != -1 )
    return node;

  // Try loading all nodes' hierarchy files on the path from root and stop when
  // one contains the point count for nodeId
  QVector<QgsPointCloudNodeId> pathToRoot = nodePathToRoot( id );
  for ( int i = pathToRoot.size() - 1; i >= 0; --i )
  {
    loadSingleNodeHierarchy( pathToRoot[i] );

    QMutexLocker locker( &mHierarchyMutex );
    qint64 pointCount = mHierarchy.value( id, -1 );
    if ( pointCount != -1 )
      return QgsPointCloudNode( id, pointCount, node.children(), node.error(), node.bounds() );
  }

  // If we fail, return with pointCount = -1 anyway
  return node;
}

QgsPointCloudStatistics QgsEptPointCloudIndex::metadataStatistics() const
{
  QMap<QString, QgsPointCloudAttributeStatistics> statsMap;
  for ( QgsPointCloudAttribute attribute : attributes().attributes() )
  {
    QString name = attribute.name();
    const AttributeStatistics &stats = mMetadataStats[ name ];
    if ( !stats.minimum.isValid() )
      continue;
    QgsPointCloudAttributeStatistics s;
    s.minimum = stats.minimum.toDouble();
    s.maximum = stats.maximum.toDouble();
    s.mean = stats.mean;
    s.stDev = stats.stDev;
    s.count = stats.count;

    s.classCount = mAttributeClasses[ name ];

    statsMap[ name ] = std::move( s );
  }
  return QgsPointCloudStatistics( pointCount(), statsMap );
}

bool QgsEptPointCloudIndex::loadSingleNodeHierarchy( const QgsPointCloudNodeId &nodeId ) const
{
  mHierarchyMutex.lock();
  const bool foundInHierarchy = mHierarchy.contains( nodeId );
  const bool foundInHierarchyNodes = mHierarchyNodes.contains( nodeId );
  mHierarchyMutex.unlock();
  // The hierarchy of the node is found => No need to load its file
  if ( foundInHierarchy )
    return true;
  // We don't know that this node has a hierarchy file => We have nothing to load
  if ( !foundInHierarchyNodes )
    return true;

  const QString filePath = QStringLiteral( "%1/ept-hierarchy/%2.json" ).arg( mUrlDirectoryPart, nodeId.toString() );

  QByteArray dataJsonH;
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    QNetworkRequest nr( filePath );
    QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsEptPointCloudIndex" ) );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    std::unique_ptr<QgsTileDownloadManagerReply> reply( QgsApplication::tileDownloadManager()->get( nr ) );

    QEventLoop loop;
    QObject::connect( reply.get(), &QgsTileDownloadManagerReply::finished, &loop, &QEventLoop::quit );
    loop.exec();

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsDebugError( QStringLiteral( "Request failed: " ) + filePath );
      return false;
    }

    dataJsonH = reply->data();
  }
  else
  {
    QFile file( filePath );
    if ( ! file.open( QIODevice::ReadOnly ) )
    {
      QgsDebugError( QStringLiteral( "Loading file failed: " ) + filePath );
      return false;
    }
    dataJsonH = file.readAll();
  }

  QJsonParseError errH;
  const QJsonDocument docH = QJsonDocument::fromJson( dataJsonH, &errH );
  if ( errH.error != QJsonParseError::NoError )
  {
    QgsDebugMsgLevel( QStringLiteral( "QJsonParseError when reading hierarchy from file %1" ).arg( filePath ), 2 );
    return false;
  }

  QMutexLocker locker( &mHierarchyMutex );
  const QJsonObject rootHObj = docH.object();
  for ( auto it = rootHObj.constBegin(); it != rootHObj.constEnd(); ++it )
  {
    const QString nodeIdStr = it.key();
    const int nodePointCount = it.value().toInt();
    const QgsPointCloudNodeId nodeId = QgsPointCloudNodeId::fromString( nodeIdStr );
    if ( nodePointCount >= 0 )
      mHierarchy[nodeId] = nodePointCount;
    else if ( nodePointCount == -1 )
      mHierarchyNodes.insert( nodeId );
  }

  return true;
}

QVector<QgsPointCloudNodeId> QgsEptPointCloudIndex::nodePathToRoot( const QgsPointCloudNodeId &nodeId ) const
{
  QVector<QgsPointCloudNodeId> path;
  QgsPointCloudNodeId currentNode = nodeId;
  do
  {
    path.push_back( currentNode );
    currentNode = currentNode.parentNode();
  }
  while ( currentNode.d() >= 0 );

  return path;
}

bool QgsEptPointCloudIndex::loadNodeHierarchy( const QgsPointCloudNodeId &nodeId ) const
{
  bool found;
  {
    QMutexLocker lock( &mHierarchyMutex );
    found = mHierarchy.contains( nodeId );
  }
  if ( found )
    return true;

  QVector<QgsPointCloudNodeId> pathToRoot = nodePathToRoot( nodeId );
  for ( int i = pathToRoot.size() - 1; i >= 0 && !mHierarchy.contains( nodeId ); --i )
  {
    const QgsPointCloudNodeId node = pathToRoot[i];
    if ( !loadSingleNodeHierarchy( node ) )
      return false;
  }

  {
    QMutexLocker lock( &mHierarchyMutex );
    found = mHierarchy.contains( nodeId );
  }

  return found;
}


bool QgsEptPointCloudIndex::isValid() const
{
  return mIsValid;
}

Qgis::PointCloudAccessType QgsEptPointCloudIndex::accessType() const
{
  return mAccessType;
}

void QgsEptPointCloudIndex::copyCommonProperties( QgsEptPointCloudIndex *destination ) const
{
  QgsAbstractPointCloudIndex::copyCommonProperties( destination );

  // QgsEptPointCloudIndex specific fields
  destination->mIsValid = mIsValid;
  destination->mAccessType = mAccessType;
  destination->mDataType = mDataType;
  destination->mUrlDirectoryPart = mUrlDirectoryPart;
  destination->mWkt = mWkt;
  destination->mHierarchyNodes = mHierarchyNodes;
  destination->mPointCount = mPointCount;
  destination->mMetadataStats = mMetadataStats;
  destination->mAttributeClasses = mAttributeClasses;
  destination->mOriginalMetadata = mOriginalMetadata;
}

#undef PROVIDER_KEY
#undef PROVIDER_DESCRIPTION

///@endcond
