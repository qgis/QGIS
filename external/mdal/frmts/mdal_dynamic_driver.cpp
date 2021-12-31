/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/


#include "mdal_dynamic_driver.hpp"
#include "mdal_logger.hpp"
#ifndef WIN32
#include <dlfcn.h>
#endif
#include <string.h>
#include <iostream>


MDAL::DriverDynamic::DriverDynamic( const std::string &name, const std::string &longName, const std::string &filters, int capabilityFlags, int maxVertexPerFace, const MDAL::Library &lib ):
  Driver( name, longName, filters, capabilityFlags ),
  mLibrary( lib ),
  mCapabilityFlags( capabilityFlags ),
  mMaxVertexPerFace( maxVertexPerFace )
{}

MDAL::Driver *MDAL::DriverDynamic::create()
{
  std::unique_ptr<MDAL::DriverDynamic> driver( new DriverDynamic( name(), longName(), filters(), mCapabilityFlags, mMaxVertexPerFace, mLibrary ) );
  if ( driver->loadSymbols() )
    return driver.release();
  else
    return nullptr;
}

bool MDAL::DriverDynamic::canReadMesh( const std::string &uri )
{
  if ( mCanReadMeshFunction )
  {
    return mCanReadMeshFunction( uri.c_str() );
  }
  return false;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverDynamic::load( const std::string &uri, const std::string &meshName )
{
  if ( !mOpenMeshFunction )
    return std::unique_ptr<MDAL::Mesh>();

  int meshId = mOpenMeshFunction( uri.c_str(), meshName.c_str() );
  if ( meshId != -1 )
  {
    if ( mMeshIds.find( meshId ) == mMeshIds.end() )
    {
      std::unique_ptr<MDAL::MeshDynamicDriver> mesh( new MeshDynamicDriver( name(), mMaxVertexPerFace, uri, mLibrary, meshId ) );
      if ( mesh->loadSymbol() )
      {
        mMeshIds.insert( meshId );
        mesh->setProjection();
        if ( mesh->populateDatasetGroups() )
          return mesh;
      }
    }
  }
  MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "Unable to load the mesh" );
  return std::unique_ptr<MDAL::Mesh>();
}

MDAL::Driver *MDAL::DriverDynamic::create( const std::string &libFile )
{
  Library library( libFile );

  std::function<const char *()> driverNameFunction = library.getSymbol<const char *>( "MDAL_DRIVER_driverName" );
  std::function<const char *()> driverLongNameFunction = library.getSymbol<const char *>( "MDAL_DRIVER_driverLongName" );
  std::function<const char *()> driverFiltersFunction = library.getSymbol<const char *>( "MDAL_DRIVER_filters" );
  std::function<int()> driverCapabilitiesFunction = library.getSymbol<int>( "MDAL_DRIVER_capabilities" );
  std::function<int()> driverMaxVertexPerFaceFunction = library.getSymbol<int>( "MDAL_DRIVER_maxVertexPerFace" );

  if ( !driverNameFunction ||
       !driverLongNameFunction ||
       !driverFiltersFunction ||
       !driverCapabilitiesFunction ||
       !driverMaxVertexPerFaceFunction )
  {
    // No log error here because MDAL can try any files to find the good one
    return nullptr;
  }

  std::string name( driverNameFunction() );
  std::string longName( driverLongNameFunction() );
  std::string filters( driverFiltersFunction() );
  MDAL::Capability capabilities = static_cast<MDAL::Capability>( driverCapabilitiesFunction() );
  int maxVertexPerFace = driverMaxVertexPerFaceFunction();

  std::unique_ptr<DriverDynamic> driver( new DriverDynamic( name, longName, filters, capabilities, maxVertexPerFace, library ) );

  if ( !driver->loadSymbols() )
  {
    //Log error created by loadSymbols()
    return nullptr;
  }

  return driver.release();
}

bool MDAL::DriverDynamic::loadSymbols()
{
  mCanReadMeshFunction = mLibrary.getSymbol<bool, const char *>( "MDAL_DRIVER_canReadMesh" );
  mOpenMeshFunction = mLibrary.getSymbol<int, const char *, const char *>( "MDAL_DRIVER_openMesh" );

  if ( mCanReadMeshFunction == nullptr ||
       mOpenMeshFunction == nullptr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, name(), "External driver is not valid" );
    return false;
  }

  return true;
}

MDAL::MeshDynamicDriver::MeshDynamicDriver( const std::string &driverName,
    size_t faceVerticesMaximumCount,
    const std::string &uri,
    const MDAL::Library &library,
    int meshId ):
  Mesh( driverName, faceVerticesMaximumCount, uri ),
  mLibrary( library ),
  mId( meshId )
{}

MDAL::MeshDynamicDriver::~MeshDynamicDriver()
{
  mCloseMeshFunction( mId );
}

static int elementCount( int meshId, const std::function<int ( int )> &countFunction, const std::string &driverName )
{
  if ( countFunction )
  {
    int count = countFunction( meshId );
    if ( count >= 0 )
      return count;

    MDAL::Log::error( MDAL_Status::Err_InvalidData, driverName, "Invalid mesh" );
  }
  else
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, driverName, "Driver is not valid" );

  return 0;
}

size_t MDAL::MeshDynamicDriver::verticesCount() const
{
  return elementCount( mId, mMeshVertexCountFunction, driverName() );
}

size_t MDAL::MeshDynamicDriver::facesCount() const
{
  return elementCount( mId, mMeshFaceCountFunction, driverName() );
}

size_t MDAL::MeshDynamicDriver::edgesCount() const
{
  return elementCount( mId, mMeshEdgeCountFunction, driverName() );
}

MDAL::BBox MDAL::MeshDynamicDriver::extent() const
{
  if ( mMeshExtentFunction )
  {
    double xMin, xMax, yMin, yMax;
    mMeshExtentFunction( mId, &xMin, &xMax, &yMin, &yMax );
    return BBox( xMin, xMax, yMin, yMax );
  }

  return BBox( std::numeric_limits<double>::quiet_NaN(),
               std::numeric_limits<double>::quiet_NaN(),
               std::numeric_limits<double>::quiet_NaN(),
               std::numeric_limits<double>::quiet_NaN() );
}

void MDAL::MeshDynamicDriver::setProjection()
{
  if ( !mMeshProjectionFunction )
    return;

  std::string projection = mMeshProjectionFunction( mId );
  setSourceCrs( projection );
}

bool MDAL::MeshDynamicDriver::populateDatasetGroups()
{
  if ( !mMeshDatasetGroupsCountFunction )
    return false;

  int datasetGroupCount = mMeshDatasetGroupsCountFunction( mId );

  for ( int i = 0; i < datasetGroupCount; ++i )
  {
    const char *groupName = mDatasetgroupNameFunction( mId, i );
    const char *referenceTime = mDatasetGroupReferencetimeFunction( mId, i );
    bool isScalar = true;
    int dataLocation = 0;
    int datasetCount = 0;
    if ( !mDatasetDescriptionFunction( mId, i, &isScalar, &dataLocation, &datasetCount ) )
      return false;
    std::shared_ptr<DatasetGroup> group = std::make_shared<DatasetGroup>( driverName(), this, uri() );
    if ( groupName )
      group->setName( groupName );
    if ( referenceTime )
    {
      std::string referenceTimeIso8701 = referenceTime;
      group->setReferenceTime( referenceTimeIso8701 );
    }
    group->setIsScalar( isScalar );
    switch ( dataLocation )
    {
      case 1:
        group->setDataLocation( MDAL_DataLocation::DataOnVertices );
        break;
      case 2:
        group->setDataLocation( MDAL_DataLocation::DataOnFaces );
        break;
      case 3:
        group->setDataLocation( MDAL_DataLocation::DataOnVolumes );
        break;
      case 4:
        group->setDataLocation( MDAL_DataLocation::DataOnEdges );
        break;
      default:
        group->setDataLocation( MDAL_DataLocation::DataInvalidLocation );
        break;
    }

    int metadataCount = mDatasetGroupMetadataCountFunction( mId, i );
    if ( metadataCount > 0 )
    {
      for ( int metaIndex = 0; metaIndex < metadataCount; ++metaIndex )
      {
        std::string key( mDatasetGroupMetadataKeyFunction( mId, i, metaIndex ) );
        std::string value( mDatasetGroupMetadataValueFunction( mId, i, metaIndex ) );
        group->setMetadata( key, value );
      }
    }

    for ( int d = 0; d < datasetCount ; ++d )
    {
      std::shared_ptr<Dataset> dataset;

      switch ( group->dataLocation() )
      {
        case DataInvalidLocation:
          continue;
          break;
        case DataOnVertices:
        case DataOnEdges:
        case DataOnFaces:
        {
          std::shared_ptr<DatasetDynamicDriver2D> dataset2D = std::make_shared<DatasetDynamicDriver2D>( group.get(), mId, i, d, mLibrary );
          dataset2D->setSupportsActiveFlag( mDatasetSupportActiveFlagFunction( mId, i, d ) );

          if ( !dataset2D->loadSymbol() )
            return false;

          dataset2D->setStatistics( MDAL::calculateStatistics( dataset2D ) );
          dataset2D->unloadData();
          dataset = dataset2D;
        }
        break;
        case DataOnVolumes:
        {
          size_t maxVerticalLevelCount = mDataset3DMaximumVerticalLevelCount( mId, i, d );
          size_t volumesCount = mDataset3DVolumeCount( mId, i, d );
          std::shared_ptr<DatasetDynamicDriver3D> dataset3D =
            std::make_shared<DatasetDynamicDriver3D>( group.get(), mId, i, d, volumesCount, maxVerticalLevelCount, mLibrary );

          if ( ! dataset3D->loadSymbol() )
            return false;

          dataset3D->setStatistics( MDAL::calculateStatistics( dataset3D ) );
          dataset3D->unloadData();
          dataset = dataset3D;
        }
        break;
      }

      bool ok = true;
      double time = mDatasetTimeFunction( mId, i, d, &ok );
      if ( !ok )
        return false;
      dataset->setTime( RelativeTimestamp( time, RelativeTimestamp::hours ) );

      group->datasets.push_back( dataset );
    }

    group->setStatistics( MDAL::calculateStatistics( group ) );
    datasetGroups.push_back( group );
  }
  return true;
}

bool MDAL::MeshDynamicDriver::loadSymbol()
{
  mMeshVertexCountFunction = mLibrary.getSymbol<int, int>( "MDAL_DRIVER_M_vertexCount" ) ;
  mMeshFaceCountFunction = mLibrary.getSymbol<int, int>( "MDAL_DRIVER_M_faceCount" ) ;
  mMeshEdgeCountFunction = mLibrary.getSymbol<int, int>( "MDAL_DRIVER_M_edgeCount" ) ;
  mMeshExtentFunction = mLibrary.getSymbol<void, int, double *, double *, double *, double *>( "MDAL_DRIVER_M_extent" );
  mMeshProjectionFunction = mLibrary.getSymbol<const char *, int>( "MDAL_DRIVER_M_projection" ) ;
  mMeshDatasetGroupsCountFunction = mLibrary.getSymbol<int, int>( "MDAL_DRIVER_M_datasetGroupCount" );
  mDatasetgroupNameFunction = mLibrary.getSymbol<const char *, int, int>( "MDAL_DRIVER_G_groupName" );
  mDatasetGroupReferencetimeFunction = mLibrary.getSymbol<const char *, int, int>( "MDAL_DRIVER_G_referenceTime" );
  mDatasetGroupMetadataCountFunction = mLibrary.getSymbol<int, int, int>( "MDAL_DRIVER_G_metadataCount" );
  mDatasetGroupMetadataKeyFunction = mLibrary.getSymbol<const char *, int, int, int>( "MDAL_DRIVER_G_metadataKey" );
  mDatasetGroupMetadataValueFunction = mLibrary.getSymbol<const char *, int, int, int>( "MDAL_DRIVER_G_metadataValue" );
  mDatasetTimeFunction = mLibrary.getSymbol<double, int, int, int, bool *>( "MDAL_DRIVER_D_time" );
  mDatasetDescriptionFunction = mLibrary.getSymbol<bool, int, int, bool *, int *, int *>( "MDAL_DRIVER_G_datasetsDescription" );
  mDatasetSupportActiveFlagFunction = mLibrary.getSymbol<bool, int, int, int>( "MDAL_DRIVER_D_hasActiveFlagCapability" );
  mDataset3DMaximumVerticalLevelCount = mLibrary.getSymbol<int, int, int, int>( "MDAL_DRIVER_D_maximumVerticalLevelCount" );
  mDataset3DVolumeCount = mLibrary.getSymbol<int, int, int, int>( "MDAL_DRIVER_D_volumeCount" );
  mCloseMeshFunction = mLibrary.getSymbol<void, int>( "MDAL_DRIVER_closeMesh" );

  if ( mMeshVertexCountFunction == nullptr ||
       mMeshFaceCountFunction == nullptr ||
       mMeshEdgeCountFunction == nullptr ||
       mMeshExtentFunction == nullptr ||
       mMeshProjectionFunction == nullptr ||
       mMeshDatasetGroupsCountFunction == nullptr ||
       mDatasetgroupNameFunction == nullptr ||
       mDatasetGroupReferencetimeFunction == nullptr ||
       mDatasetGroupMetadataCountFunction == nullptr ||
       mDatasetGroupMetadataKeyFunction == nullptr ||
       mDatasetGroupMetadataValueFunction == nullptr ||
       mDatasetDescriptionFunction == nullptr ||
       mDatasetTimeFunction == nullptr ||
       mDatasetSupportActiveFlagFunction == nullptr ||
       mCloseMeshFunction == nullptr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, driverName(), "Driver is not valid, unable to load mesh access functions" );
    return false;
  }

  return true;
}


std::unique_ptr<MDAL::MeshVertexIterator> MDAL::MeshDynamicDriver::readVertices()
{
  return std::unique_ptr<MeshVertexIteratorDynamicDriver>( new MeshVertexIteratorDynamicDriver( mLibrary, mId ) );
}

std::unique_ptr<MDAL::MeshEdgeIterator> MDAL::MeshDynamicDriver::readEdges()
{
  return std::unique_ptr<MeshEdgeIterator>( new MeshEdgeIteratorDynamicDriver( mLibrary, mId ) );
}

std::unique_ptr<MDAL::MeshFaceIterator> MDAL::MeshDynamicDriver::readFaces()
{
  return std::unique_ptr<MeshFaceIterator>( new MeshFaceIteratorDynamicDriver( mLibrary, mId ) );
}


MDAL::MeshVertexIteratorDynamicDriver::MeshVertexIteratorDynamicDriver( const Library &library, int meshId ):
  mLibrary( library ),
  mMeshId( meshId )
{}

size_t MDAL::MeshVertexIteratorDynamicDriver::next( size_t vertexCount, double *coordinates )
{
  if ( !mVerticesFunction )
  {
    mVerticesFunction = mLibrary.getSymbol<int, int, int, int, double *>( "MDAL_DRIVER_M_vertices" );
    if ( !mVerticesFunction )
      return 0;
  }

  int effectiveVerticesCount = mVerticesFunction( mMeshId, mPosition, MDAL::toInt( vertexCount ), coordinates );
  if ( effectiveVerticesCount < 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Invalid mesh, unable to read vertices" );
    return 0;
  }
  mPosition += effectiveVerticesCount;

  return effectiveVerticesCount;
}

MDAL::MeshFaceIteratorDynamicDriver::MeshFaceIteratorDynamicDriver( const MDAL::Library &library, int meshId ):
  mLibrary( library ),
  mMeshId( meshId )
{}

size_t MDAL::MeshFaceIteratorDynamicDriver::next( size_t faceOffsetsBufferLen, int *faceOffsetsBuffer, size_t vertexIndicesBufferLen, int *vertexIndicesBuffer )
{
  if ( !mFacesFunction )
  {
    mFacesFunction = mLibrary.getSymbol<int, int, int, int, int *, int, int *>( "MDAL_DRIVER_M_faces" );
    if ( !mFacesFunction )
      return 0;
  }

  int effectiveFacesCount = mFacesFunction( mMeshId, mPosition, MDAL::toInt( faceOffsetsBufferLen ), faceOffsetsBuffer, MDAL::toInt( vertexIndicesBufferLen ), vertexIndicesBuffer );
  if ( effectiveFacesCount < 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Invalid mesh, unable to read faces" );
    return 0;
  }

  mPosition += effectiveFacesCount;
  return effectiveFacesCount;
}

MDAL::MeshEdgeIteratorDynamicDriver::MeshEdgeIteratorDynamicDriver( const MDAL::Library &library, int meshId ):
  mLibrary( library ),
  mMeshId( meshId )
{}

size_t MDAL::MeshEdgeIteratorDynamicDriver::next( size_t edgeCount, int *startVertexIndices, int *endVertexIndices )
{
  if ( !mEdgesFunction )
  {
    mEdgesFunction = mLibrary.getSymbol<int, int, int, int, int *, int *>( "MDAL_DRIVER_M_edges" );
    if ( !mEdgesFunction )
      return 0;
  }

  int effectiveEdgesCount = mEdgesFunction( mMeshId, mPosition, MDAL::toInt( edgeCount ), startVertexIndices, endVertexIndices );

  if ( effectiveEdgesCount < 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Invalid mesh, unable to read edges" );
    return 0;
  }

  mPosition += effectiveEdgesCount;
  return effectiveEdgesCount;
}


MDAL::DatasetDynamicDriver::DatasetDynamicDriver( int meshId, int groupIndex, int datasetIndex, const MDAL::Library &library )
  : mMeshId( meshId )
  , mGroupIndex( groupIndex )
  , mDatasetIndex( datasetIndex )
  , mLibrary( library )
{}

MDAL::DatasetDynamicDriver::~DatasetDynamicDriver() = default;

MDAL::DatasetDynamicDriver2D::DatasetDynamicDriver2D( MDAL::DatasetGroup *parentGroup, int meshId, int groupIndex, int datasetIndex, const MDAL::Library &library )
  : Dataset2D( parentGroup )
  , DatasetDynamicDriver( meshId, groupIndex, datasetIndex, library )
{}

MDAL::DatasetDynamicDriver2D::~DatasetDynamicDriver2D() = default;


MDAL::DatasetDynamicDriver3D::DatasetDynamicDriver3D( MDAL::DatasetGroup *parentGroup, int meshId, int groupIndex, int datasetIndex, size_t volumes, size_t maxVerticalLevelCount, const MDAL::Library &library )
  : Dataset3D( parentGroup, volumes, maxVerticalLevelCount )
  , DatasetDynamicDriver( meshId, groupIndex, datasetIndex, library )
{}

MDAL::DatasetDynamicDriver3D::~DatasetDynamicDriver3D() = default;

size_t MDAL::DatasetDynamicDriver3D::verticalLevelCountData( size_t indexStart, size_t count, int *buffer )
{
  if ( !mVerticalLevelCountDataFunction )
    return 0;

  return mVerticalLevelCountDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

size_t MDAL::DatasetDynamicDriver3D::verticalLevelData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mVerticalLevelDataFunction )
    return 0;

  return mVerticalLevelDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

size_t MDAL::DatasetDynamicDriver3D::faceToVolumeData( size_t indexStart, size_t count, int *buffer )
{
  if ( !mFaceToVolumeDataFunction )
    return 0;

  return mFaceToVolumeDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

size_t MDAL::DatasetDynamicDriver3D::scalarVolumesData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataFunction )
    return 0;

  return mDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

size_t MDAL::DatasetDynamicDriver3D::vectorVolumesData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataFunction )
    return 0;

  return mDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}


size_t MDAL::DatasetDynamicDriver2D::scalarData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataFunction )
    return 0;

  return mDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

size_t MDAL::DatasetDynamicDriver2D::vectorData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataFunction )
    return 0;

  return mDataFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

size_t MDAL::DatasetDynamicDriver2D::activeData( size_t indexStart, size_t count, int *buffer )
{
  if ( !supportsActiveFlag() )
    return Dataset2D::activeData( indexStart, count, buffer );

  if ( !mActiveFlagsFunction )
    return 0;

  return mActiveFlagsFunction( mMeshId, mGroupIndex, mDatasetIndex, MDAL::toInt( indexStart ), MDAL::toInt( count ), buffer );
}

bool MDAL::DatasetDynamicDriver::loadSymbol()
{
  mDataFunction = mLibrary.getSymbol<int, int, int, int, int, int, double *>( "MDAL_DRIVER_D_data" );
  mUnloadFunction = mLibrary.getSymbol<void, int, int, int>( "MDAL_DRIVER_D_unload" );

  if ( mDataFunction == nullptr ||
       mUnloadFunction == nullptr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid" );
    return false;
  }

  return true;
}

bool MDAL::DatasetDynamicDriver2D::loadSymbol()
{
  if ( !MDAL::DatasetDynamicDriver::loadSymbol() )
    return false;

  if ( supportsActiveFlag() )
    mActiveFlagsFunction = mLibrary.getSymbol<int, int, int, int, int, int, int *>( "MDAL_DRIVER_D_activeFlags" );

  if ( supportsActiveFlag() && mActiveFlagsFunction == nullptr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid" );
    return false;
  }

  return true;
}


bool MDAL::DatasetDynamicDriver3D::loadSymbol()
{
  if ( !MDAL::DatasetDynamicDriver::loadSymbol() )
    return false;

  mVerticalLevelCountDataFunction = mLibrary.getSymbol<int, int, int, int, int, int, int *>( "MDAL_DRIVER_D_verticalLevelCountData" );
  mVerticalLevelDataFunction = mLibrary.getSymbol<int, int, int, int, int, int, double *>( "MDAL_DRIVER_D_verticalLevelData" );
  mFaceToVolumeDataFunction = mLibrary.getSymbol<int, int, int, int, int, int, int *>( "MDAL_DRIVER_D_faceToVolumeData" );

  if ( mVerticalLevelCountDataFunction == nullptr ||
       mVerticalLevelDataFunction == nullptr ||
       mFaceToVolumeDataFunction == nullptr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid" );
    return false;
  }

  return true;
}

void MDAL::DatasetDynamicDriver::unloadData()
{
  if ( !mUnloadFunction )
    return;

  mUnloadFunction( mMeshId, mGroupIndex, mDatasetIndex );
}

