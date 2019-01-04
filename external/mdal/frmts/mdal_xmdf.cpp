/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include "mdal_xmdf.hpp"

#include "mdal_utils.hpp"
#include "mdal_data_model.hpp"
#include "mdal_hdf5.hpp"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

MDAL::XmdfDataset::~XmdfDataset() = default;

MDAL::XmdfDataset::XmdfDataset( DatasetGroup *grp, const HdfDataset &valuesDs, const HdfDataset &activeDs, hsize_t timeIndex )
  : Dataset( grp )
  , mHdf5DatasetValues( valuesDs )
  , mHdf5DatasetActive( activeDs )
  , mTimeIndex( timeIndex )
{
}

const HdfDataset &MDAL::XmdfDataset::dsValues() const
{
  return mHdf5DatasetValues;
}

const HdfDataset &MDAL::XmdfDataset::dsActive() const
{
  return mHdf5DatasetActive;
}

hsize_t MDAL::XmdfDataset::timeIndex() const
{
  return mTimeIndex;
}


size_t MDAL::XmdfDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  std::vector<hsize_t> offsets = {timeIndex(), indexStart};
  std::vector<hsize_t> counts = {1, count};
  std::vector<float> values = dsValues().readArray( offsets, counts );
  const float *input = values.data();
  for ( size_t j = 0; j < count; ++j )
  {
    buffer[j] = double( input[j] );
  }
  return count;
}

size_t MDAL::XmdfDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  std::vector<hsize_t> offsets = {timeIndex(), indexStart, 0};
  std::vector<hsize_t> counts = {1, count, 2};
  std::vector<float> values = dsValues().readArray( offsets, counts );
  const float *input = values.data();
  for ( size_t j = 0; j < count; ++j )
  {
    buffer[2 * j] = double( input[2 * j] );
    buffer[2 * j + 1] = double( input[2 * j + 1] );
  }

  return count;
}

size_t MDAL::XmdfDataset::activeData( size_t indexStart, size_t count, int *buffer )
{
  std::vector<hsize_t> offsets = {timeIndex(), indexStart};
  std::vector<hsize_t> counts = {1, count};
  std::vector<uchar> active = dsActive().readArrayUint8( offsets, counts );
  const uchar *input = active.data();
  for ( size_t j = 0; j < count; ++j )
  {
    buffer[j] = bool( input[ j ] );
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////////////

MDAL::DriverXmdf::DriverXmdf()
  : Driver( "XMDF",
            "TUFLOW XMDF",
            "*.xmdf",
            Capability::ReadDatasets )
{
}

MDAL::DriverXmdf *MDAL::DriverXmdf::create()
{
  return new DriverXmdf();
}

bool MDAL::DriverXmdf::canRead( const std::string &uri )
{
  HdfFile file( uri );
  if ( !file.isValid() )
  {
    return false;
  }

  HdfDataset dsFileType = file.dataset( "/File Type" );
  if ( dsFileType.readString() != "Xmdf" )
  {
    return false;
  }

  return true;
}

void MDAL::DriverXmdf::load( const std::string &datFile,  MDAL::Mesh *mesh, MDAL_Status *status )
{
  mDatFile = datFile;
  mMesh = mesh;
  if ( status ) *status = MDAL_Status::None;

  HdfFile file( mDatFile );
  if ( !file.isValid() )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return;
  }

  HdfDataset dsFileType = file.dataset( "/File Type" );
  if ( dsFileType.readString() != "Xmdf" )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return;
  }

  // TODO: check version?

  size_t vertexCount = mesh->verticesCount();
  size_t faceCount = mesh->facesCount();
  std::vector<std::string> rootGroups = file.groups();
  if ( rootGroups.size() != 1 )
  {
    MDAL::debug( "Expecting exactly one root group for the mesh data" );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return;
  }
  HdfGroup gMesh = file.group( rootGroups[0] );

  // TODO: read Times group (e.g. time of peak velocity)

  DatasetGroups groups; // DAT outputs data

  if ( gMesh.pathExists( "Temporal" ) )
  {
    HdfGroup gTemporal = gMesh.group( "Temporal" );
    if ( gTemporal.isValid() )
    {
      addDatasetGroupsFromXmdfGroup( groups, gTemporal, vertexCount, faceCount );
    }
  }

  if ( gMesh.pathExists( "Temporal" ) )
  {
    HdfGroup gMaximums = gMesh.group( "Maximums" );
    if ( gMaximums.isValid() )
    {
      for ( const std::string &groupName : gMaximums.groups() )
      {
        HdfGroup g = gMaximums.group( groupName );
        std::shared_ptr<MDAL::DatasetGroup> maxGroup = readXmdfGroupAsDatasetGroup( g, groupName + "/Maximums", vertexCount, faceCount );
        if ( !maxGroup || maxGroup->datasets.size() != 1 )
          MDAL::debug( "Maximum dataset should have just one timestep!" );
        else
          groups.push_back( maxGroup );
      }
    }
  }

  // res_to_res.exe (TUFLOW utiity tool)
  if ( gMesh.pathExists( "Difference" ) )
  {
    HdfGroup gDifference = gMesh.group( "Difference" );
    if ( gDifference.isValid() )
    {
      addDatasetGroupsFromXmdfGroup( groups, gDifference, vertexCount, faceCount );
    }
  }

  mesh->datasetGroups.insert(
    mesh->datasetGroups.end(),
    groups.begin(),
    groups.end()
  );
}

void MDAL::DriverXmdf::addDatasetGroupsFromXmdfGroup( DatasetGroups &groups, const HdfGroup &rootGroup, size_t vertexCount, size_t faceCount )
{
  for ( const std::string &groupName : rootGroup.groups() )
  {
    HdfGroup g = rootGroup.group( groupName );
    std::shared_ptr<DatasetGroup> ds = readXmdfGroupAsDatasetGroup( g, groupName, vertexCount, faceCount );
    if ( ds && ds->datasets.size() > 0 )
      groups.push_back( ds );
  }
}


std::shared_ptr<MDAL::DatasetGroup> MDAL::DriverXmdf::readXmdfGroupAsDatasetGroup(
  const HdfGroup &rootGroup, const std::string &groupName, size_t vertexCount, size_t faceCount )
{
  std::shared_ptr<DatasetGroup> group;
  std::vector<std::string> gDataNames = rootGroup.datasets();
  if ( !MDAL::contains( gDataNames, "Times" ) ||
       !MDAL::contains( gDataNames, "Values" ) ||
       !MDAL::contains( gDataNames, "Active" ) ||
       !MDAL::contains( gDataNames, "Mins" ) ||
       !MDAL::contains( gDataNames, "Maxs" ) )
  {
    MDAL::debug( "ignoring dataset " + groupName + " - not having required arrays" );
    return group;
  }

  HdfDataset dsTimes = rootGroup.dataset( "Times" );
  HdfDataset dsValues = rootGroup.dataset( "Values" );
  HdfDataset dsActive = rootGroup.dataset( "Active" );
  HdfDataset dsMins = rootGroup.dataset( "Mins" );
  HdfDataset dsMaxs = rootGroup.dataset( "Maxs" );

  std::vector<hsize_t> dimTimes = dsTimes.dims();
  std::vector<hsize_t> dimValues = dsValues.dims();
  std::vector<hsize_t> dimActive = dsActive.dims();
  std::vector<hsize_t> dimMins = dsMins.dims();
  std::vector<hsize_t> dimMaxs = dsMaxs.dims();

  if ( dimTimes.size() != 1 ||
       ( dimValues.size() != 2 && dimValues.size() != 3 ) ||
       dimActive.size() != 2 ||
       dimMins.size() != 1 ||
       dimMaxs.size() != 1
     )
  {
    MDAL::debug( "ignoring dataset " + groupName + " - arrays not having correct dimension counts" );
    return group;
  }
  hsize_t nTimeSteps = dimTimes[0];

  if ( dimValues[0] != nTimeSteps ||
       dimActive[0] != nTimeSteps ||
       dimMins[0] != nTimeSteps ||
       dimMaxs[0] != nTimeSteps )
  {
    MDAL::debug( "ignoring dataset " + groupName + " - arrays not having correct dimension sizes" );
    return group;
  }
  if ( dimValues[1] != vertexCount || dimActive[1] != faceCount )
  {
    MDAL::debug( "ignoring dataset " + groupName + " - not aligned with the used mesh" );
    return group;
  }

  bool isVector = dimValues.size() == 3;

  std::vector<float> times = dsTimes.readArray();

  // all fine, set group and return
  group = std::make_shared<MDAL::DatasetGroup>(
            name(),
            mMesh,
            mDatFile,
            groupName
          );
  group->setIsScalar( !isVector );
  group->setIsOnVertices( true );

  // lazy loading of min and max of the dataset group
  std::vector<float> mins = dsMins.readArray();
  std::vector<float> maxs = dsMaxs.readArray();
  Statistics grpStats;
  grpStats.minimum = static_cast<double>( *std::min_element( mins.begin(), mins.end() ) );
  grpStats.maximum = static_cast<double>( *std::max_element( maxs.begin(), maxs.end() ) );
  group->setStatistics( grpStats );

  for ( hsize_t i = 0; i < nTimeSteps; ++i )
  {
    std::shared_ptr<XmdfDataset> dataset = std::make_shared< XmdfDataset >( group.get(), dsValues, dsActive, i );
    dataset->setTime( double( times[i] ) );
    Statistics stats;
    stats.minimum = static_cast<double>( mins[i] );
    stats.maximum = static_cast<double>( maxs[i] );
    dataset->setStatistics( stats );
    group->datasets.push_back( dataset );
  }

  return group;
}
