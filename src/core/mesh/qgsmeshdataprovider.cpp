/***************************************************************************
                         qgsmeshdataprovider.cpp
                         -----------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
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

#include "qgis.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshdataprovidertemporalcapabilities.h"
#include "qgsrectangle.h"

QgsMeshDataProvider::QgsMeshDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( uri, options, flags )
{
}

QgsMeshDataProviderTemporalCapabilities *QgsMeshDataProvider::temporalCapabilities()
{
  return mTemporalCapabilities.get();
}

const QgsMeshDataProviderTemporalCapabilities *QgsMeshDataProvider::temporalCapabilities() const
{
  return mTemporalCapabilities.get();
}

void QgsMeshDataProvider::setTemporalUnit( QgsUnitTypes::TemporalUnit unit )
{
  const QgsUnitTypes::TemporalUnit oldUnit = mTemporalCapabilities->temporalUnit();
  mTemporalCapabilities->setTemporalUnit( unit );
  if ( oldUnit != unit )
    reloadData();
}

QgsMeshDriverMetadata QgsMeshDataProvider::driverMetadata() const { return QgsMeshDriverMetadata();}

QgsMeshDatasetIndex QgsMeshDatasetSourceInterface::datasetIndexAtTime(
  const QDateTime &referenceTime,
  int groupIndex, qint64 time,
  QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod method ) const
{
  const QDateTime requestDateTime = referenceTime.addMSecs( time );
  qint64 providerTime;
  const QDateTime providerReferenceTime = mTemporalCapabilities->referenceTime();
  if ( providerReferenceTime.isValid() )
    providerTime = providerReferenceTime.msecsTo( requestDateTime );
  else
    providerTime = time;

  switch ( method )
  {
    case QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetBeforeStartRangeTime:
      return mTemporalCapabilities->datasetIndexClosestBeforeRelativeTime( groupIndex, providerTime );
      break;
    case QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetFromStartRangeTime:
      return mTemporalCapabilities->datasetIndexClosestFromRelativeTime( groupIndex, providerTime );
      break;
  }

  return QgsMeshDatasetIndex();
}

QList<QgsMeshDatasetIndex> QgsMeshDatasetSourceInterface::datasetIndexInTimeInterval( const QDateTime &referenceTime, int groupIndex, qint64 time1, qint64 time2 ) const
{
  const QDateTime requestDateTime = referenceTime.addMSecs( time1 );
  qint64 providerTime1;
  qint64 providerTime2;
  const QDateTime providerReferenceTime = mTemporalCapabilities->referenceTime();
  if ( mTemporalCapabilities->referenceTime().isValid() )
  {
    providerTime1 = providerReferenceTime.msecsTo( requestDateTime );
    providerTime2 = providerTime1 - time1 + time2;
  }
  else
  {
    providerTime1 = time1;
    providerTime2 = time2;
  }

  QList<QgsMeshDatasetIndex> ret;
  for ( int i = 0; i < datasetCount( groupIndex ); ++i )
  {
    QgsMeshDatasetIndex datasetIndex( groupIndex, i );
    qint64 time = mTemporalCapabilities->datasetTime( datasetIndex );
    if ( time >= providerTime1 && time <= providerTime2 )
      ret.append( datasetIndex );
  }

  return ret;
}

QgsMeshDatasetSourceInterface::QgsMeshDatasetSourceInterface():
  mTemporalCapabilities( std::make_unique<QgsMeshDataProviderTemporalCapabilities>() ) {}

int QgsMeshDatasetSourceInterface::datasetCount( QgsMeshDatasetIndex index ) const
{
  return datasetCount( index.group() );
}

QgsMeshDatasetGroupMetadata QgsMeshDatasetSourceInterface::datasetGroupMetadata( QgsMeshDatasetIndex index ) const
{
  return datasetGroupMetadata( index.group() );
}

bool QgsMeshDatasetSourceInterface::persistDatasetGroup(
  const QString &path,
  const QgsMeshDatasetGroupMetadata &meta,
  const QVector<QgsMeshDataBlock> &datasetValues,
  const QVector<QgsMeshDataBlock> &datasetActive,
  const QVector<double> &times )
{
  // Form DRIVER:filename
  QString filename = path;
  // ASCII dat supports face, edge and vertex datasets
  QString driverName = QStringLiteral( "DAT" );
  QStringList parts = path.split( ':' );
  if ( parts.size() > 1 )
  {
    driverName = parts[0];
    parts.removeFirst();
    filename = parts.join( QString() );
  }
  return persistDatasetGroup( filename, driverName, meta, datasetValues, datasetActive, times );
}

QgsMeshVertex QgsMesh::vertex( int index ) const
{
  if ( index < vertices.size() && index >= 0 )
    return vertices[index];
  return QgsMeshVertex();
}

QgsMeshFace QgsMesh::face( int index ) const
{
  if ( index < faces.size() && index >= 0 )
    return faces[index];
  return QgsMeshFace();
}

QgsMeshEdge QgsMesh::edge( int index ) const
{
  if ( index < edges.size() && index >= 0 )
    return edges[index];
  return QgsMeshEdge();
}

void QgsMesh::clear()
{
  vertices.clear();
  edges.clear();
  faces.clear();
}

bool QgsMesh::compareFaces( const QgsMeshFace &face1, const QgsMeshFace &face2 )
{
  if ( face1.count() != face2.count() )
    return false;

  int startFace2 = 0;
  for ( int i = 0; i < face2.count(); ++i )
    if ( face2.at( i ) == face1.at( 0 ) )
    {
      startFace2 = i;
      break;
    }

  for ( int i = 0; i < face1.count(); ++i )
    if ( face1.at( i ) != face2.at( ( i + startFace2 ) % ( face2.count() ) ) )
      return false;

  return true;
}

bool QgsMesh::contains( const QgsMesh::ElementType &type ) const
{
  switch ( type )
  {
    case ElementType::Vertex:
      return !vertices.isEmpty();
    case ElementType::Edge:
      return !edges.isEmpty();
    case ElementType::Face:
      return !faces.isEmpty();
  }
  return false;
}

int QgsMesh::vertexCount() const
{
  return vertices.size();
}

int QgsMesh::faceCount() const
{
  return faces.size();
}

int QgsMesh::edgeCount() const
{
  return edges.size();
}

bool QgsMeshDataSourceInterface::contains( const QgsMesh::ElementType &type ) const
{
  switch ( type )
  {
    case QgsMesh::ElementType::Vertex:
      return vertexCount() != 0;
    case QgsMesh::ElementType::Edge:
      return edgeCount() != 0;
    case QgsMesh::ElementType::Face:
      return faceCount() != 0;
  }
  return false;
}
