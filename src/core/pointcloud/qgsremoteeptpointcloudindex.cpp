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

#include "qgsremoteeptpointcloudindex.h"

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
#include "qgsfiledownloader.h"

#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "qgspointcloudblockhandle.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "ept" )
#define PROVIDER_DESCRIPTION QStringLiteral( "EPT point cloud provider" )

QgsRemoteEptPointCloudIndex::QgsRemoteEptPointCloudIndex() : QgsPointCloudIndex()
{
  mTileDownloadManager = QgsApplication::tileDownloadManager();
}

QgsRemoteEptPointCloudIndex::~QgsRemoteEptPointCloudIndex() = default;

void QgsRemoteEptPointCloudIndex::load( const QString &url )
{
  mUrl = QUrl( url );

  QStringList splittedUrl = url.split( '/' );

  mUrlFileNamePart = splittedUrl.back();
  splittedUrl.pop_back();
  mUrlDirectoryPart = splittedUrl.join( '/' );

  QNetworkRequest nr( url );

  QgsBlockingNetworkRequest req;
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + url );
    mIsValid = false;
    return;
  }

  QgsNetworkReplyContent reply = req.reply();
  bool success = loadSchema( reply.content() );
  if ( success )
  {
    success = loadHierarchy();
  }

  mIsValid = success;
}

bool QgsRemoteEptPointCloudIndex::loadSchema( const QByteArray &data )
{
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( data, &err );
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
  mPointCount = result.value( QLatin1String( "points" ) ).toInt();

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

    int size = schemaObj.value( QLatin1String( "size" ) ).toInt();

    if ( type == QLatin1String( "float" ) && ( size == 4 ) )
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
    if ( schemaObj.contains( QLatin1String( "count" ) ) )
      stats.count = schemaObj.value( QLatin1String( "count" ) ).toInt();
    if ( schemaObj.contains( QLatin1String( "minimum" ) ) )
      stats.minimum = schemaObj.value( QLatin1String( "minimum" ) ).toDouble();
    if ( schemaObj.contains( QLatin1String( "maximum" ) ) )
      stats.maximum = schemaObj.value( QLatin1String( "maximum" ) ).toDouble();
    if ( schemaObj.contains( QLatin1String( "count" ) ) )
      stats.mean = schemaObj.value( QLatin1String( "mean" ) ).toDouble();
    if ( schemaObj.contains( QLatin1String( "stddev" ) ) )
      stats.stDev = schemaObj.value( QLatin1String( "stddev" ) ).toDouble();
    if ( schemaObj.contains( QLatin1String( "variance" ) ) )
      stats.variance = schemaObj.value( QLatin1String( "variance" ) ).toDouble();
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

  // try to import the metadata too!

  QNetworkRequest nr( QStringLiteral( "%1/ept-sources/manifest.json" ).arg( mUrlDirectoryPart ) );

  QgsBlockingNetworkRequest req;
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
  if ( errCode == QgsBlockingNetworkRequest::NoError )
  {
    QgsNetworkReplyContent reply = req.reply();
    const QByteArray manifestJson = reply.content();
    const QJsonDocument manifestDoc = QJsonDocument::fromJson( manifestJson, &err );
    if ( err.error == QJsonParseError::NoError )
    {
      const QJsonArray manifestArray = manifestDoc.array();
      // TODO how to handle multiple?
      if ( ! manifestArray.empty() )
      {
        const QJsonObject sourceObject = manifestArray.at( 0 ).toObject();
        const QString metadataPath = sourceObject.value( QStringLiteral( "metadataPath" ) ).toString();

        QNetworkRequest metadataFileNetworkRequest( QStringLiteral( "%1/ept-sources/%2" ).arg( mUrlDirectoryPart, metadataPath ) );
        QgsBlockingNetworkRequest::ErrorCode errCode = req.get( metadataFileNetworkRequest );
        if ( errCode == QgsBlockingNetworkRequest::NoError )
        {
          QgsNetworkReplyContent reply = req.reply();
          const QByteArray metadataJson = reply.content();
          const QJsonDocument metadataDoc = QJsonDocument::fromJson( metadataJson, &err );
          if ( err.error == QJsonParseError::NoError )
          {
            const QJsonObject metadataObject = metadataDoc.object().value( QStringLiteral( "metadata" ) ).toObject();
            if ( !metadataObject.empty() )
            {
              const QJsonObject sourceMetadata = metadataObject.constBegin().value().toObject();
              mOriginalMetadata = sourceMetadata.toVariantMap();
            }
          }
        }
      }
    }
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
                  ( ymin - mOffset.y() ) / mScale.y(),
                  ( zmin - mOffset.z() ) / mScale.z(),
                  ( xmax - mOffset.x() ) / mScale.x(),
                  ( ymax - mOffset.y() ) / mScale.y(),
                  ( zmax - mOffset.z() ) / mScale.z()
                );


#ifdef QGIS_DEBUG
  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  QgsDebugMsgLevel( QStringLiteral( "lvl0 node size in CRS units: %1 %2 %3" ).arg( dx ).arg( dy ).arg( dz ), 2 );    // all dims should be the same
  QgsDebugMsgLevel( QStringLiteral( "res at lvl0 %1" ).arg( dx / mSpan ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl1 %1" ).arg( dx / mSpan / 2 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl2 %1 with node size %2" ).arg( dx / mSpan / 4 ).arg( dx / 4 ), 2 );
#endif

  return true;
}

QgsPointCloudBlock *QgsRemoteEptPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  qDebug() << __PRETTY_FUNCTION__;
  if ( !mHierarchy.contains( n ) )
    return nullptr;
  QString fileUrl;
  QString fileName;
  if ( mDataType == QLatin1String( "binary" ) )
  {

    fileUrl = QStringLiteral( "%1/ept-data/%2.bin" ).arg( mUrlDirectoryPart, n.toString() );
    fileName = QStringLiteral( "/tmp/%1.bin" ).arg( n.toString() );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.zst" ).arg( mUrlDirectoryPart, n.toString() );
    fileName = QStringLiteral( "/tmp/%1.zst" ).arg( n.toString() );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.laz" ).arg( mUrlDirectoryPart, n.toString() );
    fileName = QStringLiteral( "/tmp/%1.laz" ).arg( n.toString() );
  }
  else
  {
    return nullptr;  // unsupported
  }
  std::unique_ptr<QgsFileDownloader> downloader( new QgsFileDownloader( fileUrl, fileName ) );

  int timeout = 10000;
  QTimer timer;
  timer.setSingleShot( true );
  QEventLoop loop;

  QUrl downloadedUrl;
  QStringList errorMessages;
  connect( downloader.get(), &QgsFileDownloader::downloadCompleted, [&]( const QUrl & url )
  {
    downloadedUrl = url;
    loop.quit();
  } );
  connect( downloader.get(), &QgsFileDownloader::downloadCanceled, &loop, &QEventLoop::quit );
  connect( downloader.get(), &QgsFileDownloader::downloadError, [&]( QStringList errorMessagesList )
  {
    errorMessages = errorMessagesList;
    loop.exit();
  } );

  connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
  downloader->startDownload();
  timer.start( timeout );
  loop.exec();

  if ( downloadedUrl.isEmpty() )
  {
    qDebug() << "Error downloading " << fileUrl << " : " << errorMessages;
    return nullptr;
  }
  qDebug() << "File downloaded";

  if ( mDataType == QLatin1String( "binary" ) )
  {
    return QgsEptDecoder::decompressBinary( fileName, attributes(), request.attributes() );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    return QgsEptDecoder::decompressZStandard( fileName, attributes(), request.attributes() );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    return QgsEptDecoder::decompressLaz( fileName, attributes(), request.attributes() );
  }
  return nullptr;
}

QgsPointCloudBlockHandle *QgsRemoteEptPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( !mHierarchy.contains( n ) )
    return nullptr;
  QgsPointCloudBlockHandle *handle = nullptr;

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
  QNetworkRequest nr( fileUrl );
  QgsTileDownloadManagerReply *reply = mTileDownloadManager->get( nr );
  handle = new QgsPointCloudBlockHandle( mDataType, attributes(), request.attributes(), reply );
  return handle;
}

QgsCoordinateReferenceSystem QgsRemoteEptPointCloudIndex::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt( mWkt );
}

int QgsRemoteEptPointCloudIndex::pointCount() const
{
  return mPointCount;
}

QVariant QgsRemoteEptPointCloudIndex::metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const
{
  if ( !mMetadataStats.contains( attribute ) )
    return QVariant();

  const AttributeStatistics &stats = mMetadataStats[ attribute ];
  switch ( statistic )
  {
    case QgsStatisticalSummary::Count:
      return stats.count >= 0 ? QVariant( stats.count ) : QVariant();

    case QgsStatisticalSummary::Mean:
      return std::isnan( stats.mean ) ? QVariant() : QVariant( stats.mean );

    case QgsStatisticalSummary::StDev:
      return std::isnan( stats.stDev ) ? QVariant() : QVariant( stats.stDev );

    case QgsStatisticalSummary::Min:
      return stats.minimum;

    case QgsStatisticalSummary::Max:
      return stats.maximum;

    case QgsStatisticalSummary::Range:
      return stats.minimum.isValid() && stats.maximum.isValid() ? QVariant( stats.maximum.toDouble() - stats.minimum.toDouble() ) : QVariant();

    case QgsStatisticalSummary::CountMissing:
    case QgsStatisticalSummary::Sum:
    case QgsStatisticalSummary::Median:
    case QgsStatisticalSummary::StDevSample:
    case QgsStatisticalSummary::Minority:
    case QgsStatisticalSummary::Majority:
    case QgsStatisticalSummary::Variety:
    case QgsStatisticalSummary::FirstQuartile:
    case QgsStatisticalSummary::ThirdQuartile:
    case QgsStatisticalSummary::InterQuartileRange:
    case QgsStatisticalSummary::First:
    case QgsStatisticalSummary::Last:
    case QgsStatisticalSummary::All:
      return QVariant();
  }
  return QVariant();
}

QVariantList QgsRemoteEptPointCloudIndex::metadataClasses( const QString &attribute ) const
{
  QVariantList classes;
  const QMap< int, int > values =  mAttributeClasses.value( attribute );
  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    classes << it.key();
  }
  return classes;
}

QVariant QgsRemoteEptPointCloudIndex::metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const
{
  if ( statistic != QgsStatisticalSummary::Count )
    return QVariant();

  const QMap< int, int > values =  mAttributeClasses.value( attribute );
  if ( !values.contains( value.toInt() ) )
    return QVariant();
  return values.value( value.toInt() );
}

bool QgsRemoteEptPointCloudIndex::loadHierarchy()
{
  QQueue<QString> queue;
  queue.enqueue( QStringLiteral( "0-0-0-0" ) );
  while ( !queue.isEmpty() )
  {
    //const QString filename = QStringLiteral( "%1/ept-hierarchy/%2.json" ).arg( mDirectory, queue.dequeue() );
    const QString fileUrl = QStringLiteral( "%1/ept-hierarchy/%2.json" ).arg( mUrlDirectoryPart, queue.dequeue() );
    QNetworkRequest nr( fileUrl );

    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "unable to read hierarchy from file %1" ).arg( fileUrl ), 2 );
      return false;
    }

    QgsNetworkReplyContent reply = req.reply();

    QByteArray dataJsonH = reply.content();
    QJsonParseError errH;
    const QJsonDocument docH = QJsonDocument::fromJson( dataJsonH, &errH );
    if ( errH.error != QJsonParseError::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "QJsonParseError when reading hierarchy from file %1" ).arg( fileUrl ), 2 );
      return false;
    }

    const QJsonObject rootHObj = docH.object();
    for ( auto it = rootHObj.constBegin(); it != rootHObj.constEnd(); ++it )
    {
      QString nodeIdStr = it.key();
      int nodePointCount = it.value().toInt();
      if ( nodePointCount < 0 )
      {
        queue.enqueue( nodeIdStr );
      }
      else
      {
        IndexedPointCloudNode nodeId = IndexedPointCloudNode::fromString( nodeIdStr );
        mHierarchy[nodeId] = nodePointCount;
      }
    }
  }
  return true;
}

bool QgsRemoteEptPointCloudIndex::isValid() const
{
  return mIsValid;
}

///@endcond
