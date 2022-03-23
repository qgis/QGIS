/***************************************************************************
                         qgscopcpointcloudindex.cpp
                         --------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
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

#include "qgscopcpointcloudindex.h"
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

#include "lazperf/lazperf.hpp"
#include "lazperf/readers.hpp"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud provider" )

QgsCopcPointCloudIndex::QgsCopcPointCloudIndex() = default;

QgsCopcPointCloudIndex::~QgsCopcPointCloudIndex() = default;

void QgsCopcPointCloudIndex::load( const QString &fileName )
{
  QFile f( fileName );
  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to open %1 for reading" ).arg( fileName ) );
    mIsValid = false;
    return;
  }

  mFileName = fileName;

  bool success = loadSchema( fileName );

  if ( success )
  {
    success = loadHierarchy( fileName );
  }

  mIsValid = success;
}

bool QgsCopcPointCloudIndex::loadSchema( const QString &filename )
{
  std::ifstream file( filename.toStdString() );
  lazperf::reader::generic_file f( file );

  mDataType = QStringLiteral( "copc" );

  mPointCount = f.header().point_count;

  mScale = QgsVector3D( f.header().scale.x, f.header().scale.y, f.header().scale.z );
  mOffset = QgsVector3D( f.header().offset.x, f.header().offset.y, f.header().offset.z );

  // The COPC format only uses PDRF 6, 7 or 8. So there should be a OGC Coordinate System WKT record
  mWkt = QString();
  std::vector<char> wktRecordData = f.vlrData( "LASF_Projection", 2112 );
  if ( !wktRecordData.empty() )
  {
    lazperf::wkt_vlr wktVlr;
    wktVlr.fill( wktRecordData.data(), wktRecordData.size() );
    mWkt = QString::fromStdString( wktVlr.wkt );
  }

  mExtent.set( f.header().minx, f.header().miny, f.header().maxx, f.header().maxy );

  mZMin = f.header().minz;
  mZMax = f.header().maxz;


  // Attributes for COPC format
  // COPC supports only PDRF 6, 7 and 8

  // TODO: How to handle bitfields in LAZ

  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( "X", ( QgsPointCloudAttribute::DataType ) 9 ) );
  attributes.push_back( QgsPointCloudAttribute( "Y", ( QgsPointCloudAttribute::DataType ) 9 ) );
  attributes.push_back( QgsPointCloudAttribute( "Z", ( QgsPointCloudAttribute::DataType ) 9 ) );
  attributes.push_back( QgsPointCloudAttribute( "Classification", ( QgsPointCloudAttribute::DataType ) 0 ) );
  attributes.push_back( QgsPointCloudAttribute( "Intensity", ( QgsPointCloudAttribute::DataType ) 3 ) );
  attributes.push_back( QgsPointCloudAttribute( "ReturnNumber", ( QgsPointCloudAttribute::DataType ) 0 ) );
  attributes.push_back( QgsPointCloudAttribute( "NumberOfReturns", ( QgsPointCloudAttribute::DataType ) 0 ) );
  attributes.push_back( QgsPointCloudAttribute( "ScanDirectionFlag", ( QgsPointCloudAttribute::DataType ) 0 ) );
  attributes.push_back( QgsPointCloudAttribute( "EdgeOfFlightLine", ( QgsPointCloudAttribute::DataType ) 0 ) );
  attributes.push_back( QgsPointCloudAttribute( "ScanAngleRank", ( QgsPointCloudAttribute::DataType ) 8 ) );
  attributes.push_back( QgsPointCloudAttribute( "UserData", ( QgsPointCloudAttribute::DataType ) 0 ) );
  attributes.push_back( QgsPointCloudAttribute( "PointSourceId", ( QgsPointCloudAttribute::DataType ) 3 ) );
  attributes.push_back( QgsPointCloudAttribute( "GpsTime", ( QgsPointCloudAttribute::DataType ) 9 ) );

  switch ( f.header().point_format_id )
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

  // TODO: add extrabyte attributes

  setAttributes( attributes );

  std::vector<char> copcInfoVlrData = f.vlrData( "copc", 1 );

  lazperf::copc_info_vlr copcInfoVlr;
  copcInfoVlr.fill( copcInfoVlrData.data(), copcInfoVlrData.size() );

  const double xmin = copcInfoVlr.center_x - copcInfoVlr.halfsize;
  const double ymin = copcInfoVlr.center_y - copcInfoVlr.halfsize;
  const double zmin = copcInfoVlr.center_z - copcInfoVlr.halfsize;
  const double xmax = copcInfoVlr.center_x + copcInfoVlr.halfsize;
  const double ymax = copcInfoVlr.center_y + copcInfoVlr.halfsize;
  const double zmax = copcInfoVlr.center_z + copcInfoVlr.halfsize;

  mRootBounds = QgsPointCloudDataBounds(
                  ( xmin - mOffset.x() ) / mScale.x(),
                  ( ymin - mOffset.y() ) / mScale.y(),
                  ( zmin - mOffset.z() ) / mScale.z(),
                  ( xmax - mOffset.x() ) / mScale.x(),
                  ( ymax - mOffset.y() ) / mScale.y(),
                  ( zmax - mOffset.z() ) / mScale.z()
                );

  double calculatedSpan = nodeMapExtent( root() ).width() / copcInfoVlr.spacing;
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

QgsPointCloudBlock *QgsCopcPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  mHierarchyMutex.lock();
  const bool found = mHierarchy.contains( n );
  int pointCount = mHierarchy[n];
  uint64_t blockOffset = mHierarchyNodeOffset[n];
  int32_t blockSize = mHierarchyNodeByteSize[n];
  mHierarchyMutex.unlock();
  if ( !found )
    return nullptr;

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object mighgt be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );

  return QgsEptDecoder::decompressCopc( mFileName, blockOffset, blockSize, pointCount, attributes(), requestAttributes, scale(), offset(), filterExpression );
}

QgsPointCloudBlockRequest *QgsCopcPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  Q_UNUSED( n );
  Q_UNUSED( request );
  Q_ASSERT( false );
  return nullptr; // unsupported
}

QgsCoordinateReferenceSystem QgsCopcPointCloudIndex::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt( mWkt );
}

qint64 QgsCopcPointCloudIndex::pointCount() const
{
  return mPointCount;
}

QVariant QgsCopcPointCloudIndex::metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const
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

QVariantList QgsCopcPointCloudIndex::metadataClasses( const QString &attribute ) const
{
  QVariantList classes;
  const QMap< int, int > values =  mAttributeClasses.value( attribute );
  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    classes << it.key();
  }
  return classes;
}

QVariant QgsCopcPointCloudIndex::metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const
{
  if ( statistic != QgsStatisticalSummary::Count )
    return QVariant();

  const QMap< int, int > values =  mAttributeClasses.value( attribute );
  if ( !values.contains( value.toInt() ) )
    return QVariant();
  return values.value( value.toInt() );
}

bool QgsCopcPointCloudIndex::loadHierarchy( const QString &filename )
{
  std::ifstream file( filename.toStdString() );
  lazperf::reader::generic_file f( file );

  std::vector<char> copcInfoVlrData = f.vlrData( "copc", 1 );

  lazperf::copc_info_vlr copcInfoVlr;
  copcInfoVlr.fill( copcInfoVlrData.data(), copcInfoVlrData.size() );

  QQueue<std::pair<uint64_t, uint64_t>> queue;
  queue.push_back( std::make_pair( copcInfoVlr.root_hier_offset, copcInfoVlr.root_hier_size ) );

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

  while ( !queue.isEmpty() )
  {
    auto [offset, size] = queue.dequeue();

    file.seekg( offset );
    std::unique_ptr<char> data( new char[ offset ] );
    file.read( data.get(), size );

    for ( uint64_t i = 0; i < size; i += sizeof( CopcEntry ) )
    {
      CopcEntry *entry = reinterpret_cast<CopcEntry *>( data.get() + i );
      if ( entry->pointCount < 0 )
      {
        queue.enqueue( std::make_pair( entry->offset, entry->byteSize ) );
      }
      else if ( entry->pointCount > 0 )
      {
        const IndexedPointCloudNode nodeId( entry->key.level, entry->key.x, entry->key.y, entry->key.z );
        mHierarchyMutex.lock();
        mHierarchy[nodeId] = entry->pointCount;
        mHierarchyNodeOffset[nodeId] = entry->offset;
        mHierarchyNodeByteSize[nodeId] = entry->byteSize;
        mHierarchyMutex.unlock();
      }
    }
  }
  return true;
}

bool QgsCopcPointCloudIndex::isValid() const
{
  return mIsValid;
}

///@endcond
