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
#include "frmts/mdal_dynamic_driver.hpp"
#include "mdal_utils.hpp"

#ifdef BUILD_PLY
#include "frmts/mdal_ply.hpp"
#endif

#ifdef HAVE_HDF5
#include "frmts/mdal_xmdf.hpp"
#include "frmts/mdal_flo2d.hpp"
#include "frmts/mdal_hec2d.hpp"
#endif

#ifdef HAVE_GDAL
#include "frmts/mdal_gdal_grib.hpp"
#include "frmts/mdal_h2i.hpp"
#endif

#ifdef HAVE_NETCDF
#include "frmts/mdal_ugrid.hpp"
#include "frmts/mdal_sww.hpp"
#include "frmts/mdal_tuflowfv.hpp"
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
#include "frmts/mdal_gdal_netcdf.hpp"
#endif

#if defined HAVE_HDF5 && defined HAVE_XML
#include "frmts/mdal_xdmf.hpp"
#endif

#if defined HAVE_SQLITE3 && defined HAVE_NETCDF
#include "frmts/mdal_3di.hpp"
#endif

std::string MDAL::DriverManager::getUris( const std::string &file, const std::string &driverName ) const
{
  if ( !MDAL::fileExists( file ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "File " + file + " could not be found" );
    return std::string();
  }

  if ( !driverName.empty() )
  {
    std::shared_ptr<MDAL::Driver> requestedDriver = driver( driverName );
    if ( !requestedDriver )
    {
      MDAL::Log::error( MDAL_Status::Err_MissingDriver, "No such driver with name " + driverName );
      return std::string();
    }

    std::unique_ptr<MDAL::Driver> drv( requestedDriver->create() );
    return drv->buildUri( file );
  }
  else
  {
    for ( const auto &driver : mDrivers )
    {
      if ( ( driver->hasCapability( Capability::ReadMesh ) ) &&
           driver->canReadMesh( file ) )
      {
        std::unique_ptr<MDAL::Driver> drv( driver->create() );
        return drv->buildUri( file );
      }
    }
  }

  return std::string();
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverManager::load( const std::string &meshFile, const std::string &meshName ) const
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
      std::unique_ptr<MDAL::Driver> drv( driver->create() );

      mesh = drv->load( meshFile, meshName );
      if ( mesh ) // stop if he have the mesh
        break;
    }
  }

  if ( !mesh )
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, "Unable to load mesh (null)" );

  return mesh;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverManager::load(
  const std::string &driverName,
  const std::string &meshFile,
  const std::string &meshName ) const
{
  std::unique_ptr<MDAL::Mesh> mesh;

  if ( !MDAL::fileExists( meshFile ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "File " + meshFile + " could not be found" );
    return mesh;
  }

  std::shared_ptr<MDAL::Driver> requestedDriver;
  requestedDriver = driver( driverName );
  if ( !requestedDriver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Could not find driver with name: " + driverName );
    return mesh;
  }

  std::unique_ptr<Driver> drv( requestedDriver->create() );
  mesh = drv->load( meshFile, meshName );

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

void  MDAL::DriverManager::save( Mesh *mesh, const std::string &uri ) const
{
  std::string driverName;
  std::string meshName;
  std::string fileName;

  MDAL::parseDriverAndMeshFromUri( uri, driverName, fileName, meshName );

  std::shared_ptr<MDAL::Driver> selectedDriver = driver( driverName );

  if ( !selectedDriver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Could not find driver with name: " + driverName );
    return;
  }

  std::unique_ptr<Driver> drv( selectedDriver->create() );

  drv->save( fileName, meshName, mesh );
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

#ifdef BUILD_PLY
  mDrivers.push_back( std::make_shared<MDAL::DriverPly>() );
#endif

#ifdef HAVE_HDF5
  mDrivers.push_back( std::make_shared<MDAL::DriverFlo2D>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverHec2D>() );
#endif

#ifdef HAVE_NETCDF
  mDrivers.push_back( std::make_shared<MDAL::DriverTuflowFV>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverSWW>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverUgrid>() );
#endif

#if defined HAVE_SQLITE3 && defined HAVE_NETCDF
  mDrivers.push_back( std::make_shared<MDAL::Driver3Di>() );
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
  mDrivers.push_back( std::make_shared<MDAL::DriverGdalNetCDF>() );
#endif

#ifdef HAVE_GDAL
  mDrivers.push_back( std::make_shared<MDAL::DriverGdalGrib>() );
  mDrivers.push_back( std::make_shared<MDAL::DriverH2i>() );
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

  loadDynamicDrivers();
}

void MDAL::DriverManager::loadDynamicDrivers()
{
  std::string dirPath = MDAL::getEnvVar( "MDAL_DRIVER_PATH" );
  if ( dirPath.empty() )
    return;
  dirPath += '/';
  std::vector<std::string> libList = MDAL::Library::libraryFilesInDir( dirPath );
  for ( const std::string &libFile : libList )
  {
    std::shared_ptr<MDAL::Driver> driver( MDAL::DriverDynamic::create( dirPath + libFile ) );

    if ( driver )
      mDrivers.push_back( driver );
  }

}

