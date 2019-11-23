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
  mIsValid = splitMeshSections( uri );
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
    success = addDatasetValues( sections[i], dataset, datasetGroup.isScalar );
    if ( success )
      success = checkDatasetValidity( dataset, datasetGroup.type == QgsMeshDatasetGroupMetadata::DataOnVertices );
    if ( success )
      datasetGroup.datasets.push_back( dataset );
  }

  return success;
}

bool QgsMeshMemoryDataProvider::setDatasetGroupType( const QString &def, QgsMeshMemoryDatasetGroup &datasetGroup )
{
  const QStringList types = def.split( ' ', QString::SkipEmptyParts );

  if ( types.size() != 3 )
  {
    setError( QgsError( tr( "Invalid type definition, must be Vertex/Face Vector/Scalar Name" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  if ( 0 == QString::compare( types[0].trimmed(), QStringLiteral( "vertex" ), Qt::CaseInsensitive ) )
    datasetGroup.type = QgsMeshDatasetGroupMetadata::DataOnVertices;
  else
    datasetGroup.type = QgsMeshDatasetGroupMetadata::DataOnFaces;

  datasetGroup.isScalar = 0 == QString::compare( types[1].trimmed(), QStringLiteral( "scalar" ), Qt::CaseInsensitive );
  datasetGroup.name = types[2].trimmed();

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

    datasetGroup.metadata.insert( keyVal.at( 0 ).trimmed(), keyVal.at( 1 ).trimmed() );
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

bool QgsMeshMemoryDataProvider::checkDatasetValidity( std::shared_ptr<QgsMeshMemoryDataset> &dataset, bool isOnVertices )
{
  bool valid = true;

  if ( isOnVertices )
  {
    if ( dataset->values.count() != vertexCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on vertices has {} values, but mesh {}" ).arg( dataset->values.count(), vertexCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }
  else
  {
    // on faces
    if ( dataset->values.count() != faceCount() )
    {
      valid = false;
      setError( QgsError( tr( "Dataset defined on faces has {} values, but mesh {}" ).arg( dataset->values.count(), faceCount() ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
    }
  }

  dataset->valid = valid;
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

void QgsMeshMemoryDataProvider::populateMesh( QgsMesh *mesh ) const
{
  if ( mesh )
  {
    mesh->faces = mFaces;
    mesh->vertices = mVertices;
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

  calculateMinMaxForDatasetGroup( group );
  mDatasetGroups.push_back( group );

  if ( valid )
  {
    mExtraDatasetUris << uri;
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
    return mDatasetGroups[groupIndex].datasets.count();
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

QgsMeshDatasetGroupMetadata QgsMeshMemoryDatasetGroup::groupMetadata() const
{
  return QgsMeshDatasetGroupMetadata(
           name,
           isScalar,
           type == QgsMeshDatasetGroupMetadata::DataOnVertices,
           minimum,
           maximum,
           metadata
         );
}

int QgsMeshMemoryDatasetGroup::datasetCount() const
{
  return datasets.size();
}

void QgsMeshMemoryDatasetGroup::addDataset( std::shared_ptr<QgsMeshMemoryDataset> dataset )
{
  datasets.push_back( dataset );
}

void QgsMeshMemoryDatasetGroup::clearDatasets()
{
  datasets.clear();
}

std::shared_ptr<const QgsMeshMemoryDataset> QgsMeshMemoryDatasetGroup::constDataset( int index ) const
{
  return datasets[index];
}

QgsMeshDatasetMetadata QgsMeshMemoryDataProvider::datasetMetadata( QgsMeshDatasetIndex index ) const
{
  if ( ( index.group() >= 0 ) && ( index.group() < datasetGroupCount() ) &&
       ( index.dataset() >= 0 ) && ( index.dataset() < datasetCount( index.group() ) )
     )
  {
    const QgsMeshMemoryDatasetGroup &grp = mDatasetGroups.at( index.group() );
    QgsMeshDatasetMetadata metadata(
      grp.datasets[index.dataset()]->time,
      grp.datasets[index.dataset()]->valid,
      grp.datasets[index.dataset()]->minimum,
      grp.datasets[index.dataset()]->maximum
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
       ( valueIndex >= 0 ) && ( valueIndex < mDatasetGroups[index.group()].datasets[index.dataset()]->values.count() ) )
  {
    return mDatasetGroups[index.group()].datasets[index.dataset()]->values[valueIndex];
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
    bool isScalar = group.isScalar;
    if ( ( index.dataset() >= 0 ) && ( index.dataset() < group.datasets.size() ) )
    {
      return group.datasets[index.dataset()]->datasetValues( isScalar, valueIndex, count );
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

QgsMeshDataBlock QgsMeshMemoryDataset::datasetValues( bool isScalar, int valueIndex, int count ) const
{
  QgsMeshDataBlock ret( isScalar ? QgsMeshDataBlock::ScalarDouble : QgsMeshDataBlock::Vector2DDouble, count );
  double *buf = static_cast<double *>( ret.buffer() );

  for ( int i = 0; i < count; ++i )
  {
    int idx = valueIndex + i;
    if ( ( idx < 0 ) || ( idx >= values.size() ) )
      return ret;

    QgsMeshDatasetValue val = values[ valueIndex + i ];
    if ( isScalar )
      buf[i] = val.x();
    else
    {
      buf[2 * i] = val.x();
      buf[2 * i + 1] = val.y();
    }
  }
  return ret;
}

bool QgsMeshMemoryDataProvider::isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const
{
  if ( mDatasetGroups[index.group()].datasets[index.dataset()]->active.isEmpty() )
    return true;
  else
    return mDatasetGroups[index.group()].datasets[index.dataset()]->active[faceIndex];
}

QgsMeshDataBlock QgsMeshMemoryDataProvider::areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  if ( ( index.group() >= 0 ) && ( index.group() < datasetGroupCount() ) )
  {
    const QgsMeshMemoryDatasetGroup group = mDatasetGroups[index.group()];
    if ( ( index.dataset() >= 0 ) && ( index.dataset() < group.datasets.size() ) )
    {
      return group.datasets[index.dataset()]->areFacesActive( faceIndex, count );
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

QgsMeshDataBlock QgsMeshMemoryDataset::areFacesActive( int faceIndex, int count ) const
{
  QgsMeshDataBlock ret( QgsMeshDataBlock::ActiveFlagInteger, count );
  if ( active.isEmpty() ||
       ( faceIndex < 0 ) ||
       ( faceIndex + count > active.size() )
     )
    memset( ret.buffer(), 1, static_cast<size_t>( count ) * sizeof( int ) );
  else
    memcpy( ret.buffer(),
            active.data() + faceIndex,
            static_cast<size_t>( count ) * sizeof( int ) );

  return ret;
}

bool QgsMeshMemoryDataProvider::persistDatasetGroup( const QString &path,
    const QgsMeshDatasetGroupMetadata &meta,
    const QVector<QgsMeshDataBlock> &datasetValues,
    const QVector<QgsMeshDataBlock> &datasetActive,
    const QVector<double> &times )
{
  Q_UNUSED( path )
  Q_UNUSED( meta )
  Q_UNUSED( datasetValues )
  Q_UNUSED( datasetActive )
  Q_UNUSED( times )
  return true; // not implemented/supported
}

void QgsMeshMemoryDataProvider::calculateMinMaxForDatasetGroup( QgsMeshMemoryDatasetGroup &grp ) const
{
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::min();

  int count = grp.datasets.size();
  for ( int i = 0; i < count; ++i )
  {
    calculateMinMaxForDataset( grp.datasets[i] );
    min = std::min( min, grp.datasets[i]->minimum );
    max = std::max( max, grp.datasets[i]->maximum );
  }

  grp.minimum = min;
  grp.maximum = max;
}

void QgsMeshMemoryDataProvider::calculateMinMaxForDataset( std::shared_ptr<QgsMeshMemoryDataset> &dataset ) const
{
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::min();

  if ( !dataset->valid )
  {
    return;
  }

  bool firstIteration = true;
  for ( int i = 0; i < dataset->values.size(); ++i )
  {
    double v = dataset->values[i].scalar();

    if ( std::isnan( v ) )
      continue;
    if ( firstIteration )
    {
      firstIteration = false;
      min = v;
      max = v;
    }
    else
    {
      if ( v < min )
      {
        min = v;
      }
      if ( v > max )
      {
        max = v;
      }
    }
  }

  dataset->minimum = min;
  dataset->maximum = max;
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


QgsMeshMemoryDatasetGroup::QgsMeshMemoryDatasetGroup( const QString &nm ):
  name( nm )
{
}

QgsMeshMemoryDatasetGroup::QgsMeshMemoryDatasetGroup() = default;
QgsMeshMemoryDataset::QgsMeshMemoryDataset() = default;

///@endcond
