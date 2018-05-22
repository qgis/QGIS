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

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "mesh_memory" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "Mesh memory provider" );

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

QgsMeshMemoryDataProvider::QgsMeshMemoryDataProvider( const QString &uri )
  : QgsMeshDataProvider( uri )
{
  mIsValid = splitMeshSections( uri );
}

QgsMeshMemoryDataProvider::~QgsMeshMemoryDataProvider()
{
}

QString QgsMeshMemoryDataProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMeshMemoryDataProvider::providerDescription()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsMeshMemoryDataProvider *QgsMeshMemoryDataProvider::createProvider( const QString &uri )
{
  return new QgsMeshMemoryDataProvider( uri );
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
    return addMeshFaces( sections[1] );
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

bool QgsMeshMemoryDataProvider::addMeshFaces( const QString &def )
{
  QVector<QgsMeshFace> faces;

  const QStringList facesVertices = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < facesVertices.size(); ++i )
  {
    const QStringList vertices = facesVertices[i].split( ',', QString::SkipEmptyParts );
    if ( vertices.size() < 3 )
    {
      setError( QgsError( tr( "Invalid mesh definition, face must contain at least 3 vertices" ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
      return false;
    }
    QgsMeshFace face;
    for ( int j = 0; j < vertices.size(); ++j )
    {
      int vertex_id = vertices[j].toInt();
      if ( vertex_id < 0 )
      {
        setError( QgsError( tr( "Invalid mesh definition, vertex index must be positive value" ),
                            QStringLiteral( "Mesh Memory Provider" ) ) );
        return false;
      }
      if ( mVertices.size() < vertex_id )
      {
        setError( QgsError( tr( "Invalid mesh definition, missing vertex id defined in face" ),
                            QStringLiteral( "Mesh Memory Provider" ) ) );
        return false;
      }

      face.push_back( vertex_id );
    }
    faces.push_back( face );
  }

  mFaces = faces;
  return true;
}


bool QgsMeshMemoryDataProvider::splitDatasetSections( const QString &uri, QgsMeshMemoryDataset &dataset )
{
  const QStringList sections = uri.split( QStringLiteral( "---" ), QString::SkipEmptyParts );

  bool success = sections.size() == 3;
  if ( !success )
  {
    setError( QgsError( tr( "Invalid dataset definition, does not contain 3 sections" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
  }

  if ( success )
    success = setDatasetType( sections[0], dataset );
  if ( success )
    success = addDatasetMetadata( sections[1], dataset );
  if ( success )
    success = addDatasetValues( sections[2], dataset );
  if ( success )
    success = checkDatasetValidity( dataset );

  return success;
}

bool QgsMeshMemoryDataProvider::setDatasetType( const QString &def, QgsMeshMemoryDataset &dataset )
{
  const QStringList types = def.split( ' ', QString::SkipEmptyParts );

  if ( types.size() != 2 )
  {
    setError( QgsError( tr( "Invalid type definition, must be Vertex/Face Vector/Scalar" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  dataset.isOnVertices = 0 == QString::compare( types[0].trimmed(), QStringLiteral( "vertex" ), Qt::CaseInsensitive );
  dataset.isScalar = 0 == QString::compare( types[1].trimmed(), QStringLiteral( "scalar" ), Qt::CaseInsensitive );

  return true;
}

bool QgsMeshMemoryDataProvider::addDatasetMetadata( const QString &def, QgsMeshMemoryDataset &dataset )
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

    dataset.metadata.insert( keyVal.at( 0 ).trimmed(), keyVal.at( 1 ).trimmed() );
  }
  return true;
}

bool QgsMeshMemoryDataProvider::addDatasetValues( const QString &def, QgsMeshMemoryDataset &dataset )
{
  const QStringList valuesLines = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < valuesLines.size(); ++i )
  {
    const QStringList values = valuesLines[i].split( ',', QString::SkipEmptyParts );
    QgsMeshDatasetValue point;

    if ( dataset.isScalar )
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

    dataset.values.push_back( point );
  }
  return true;
}

bool QgsMeshMemoryDataProvider::checkDatasetValidity( QgsMeshMemoryDataset &dataset )
{
  bool valid = true;

  if ( dataset.isOnVertices )
  {
    if ( dataset.values.count() != vertexCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on vertices has {} values, but mesh {}" ).arg( dataset.values.count(), vertexCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }
  else
  {
    // on faces
    if ( dataset.values.count() != faceCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on faces has {} values, but mesh {}" ).arg( dataset.values.count(), faceCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }

  return valid;
}

int QgsMeshMemoryDataProvider::vertexCount() const
{
  return mVertices.size();
}

int QgsMeshMemoryDataProvider::faceCount() const
{
  return mFaces.size();
}

QgsMeshVertex QgsMeshMemoryDataProvider::vertex( int index ) const
{
  Q_ASSERT( vertexCount() > index );
  return mVertices[index];
}

QgsMeshFace QgsMeshMemoryDataProvider::face( int index ) const
{
  Q_ASSERT( faceCount() > index );
  return mFaces[index];
}

bool QgsMeshMemoryDataProvider::addDataset( const QString &uri )
{
  QgsMeshMemoryDataset ds;

  if ( mIsValid )
  {
    ds.valid = splitDatasetSections( uri, ds );
  }
  else
  {
    setError( QgsError( tr( "Unable to add dataset to invalid mesh" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
  }

  mDatasets.push_back( ds );

  return ds.valid;
}

int QgsMeshMemoryDataProvider::datasetCount() const
{
  return mDatasets.count();
}

QgsMeshDatasetMetadata QgsMeshMemoryDataProvider::datasetMetadata( int index ) const
{
  if ( ( index >= 0 ) && ( index < datasetCount() ) )
  {
    QgsMeshDatasetMetadata metadata(
      mDatasets[index].isScalar,
      mDatasets[index].valid,
      mDatasets[index].isOnVertices,
      mDatasets[index].metadata
    );
    return metadata;
  }
  else
  {
    return QgsMeshDatasetMetadata();
  }
}

QgsMeshDatasetValue QgsMeshMemoryDataProvider::datasetValue( int datasetIndex, int valueIndex ) const
{
  if ( ( datasetIndex >= 0 ) &&
       ( datasetIndex < datasetCount() )  &&
       ( valueIndex >= 0 ) &&
       ( valueIndex < mDatasets[datasetIndex].values.count() ) )
  {
    return mDatasets[datasetIndex].values[valueIndex];
  }
  else
  {
    return QgsMeshDatasetValue();
  }
}


///@endcond
