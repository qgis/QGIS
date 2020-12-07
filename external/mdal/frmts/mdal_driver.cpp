/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
*/

#include <string.h>
#include "mdal_driver.hpp"
#include "mdal_utils.hpp"
#include "mdal_memory_data_model.hpp"

MDAL::Driver::Driver( const std::string &name,
                      const std::string &longName,
                      const std::string &filters,
                      int capabilityFlags )
  : mName( name )
  , mLongName( longName )
  , mFilters( filters )
  , mCapabilityFlags( capabilityFlags )
{
}

MDAL::Driver::~Driver() = default;

std::string MDAL::Driver::name() const
{
  return mName;
}

std::string MDAL::Driver::longName() const
{
  return mLongName;
}

std::string MDAL::Driver::filters() const
{
  return mFilters;
}

std::string MDAL::Driver::writeDatasetOnFileSuffix() const
{

  return std::string();
}

bool MDAL::Driver::hasCapability( MDAL::Capability capability ) const
{
  return capability == ( mCapabilityFlags & capability );
}

bool MDAL::Driver::canReadMesh( const std::string & ) { return false; }

bool MDAL::Driver::canReadDatasets( const std::string & ) { return false; }

bool MDAL::Driver::hasWriteDatasetCapability( MDAL_DataLocation location ) const
{
  switch ( location )
  {
    case MDAL_DataLocation::DataOnVertices:
      return hasCapability( MDAL::Capability::WriteDatasetsOnVertices );
    case MDAL_DataLocation::DataOnFaces:
      return hasCapability( MDAL::Capability::WriteDatasetsOnFaces );
    case MDAL_DataLocation::DataOnVolumes:
      return hasCapability( MDAL::Capability::WriteDatasetsOnVolumes );
    case MDAL_DataLocation::DataOnEdges:
      return hasCapability( MDAL::Capability::WriteDatasetsOnEdges );
    default:
      return false;
  }
}

int MDAL::Driver::faceVerticesMaximumCount() const { return -1; }

std::string MDAL::Driver::buildUri( const std::string &meshFile )
{
  return MDAL::buildMeshUri( meshFile, "", this->name() );
}

std::unique_ptr< MDAL::Mesh > MDAL::Driver::load( const std::string &, const std::string & ) { return std::unique_ptr< MDAL::Mesh >(); }

void MDAL::Driver::load( const std::string &, Mesh * ) {}

void MDAL::Driver::save( const std::string &, MDAL::Mesh * ) {}

void MDAL::Driver::createDatasetGroup( MDAL::Mesh *mesh, const std::string &groupName, MDAL_DataLocation dataLocation, bool hasScalarData, const std::string &datasetGroupFile )
{
  std::shared_ptr<MDAL::DatasetGroup> grp(
    new MDAL::DatasetGroup( name(),
                            mesh,
                            datasetGroupFile )
  );
  grp->setName( groupName );
  grp->setDataLocation( dataLocation );
  grp->setIsScalar( hasScalarData );
  grp->startEditing();
  mesh->datasetGroups.push_back( grp );
}

void MDAL::Driver::createDataset( MDAL::DatasetGroup *group, MDAL::RelativeTimestamp time, const double *values, const int *active )
{
  bool supportsActiveFlag = ( active != nullptr );
  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MemoryDataset2D >( group, supportsActiveFlag );
  dataset->setTime( time );
  size_t count = dataset->valuesCount();

  if ( !group->isScalar() )
    count *= 2;

  memcpy( dataset->values(), values, sizeof( double ) * count );
  if ( dataset->supportsActiveFlag() )
    dataset->setActive( active );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
}

bool MDAL::Driver::persist( MDAL::DatasetGroup * ) { return true; } // failure
