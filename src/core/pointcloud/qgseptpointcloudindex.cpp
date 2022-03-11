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

#include "qgseptdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgspointcloudexpression.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "ept" )
#define PROVIDER_DESCRIPTION QStringLiteral( "EPT point cloud provider" )

QgsEptPointCloudIndex::QgsEptPointCloudIndex() = default;

QgsEptPointCloudIndex::~QgsEptPointCloudIndex() = default;

void QgsEptPointCloudIndex::load( const QString &fileName )
{
  QFile f( fileName );
  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to open %1 for reading" ).arg( fileName ) );
    mIsValid = false;
    return;
  }

  const QDir directory = QFileInfo( fileName ).absoluteDir();
  mDirectory = directory.absolutePath();

  const QByteArray dataJson = f.readAll();
  bool success = loadSchema( dataJson );

  if ( success )
  {
    // try to import the metadata too!
    QFile manifestFile( mDirectory + QStringLiteral( "/ept-sources/manifest.json" ) );
    if ( manifestFile.open( QIODevice::ReadOnly ) )
    {
      const QByteArray manifestJson = manifestFile.readAll();
      loadManifest( manifestJson );
    }
  }

  if ( success )
  {
    success = loadHierarchy();
  }

  mIsValid = success;
}

void QgsEptPointCloudIndex::loadManifest( const QByteArray &manifestJson )
{
  QJsonParseError err;
  // try to import the metadata too!
  const QJsonDocument manifestDoc = QJsonDocument::fromJson( manifestJson, &err );
  if ( err.error == QJsonParseError::NoError )
  {
    const QJsonArray manifestArray = manifestDoc.array();
    // TODO how to handle multiple?
    if ( ! manifestArray.empty() )
    {
      const QJsonObject sourceObject = manifestArray.at( 0 ).toObject();
      const QString metadataPath = sourceObject.value( QStringLiteral( "metadataPath" ) ).toString();
      QFile metadataFile( mDirectory + QStringLiteral( "/ept-sources/" ) + metadataPath );
      if ( metadataFile.open( QIODevice::ReadOnly ) )
      {
        const QByteArray metadataJson = metadataFile.readAll();
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

  // save mRootBounds

  // bounds (cube - octree volume)
  const double xmin = bounds[0].toDouble();
  const double ymin = bounds[1].toDouble();
  const double zmin = bounds[2].toDouble();
  const double xmax = bounds[3].toDouble();
  const double ymax = bounds[4].toDouble();
  const double zmax = bounds[5].toDouble();

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

QgsPointCloudBlock *QgsEptPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  mHierarchyMutex.lock();
  const bool found = mHierarchy.contains( n );
  mHierarchyMutex.unlock();
  if ( !found )
    return nullptr;

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object mighgt be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );

  if ( mDataType == QLatin1String( "binary" ) )
  {
    const QString filename = QStringLiteral( "%1/ept-data/%2.bin" ).arg( mDirectory, n.toString() );
    return QgsEptDecoder::decompressBinary( filename, attributes(), requestAttributes, scale(), offset(), filterExpression );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    const QString filename = QStringLiteral( "%1/ept-data/%2.zst" ).arg( mDirectory, n.toString() );
    return QgsEptDecoder::decompressZStandard( filename, attributes(), request.attributes(), scale(), offset(), filterExpression );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    const QString filename = QStringLiteral( "%1/ept-data/%2.laz" ).arg( mDirectory, n.toString() );
    return QgsEptDecoder::decompressLaz( filename, attributes(), requestAttributes, scale(), offset(), filterExpression );
  }
  else
  {
    return nullptr;  // unsupported
  }
}

QgsPointCloudBlockRequest *QgsEptPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  Q_UNUSED( n );
  Q_UNUSED( request );
  Q_ASSERT( false );
  return nullptr; // unsupported
}

QgsCoordinateReferenceSystem QgsEptPointCloudIndex::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt( mWkt );
}

qint64 QgsEptPointCloudIndex::pointCount() const
{
  return mPointCount;
}

QVariant QgsEptPointCloudIndex::metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const
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

QVariantList QgsEptPointCloudIndex::metadataClasses( const QString &attribute ) const
{
  QVariantList classes;
  const QMap< int, int > values =  mAttributeClasses.value( attribute );
  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    classes << it.key();
  }
  return classes;
}

QVariant QgsEptPointCloudIndex::metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const
{
  if ( statistic != QgsStatisticalSummary::Count )
    return QVariant();

  const QMap< int, int > values =  mAttributeClasses.value( attribute );
  if ( !values.contains( value.toInt() ) )
    return QVariant();
  return values.value( value.toInt() );
}

bool QgsEptPointCloudIndex::loadHierarchy()
{
  QQueue<QString> queue;
  queue.enqueue( QStringLiteral( "0-0-0-0" ) );
  while ( !queue.isEmpty() )
  {
    const QString filename = QStringLiteral( "%1/ept-hierarchy/%2.json" ).arg( mDirectory, queue.dequeue() );
    QFile fH( filename );
    if ( !fH.open( QIODevice::ReadOnly ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "unable to read hierarchy from file %1" ).arg( filename ), 2 );
      return false;
    }

    const QByteArray dataJsonH = fH.readAll();
    QJsonParseError errH;
    const QJsonDocument docH = QJsonDocument::fromJson( dataJsonH, &errH );
    if ( errH.error != QJsonParseError::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "QJsonParseError when reading hierarchy from file %1" ).arg( filename ), 2 );
      return false;
    }

    const QJsonObject rootHObj = docH.object();
    for ( auto it = rootHObj.constBegin(); it != rootHObj.constEnd(); ++it )
    {
      const QString nodeIdStr = it.key();
      const int nodePointCount = it.value().toInt();
      if ( nodePointCount < 0 )
      {
        queue.enqueue( nodeIdStr );
      }
      else
      {
        const IndexedPointCloudNode nodeId = IndexedPointCloudNode::fromString( nodeIdStr );
        mHierarchyMutex.lock();
        mHierarchy[nodeId] = nodePointCount;
        mHierarchyMutex.unlock();
      }
    }
  }
  return true;
}

bool QgsEptPointCloudIndex::isValid() const
{
  return mIsValid;
}

///@endcond
