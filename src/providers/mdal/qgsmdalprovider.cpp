/***************************************************************************
                         qgsmdalprovider.cpp
                         -------------------
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
#include "qgstriangularmesh.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsmeshdataprovidertemporalcapabilities.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QIcon>
#include <mutex>

const QString QgsMdalProvider::MDAL_PROVIDER_KEY = QStringLiteral( "mdal" );
const QString QgsMdalProvider::MDAL_PROVIDER_DESCRIPTION = QStringLiteral( "MDAL provider" );

bool QgsMdalProvider::isValid() const
{
  return mMeshH != nullptr;
}

QString QgsMdalProvider::name() const
{
  return MDAL_PROVIDER_KEY;
}

QString QgsMdalProvider::description() const
{
  return MDAL_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsMdalProvider::crs() const
{
  return mCrs;
}

QStringList QgsMdalProvider::subLayers() const
{
  return mSubLayersUris;
}

QgsMdalProvider::QgsMdalProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsMeshDataProvider( uri, options, flags )
{
  temporalCapabilities()->setTemporalUnit( QgsUnitTypes::TemporalHours );
  QByteArray curi = dataSourceUri().toUtf8();

  if ( uri.contains( "\":" ) ) //contains a mesh name, so can be directly loaded;
    loadData();
  else
  {
    QStringList meshNames = QString( MDAL_MeshNames( curi ) ).split( QStringLiteral( ";;" ) );

    if ( meshNames.count() == 1 ) //only one uri is present, can be directly loaded
      loadData();
    else
      mSubLayersUris = meshNames; //several meshes are present, the layer is invalid and has sublayers
  }
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

int QgsMdalProvider::edgeCount() const
{
  if ( mMeshH )
    return MDAL_M_edgeCount( mMeshH );
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

void QgsMdalProvider::populateMesh( QgsMesh *mesh ) const
{
  if ( mesh )
  {
    mesh->faces = faces();
    mesh->vertices = vertices();
    mesh->edges = edges();
  }
}

QVector<QgsMeshVertex> QgsMdalProvider::vertices( ) const
{
  const int bufferSize = std::min( vertexCount(), 1000 );
  QVector<QgsMeshVertex> ret( vertexCount() );
  QVector<double> buffer( bufferSize * 3 );
  MDAL_MeshVertexIteratorH it = MDAL_M_vertexIterator( mMeshH );
  int vertexIndex = 0;
  while ( vertexIndex < vertexCount() )
  {
    int verticesRead = MDAL_VI_next( it, bufferSize, buffer.data() );
    if ( verticesRead == 0 )
      break;
    for ( int i = 0; i < verticesRead; i++ )
    {
      QgsMeshVertex vertex(
        buffer[3 * i],
        buffer[3 * i + 1],
        buffer[3 * i + 2]
      );
      ret[vertexIndex + i] = vertex;
    }
    vertexIndex += verticesRead;
  }
  MDAL_VI_close( it );
  return ret;
}

QVector<QgsMeshEdge> QgsMdalProvider::edges( ) const
{
  const int edgesCount = edgeCount();
  const int bufferSize = std::min( edgesCount, 1000 );
  QVector<QgsMeshEdge> ret( edgesCount );
  QVector<int> startBuffer( bufferSize );
  QVector<int> endBuffer( bufferSize );
  MDAL_MeshEdgeIteratorH it = MDAL_M_edgeIterator( mMeshH );
  int edgeIndex = 0;
  while ( edgeIndex < edgesCount )
  {
    int edgesRead = MDAL_EI_next( it, bufferSize, startBuffer.data(), endBuffer.data() );
    if ( edgesRead == 0 )
      break;
    for ( int i = 0; i < edgesRead; i++ )
    {
      QgsMeshEdge edge(
        startBuffer[i],
        endBuffer[i]
      );
      ret[edgeIndex + i] = edge;
    }
    edgeIndex += edgesRead;
  }
  MDAL_EI_close( it );
  return ret;
}

QVector<QgsMeshFace> QgsMdalProvider::faces( ) const
{
  const int faceOffsetsBufferLen = std::min( faceCount(), 1000 );
  const int vertexIndicesBufferLen = faceOffsetsBufferLen * 4; // most usually we have quads
  int facesCount = faceCount();

  QVector<QgsMeshFace> ret( facesCount );
  QVector<int> faceOffsetsBuffer( faceOffsetsBufferLen );
  QVector<int> vertexIndicesBuffer( vertexIndicesBufferLen );

  MDAL_MeshFaceIteratorH it = MDAL_M_faceIterator( mMeshH );
  int faceIndex = 0;
  while ( faceIndex < facesCount )
  {
    int facesRead = MDAL_FI_next( it,
                                  faceOffsetsBufferLen,
                                  faceOffsetsBuffer.data(),
                                  vertexIndicesBufferLen,
                                  vertexIndicesBuffer.data() );
    if ( facesRead == 0 )
      break;

    for ( int i = 0; i < facesRead; i++ )
    {
      QgsMeshFace face;
      int startIndex = 0;
      if ( i > 0 )
        startIndex = faceOffsetsBuffer[ i - 1 ];
      int endIndex = faceOffsetsBuffer[ i ];

      for ( int j = startIndex; j < endIndex; ++j )
      {
        int vertexIndex = vertexIndicesBuffer[j];
        face.push_back( vertexIndex );
      }
      ret[faceIndex + i] = face;
    }
    faceIndex += facesRead;
  }
  MDAL_FI_close( it );
  return ret;
}

QgsRectangle QgsMdalProvider::extent() const
{
  double xMin, yMin, xMax, yMax;
  MDAL_M_extent( mMeshH, &xMin, &xMax, &yMin, &yMax );
  QgsRectangle ret( xMin, yMin, xMax, yMax );
  return ret;
}

int QgsMdalProvider::maximumVerticesCountPerFace() const
{
  return driverMetadata().maximumVerticesCountPerFace();
}

QgsMeshDriverMetadata QgsMdalProvider::driverMetadata() const
{
  if ( !mMeshH )
    return QgsMeshDriverMetadata();

  QString name = MDAL_M_driverName( mMeshH );
  MDAL_DriverH mdalDriver = MDAL_driverFromName( name.toStdString().c_str() );
  QString longName = MDAL_DR_longName( mdalDriver );
  QString writeDatasetSuffix = MDAL_DR_writeDatasetsSuffix( mdalDriver );
  QString writeMeshFrameSuffix = MDAL_DR_saveMeshSuffix( mdalDriver );
  int maxVerticesPerFace = MDAL_DR_faceVerticesMaximumCount( mdalDriver );

  QgsMeshDriverMetadata::MeshDriverCapabilities capabilities;
  bool hasSaveFaceDatasetsCapability = MDAL_DR_writeDatasetsCapability( mdalDriver, MDAL_DataLocation::DataOnFaces );
  if ( hasSaveFaceDatasetsCapability )
    capabilities |= QgsMeshDriverMetadata::CanWriteFaceDatasets;
  bool hasSaveVertexDatasetsCapability = MDAL_DR_writeDatasetsCapability( mdalDriver, MDAL_DataLocation::DataOnVertices );
  if ( hasSaveVertexDatasetsCapability )
    capabilities |= QgsMeshDriverMetadata::CanWriteVertexDatasets;
  bool hasSaveEdgeDatasetsCapability = MDAL_DR_writeDatasetsCapability( mdalDriver, MDAL_DataLocation::DataOnEdges );
  if ( hasSaveEdgeDatasetsCapability )
    capabilities |= QgsMeshDriverMetadata::CanWriteEdgeDatasets;
  bool hasMeshSaveCapability = MDAL_DR_saveMeshCapability( mdalDriver );
  if ( hasMeshSaveCapability )
    capabilities |= QgsMeshDriverMetadata::CanWriteMeshData;
  const QgsMeshDriverMetadata meta( name, longName, capabilities, writeDatasetSuffix, writeMeshFrameSuffix, maxVerticesPerFace );

  return meta;
}

bool QgsMdalProvider::persistDatasetGroup(
  const QString &outputFilePath,
  const QString &outputDriver,
  const QgsMeshDatasetGroupMetadata &meta,
  const QVector<QgsMeshDataBlock> &datasetValues,
  const QVector<QgsMeshDataBlock> &datasetActive,
  const QVector<double> &times
)
{
  if ( !mMeshH )
    return true;

  // Check that the input vectors have consistent size
  if ( times.size() != datasetValues.size() )
    return true;

  if ( !datasetActive.isEmpty() && ( times.size() != datasetActive.size() ) )
    return true;

  // Check that input data are for all values
  int valuesCount = meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ? vertexCount() : faceCount();
  for ( int i = 0; i < datasetValues.size(); ++i )
  {
    if ( datasetValues.at( i ).count() != valuesCount )
      return true;

    if ( !datasetActive.isEmpty() && ( datasetActive.at( i ).count() != faceCount() ) )
      return true;
  }

  if ( outputFilePath.isEmpty() )
    return true;

  MDAL_DriverH driver = MDAL_driverFromName( outputDriver.toStdString().c_str() );
  if ( !driver )
    return true;

  MDAL_DataLocation location = MDAL_DataLocation::DataInvalidLocation;
  switch ( meta.dataType() )
  {
    case QgsMeshDatasetGroupMetadata::DataOnFaces:
      location = MDAL_DataLocation::DataOnFaces;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVertices:
      location = MDAL_DataLocation::DataOnVertices;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnEdges:
      location = MDAL_DataLocation::DataOnEdges;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVolumes:
      location = MDAL_DataLocation::DataOnVolumes;
      break;
  }

  MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
                           mMeshH,
                           meta.name().toStdString().c_str(),
                           location,
                           meta.isScalar(),
                           driver,
                           outputFilePath.toStdString().c_str()
                         );
  if ( !g )
    return true;

  const auto extraOptions = meta.extraOptions();
  for ( auto it = extraOptions.cbegin(); it != extraOptions.cend(); ++it )
  {
    MDAL_G_setMetadata( g, it.key().toStdString().c_str(), it.value().toStdString().c_str() );
  }

  if ( meta.referenceTime().isValid() )
  {
    MDAL_G_setReferenceTime( g, meta.referenceTime().toString( Qt::ISODateWithMs ).toStdString().c_str() );
  }

  for ( int i = 0; i < datasetValues.size(); ++i )
  {
    const QVector<double> values = datasetValues.at( i ).values();
    QVector<int> active;
    if ( !datasetActive.isEmpty() )
      active = datasetActive.at( i ).active();

    MDAL_G_addDataset( g,
                       times.at( i ),
                       values.constData(),
                       active.isEmpty() ? nullptr : active.constData()
                     );
  }

  MDAL_G_closeEditMode( g );
  if ( MDAL_LastStatus() == 0 )
  {
    QgsMeshDatasetGroupMetadata meta = datasetGroupMetadata( datasetGroupCount() - 1 );
    QString newUri = meta.uri();
    if ( !mExtraDatasetUris.contains( newUri ) )
      mExtraDatasetUris << newUri;
    emit datasetGroupsAdded( 1 );
    emit dataChanged();
    return false;
  }
  else
    return true;
}

bool QgsMdalProvider::persistDatasetGroup( const QString &outputFilePath, const QString &outputDriver, QgsMeshDatasetSourceInterface *source, int datasetGroupIndex )
{
  if ( !mMeshH )
    return true;

  QgsMeshDatasetGroupMetadata meta = source->datasetGroupMetadata( datasetGroupIndex );
  int faceValueCount = faceCount();
  int valuesCount = meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ? vertexCount() : faceValueCount;
  int datasetCount = source->datasetCount( datasetGroupIndex );

  if ( outputFilePath.isEmpty() )
    return true;

  MDAL_DriverH driver = MDAL_driverFromName( outputDriver.toStdString().c_str() );
  if ( !driver )
    return true;

  MDAL_DataLocation location = MDAL_DataLocation::DataInvalidLocation;
  switch ( meta.dataType() )
  {
    case QgsMeshDatasetGroupMetadata::DataOnFaces:
      location = MDAL_DataLocation::DataOnFaces;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVertices:
      location = MDAL_DataLocation::DataOnVertices;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnEdges:
      location = MDAL_DataLocation::DataOnEdges;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVolumes:
      location = MDAL_DataLocation::DataOnVolumes;
      break;
  }

  MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
                           mMeshH,
                           meta.name().toStdString().c_str(),
                           location,
                           meta.isScalar(),
                           driver,
                           outputFilePath.toStdString().c_str()
                         );
  if ( !g )
    return true;

  const auto extraOptions = meta.extraOptions();
  for ( auto it = extraOptions.cbegin(); it != extraOptions.cend(); ++it )
  {
    MDAL_G_setMetadata( g, it.key().toStdString().c_str(), it.value().toStdString().c_str() );
  }

  if ( meta.referenceTime().isValid() )
  {
    MDAL_G_setReferenceTime( g, meta.referenceTime().toString( Qt::ISODateWithMs ).toStdString().c_str() );
  }

  bool fail = false;
  for ( int i = 0; i < datasetCount; ++i )
  {
    QgsMeshDatasetIndex index( datasetGroupIndex, i );
    QgsMeshDataBlock values = source->datasetValues( index, 0, valuesCount );
    QgsMeshDataBlock active = source->areFacesActive( index, 0, faceValueCount );
    QgsMeshDatasetMetadata dsm = source->datasetMetadata( index );
    if ( !values.isValid() || !dsm.isValid() )
    {
      fail = true;
      break;
    }

    MDAL_G_addDataset( g,
                       dsm.time(),
                       values.values().constData(),
                       active.active().isEmpty() ? nullptr : active.active().constData()
                     );

  }

  if ( fail )
    return true;

  MDAL_G_closeEditMode( g );
  if ( MDAL_LastStatus() == 0 )
  {
    QgsMeshDatasetGroupMetadata meta = datasetGroupMetadata( datasetGroupCount() - 1 );
    QString newUri = meta.uri();
    if ( !mExtraDatasetUris.contains( newUri ) )
      mExtraDatasetUris << newUri;
    addGroupToTemporalCapabilities( datasetGroupCount() - 1 );
    return false;
  }
  else
    return true;
}

bool QgsMdalProvider::saveMeshFrame( const QgsMesh &mesh )
{
  QgsMdalProviderMetadata mdalProviderMetaData;

  QVariantMap uriComponent = mdalProviderMetaData.decodeUri( dataSourceUri() );

  if ( uriComponent.contains( QStringLiteral( "driver" ) ) )
    return mdalProviderMetaData.createMeshData( mesh, dataSourceUri(), crs() );
  else if ( uriComponent.contains( QStringLiteral( "path" ) ) )
    return mdalProviderMetaData.createMeshData( mesh, uriComponent.value( QStringLiteral( "path" ) ).toString(), mDriverName, crs() );

  return false;

}

void QgsMdalProvider::close()
{
  if ( mMeshH )
    MDAL_CloseMesh( mMeshH );
  mMeshH = nullptr;

  mExtraDatasetUris.clear();
}

void QgsMdalProvider::loadData()
{
  QByteArray curi = dataSourceUri().toUtf8();
  mMeshH = MDAL_LoadMesh( curi.constData() );
  temporalCapabilities()->clear();

  if ( mMeshH )
  {
    mDriverName = MDAL_M_driverName( mMeshH );
    const QString proj = MDAL_M_projection( mMeshH );
    if ( !proj.isEmpty() )
      mCrs.createFromString( proj );

    int dsGroupCount = MDAL_M_datasetGroupCount( mMeshH );
    for ( int i = 0; i < dsGroupCount; ++i )
      addGroupToTemporalCapabilities( i );
  }
}


void QgsMdalProvider::addGroupToTemporalCapabilities( int indexGroup )
{
  if ( !mMeshH )
    return;
  QgsMeshDataProviderTemporalCapabilities *tempCap = temporalCapabilities();
  QgsMeshDatasetGroupMetadata dsgMetadata = datasetGroupMetadata( indexGroup );
  QDateTime refTime = dsgMetadata.referenceTime();
  tempCap->addGroupReferenceDateTime( indexGroup, refTime );
  int dsCount = datasetCount( indexGroup );

  if ( dsgMetadata.isTemporal() )
  {
    tempCap->setHasTemporalCapabilities( true );
    for ( int dsi = 0; dsi < dsCount; ++dsi )
    {
      QgsMeshDatasetMetadata dsMeta = datasetMetadata( QgsMeshDatasetIndex( indexGroup, dsi ) );
      if ( dsMeta.isValid() )
        tempCap->addDatasetTime( indexGroup, dsMeta.time() );
    }
  }

}

void QgsMdalProvider::reloadProviderData()
{
  if ( mMeshH )
    MDAL_CloseMesh( mMeshH );

  loadData();

  int datasetCountBeforeAdding = datasetGroupCount();

  if ( mMeshH )
    for ( const QString &uri : std::as_const( mExtraDatasetUris ) )
    {
      std::string str = uri.toStdString();
      MDAL_M_LoadDatasets( mMeshH, str.c_str() );
      int datasetCount = datasetGroupCount();
      for ( ; datasetCountBeforeAdding < datasetCount; datasetCountBeforeAdding++ )
        addGroupToTemporalCapabilities( datasetCountBeforeAdding );
    }
}

void QgsMdalProvider::fileMeshFilters( QString &fileMeshFiltersString, QString &fileMeshDatasetFiltersString )
{
  MDAL_DriverH mdalDriver;

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, well, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.

  fileMeshFiltersString.clear();
  fileMeshDatasetFiltersString.clear();

  int driverCount = MDAL_driverCount();

  QgsDebugMsgLevel( QStringLiteral( "MDAL driver count: %1" ).arg( driverCount ), 2 );

  for ( int i = 0; i < driverCount; ++i )
  {
    mdalDriver = MDAL_driverFromIndex( i );
    if ( !mdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    QString longName = MDAL_DR_longName( mdalDriver );
    QString driverFilters = MDAL_DR_filters( mdalDriver );
    driverFilters = driverFilters.replace( QLatin1String( ";;" ), QLatin1String( " " ) );

    bool isMeshDriver = MDAL_DR_meshLoadCapability( mdalDriver );

    if ( longName.isEmpty() )
    {
      QgsLogger::warning( "invalid driver long name " + QString::number( i ) );
      continue;
    }

    if ( !driverFilters.isEmpty() )
    {
      QString driverFilter = longName + " (" + driverFilters + ");;";
      if ( isMeshDriver )
        fileMeshFiltersString += driverFilter;
      else
        fileMeshDatasetFiltersString += driverFilter;
    }
  }

  // sort file filters alphabetically
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList filters = fileMeshFiltersString.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
#else
  QStringList filters = fileMeshFiltersString.split( QStringLiteral( ";;" ), Qt::SkipEmptyParts );
#endif
  filters.sort();
  fileMeshFiltersString = filters.join( QLatin1String( ";;" ) ) + ";;";

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  filters = fileMeshDatasetFiltersString.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
#else
  filters = fileMeshDatasetFiltersString.split( QStringLiteral( ";;" ), Qt::SkipEmptyParts );
#endif
  filters.sort();
  fileMeshDatasetFiltersString = filters.join( QLatin1String( ";;" ) ) + ";;";

  // can't forget the default case - first
  fileMeshFiltersString.prepend( QObject::tr( "All files" ) + " (*);;" );
  fileMeshDatasetFiltersString.prepend( QObject::tr( "All files" ) + " (*);;" );

  // cleanup
  if ( fileMeshFiltersString.endsWith( QLatin1String( ";;" ) ) ) fileMeshFiltersString.chop( 2 );
  if ( fileMeshDatasetFiltersString.endsWith( QLatin1String( ";;" ) ) ) fileMeshDatasetFiltersString.chop( 2 );

  QgsDebugMsgLevel( "Mesh filter list built: " + fileMeshFiltersString, 2 );
  QgsDebugMsgLevel( "Mesh dataset filter list built: " + fileMeshDatasetFiltersString, 2 );
}

void QgsMdalProvider::fileMeshExtensions( QStringList &fileMeshExtensions,
    QStringList &fileMeshDatasetExtensions )
{
  MDAL_DriverH mdalDriver;

  // Grind through all the drivers and their respective metadata.
  // We'll add a file extension for those drivers that have a file
  // extension defined for them

  fileMeshExtensions.clear();
  fileMeshDatasetExtensions.clear();

  int driverCount = MDAL_driverCount();

  for ( int i = 0; i < driverCount; ++i )
  {
    mdalDriver = MDAL_driverFromIndex( i );
    if ( !mdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    const QString driverFilters = MDAL_DR_filters( mdalDriver );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList extensions = driverFilters.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
#else
    QStringList extensions = driverFilters.split( QStringLiteral( ";;" ), Qt::SkipEmptyParts );
#endif
    bool isMeshDriver = MDAL_DR_meshLoadCapability( mdalDriver );

    if ( !extensions.isEmpty() )
    {
      for ( auto ext : extensions )
      {
        ext.remove( QStringLiteral( "*." ) );
        ext = ext.toLower();
        if ( isMeshDriver )
          fileMeshExtensions += ext;
        else
          fileMeshDatasetExtensions += ext;
      }
    }
  }

  // sort file extensions alphabetically
  fileMeshExtensions.sort();
  fileMeshDatasetExtensions.sort();

  // remove duplicates
  fileMeshExtensions.erase( std::unique( fileMeshExtensions.begin(), fileMeshExtensions.end() ), fileMeshExtensions.end() );
  fileMeshDatasetExtensions.erase( std::unique( fileMeshDatasetExtensions.begin(), fileMeshDatasetExtensions.end() ), fileMeshDatasetExtensions.end() );

  QgsDebugMsgLevel( "Mesh extensions list built: " + fileMeshExtensions.join( QLatin1String( ";;" ) ), 2 );
  QgsDebugMsgLevel( "Mesh dataset extensions list built: " + fileMeshDatasetExtensions.join( QLatin1String( ";;" ) ), 2 );
}

/*----------------------------------------------------------------------------------------------*/

bool QgsMdalProvider::addDataset( const QString &uri )
{
  int datasetCount = datasetGroupCount();

  std::string str = uri.toStdString();
  MDAL_M_LoadDatasets( mMeshH, str.c_str() );

  if ( datasetCount == datasetGroupCount() )
  {
    return false;
  }
  else
  {
    if ( !mExtraDatasetUris.contains( uri ) )
      mExtraDatasetUris << uri;
    int datasetCountAfterAdding = datasetGroupCount();
    int datasetCountAdded = datasetCountAfterAdding - datasetCount;
    for ( ; datasetCount < datasetCountAfterAdding; datasetCount++ )
      addGroupToTemporalCapabilities( datasetCount );

    emit datasetGroupsAdded( datasetCountAdded );
    emit dataChanged();
    return true; // Ok
  }
}

QStringList QgsMdalProvider::extraDatasets() const
{
  return mExtraDatasetUris;
}

int QgsMdalProvider::datasetGroupCount() const
{
  return MDAL_M_datasetGroupCount( mMeshH );
}


int QgsMdalProvider::datasetCount( int groupIndex ) const
{
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, groupIndex );
  if ( !group )
    return 0;
  return MDAL_G_datasetCount( group );
}

QgsMeshDatasetGroupMetadata QgsMdalProvider::datasetGroupMetadata( int groupIndex ) const
{
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, groupIndex );
  if ( !group )
    return QgsMeshDatasetGroupMetadata();


  bool isScalar = MDAL_G_hasScalarData( group );
  MDAL_DataLocation location = MDAL_G_dataLocation( group );
  QgsMeshDatasetGroupMetadata::DataType type = QgsMeshDatasetGroupMetadata::DataOnFaces;
  switch ( location )
  {
    case MDAL_DataLocation::DataOnFaces:
      type = QgsMeshDatasetGroupMetadata::DataOnFaces;
      break;
    case MDAL_DataLocation::DataOnVertices:
      type = QgsMeshDatasetGroupMetadata::DataOnVertices;
      break;
    case MDAL_DataLocation::DataOnEdges:
      type = QgsMeshDatasetGroupMetadata::DataOnEdges;
      break;
    case MDAL_DataLocation::DataOnVolumes:
      type = QgsMeshDatasetGroupMetadata::DataOnVolumes;
      break;
    case MDAL_DataLocation::DataInvalidLocation:
      return QgsMeshDatasetGroupMetadata();
  }

  QString name = MDAL_G_name( group );
  QString uri = MDAL_G_uri( group );
  double min, max;
  MDAL_G_minimumMaximum( group, &min, &max );

  int maximumVerticalLevels = MDAL_G_maximumVerticalLevelCount( group );

  QMap<QString, QString> metadata;
  int n = MDAL_G_metadataCount( group );
  for ( int i = 0; i < n; ++i )
  {
    QString key = MDAL_G_metadataKey( group, i );
    QString value = MDAL_G_metadataValue( group, i );
    metadata[key] = value;
  }

  QString referenceTimeString( MDAL_G_referenceTime( group ) );
  if ( !referenceTimeString.isEmpty() )
    referenceTimeString.append( 'Z' );//For now provider doesn't support time zone and return always in local time, force UTC
  QDateTime referenceTime = QDateTime::fromString( referenceTimeString, Qt::ISODate );

  bool isTemporal = MDAL_G_isTemporal( group );

  QgsMeshDatasetGroupMetadata meta(
    name,
    uri,
    isScalar,
    type,
    min,
    max,
    maximumVerticalLevels,
    referenceTime,
    isTemporal,
    metadata
  );

  return meta;
}

QgsMeshDatasetMetadata QgsMdalProvider::datasetMetadata( QgsMeshDatasetIndex index ) const
{
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDatasetMetadata();

  MDAL_DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDatasetMetadata();

  bool isValid = MDAL_D_isValid( dataset );
  double time = MDAL_D_time( dataset );
  double min, max;
  MDAL_D_minimumMaximum( dataset, &min, &max );
  const int maxLevels = MDAL_D_maximumVerticalLevelCount( dataset );

  QgsMeshDatasetMetadata meta(
    time,
    isValid,
    min,
    max,
    maxLevels
  );

  return meta;

}

QgsMeshDatasetValue QgsMdalProvider::datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const
{
  QgsMeshDataBlock vals = datasetValues( index, valueIndex, 1 );
  return vals.value( 0 );
}

QgsMeshDataBlock QgsMdalProvider::datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const
{
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDataBlock();

  MDAL_DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDataBlock();

  bool isScalar = MDAL_G_hasScalarData( group );

  QgsMeshDataBlock ret( isScalar ? QgsMeshDataBlock::ScalarDouble : QgsMeshDataBlock::Vector2DDouble, count );
  QVector<double> buffer( isScalar ? count : 2 * count );
  int valRead = MDAL_D_data( dataset,
                             valueIndex,
                             count,
                             isScalar ? MDAL_DataType::SCALAR_DOUBLE : MDAL_DataType::VECTOR_2D_DOUBLE,
                             buffer.data() );
  if ( valRead != count )
    return QgsMeshDataBlock();

  ret.setValues( buffer );
  return ret;
}

QgsMesh3dDataBlock QgsMdalProvider::dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMesh3dDataBlock();

  MDAL_DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMesh3dDataBlock();

  if ( count < 1 )
    return QgsMesh3dDataBlock();

  bool isScalar = MDAL_G_hasScalarData( group );

  QgsMesh3dDataBlock ret( count, !isScalar );
  {
    QVector<int> faceToVolumeIndexBuffer( count );
    int valRead = MDAL_D_data( dataset,
                               faceIndex,
                               count,
                               MDAL_DataType::FACE_INDEX_TO_VOLUME_INDEX_INTEGER,
                               faceToVolumeIndexBuffer.data() );
    if ( valRead != count )
      return QgsMesh3dDataBlock();
    ret.setFaceToVolumeIndex( faceToVolumeIndexBuffer );
  }

  {
    QVector<int> verticalLevelCountBuffer( count );
    int valRead = MDAL_D_data( dataset,
                               faceIndex,
                               count,
                               MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER,
                               verticalLevelCountBuffer.data() );
    if ( valRead != count )
      return QgsMesh3dDataBlock();

    ret.setVerticalLevelsCount( verticalLevelCountBuffer );
  }

  const int firstVolumeIndex = ret.firstVolumeIndex();
  const int lastVolumeIndex = ret.lastVolumeIndex();
  const int nVolumes = lastVolumeIndex - firstVolumeIndex;
  if ( firstVolumeIndex < 0 || lastVolumeIndex < 0 || nVolumes < 1 )
    return QgsMesh3dDataBlock();

  const int nVerticalLevelFaces = nVolumes + count; // all volumes top face + bottom face
  const int startIndexVerticalFaces = firstVolumeIndex + faceIndex;

  {
    QVector<double> verticalLevels( nVerticalLevelFaces );
    int valRead = MDAL_D_data( dataset,
                               startIndexVerticalFaces,
                               nVerticalLevelFaces,
                               MDAL_DataType::VERTICAL_LEVEL_DOUBLE,
                               verticalLevels.data() );
    if ( valRead != nVerticalLevelFaces )
      return QgsMesh3dDataBlock();
    ret.setVerticalLevels( verticalLevels );
  }

  {
    QVector<double> values( isScalar ? nVolumes :  2 * nVolumes );
    int valRead = MDAL_D_data( dataset,
                               firstVolumeIndex,
                               nVolumes,
                               isScalar ? MDAL_DataType::SCALAR_VOLUMES_DOUBLE : MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE,
                               values.data() );
    if ( valRead != nVolumes )
      return QgsMesh3dDataBlock();
    ret.setValues( values );
  }

  ret.setValid( true );
  return ret;
}

bool QgsMdalProvider::isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const
{
  QgsMeshDataBlock vals = areFacesActive( index, faceIndex, 1 );
  return vals.active( 0 );
}

QgsMeshDataBlock QgsMdalProvider::areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDataBlock();

  MDAL_DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDataBlock();

  QgsMeshDataBlock ret( QgsMeshDataBlock::ActiveFlagInteger, count );

  if ( MDAL_D_hasActiveFlagCapability( dataset ) )
  {
    QVector<int> buf( count );
    int valRead = MDAL_D_data( dataset, faceIndex, count, MDAL_DataType::ACTIVE_INTEGER, buf.data() );
    if ( valRead != count )
      return QgsMeshDataBlock();
    ret.setActive( buf );
  }
  else
  {
    ret.setValid( true );
  }
  return ret;
}

/*----------------------------------------------------------------------------------------------*/

QgsMdalProvider *QgsMdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsMdalProvider( uri, options, flags );
}


static MDAL_MeshH createMDALMesh( const QgsMesh &mesh, const QString &driverName, const QgsCoordinateReferenceSystem &crs )
{
  MDAL_DriverH driver = MDAL_driverFromName( driverName.toStdString().c_str() );
  if ( !driver )
    return nullptr;

  MDAL_MeshH mdalMesh = MDAL_CreateMesh( driver );

  if ( !mdalMesh )
    return nullptr;

  int bufferSize = 2000;
  int vertexIndex = 0;
  int faceIndex = 0;

  while ( vertexIndex < mesh.vertexCount() )
  {
    int vertexCount = std::min( bufferSize, mesh.vertexCount() - vertexIndex );
    QVector<double> verticesCoordinates( vertexCount * 3 );
    for ( int i = 0; i < vertexCount ; ++i )
    {
      int globalIndex = vertexIndex + i;
      const QgsMeshVertex &vert = mesh.vertex( globalIndex );
      verticesCoordinates[i * 3  ] = vert.x();
      verticesCoordinates[i * 3 + 1] = vert.y();
      verticesCoordinates[i * 3 + 2] = vert.z();
    }
    vertexIndex += vertexCount;

    MDAL_M_addVertices( mdalMesh, vertexCount, verticesCoordinates.data() );
  }

  while ( faceIndex < mesh.faceCount() )
  {
    int faceCount = std::min( bufferSize, mesh.faceCount() - faceIndex );
    QVector<int> faceSizes( faceCount );
    QVector<int> indices;
    for ( int i = 0; i < faceCount; ++i )
    {
      const QgsMeshFace &face = mesh.face( faceIndex + i );
      faceSizes[i] = face.count();
      for ( int j = 0; j < faceSizes[i]; ++j )
        indices.push_back( face.at( j ) );
    }
    MDAL_M_addFaces( mdalMesh, faceCount, faceSizes.data(), indices.data() );
    if ( MDAL_LastStatus() != MDAL_Status::None )
    {
      MDAL_CloseMesh( mdalMesh );
      return nullptr;
    }
    faceIndex += faceCount;
  }

  MDAL_M_setProjection( mdalMesh, crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ).toStdString().c_str() );

  return mdalMesh;
}


bool QgsMdalProviderMetadata::createMeshData( const QgsMesh &mesh, const QString &fileName, const QString &driverName, const QgsCoordinateReferenceSystem &crs ) const
{
  MDAL_MeshH mdalMesh = createMDALMesh( mesh, driverName, crs );

  if ( !mdalMesh )
    return false;

  MDAL_SaveMesh( mdalMesh, fileName.toStdString().c_str(), driverName.toStdString().c_str() );

  if ( MDAL_LastStatus() != MDAL_Status::None )
  {
    MDAL_CloseMesh( mdalMesh );
    return false;
  }

  MDAL_CloseMesh( mdalMesh );
  return true;
}

bool QgsMdalProviderMetadata::createMeshData( const QgsMesh &mesh, const QString &uri, const QgsCoordinateReferenceSystem &crs ) const
{
  QVariantMap uriComponents = decodeUri( uri );

  if ( !uriComponents.contains( QStringLiteral( "driver" ) ) || !uriComponents.contains( QStringLiteral( "path" ) ) )
    return false;

  MDAL_MeshH mdalMesh = createMDALMesh( mesh,
                                        uriComponents.value( QStringLiteral( "driver" ) ).toString()
                                        , crs );

  if ( !mdalMesh )
    return false;

  MDAL_SaveMeshWithUri( mdalMesh, uri.toStdString().c_str() );

  if ( MDAL_LastStatus() != MDAL_Status::None )
  {
    MDAL_CloseMesh( mdalMesh );
    return false;
  }

  MDAL_CloseMesh( mdalMesh );
  return true;
}

QVariantMap QgsMdalProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;

  const QRegularExpression layerRegex( QStringLiteral( "^([a-zA-Z0-9_]+?):\"(.*)\"(?::([a-zA-Z0-9_ ]+?$)|($))" ) );
  const QRegularExpressionMatch layerNameMatch = layerRegex.match( uri );
  if ( layerNameMatch.hasMatch() )
  {
    uriComponents.insert( QStringLiteral( "driver" ), layerNameMatch.captured( 1 ) );
    uriComponents.insert( QStringLiteral( "path" ), layerNameMatch.captured( 2 ) );
    uriComponents.insert( QStringLiteral( "layerName" ), layerNameMatch.captured( 3 ) );
  }
  else
  {
    uriComponents.insert( QStringLiteral( "path" ), uri );
  }

  return uriComponents;
}

QString QgsMdalProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  if ( !parts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() && !parts.value( QStringLiteral( "driver" ) ).toString().isEmpty() )
  {
    return QStringLiteral( "%1:\"%2\":%3" ).arg( parts.value( QStringLiteral( "driver" ) ).toString(),
           parts.value( QStringLiteral( "path" ) ).toString(),
           parts.value( QStringLiteral( "layerName" ) ).toString() );
  }
  else if ( !parts.value( QStringLiteral( "driver" ) ).toString().isEmpty() )
  {
    return QStringLiteral( "%1:\"%2\"" ).arg( parts.value( QStringLiteral( "driver" ) ).toString(),
           parts.value( QStringLiteral( "path" ) ).toString() );
  }
  else
  {
    return parts.value( QStringLiteral( "path" ) ).toString();
  }
}

QgsProviderMetadata::ProviderCapabilities QgsMdalProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsMdalProviderMetadata::capabilities() const
{
  return QuerySublayers;
}

QList<QgsProviderSublayerDetails> QgsMdalProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags, QgsFeedback * ) const
{
  if ( uri.isEmpty() )
    return {};

  const QVariantMap uriParts = decodeUri( uri );
  const QString path = uriParts.value( QStringLiteral( "path" ), uri ).toString();
  const QString layerName = uriParts.value( QStringLiteral( "layerName" ) ).toString();

  const QFileInfo info( path );

  if ( info.isDir() )
    return {};

  if ( info.isFile() )
  {
    const QString suffix = info.suffix().toLower();

    static QStringList sExtensions;
    static std::once_flag initialized;
    std::call_once( initialized, [ = ]( )
    {
      QStringList meshExtensions;
      QStringList datasetsExtensions;
      QgsMdalProvider::fileMeshExtensions( sExtensions, datasetsExtensions );
      Q_UNUSED( datasetsExtensions )
    } );

    // Filter files by extension
    if ( !sExtensions.contains( suffix ) )
      return {};

    // special handling for .adf files -- although mdal reports support for the .adf file format, we only
    // want to report sublayers for tdenv.adf or tdenv9.adf files (otherwise we are reporting that any arcinfo grids or coverages are meshes)
    if ( suffix == QLatin1String( "adf" )
         && !info.completeBaseName().startsWith( QLatin1String( "tdenv" ), Qt::CaseInsensitive ) )
      return {};
  }

  if ( flags & Qgis::SublayerQueryFlag::FastScan )
  {
    if ( !info.isFile() )
      return {};

    QgsProviderSublayerDetails details;
    details.setType( QgsMapLayerType::MeshLayer );
    details.setProviderKey( QStringLiteral( "mdal" ) );
    details.setUri( uri );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( path ) );
    // treat all mesh files as potentially being containers (is this correct?)
    details.setSkippedContainerScan( true );
    return {details};
  }

  const QStringList meshNames = QString( MDAL_MeshNames( path.toUtf8() ) ).split( QStringLiteral( ";;" ) );

  QList<QgsProviderSublayerDetails> res;
  res.reserve( meshNames.size() );
  int layerIndex = 0;
  for ( const QString &layerUri : meshNames )
  {
    if ( layerUri.isEmpty() )
      continue;

    const QVariantMap layerUriParts = decodeUri( layerUri );
    //if an explicit layer name was included in the original uri, we only keep that layer in the results
    if ( !layerName.isEmpty() && layerUriParts.value( QStringLiteral( "layerName" ) ).toString() != layerName )
      continue;

    QgsProviderSublayerDetails details;
    details.setUri( layerUri );
    details.setProviderKey( QStringLiteral( "mdal" ) );
    details.setType( QgsMapLayerType::MeshLayer );
    details.setLayerNumber( layerIndex );
    details.setDriverName( layerUriParts.value( QStringLiteral( "driver" ) ).toString() );

    // strip the driver name and path from the MDAL uri to get the layer name
    details.setName( layerUriParts.value( QStringLiteral( "layerName" ) ).toString() );
    if ( details.name().isEmpty() )
    {
      // use file name as layer name if no layer name available from mdal
      details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( path ) );
    }

    res << details;

    layerIndex++;
  }
  return res;
}

QList<QgsMapLayerType> QgsMdalProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::MeshLayer };
}

QString QgsMdalProviderMetadata::filters( FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterMesh:
    {
      QString fileMeshFiltersString;
      QString fileMeshDatasetFiltersString;
      QgsMdalProvider::fileMeshFilters( fileMeshFiltersString, fileMeshDatasetFiltersString );
      return fileMeshFiltersString;
    }
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
    {
      QString fileMeshFiltersString;
      QString fileMeshDatasetFiltersString;
      QgsMdalProvider::fileMeshFilters( fileMeshFiltersString, fileMeshDatasetFiltersString );
      return fileMeshDatasetFiltersString;
    }

    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterPointCloud:
      return QString();
  }
  return QString();
}

QList<QgsMeshDriverMetadata> QgsMdalProviderMetadata::meshDriversMetadata()
{
  MDAL_DriverH mdalDriver;
  QList<QgsMeshDriverMetadata> ret;

  int driverCount = MDAL_driverCount();
  for ( int i = 0; i < driverCount; ++i )
  {
    mdalDriver = MDAL_driverFromIndex( i );
    if ( !mdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    QString name = MDAL_DR_name( mdalDriver );
    QString longName = MDAL_DR_longName( mdalDriver );
    QString writeDatasetSuffix = MDAL_DR_writeDatasetsSuffix( mdalDriver );
    QString writeMeshFrameSuffix = MDAL_DR_saveMeshSuffix( mdalDriver );
    int maxVerticesPerFace = MDAL_DR_faceVerticesMaximumCount( mdalDriver );

    QgsMeshDriverMetadata::MeshDriverCapabilities capabilities;
    bool hasSaveFaceDatasetsCapability = MDAL_DR_writeDatasetsCapability( mdalDriver, MDAL_DataLocation::DataOnFaces );
    if ( hasSaveFaceDatasetsCapability )
      capabilities |= QgsMeshDriverMetadata::CanWriteFaceDatasets;
    bool hasSaveVertexDatasetsCapability = MDAL_DR_writeDatasetsCapability( mdalDriver, MDAL_DataLocation::DataOnVertices );
    if ( hasSaveVertexDatasetsCapability )
      capabilities |= QgsMeshDriverMetadata::CanWriteVertexDatasets;
    bool hasSaveEdgeDatasetsCapability = MDAL_DR_writeDatasetsCapability( mdalDriver, MDAL_DataLocation::DataOnEdges );
    if ( hasSaveEdgeDatasetsCapability )
      capabilities |= QgsMeshDriverMetadata::CanWriteEdgeDatasets;
    bool hasMeshSaveCapability = MDAL_DR_saveMeshCapability( mdalDriver );
    if ( hasMeshSaveCapability )
      capabilities |= QgsMeshDriverMetadata::CanWriteMeshData;
    const QgsMeshDriverMetadata meta( name, longName, capabilities, writeDatasetSuffix, writeMeshFrameSuffix, maxVerticesPerFace );
    ret.push_back( meta );
  }
  return ret;
}

QgsMdalProviderMetadata::QgsMdalProviderMetadata()
  : QgsProviderMetadata( QgsMdalProvider::MDAL_PROVIDER_KEY, QgsMdalProvider::MDAL_PROVIDER_DESCRIPTION )
{}

QIcon QgsMdalProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconMeshLayer.svg" ) );
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsMdalProviderMetadata();
}
