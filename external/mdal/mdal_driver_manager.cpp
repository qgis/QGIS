/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_config.hpp"
#include "mdal_driver_manager.hpp"
#include "frmts/mdal_2dm.hpp"
#include "frmts/mdal_xms_tin.hpp"
#include "frmts/mdal_ascii_dat.hpp"
#include "frmts/mdal_binary_dat.hpp"
#include "frmts/mdal_selafin.hpp"
#include "frmts/mdal_esri_tin.hpp"
#include "mdal_utils.hpp"

#ifdef HAVE_HDF5
#include "frmts/mdal_xmdf.hpp"
#include "frmts/mdal_flo2d.hpp"
#include "frmts/mdal_hec2d.hpp"
#endif

#ifdef HAVE_GDAL
#include "frmts/mdal_gdal_grib.hpp"
#endif

#ifdef HAVE_NETCDF
#include "frmts/mdal_ugrid.hpp"
#include "frmts/mdal_3di.hpp"
#include "frmts/mdal_sww.hpp"
#include "frmts/mdal_tuflowfv.hpp"
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
#include "frmts/mdal_gdal_netcdf.hpp"
#endif

#if defined HAVE_HDF5 && defined HAVE_XML
#include "frmts/mdal_xdmf.hpp"
#endif

std::unique_ptr<MDAL::Mesh> MDAL::DriverManager::load( const std::string &meshFile ) const
{
  std::unique_ptr<MDAL::Mesh> mesh;

  if ( !MDAL::fileExists( meshFile ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "File " + meshFile + " could not be found" );
    return std::unique_ptr<MDAL::Mesh>();
  }

  for ( const auto &driver : mDrivers )
  {
    if ( ( driver->hasCapability( Capability::ReadMesh ) ) &&
         driver->canReadMesh( meshFile ) )
    {
      std::unique_ptr<Driver> drv( driver->create() );
      mesh = drv->load( meshFile );
      if ( mesh ) // stop if he have the mesh
        break;
    }
  }

  if ( !mesh )
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, "Unable to load mesh (null)" );

  return mesh;
}

void MDAL::DriverManager::loadDatasets( Mesh *mesh, const std::string &datasetFile ) const
{
  if ( !MDAL::fileExists( datasetFile ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "File " + datasetFile + " could not be found" );
    return;
  }

  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return;
  }

  for ( const auto &driver : mDrivers )
  {
    if ( driver->hasCapability( Capability::ReadDatasets ) &&
         driver->canReadDatasets( datasetFile ) )
    {
      std::unique_ptr<Driver> drv( driver->create() );
      drv->load( datasetFile, mesh );
      return;
    }
  }

  MDAL::Log::error( MDAL_Status::Err_UnknownFormat, "No driver was able to load requested file: " + datasetFile );
}

void MDAL::DriverManager::save( MDAL::Mesh *mesh, const std::string &uri, const std::string &driverName ) const
{
  auto selectedDriver = driver( driverName );

  std::unique_ptr<Driver> drv( selectedDriver->create() );

  drv->save( uri, mesh );
}

size_t MDAL::DriverManager::driversCount() const
{
  return mDrivers.size();
}

std::shared_ptr<MDAL::Driver> MDAL::DriverManager::driver( size_t index ) const
{
  if ( mDrivers.size() <= index )
  {
    return std::shared_ptr<MDAL::Driver>();
  }
  else
  {
    return mDrivers[index];
  }
}

std::shared_ptr<MDAL::Driver> MDAL::DriverManager::driver( const std::string &driverName ) const
{
  for ( const auto &dr : mDrivers )
  {
    if ( dr->name() == driverName )
      return dr;
  }
  return std::shared_ptr<MDAL::Driver>();
}

MDAL::DriverManager::DriverManager()
{
  // MESH DRIVERS
  mDrivers.push_back( std::make_shared<MDAL::Driver2dm>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverXmsTin>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverSelafin>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverEsriTin>() );

#ifdef HAVE_HDF5
  mDrivers.push_back( std::make_shared<MDAL::DriverFlo2D>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverHec2D>() );
#endif

#ifdef HAVE_NETCDF
  mDrivers.push_back( std::make_shared<MDAL::DriverTuflowFV>() );
  mDrivers.push_back( std::make_shared<MDAL::Driver3Di>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverSWW>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverUgrid>() );
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
  mDrivers.push_back( std::make_shared<MDAL::DriverGdalNetCDF>() );
#endif

#ifdef HAVE_GDAL
  mDrivers.push_back( std::make_shared<MDAL::DriverGdalGrib>() );
#endif

  // DATASET DRIVERS
  mDrivers.push_back( std::make_shared<MDAL::DriverAsciiDat>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverBinaryDat>() );
#ifdef HAVE_HDF5
  mDrivers.push_back( std::make_shared<MDAL::DriverXmdf>() );
#endif

#if defined HAVE_HDF5 && defined HAVE_XML
  mDrivers.push_back( std::make_shared<MDAL::DriverXdmf>() );
#endif
}

