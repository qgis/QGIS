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


MDAL::LoaderXmdf::LoaderXmdf( const std::string &datFile )
  : mDatFile( datFile )
{}

void MDAL::LoaderXmdf::load( MDAL::Mesh *mesh, MDAL_Status *status )
{
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

  size_t vertexCount = mesh->vertices.size();
  size_t faceCount = mesh->faces.size();
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

  HdfGroup gTemporal = gMesh.group( "Temporal" );
  if ( gTemporal.isValid() )
  {
    addDatasetGroupsFromXmdfGroup( groups, gTemporal, vertexCount, faceCount );
  }

  HdfGroup gMaximums = gMesh.group( "Maximums" );
  if ( gMaximums.isValid() )
  {
    for ( const std::string &name : gMaximums.groups() )
    {
      HdfGroup g = gMaximums.group( name );
      std::shared_ptr<MDAL::DatasetGroup> maxGroup = readXmdfGroupAsDatasetGroup( g, name + "/Maximums", vertexCount, faceCount );
      if ( maxGroup->datasets.size() != 1 )
        MDAL::debug( "Maximum dataset should have just one timestep!" );
      else
        groups.push_back( maxGroup );
    }
  }

  // res_to_res.exe (TUFLOW utiity tool)
  HdfGroup gDifference = gMesh.group( "Difference" );
  if ( gDifference.isValid() )
  {
    addDatasetGroupsFromXmdfGroup( groups, gDifference, vertexCount, faceCount );
  }

  mesh->datasetGroups.insert(
    mesh->datasetGroups.end(),
    groups.begin(),
    groups.end()
  );
}

void MDAL::LoaderXmdf::addDatasetGroupsFromXmdfGroup( DatasetGroups &groups, const HdfGroup &rootGroup, size_t vertexCount, size_t faceCount )
{
  for ( const std::string &name : rootGroup.groups() )
  {
    HdfGroup g = rootGroup.group( name );
    std::shared_ptr<DatasetGroup> ds = readXmdfGroupAsDatasetGroup( g, name, vertexCount, faceCount );
    groups.push_back( ds );
  }
}

std::shared_ptr<MDAL::DatasetGroup> MDAL::LoaderXmdf::readXmdfGroupAsDatasetGroup(
  const HdfGroup &rootGroup, const std::string &name, size_t vertexCount, size_t faceCount )
{
  std::shared_ptr<DatasetGroup> group( new DatasetGroup() );
  std::vector<std::string> gDataNames = rootGroup.datasets();
  if ( !MDAL::contains( gDataNames, "Times" ) ||
       !MDAL::contains( gDataNames, "Values" ) ||
       !MDAL::contains( gDataNames, "Active" ) )
  {
    MDAL::debug( "ignoring dataset " + name + " - not having required arrays" );
    return group;
  }

  HdfDataset dsTimes = rootGroup.dataset( "Times" );
  HdfDataset dsValues = rootGroup.dataset( "Values" );
  HdfDataset dsActive = rootGroup.dataset( "Active" );

  std::vector<hsize_t> dimTimes = dsTimes.dims();
  std::vector<hsize_t> dimValues = dsValues.dims();
  std::vector<hsize_t> dimActive = dsActive.dims();

  if ( dimTimes.size() != 1 || ( dimValues.size() != 2 && dimValues.size() != 3 ) || dimActive.size() != 2 )
  {
    MDAL::debug( "ignoring dataset " + name + " - arrays not having correct dimension counts" );
    return group;
  }
  hsize_t nTimeSteps = dimTimes[0];

  if ( dimValues[0] != nTimeSteps || dimActive[0] != nTimeSteps )
  {
    MDAL::debug( "ignoring dataset " + name + " - arrays not having correct dimension sizes" );
    return group;
  }
  if ( dimValues[1] != vertexCount || dimActive[1] != faceCount )
  {
    MDAL::debug( "ignoring dataset " + name + " - not aligned with the used mesh" );
    return group;
  }

  bool isVector = dimValues.size() == 3;

  std::vector<float> times = dsTimes.readArray();
  std::vector<float> values = dsValues.readArray();
  std::vector<uchar> active = dsActive.readArrayUint8();

  group->setName( name );
  group->isScalar = !isVector;
  group->isOnVertices = true;
  group->uri = mDatFile;
  for ( hsize_t i = 0; i < nTimeSteps; ++i )
  {
    std::shared_ptr<Dataset> dataset( new Dataset() );
    dataset->values.resize( vertexCount );
    dataset->active.resize( faceCount );
    dataset->parent = group.get();
    dataset->time = double( times[i] );

    if ( isVector )
    {
      const float *input = values.data() + 2 * i * vertexCount;
      for ( size_t j = 0; j < vertexCount; ++j )
      {
        dataset->values[j].x = double( input[2 * j] );
        dataset->values[j].y = double( input[2 * j + 1] );
      }
    }
    else
    {
      const float *input = values.data() + i * vertexCount;
      for ( size_t j = 0; j < vertexCount; ++j )
      {
        dataset->values[j].x = double( input[j] );
      }
    }

    const uchar *input = active.data() + i * faceCount;
    for ( size_t j = 0; j < faceCount; ++j )
    {
      dataset->active[j] = input[j];
    }
    group->datasets.push_back( dataset );
  }

  return group;
}
