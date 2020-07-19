/***************************************************************************
                         qgsmeshmemorydataprovider.cpp
                         -----------------------------
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
///@cond PRIVATE

#include "qgsmeshmemorydataprovider.h"
#include "qgsmeshdataprovidertemporalcapabilities.h"
#include "qgsmeshlayerutils.h"
#include "qgstriangularmesh.h"
#include <cstring>

#define TEXT_PROVIDER_KEY QStringLiteral( "mesh_memory" )
#define TEXT_PROVIDER_DESCRIPTION QStringLiteral( "Mesh memory provider" )

bool QgsMeshMemoryDataProvider::isValid() const
{
  return true;
}

QString QgsMeshMemoryDataProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMeshMemoryDataProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsMeshMemoryDataProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QgsMeshMemoryDataProvider::QgsMeshMemoryDataProvider( const QString &uri, const ProviderOptions &options )
  : QgsMeshDataProvider( uri, options )
{
  QString data( uri );
  // see QgsMeshLayer::setDataProvider how mDataSource is created for memory layers
  if ( uri.contains( "&uid=" ) )
  {
    data = uri.split( "&uid=" )[0];
  }
  mIsValid = splitMeshSections( data );

  temporalCapabilities()->setTemporalUnit( QgsUnitTypes::TemporalHours );
}

QString QgsMeshMemoryDataProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMeshMemoryDataProvider::providerDescription()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsMeshMemoryDataProvider *QgsMeshMemoryDataProvider::createProvider( const QString &uri, const ProviderOptions &options )
{
  return new QgsMeshMemoryDataProvider( uri, options );
}

bool QgsMeshMemoryDataProvider::splitMeshSections( const QString &uri )
{
  const QStringList sections = uri.split( QStringLiteral( "---" ), QString::SkipEmptyParts );
  if ( sections.size() != 2 )
  {
    setError( QgsError( tr( "Invalid mesh definition, does not contain 2 sections" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  if ( addMeshVertices( sections[0] ) )
    return addMeshFacesOrEdges( sections[1] );
  else
    return false;
}

bool QgsMeshMemoryDataProvider::addMeshVertices( const QString &def )
{
  QVector<QgsMeshVertex> vertices;

  const QStringList verticesCoords = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < verticesCoords.size(); ++i )
  {
    const QStringList coords = verticesCoords[i].split( ',', QString::SkipEmptyParts );
    if ( coords.size() != 2 )
    {
      setError( QgsError( tr( "Invalid mesh definition, vertex definition does not contain x, y" ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
      return false;
    }
    double x = coords.at( 0 ).toDouble();
    double y = coords.at( 1 ).toDouble();
    QgsMeshVertex vertex( x, y );
    vertices.push_back( vertex );
  }

  mVertices = vertices;
  return true;
}

bool QgsMeshMemoryDataProvider::addMeshFacesOrEdges( const QString &def )
{
  QVector<QgsMeshFace> faces;
  QVector<QgsMeshEdge> edges;

  const QStringList elements = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < elements.size(); ++i )
  {
    const QStringList vertices = elements[i].split( ',', QString::SkipEmptyParts );
    if ( vertices.size() < 2 )
    {
      setError( QgsError( tr( "Invalid mesh definition, edge must contain at least 2 vertices" ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
      return false;
    }
    else if ( vertices.size() == 2 )
    {
      QgsMeshEdge edge;
      edge.first = vertices[0].toInt();
      edge.second = vertices[1].toInt();
      if ( !checkVertexId( edge.first ) ) return false;
      if ( !checkVertexId( edge.second ) ) return false;
      edges.push_back( edge );
    }
    else
    {
      QgsMeshFace face;
      for ( int j = 0; j < vertices.size(); ++j )
      {
        int vertex_id = vertices[j].toInt();
        if ( !checkVertexId( vertex_id ) ) return false;
        face.push_back( vertex_id );
      }
      faces.push_back( face );
    }
  }

  mFaces = faces;
  mEdges = edges;

  if ( mFaces.size() > 0 && mEdges.size() > 0 )
  {
    setError( QgsError( tr( "Invalid mesh definition, unable to read mesh with both edges and faces" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  return true;
}


bool QgsMeshMemoryDataProvider::splitDatasetSections( const QString &uri, QgsMeshMemoryDatasetGroup &datasetGroup )
{
  const QStringList sections = uri.split( QStringLiteral( "---" ), QString::SkipEmptyParts );

  bool success = sections.size() > 2;
  if ( !success )
  {
    setError( QgsError( tr( "Invalid dataset definition, does not contain 3+ sections" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
  }

  if ( success )
    success = setDatasetGroupType( sections[0], datasetGroup );
  if ( success )
    success = addDatasetGroupMetadata( sections[1], datasetGroup );

  for ( int i = 2; i < sections.size(); ++i )
  {
    if ( !success )
      break;
    std::shared_ptr<QgsMeshMemoryDataset> dataset = std::make_shared<QgsMeshMemoryDataset>();
    success = addDatasetValues( sections[i], dataset, datasetGroup.isScalar() );
    if ( success )
      success = checkDatasetValidity( dataset, datasetGroup.dataType() );
    if ( success )
      datasetGroup.memoryDatasets.push_back( dataset );
  }

  return success;
}

bool QgsMeshMemoryDataProvider::setDatasetGroupType( const QString &def, QgsMeshMemoryDatasetGroup &datasetGroup )
{
  const QStringList types = def.split( ' ', QString::SkipEmptyParts );

  if ( types.size() != 3 )
  {
    setError( QgsError( tr( "Invalid type definition, must be Vertex/Edge/Face Vector/Scalar Name" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  QgsMeshDatasetGroupMetadata::DataType type;
  if ( 0 == QString::compare( types[0].trimmed(), QStringLiteral( "vertex" ), Qt::CaseInsensitive ) )
    type = QgsMeshDatasetGroupMetadata::DataOnVertices;
  else if ( 0 == QString::compare( types[0].trimmed(), QStringLiteral( "edge" ), Qt::CaseInsensitive ) )
    type = QgsMeshDatasetGroupMetadata::DataOnEdges;
  else
    type = QgsMeshDatasetGroupMetadata::DataOnFaces;

  datasetGroup.setDataType( type );
  datasetGroup.setIsScalar( 0 == QString::compare( types[1].trimmed(), QStringLiteral( "scalar" ), Qt::CaseInsensitive ) );
  datasetGroup.setName( types[2].trimmed() );

  return true;
}

bool QgsMeshMemoryDataProvider::addDatasetGroupMetadata( const QString &def, QgsMeshMemoryDatasetGroup &datasetGroup )
{
  const QStringList metadataLines = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < metadataLines.size(); ++i )
  {
    const QStringList keyVal = metadataLines[i].split( ':', QString::SkipEmptyParts );
    if ( keyVal.size() != 2 )
    {
      setError( QgsError( tr( "Invalid dataset definition, dataset metadata does not contain key: value" ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
      return false;
    }

    datasetGroup.addExtraMetadata( keyVal.at( 0 ).trimmed(), keyVal.at( 1 ).trimmed() );
  }
  return true;
}

bool QgsMeshMemoryDataProvider::addDatasetValues( const QString &def, std::shared_ptr<QgsMeshMemoryDataset> &dataset, bool isScalar )
{
  const QStringList valuesLines = def.split( '\n', QString::SkipEmptyParts );
  // first line is time
  if ( valuesLines.size() < 2 )
  {
    setError( QgsError( tr( "Invalid dataset definition, must contain at least 1 line (time)" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  dataset->time = valuesLines[0].toDouble();

  for ( int i = 1; i < valuesLines.size(); ++i )
  {
    const QStringList values = valuesLines[i].split( ',', QString::SkipEmptyParts );
    QgsMeshDatasetValue point;

    if ( isScalar )
    {
      if ( values.size() != 1 )
      {
        setError( QgsError( tr( "Invalid dataset definition, dataset scalar values must be x" ),
                            QStringLiteral( "Mesh Memory Provider" ) ) );
        return false;
      }
      else
      {
        point.setX( values[0].toDouble() );
      }
    }
    else
    {
      if ( values.size() < 2 )
      {
        setError( QgsError( tr( "Invalid dataset definition, dataset vector values must be x, y" ),
                            QStringLiteral( "Mesh Memory Provider" ) ) );
        return false;
      }
      else
      {
        point.setX( values[0].toDouble() );
        point.setY( values[1].toDouble() );
      }
    }

    dataset->values.push_back( point );
  }
  return true;
}

bool QgsMeshMemoryDataProvider::checkDatasetValidity( std::shared_ptr<QgsMeshMemoryDataset> &dataset, QgsMeshDatasetGroupMetadata::DataType dataType )
{
  bool valid = true;

  if ( dataType == QgsMeshDatasetGroupMetadata::DataOnVertices )
  {
    if ( dataset->values.count() != vertexCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on vertices has {} values, but mesh {}" ).arg( dataset->values.count(), vertexCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }
  else if ( dataType == QgsMeshDatasetGroupMetadata::DataOnFaces )
  {
    // on faces
    if ( dataset->values.count() != faceCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on faces has {} values, but mesh {}" ).arg( dataset->values.count(), faceCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }
  else if ( dataType == QgsMeshDatasetGroupMetadata::DataOnEdges )
  {
    // on edges
    if ( dataset->values.count() != edgeCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on edges has {} values, but mesh {}" ).arg( dataset->values.count(), edgeCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }
  dataset->valid = valid;
  return valid;
}

bool QgsMeshMemoryDataProvider::checkVertexId( int vertexIndex )
{
  if ( vertexIndex < 0 )
  {
    setError( QgsError( tr( "Invalid mesh definition, vertex index must be positive value" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }
  if ( mVertices.size() <= vertexIndex )
  {
    setError( QgsError( tr( "Invalid mesh definition, missing vertex id defined in face" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  return true;
}

void QgsMeshMemoryDataProvider::addGroupToTemporalCapabilities( int groupIndex, const QgsMeshMemoryDatasetGroup &group )
{
  QgsMeshDataProviderTemporalCapabilities *tempCap = temporalCapabilities();
  if ( !tempCap )
    return;

  if ( group.datasetCount() > 1 ) //non temporal dataset groups (count=1) have no time in the capabilities
  {
    for ( int i = 0; i < group.memoryDatasets.count(); ++i )
      if ( group.memoryDatasets.at( i ) )
        tempCap->addDatasetTime( groupIndex, group.memoryDatasets.at( i )->time );
  }

}

int QgsMeshMemoryDataProvider::vertexCount() const
{
  return mVertices.size();
}

int QgsMeshMemoryDataProvider::faceCount() const
{
  return mFaces.size();
}

int QgsMeshMemoryDataProvider::edgeCount() const
{
  return mEdges.size();
}

void QgsMeshMemoryDataProvider::populateMesh( QgsMesh *mesh ) const
{
  if ( mesh )
  {
    mesh->faces = mFaces;
    mesh->vertices = mVertices;
    mesh->edges = mEdges;
  }
}

QgsRectangle QgsMeshMemoryDataProvider::extent() const
{
  return calculateExtent( );
}

bool QgsMeshMemoryDataProvider::addDataset( const QString &uri )
{
  QgsMeshMemoryDatasetGroup group;

  bool valid = false;
  if ( mIsValid )
  {
    valid = splitDatasetSections( uri, group );
  }
  else
  {
    setError( QgsError( tr( "Unable to add dataset group to invalid mesh" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
  }

  group.calculateStatistic();
  mDatasetGroups.push_back( group );
  addGroupToTemporalCapabilities( mDatasetGroups.count() - 1, group );

  if ( valid )
  {
    if ( !mExtraDatasetUris.contains( uri ) )
      mExtraDatasetUris << uri;
    temporalCapabilities()->setHasTemporalCapabilities( true );
    emit datasetGroupsAdded( 1 );
    emit dataChanged();
  }

  return valid;
}

QStringList QgsMeshMemoryDataProvider::extraDatasets() const
{
  return mExtraDatasetUris;
}

int QgsMeshMemoryDataProvider::datasetGroupCount() const
{
  return mDatasetGroups.count();
}

int QgsMeshMemoryDataProvider::datasetCount( int groupIndex ) const
{
  if ( ( groupIndex >= 0 ) && ( groupIndex < datasetGroupCount() ) )
    return mDatasetGroups[groupIndex].memoryDatasets.count();
  else
    return 0;
}

QgsMeshDatasetGroupMetadata QgsMeshMemoryDataProvider::datasetGroupMetadata( int groupIndex ) const
{
  if ( ( groupIndex >= 0 ) && ( groupIndex < datasetGroupCount() ) )
  {
    return mDatasetGroups[groupIndex].groupMetadata();
  }
  else
  {
    return QgsMeshDatasetGroupMetadata();
  }
}



QgsMeshDatasetMetadata QgsMeshMemoryDataProvider::datasetMetadata( QgsMeshDatasetIndex index ) const
{
  if ( ( index.group() >= 0 ) && ( index.group() < datasetGroupCount() ) &&
       ( index.dataset() >= 0 ) && ( index.dataset() < datasetCount( index.group() ) )
     )
  {
    const QgsMeshMemoryDatasetGroup &grp = mDatasetGroups.at( index.group() );
    QgsMeshDatasetMetadata metadata(
      grp.memoryDatasets[index.dataset()]->time,
      grp.memoryDatasets[index.dataset()]->valid,
      grp.memoryDatasets[index.dataset()]->minimum,
      grp.memoryDatasets[index.dataset()]->maximum,
      0
    );
    return metadata;
  }
  else
  {
    return QgsMeshDatasetMetadata();
  }
}

QgsMeshDatasetValue QgsMeshMemoryDataProvider::datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const
{
  if ( ( index.group() >= 0 ) && ( index.group() < datasetGroupCount() ) &&
       ( index.dataset() >= 0 ) && ( index.dataset() < datasetCount( index.group() ) ) &&
       ( valueIndex >= 0 ) && ( valueIndex < mDatasetGroups[index.group()].memoryDatasets[index.dataset()]->values.count() ) )
  {
    return mDatasetGroups[index.group()].memoryDatasets[index.dataset()]->values[valueIndex];
  }
  else
  {
    return QgsMeshDatasetValue();
  }
}

QgsMeshDataBlock QgsMeshMemoryDataProvider::datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const
{
  if ( ( index.group() >= 0 ) && ( index.group() < datasetGroupCount() ) )
  {
    const QgsMeshMemoryDatasetGroup group = mDatasetGroups[index.group()];
    bool isScalar = group.isScalar();
    if ( ( index.dataset() >= 0 ) && ( index.dataset() < group.memoryDatasets.size() ) )
    {
      return group.memoryDatasets[index.dataset()]->datasetValues( isScalar, valueIndex, count );
    }
    else
    {
      return QgsMeshDataBlock();
    }
  }
  else
  {
    return QgsMeshDataBlock();
  }
}

QgsMesh3dDataBlock QgsMeshMemoryDataProvider::dataset3dValues( QgsMeshDatasetIndex, int, int ) const
{
  // 3d stacked meshes are not supported by memory provider
  return QgsMesh3dDataBlock();
}



bool QgsMeshMemoryDataProvider::isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const
{
  if ( mDatasetGroups[index.group()].memoryDatasets[index.dataset()]->active.isEmpty() )
    return true;
  else
    return mDatasetGroups[index.group()].memoryDatasets[index.dataset()]->active[faceIndex];
}

QgsMeshDataBlock QgsMeshMemoryDataProvider::areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  if ( ( index.group() >= 0 ) && ( index.group() < datasetGroupCount() ) )
  {
    const QgsMeshMemoryDatasetGroup group = mDatasetGroups[index.group()];
    if ( ( index.dataset() >= 0 ) && ( index.dataset() < group.memoryDatasets.size() ) )
    {
      return group.memoryDatasets[index.dataset()]->areFacesActive( faceIndex, count );
    }
    else
    {
      return QgsMeshDataBlock();
    }
  }
  else
  {
    return QgsMeshDataBlock();
  }
}



bool QgsMeshMemoryDataProvider::persistDatasetGroup( const QString &outputFilePath,
    const QString &outputDriver,
    const QgsMeshDatasetGroupMetadata &meta,
    const QVector<QgsMeshDataBlock> &datasetValues,
    const QVector<QgsMeshDataBlock> &datasetActive,
    const QVector<double> &times )
{
  Q_UNUSED( outputFilePath )
  Q_UNUSED( outputDriver )
  Q_UNUSED( meta )
  Q_UNUSED( datasetValues )
  Q_UNUSED( datasetActive )
  Q_UNUSED( times )
  return true; // not implemented/supported
}

bool QgsMeshMemoryDataProvider::persistDatasetGroup( const QString &outputFilePath,
    const QString &outputDriver,
    QgsMeshDatasetSourceInterface *source,
    int datasetGroupIndex )
{
  Q_UNUSED( outputFilePath )
  Q_UNUSED( outputDriver )
  Q_UNUSED( source )
  Q_UNUSED( datasetGroupIndex )
  return true; // not implemented/supported
}

QgsRectangle QgsMeshMemoryDataProvider::calculateExtent() const
{
  QgsRectangle rec;
  rec.setMinimal();
  for ( const QgsMeshVertex &v : mVertices )
  {
    rec.setXMinimum( std::min( rec.xMinimum(), v.x() ) );
    rec.setYMinimum( std::min( rec.yMinimum(), v.y() ) );
    rec.setXMaximum( std::max( rec.xMaximum(), v.x() ) );
    rec.setYMaximum( std::max( rec.yMaximum(), v.y() ) );
  }
  return rec;
}



///@endcond
