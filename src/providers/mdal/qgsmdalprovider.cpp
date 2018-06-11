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

#include <string>

#include "qgsmdalprovider.h"

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "mdal" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "MDAL provider" );

bool QgsMdalProvider::isValid() const
{
  return mMeshH != nullptr;
}

QString QgsMdalProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMdalProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsMdalProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QgsMdalProvider::QgsMdalProvider( const QString &uri, const ProviderOptions &options )
  : QgsMeshDataProvider( uri, options )
{
  QByteArray curi = uri.toAscii();
  mMeshH = MDAL_LoadMesh( curi.constData() );
  refreshDatasets();
}

QgsMdalProvider::~QgsMdalProvider()
{
  if ( mMeshH )
    MDAL_CloseMesh( mMeshH );
}

int QgsMdalProvider::vertexCount() const
{
  if ( mMeshH )
    return MDAL_M_vertexCount( mMeshH );
  else
    return 0;
}

int QgsMdalProvider::faceCount() const
{
  if ( mMeshH )
    return MDAL_M_faceCount( mMeshH );
  else
    return 0;
}

QgsMeshVertex QgsMdalProvider::vertex( int index ) const
{
  Q_ASSERT( index < vertexCount() );
  double x = MDAL_M_vertexXCoordinatesAt( mMeshH, index );
  double y = MDAL_M_vertexYCoordinatesAt( mMeshH, index );
  QgsMeshVertex vertex( x, y );
  return vertex;
}

QgsMeshFace QgsMdalProvider::face( int index ) const
{
  Q_ASSERT( index < faceCount() );
  QgsMeshFace face;
  int n_face_vertices = MDAL_M_faceVerticesCountAt( mMeshH, index );
  for ( int j = 0; j < n_face_vertices; ++j )
  {
    int vertex_index = MDAL_M_faceVerticesIndexAt( mMeshH, index, j );
    face.push_back( vertex_index );
  }
  return face;
}

/*----------------------------------------------------------------------------------------------*/

bool QgsMdalProvider::addDataset( const QString &uri )
{
  int datasetCount = mDatasets.count();

  std::string str = uri.toStdString();
  MDAL_M_LoadDatasets( mMeshH, str.c_str() );
  refreshDatasets();

  if ( datasetCount == mDatasets.count() )
  {
    return false;
  }
  else
  {
    emit dataChanged();
    return true; // Ok
  }
}

int QgsMdalProvider::datasetCount() const
{
  return MDAL_M_datasetCount( mMeshH );
}

QgsMeshDatasetMetadata QgsMdalProvider::datasetMetadata( int datasetIndex ) const
{
  if ( datasetIndex >= mDatasets.length() )
    return QgsMeshDatasetMetadata();

  if ( datasetIndex < 0 )
    return QgsMeshDatasetMetadata();

  DatasetH dataset = mDatasets[datasetIndex];

  bool isScalar = MDAL_D_hasScalarData( dataset );
  bool isValid = MDAL_D_isValid( dataset );
  bool isOnVertices = MDAL_D_isOnVertices( dataset );

  QMap<QString, QString> metadata;
  int n = MDAL_D_metadataCount( dataset );
  for ( int i = 0; i < n; ++i )
  {
    QString key = MDAL_D_metadataKey( dataset, i );
    QString value = MDAL_D_metadataValue( dataset, i );
    metadata[key] = value;
  }

  QgsMeshDatasetMetadata meta(
    isScalar,
    isValid,
    isOnVertices,
    metadata
  );

  return meta;
}

QgsMeshDatasetValue QgsMdalProvider::datasetValue( int datasetIndex, int valueIndex ) const
{
  if ( datasetIndex >= mDatasets.length() )
    return QgsMeshDatasetValue();

  if ( datasetIndex < 0 )
    return QgsMeshDatasetValue();

  DatasetH dataset = mDatasets[datasetIndex];
  QgsMeshDatasetValue val;

  if ( MDAL_D_hasScalarData( dataset ) )
  {
    val.setX( MDAL_D_value( dataset, valueIndex ) );
  }
  else
  {
    val.setX( MDAL_D_valueX( dataset, valueIndex ) );
    val.setY( MDAL_D_valueY( dataset, valueIndex ) );
  }

  return val;
}

void QgsMdalProvider::refreshDatasets()
{
  int n = MDAL_M_datasetCount( mMeshH );
  mDatasets.resize( 0 ); // keeps allocated space - potentially avoids reallocation
  mDatasets.reserve( n );
  for ( int i = 0; i < n; ++i )
  {
    DatasetH dataset = MDAL_M_dataset( mMeshH, i );
    mDatasets.push_back( dataset );
  }
}

/*----------------------------------------------------------------------------------------------*/

/**
 * Class factory to return a pointer to a newly created
 * QgsGdalProvider object
 */
QGISEXTERN QgsMdalProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsMdalProvider( *uri, options );
}

/**
 * Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN void cleanupProvider()
{
}

