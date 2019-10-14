/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_config.hpp"
#include "mdal_driver_manager.hpp"
#include "frmts/mdal_2dm.hpp"
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
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
#include "frmts/mdal_gdal_netcdf.hpp"
#endif

#if defined HAVE_HDF5 && defined HAVE_XML
#include "frmts/mdal_xdmf.hpp"
#endif

std::unique_ptr<MDAL::Mesh> MDAL::DriverManager::load( const std::string &meshFile, MDAL_Status *status ) const
{
  std::unique_ptr<MDAL::Mesh> mesh;

  if ( !MDAL::fileExists( meshFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return std::unique_ptr<MDAL::Mesh>();
  }

  for ( const auto &driver : mDrivers )
  {
    if ( ( driver->hasCapability( Capability::ReadMesh ) ) &&
         driver->canRead( meshFile ) )
    {
      std::unique_ptr<Driver> drv( driver->create() );
      mesh = drv->load( meshFile, status );
      if ( mesh ) // stop if he have the mesh
        break;
    }
  }

  if ( status && !mesh )
    *status = MDAL_Status::Err_UnknownFormat;

  return mesh;
}

void MDAL::DriverManager::loadDatasets( Mesh *mesh, const std::string &datasetFile, MDAL_Status *status ) const
{
  if ( !MDAL::fileExists( datasetFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return;
  }

  if ( !mesh )
  {
    if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
    return;
  }

  for ( const auto &driver : mDrivers )
  {
    if ( driver->hasCapability( Capability::ReadDatasets ) &&
         driver->canRead( datasetFile ) )
    {
      std::unique_ptr<Driver> drv( driver->create() );
      drv->load( datasetFile, mesh, status );
      return;
    }
  }

  if ( status )
    *status = MDAL_Status::Err_UnknownFormat;
}

void MDAL::DriverManager::save( MDAL::Mesh *mesh, const std::string &uri, const std::string &driverName, MDAL_Status *status ) const
{
  auto selectedDriver = driver( driverName );

  std::unique_ptr<Driver> drv( selectedDriver->create() );

  drv->save( uri, mesh, status );
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
  mDrivers.push_back( std::make_shared<MDAL::DriverSelafin>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverEsriTin>() );

#ifdef HAVE_HDF5
  mDrivers.push_back( std::make_shared<MDAL::DriverFlo2D>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverHec2D>() );
#endif

#ifdef HAVE_NETCDF
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

