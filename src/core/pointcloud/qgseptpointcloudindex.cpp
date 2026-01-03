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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscachedpointcloudblockrequest.h"
#include "qgscoordinatereferencesystem.h"
#include "qgseptdecoder.h"
#include "qgseptpointcloudblockrequest.h"
#include "qgslazdecoder.h"
#include "qgslogger.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudblockrequest.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudstatistics.h"
#include "qgssetrequestinitiator_p.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QQueue>
#include <QTime>
#include <QtDebug>

///@cond PRIVATE

#define PROVIDER_KEY u"ept"_s
#define PROVIDER_DESCRIPTION u"EPT point cloud provider"_s

QgsEptPointCloudIndex::QgsEptPointCloudIndex()
{
  mHierarchyNodes.insert( QgsPointCloudNodeId( 0, 0, 0, 0 ) );
}

QgsEptPointCloudIndex::~QgsEptPointCloudIndex() = default;

void QgsEptPointCloudIndex::load( const QString &urlString, const QString &authcfg )
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
    mAuthCfg = authcfg;
    QNetworkRequest nr = QNetworkRequest( QUrl( mUri ) );
    QgsSetRequestInitiatorClass( nr, u"QgsEptPointCloudIndex"_s );

    QgsBlockingNetworkRequest req;
    req.setAuthCfg( mAuthCfg );
    if ( req.get( nr ) != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( u"Request failed: "_s + mUri );
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
    const QString manifestPath = mUrlDirectoryPart + u"/ept-sources/manifest.json"_s;
    QByteArray manifestJson;
    if ( mAccessType == Qgis::PointCloudAccessType::Remote )
    {
      QUrl manifestUrl( manifestPath );

      QNetworkRequest nr = QNetworkRequest( QUrl( manifestUrl ) );
      QgsSetRequestInitiatorClass( nr, u"QgsEptPointCloudIndex"_s );
      QgsBlockingNetworkRequest req;
      req.setAuthCfg( mAuthCfg );
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
    QgsDebugError( u"Failed to load root EPT node"_s );
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
  const QString metadataPath = sourceObject.value( u"metadataPath"_s ).toString();
  const QString fullMetadataPath = mUrlDirectoryPart + u"/ept-sources/"_s + metadataPath;

  QByteArray metadataJson;
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    QUrl metadataUrl( fullMetadataPath );
    QNetworkRequest nr = QNetworkRequest( QUrl( metadataUrl ) );
    QgsSetRequestInitiatorClass( nr, u"QgsEptPointCloudIndex"_s );
    QgsBlockingNetworkRequest req;
    req.setAuthCfg( mAuthCfg );
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

  const QJsonObject metadataObject = metadataDoc.object().value( u"metadata"_s ).toObject();
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
  mDataType = result.value( "dataType"_L1 ).toString();  // "binary" or "laszip"
  if ( mDataType != "laszip"_L1 && mDataType != "binary"_L1 && mDataType != "zstandard"_L1 )
    return false;

  const QString hierarchyType = result.value( "hierarchyType"_L1 ).toString();  // "json" or "gzip"
  if ( hierarchyType != "json"_L1 )
    return false;

  mSpan = result.value( "span"_L1 ).toInt();
  mPointCount = result.value( "points"_L1 ).toDouble();

  // WKT
  const QJsonObject srs = result.value( "srs"_L1 ).toObject();
  mWkt = srs.value( "wkt"_L1 ).toString();

  // rectangular
  const QJsonArray bounds = result.value( "bounds"_L1 ).toArray();
  if ( bounds.size() != 6 )
    return false;

  const QJsonArray boundsConforming = result.value( "boundsConforming"_L1 ).toArray();
  if ( boundsConforming.size() != 6 )
    return false;
  mExtent.set( boundsConforming[0].toDouble(), boundsConforming[1].toDouble(),
               boundsConforming[3].toDouble(), boundsConforming[4].toDouble() );
  mZMin = boundsConforming[2].toDouble();
  mZMax = boundsConforming[5].toDouble();

  const QJsonArray schemaArray = result.value( "schema"_L1 ).toArray();
  QgsPointCloudAttributeCollection attributes;

  for ( const QJsonValue &schemaItem : schemaArray )
  {
    const QJsonObject schemaObj = schemaItem.toObject();
    const QString name = schemaObj.value( "name"_L1 ).toString();
    const QString type = schemaObj.value( "type"_L1 ).toString();

    const int size = schemaObj.value( "size"_L1 ).toInt();

    if ( name == "ClassFlags"_L1 && size == 1 )
    {
      attributes.push_back( QgsPointCloudAttribute( u"Synthetic"_s, QgsPointCloudAttribute::UChar ) );
      attributes.push_back( QgsPointCloudAttribute( u"KeyPoint"_s, QgsPointCloudAttribute::UChar ) );
      attributes.push_back( QgsPointCloudAttribute( u"Withheld"_s, QgsPointCloudAttribute::UChar ) );
      attributes.push_back( QgsPointCloudAttribute( u"Overlap"_s, QgsPointCloudAttribute::UChar ) );
    }
    else if ( type == "float"_L1 && ( size == 4 ) )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Float ) );
    }
    else if ( type == "float"_L1 && ( size == 8 ) )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Double ) );
    }
    else if ( size == 1 )
    {
      attributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Char ) );
    }
    else if ( type == "unsigned"_L1 && size == 2 )
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
    if ( schemaObj.contains( "scale"_L1 ) )
      scale = schemaObj.value( "scale"_L1 ).toDouble();

    double offset = 0.f;
    if ( schemaObj.contains( "offset"_L1 ) )
      offset = schemaObj.value( "offset"_L1 ).toDouble();

    if ( name == "X"_L1 )
    {
      mOffset.set( offset, mOffset.y(), mOffset.z() );
      mScale.set( scale, mScale.y(), mScale.z() );
    }
    else if ( name == "Y"_L1 )
    {
      mOffset.set( mOffset.x(), offset, mOffset.z() );
      mScale.set( mScale.x(), scale, mScale.z() );
    }
    else if ( name == "Z"_L1 )
    {
      mOffset.set( mOffset.x(), mOffset.y(), offset );
      mScale.set( mScale.x(), mScale.y(), scale );
    }

    // store any metadata stats which are present for the attribute
    AttributeStatistics stats;
    bool foundStats = false;
    if ( schemaObj.contains( "count"_L1 ) )
    {
      stats.count = schemaObj.value( "count"_L1 ).toInt();
      foundStats = true;
    }
    if ( schemaObj.contains( "minimum"_L1 ) )
    {
      stats.minimum = schemaObj.value( "minimum"_L1 ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( "maximum"_L1 ) )
    {
      stats.maximum = schemaObj.value( "maximum"_L1 ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( "count"_L1 ) )
    {
      stats.mean = schemaObj.value( "mean"_L1 ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( "stddev"_L1 ) )
    {
      stats.stDev = schemaObj.value( "stddev"_L1 ).toDouble();
      foundStats = true;
    }
    if ( schemaObj.contains( "variance"_L1 ) )
    {
      stats.variance = schemaObj.value( "variance"_L1 ).toDouble();
      foundStats = true;
    }
    if ( foundStats )
      mMetadataStats.insert( name, stats );

    if ( schemaObj.contains( "counts"_L1 ) )
    {
      QMap< int, int >  classCounts;
      const QJsonArray counts = schemaObj.value( "counts"_L1 ).toArray();
      for ( const QJsonValue &count : counts )
      {
        const QJsonObject countObj = count.toObject();
        classCounts.insert( countObj.value( "value"_L1 ).toInt(), countObj.value( "count"_L1 ).toInt() );
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

#ifdef QGISDEBUG
  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  QgsDebugMsgLevel( u"lvl0 node size in CRS units: %1 %2 %3"_s.arg( dx ).arg( dy ).arg( dz ), 2 );    // all dims should be the same
  QgsDebugMsgLevel( u"res at lvl0 %1"_s.arg( dx / mSpan ), 2 );
  QgsDebugMsgLevel( u"res at lvl1 %1"_s.arg( dx / mSpan / 2 ), 2 );
  QgsDebugMsgLevel( u"res at lvl2 %1 with node size %2"_s.arg( dx / mSpan / 4 ).arg( dx / 4 ), 2 );
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
      QgsDebugError( u"Error downloading node %1 data, error : %2 "_s.arg( n.toString(), blockRequest->errorStr() ) );
    }
  }
  else
  {
    // we need to create a copy of the expression to pass to the decoder
    // as the same QgsPointCloudExpression object mighgt be concurrently
    // used on another thread, for example in a 3d view
    QgsPointCloudExpression filterExpression = request.ignoreIndexFilterEnabled() ? QgsPointCloudExpression() : mFilterExpression;
    QgsPointCloudAttributeCollection requestAttributes = request.attributes();
    requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
    QgsRectangle filterRect = request.filterRect();

    if ( mDataType == "binary"_L1 )
    {
      const QString filename = u"%1/ept-data/%2.bin"_s.arg( mUrlDirectoryPart, n.toString() );
      block = QgsEptDecoder::decompressBinary( filename, attributes(), requestAttributes, scale(), offset(), filterExpression, filterRect );
    }
    else if ( mDataType == "zstandard"_L1 )
    {
      const QString filename = u"%1/ept-data/%2.zst"_s.arg( mUrlDirectoryPart, n.toString() );
      block = QgsEptDecoder::decompressZStandard( filename, attributes(), request.attributes(), scale(), offset(), filterExpression, filterRect );
    }
    else if ( mDataType == "laszip"_L1 )
    {
      const QString filename = u"%1/ept-data/%2.laz"_s.arg( mUrlDirectoryPart, n.toString() );
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
  if ( mDataType == "binary"_L1 )
  {
    fileUrl = u"%1/ept-data/%2.bin"_s.arg( mUrlDirectoryPart, n.toString() );
  }
  else if ( mDataType == "zstandard"_L1 )
  {
    fileUrl = u"%1/ept-data/%2.zst"_s.arg( mUrlDirectoryPart, n.toString() );
  }
  else if ( mDataType == "laszip"_L1 )
  {
    fileUrl = u"%1/ept-data/%2.laz"_s.arg( mUrlDirectoryPart, n.toString() );
  }
  else
  {
    return nullptr;
  }

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object might be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = request.ignoreIndexFilterEnabled() ? QgsPointCloudExpression() : mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
  return new QgsEptPointCloudBlockRequest( n, fileUrl, mDataType, attributes(), requestAttributes, scale(), offset(), filterExpression, request.filterRect(), mAuthCfg );
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

  const QString filePath = u"%1/ept-hierarchy/%2.json"_s.arg( mUrlDirectoryPart, nodeId.toString() );

  QByteArray dataJsonH;
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    QNetworkRequest nr( filePath );
    QgsSetRequestInitiatorClass( nr, u"QgsEptPointCloudIndex"_s );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( nr, mAuthCfg ) )
    {
      QgsDebugError( u"Network request update failed for authcfg: %1"_s.arg( mAuthCfg ) );
      return false;
    }

    std::unique_ptr<QgsTileDownloadManagerReply> reply( QgsApplication::tileDownloadManager()->get( nr ) );

    QEventLoop loop;
    QObject::connect( reply.get(), &QgsTileDownloadManagerReply::finished, &loop, &QEventLoop::quit );
    loop.exec();

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsDebugError( u"Request failed: "_s + filePath );
      return false;
    }

    dataJsonH = reply->data();
  }
  else
  {
    QFile file( filePath );
    if ( ! file.open( QIODevice::ReadOnly ) )
    {
      QgsDebugError( u"Loading file failed: "_s + filePath );
      return false;
    }
    dataJsonH = file.readAll();
  }

  QJsonParseError errH;
  const QJsonDocument docH = QJsonDocument::fromJson( dataJsonH, &errH );
  if ( errH.error != QJsonParseError::NoError )
  {
    QgsDebugMsgLevel( u"QJsonParseError when reading hierarchy from file %1"_s.arg( filePath ), 2 );
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

#undef PROVIDER_KEY
#undef PROVIDER_DESCRIPTION

///@endcond
