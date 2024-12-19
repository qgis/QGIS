/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include "mdal_xmdf.hpp"

#include "mdal_utils.hpp"
#include "mdal_data_model.hpp"
#include "mdal_hdf5.hpp"
#include "mdal_logger.hpp"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

MDAL::XmdfDataset::~XmdfDataset() = default;

MDAL::XmdfDataset::XmdfDataset( DatasetGroup *grp, const HdfDataset &valuesDs, const HdfDataset &activeDs, hsize_t timeIndex )
  : Dataset2D( grp )
  , mHdf5DatasetValues( valuesDs )
  , mHdf5DatasetActive( activeDs )
  , mTimeIndex( timeIndex )
{
  setSupportsActiveFlag( true );
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
  if ( !dsActive().isValid() )
    return 0;
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

bool MDAL::DriverXmdf::canReadDatasets( const std::string &uri )
{
  HdfFile file( uri, HdfFile::ReadOnly );
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


void MDAL::DriverXmdf::readGroupsTree( HdfFile &file, const std::string &name, MDAL::DatasetGroups &groups, size_t vertexCount, size_t faceCount ) const
{
  HdfGroup gMesh = file.group( name );
  for ( const std::string &groupName : gMesh.groups() )
  {
    HdfGroup gGroup = gMesh.group( groupName );
    if ( gGroup.isValid() )
    {
      if ( groupName == "Maximums" )
      {
        addDatasetGroupsFromXmdfGroup( groups, gGroup, "/Maximums", vertexCount, faceCount );
      }
      else if ( groupName == "Final" )
      {
        addDatasetGroupsFromXmdfGroup( groups, gGroup, "/Final", vertexCount, faceCount );
      }
      else
      {
        addDatasetGroupsFromXmdfGroup( groups, gGroup, "", vertexCount, faceCount );
      }
    }
  }
}

void MDAL::DriverXmdf::load( const std::string &datFile,  MDAL::Mesh *mesh )
{
  mDatFile = datFile;
  mMesh = mesh;
  MDAL::Log::resetLastStatus();

  HdfFile file( mDatFile, HdfFile::ReadOnly );
  if ( !file.isValid() )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "File " + mDatFile + " is not valid" );
    return;
  }

  HdfDataset dsFileType = file.dataset( "/File Type" );
  if ( dsFileType.readString() != "Xmdf" )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "Unknown dataset file type" );
    return;
  }

  // TODO: check version?

  size_t vertexCount = mesh->verticesCount();
  size_t faceCount = mesh->facesCount();

  std::vector<std::string> rootGroups = file.groups();
  if ( rootGroups.empty() )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "Expecting at least one root group for the mesh data" );
    return;
  }

  DatasetGroups groups; // DAT outputs data

  for ( std::string &name : rootGroups )
  {
    HdfGroup rootGroup = file.group( name );
    if ( rootGroup.groups().size() > 0 )
      readGroupsTree( file, name, groups, vertexCount, faceCount );
    else
    {
      std::shared_ptr<DatasetGroup> ds = readXmdfGroupAsDatasetGroup( rootGroup, name, vertexCount, faceCount );
      if ( ds && ds->datasets.size() > 0 )
      {
        groups.push_back( ds );
      }
    }
  }

  mesh->datasetGroups.insert(
    mesh->datasetGroups.end(),
    groups.begin(),
    groups.end()
  );
}

void MDAL::DriverXmdf::addDatasetGroupsFromXmdfGroup( DatasetGroups &groups,
    const HdfGroup &rootGroup,
    const std::string &nameSuffix,
    size_t vertexCount,
    size_t faceCount ) const
{
  for ( const std::string &groupName : rootGroup.groups() )
  {
    HdfGroup g = rootGroup.group( groupName );
    std::shared_ptr<DatasetGroup> ds = readXmdfGroupAsDatasetGroup( g, groupName + nameSuffix, vertexCount, faceCount );
    if ( ds && ds->datasets.size() > 0 )
    {
      groups.push_back( ds );
    }
  }
}


std::shared_ptr<MDAL::DatasetGroup> MDAL::DriverXmdf::readXmdfGroupAsDatasetGroup(
  const HdfGroup &rootGroup, const std::string &groupName, size_t vertexCount, size_t faceCount ) const
{
  std::shared_ptr<DatasetGroup> group;
  std::vector<std::string> gDataNames = rootGroup.datasets();
  if ( !MDAL::contains( gDataNames, "Times" ) ||
       !MDAL::contains( gDataNames, "Values" ) ||
       !MDAL::contains( gDataNames, "Mins" ) ||
       !MDAL::contains( gDataNames, "Maxs" ) )
  {
    MDAL::Log::debug( "ignoring dataset " + groupName + " - not having required arrays" );
    return group;
  }

  bool activeFlagSupported = MDAL::contains( gDataNames, "Active" );

  HdfDataset dsTimes = rootGroup.dataset( "Times" );
  HdfDataset dsValues = rootGroup.dataset( "Values" );
  HdfDataset dsMins = rootGroup.dataset( "Mins" );
  HdfDataset dsMaxs = rootGroup.dataset( "Maxs" );

  std::vector<hsize_t> dimTimes = dsTimes.dims();
  std::vector<hsize_t> dimValues = dsValues.dims();
  std::vector<hsize_t> dimMins = dsMins.dims();
  std::vector<hsize_t> dimActive;
  std::vector<hsize_t> dimMaxs = dsMaxs.dims();

  HdfDataset dsActive;
  if ( activeFlagSupported )
  {
    dsActive = rootGroup.dataset( "Active" );
    dimActive = dsActive.dims();
  }

  if ( dimTimes.size() != 1 ||
       ( dimValues.size() != 2 && dimValues.size() != 3 ) ||
       ( activeFlagSupported && dimActive.size() != 2 ) ||
       dimMins.size() != 1 ||
       dimMaxs.size() != 1
     )
  {
    MDAL::Log::debug( "ignoring dataset " + groupName + " - arrays not having correct dimension counts" );
    return group;
  }
  hsize_t nTimeSteps = dimTimes[0];

  if ( dimValues[0] != nTimeSteps ||
       ( activeFlagSupported && dimActive[0] != nTimeSteps ) ||
       dimMins[0] != nTimeSteps ||
       dimMaxs[0] != nTimeSteps )
  {
    MDAL::Log::debug( "ignoring dataset " + groupName + " - arrays not having correct dimension sizes" );
    return group;
  }

  if ( dimValues[1] != vertexCount || ( activeFlagSupported && dimActive[1] != faceCount ) )
  {
    MDAL::Log::debug( "ignoring dataset " + groupName + " - not aligned with the used mesh" );
    return group;
  }

  // all fine, set group and return
  group = std::make_shared<MDAL::DatasetGroup>(
            name(),
            mMesh,
            mDatFile,
            groupName
          );

  bool isVector = dimValues.size() == 3;
  group->setIsScalar( !isVector );
  group->setDataLocation( MDAL_DataLocation::DataOnVertices );
  std::vector<double> times = dsTimes.readArrayDouble();
  std::string timeUnitString = rootGroup.attribute( "TimeUnits" ).readString();
  MDAL::RelativeTimestamp::Unit timeUnit = parseDurationTimeUnit( timeUnitString );
  HdfAttribute refTime = rootGroup.attribute( "Reftime" );
  group->setMetadata( "TIMEUNITS", timeUnitString );

  if ( refTime.isValid() )
  {
    std::string referenceTimeJulianDay = rootGroup.attribute( "Reftime" ).readString();
    double refTime;
    if ( referenceTimeJulianDay.empty() )
      refTime = rootGroup.attribute( "Reftime" ).readDouble();
    else
      refTime = MDAL::toDouble( referenceTimeJulianDay );

    if ( ! std::isnan( refTime ) )
      group->setReferenceTime( DateTime( refTime, DateTime::JulianDay ) );
  }

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
    dataset->setTime( times[i], timeUnit );
    dataset->setSupportsActiveFlag( activeFlagSupported );
    Statistics stats;
    stats.minimum = static_cast<double>( mins[i] );
    stats.maximum = static_cast<double>( maxs[i] );
    dataset->setStatistics( stats );
    group->datasets.push_back( dataset );
  }

  return group;
}
