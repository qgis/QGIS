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

#include "qgseptdecoder.h"
#include "qgscoordinatereferencesystem.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "ept" )
#define PROVIDER_DESCRIPTION QStringLiteral( "EPT point cloud provider" )

QgsEptPointCloudIndex::QgsEptPointCloudIndex() = default;

QgsEptPointCloudIndex::~QgsEptPointCloudIndex() = default;

bool QgsEptPointCloudIndex::load( const QString &fileName )
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
  QJsonObject result = doc.object();
  mDataType = result.value( QLatin1String( "dataType" ) ).toString();  // "binary" or "laszip"
  if ( mDataType != "laszip" && mDataType != "binary" && mDataType != "zstandard" )
    return false;

  QString hierarchyType = result.value( QLatin1String( "hierarchyType" ) ).toString();  // "json" or "gzip"
  if ( hierarchyType != "json" )
    return false;

  mSpan = result.value( QLatin1String( "span" ) ).toInt();

  // WKT
  QJsonObject srs = result.value( QLatin1String( "srs" ) ).toObject();
  mWkt = srs.value( QLatin1String( "wkt" ) ).toString();

  // rectangular
  QJsonArray bounds = result.value( QLatin1String( "bounds" ) ).toArray();
  if ( bounds.size() != 6 )
    return false;

  QJsonArray bounds_conforming = result.value( QLatin1String( "boundsConforming" ) ).toArray();
  if ( bounds.size() != 6 )
    return false;
  mExtent.set( bounds_conforming[0].toDouble(), bounds_conforming[1].toDouble(),
               bounds_conforming[3].toDouble(), bounds_conforming[4].toDouble() );
  mZMin = bounds_conforming[2].toDouble();
  mZMax = bounds_conforming[5].toDouble();

  QJsonArray schemaArray = result.value( QLatin1String( "schema" ) ).toArray();

  mPointRecordSize = 0;
  for ( QJsonValue schemaItem : schemaArray )
  {
    QJsonObject schemaObj = schemaItem.toObject();
    QString name = schemaObj.value( QLatin1String( "name" ) ).toString();
    QString type = schemaObj.value( QLatin1String( "type" ) ).toString();

    int size = schemaObj.value( QLatin1String( "size" ) ).toInt();
    mPointRecordSize += size;

    float scale = 1.f;
    if ( schemaObj.contains( "scale" ) )
      scale = schemaObj.value( QLatin1String( "scale" ) ).toDouble();

    float offset = 0.f;
    if ( schemaObj.contains( "offset" ) )
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
    // TODO: can parse also stats: "count", "minimum", "maximum", "mean", "stddev", "variance"
  }

  // There seems to be a bug in Entwine: https://github.com/connormanning/entwine/issues/240
  // point records for X,Y,Z seem to be written as 64-bit doubles even if schema says they are 32-bit ints
  mPointRecordSize += 3 * 4;

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

QVector<qint32> QgsEptPointCloudIndex::nodePositionDataAsInt32( const IndexedPointCloudNode &n )
{
  Q_ASSERT( mHierarchy.contains( n ) );
  if ( mDataType == "binary" )
  {
    QString filename = QString( "%1/ept-data/%2.bin" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsEptDecoder::decompressBinary( filename, mPointRecordSize );
  }
  else if ( mDataType == "zstandard" )
  {
    QString filename = QString( "%1/ept-data/%2.zst" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsEptDecoder::decompressZStandard( filename, mPointRecordSize );
  }
  else if ( mDataType == "laszip" )
  {
    QString filename = QString( "%1/ept-data/%2.laz" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsEptDecoder::decompressLaz( filename );
  }
  else
  {
    Q_ASSERT( false );  // unsupported
  }
  return QVector<qint32>();
}

QgsCoordinateReferenceSystem QgsEptPointCloudIndex::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt( mWkt );
}

QVector<char> QgsEptPointCloudIndex::nodeClassesDataAsChar( const IndexedPointCloudNode &n )
{
  Q_ASSERT( mHierarchy.contains( n ) );
  // int count = mHierarchy[n];

  if ( mDataType == "binary" )
  {
    // TODO: ugly me... reading same file twice :vomit:
    QString filename = QString( "%1/ept-data/%2.bin" ).arg( mDirectory ).arg( n.toString() );
    Q_ASSERT( QFile::exists( filename ) );
    return QgsEptDecoder::decompressBinaryClasses( filename, mPointRecordSize );
  }
  else if ( mDataType == "zstandard" )
  {
    //TODO
    Q_ASSERT( false );
  }
  else if ( mDataType == "laszip" )
  {
    //TODO
    Q_ASSERT( false );
  }
  else
  {
    Q_ASSERT( false );  // unsupported
  }
  return QVector<char>();
}

///@endcond
