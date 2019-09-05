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
#include "qgsmdaldataitems.h"


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

QgsMdalProvider::QgsMdalProvider( const QString &uri, const ProviderOptions &options )
  : QgsMeshDataProvider( uri, options )
{
  loadData();
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

void QgsMdalProvider::populateMesh( QgsMesh *mesh ) const
{
  if ( mesh )
  {
    mesh->faces = faces();
    mesh->vertices = vertices();
  }
}

QVector<QgsMeshVertex> QgsMdalProvider::vertices( ) const
{
  const int bufferSize = std::min( vertexCount(), 1000 );
  QVector<QgsMeshVertex> ret( vertexCount() );
  QVector<double> buffer( bufferSize * 3 );
  MeshVertexIteratorH it = MDAL_M_vertexIterator( mMeshH );
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

QVector<QgsMeshFace> QgsMdalProvider::faces( ) const
{
  const int faceOffsetsBufferLen = std::min( faceCount(), 1000 );
  const int vertexIndicesBufferLen = faceOffsetsBufferLen * 4; // most usually we have quads
  int facesCount = faceCount();

  QVector<QgsMeshFace> ret( facesCount );
  QVector<int> faceOffsetsBuffer( faceOffsetsBufferLen );
  QVector<int> vertexIndicesBuffer( vertexIndicesBufferLen );

  MeshFaceIteratorH it = MDAL_M_faceIterator( mMeshH );
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

bool QgsMdalProvider::persistDatasetGroup( const QString &path,
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

  const QString driverName( "BINARY_DAT" ); //nothing else is implemented in MDAL
  DriverH driver = MDAL_driverFromName( driverName.toStdString().c_str() );
  if ( !driver )
    return true;

  DatasetGroupH g = MDAL_M_addDatasetGroup(
                      mMeshH,
                      meta.name().toStdString().c_str(),
                      meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices,
                      meta.isScalar(),
                      driver,
                      path.toStdString().c_str()
                    );
  if ( !g )
    return true;

  const auto extraOptions = meta.extraOptions();
  for ( auto it = extraOptions.cbegin(); it != extraOptions.cend(); ++it )
  {
    MDAL_G_setMetadata( g, it.key().toStdString().c_str(), it.value().toStdString().c_str() );
  }


  for ( int i = 0; i < datasetValues.size(); ++i )
  {
    MDAL_G_addDataset( g,
                       times.at( i ),
                       static_cast<const double *>( datasetValues.at( i ).constBuffer() ),
                       datasetActive.isEmpty() ? nullptr : static_cast<const int *>( datasetActive.at( i ).constBuffer() )
                     );
  }

  MDAL_G_closeEditMode( g );

  emit datasetGroupsAdded( 1 );
  emit dataChanged();

  return false;
}

void QgsMdalProvider::loadData()
{
  QByteArray curi = dataSourceUri().toUtf8();
  mMeshH = MDAL_LoadMesh( curi.constData() );
  if ( mMeshH )
  {
    const QString proj = MDAL_M_projection( mMeshH );
    if ( !proj.isEmpty() )
      mCrs.createFromString( proj );
  }
}

void QgsMdalProvider::reloadData()
{
  if ( mMeshH )
    MDAL_CloseMesh( mMeshH );

  loadData();

  if ( mMeshH )
    for ( auto uri : mExtraDatasetUris )
    {
      std::string str = uri.toStdString();
      MDAL_M_LoadDatasets( mMeshH, str.c_str() );
    }
}

void QgsMdalProvider::fileMeshFilters( QString &fileMeshFiltersString, QString &fileMeshDatasetFiltersString )
{
  DriverH mdalDriver;

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
    driverFilters = driverFilters.replace( QStringLiteral( ";;" ), QStringLiteral( " " ) );

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
  QStringList filters = fileMeshFiltersString.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
  filters.sort();
  fileMeshFiltersString = filters.join( QStringLiteral( ";;" ) ) + ";;";

  filters = fileMeshDatasetFiltersString.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
  filters.sort();
  fileMeshDatasetFiltersString = filters.join( QStringLiteral( ";;" ) ) + ";;";

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
  DriverH mdalDriver;

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
    QStringList extensions = driverFilters.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
    bool isMeshDriver = MDAL_DR_meshLoadCapability( mdalDriver );

    if ( !extensions.isEmpty() )
    {
      for ( auto ext : extensions )
      {
        ext.remove( QStringLiteral( "*." ) );
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

  QgsDebugMsgLevel( "Mesh extensions list built: " + fileMeshExtensions.join( QStringLiteral( ";;" ) ), 2 );
  QgsDebugMsgLevel( "Mesh dataset extensions list built: " + fileMeshDatasetExtensions.join( QStringLiteral( ";;" ) ), 2 );
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
    mExtraDatasetUris << uri;
    emit datasetGroupsAdded( datasetGroupCount() - datasetCount );
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
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, groupIndex );
  if ( !group )
    return 0;
  return MDAL_G_datasetCount( group );
}

QgsMeshDatasetGroupMetadata QgsMdalProvider::datasetGroupMetadata( int groupIndex ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, groupIndex );
  if ( !group )
    return QgsMeshDatasetGroupMetadata();


  bool isScalar = MDAL_G_hasScalarData( group );
  bool isOnVertices = MDAL_G_isOnVertices( group );
  QString name = MDAL_G_name( group );
  double min, max;
  MDAL_G_minimumMaximum( group, &min, &max );

  QMap<QString, QString> metadata;
  int n = MDAL_G_metadataCount( group );
  for ( int i = 0; i < n; ++i )
  {
    QString key = MDAL_G_metadataKey( group, i );
    QString value = MDAL_G_metadataValue( group, i );
    metadata[key] = value;
  }

  QgsMeshDatasetGroupMetadata meta(
    name,
    isScalar,
    isOnVertices,
    min,
    max,
    metadata
  );

  return meta;
}

QgsMeshDatasetMetadata QgsMdalProvider::datasetMetadata( QgsMeshDatasetIndex index ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDatasetMetadata();

  DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDatasetMetadata();

  bool isValid = MDAL_D_isValid( dataset );
  double time = MDAL_D_time( dataset );
  double min, max;
  MDAL_D_minimumMaximum( dataset, &min, &max );

  QgsMeshDatasetMetadata meta(
    time,
    isValid,
    min,
    max
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
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDataBlock();

  DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDataBlock();

  bool isScalar = MDAL_G_hasScalarData( group );

  QgsMeshDataBlock ret( isScalar ? QgsMeshDataBlock::ScalarDouble : QgsMeshDataBlock::Vector2DDouble, count );
  int valRead = MDAL_D_data( dataset,
                             valueIndex,
                             count,
                             isScalar ? MDAL_DataType::SCALAR_DOUBLE : MDAL_DataType::VECTOR_2D_DOUBLE,
                             ret.buffer() );
  if ( valRead != count )
    return QgsMeshDataBlock();

  return ret;
}

bool QgsMdalProvider::isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const
{
  QgsMeshDataBlock vals = areFacesActive( index, faceIndex, 1 );
  return vals.active( 0 );
}

QgsMeshDataBlock QgsMdalProvider::areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDataBlock();

  DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDataBlock();

  QgsMeshDataBlock ret( QgsMeshDataBlock::ActiveFlagInteger, count );

  int valRead = MDAL_D_data( dataset, faceIndex, count, MDAL_DataType::ACTIVE_INTEGER, ret.buffer() );
  if ( valRead != count )
    return ret;

  return ret;
}

/*----------------------------------------------------------------------------------------------*/

QgsMdalProvider *QgsMdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsMdalProvider( uri, options );
}

QList<QgsDataItemProvider *> QgsMdalProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsMdalDataItemProvider;
  return providers;
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
    default:
      return QString();
  }
}

QgsMdalProviderMetadata::QgsMdalProviderMetadata()
  : QgsProviderMetadata( QgsMdalProvider::MDAL_PROVIDER_KEY, QgsMdalProvider::MDAL_PROVIDER_DESCRIPTION )
{}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsMdalProviderMetadata();
}
