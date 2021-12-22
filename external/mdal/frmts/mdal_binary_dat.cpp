/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <memory>

#include "mdal_binary_dat.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

#include <math.h>

static const int CT_VERSION   = 3000;
static const int CT_OBJTYPE   = 100;
static const int CT_SFLT      = 110;
static const int CT_SFLG      = 120;
static const int CT_BEGSCL    = 130;
static const int CT_BEGVEC    = 140;
static const int CT_VECTYPE   = 150;
static const int CT_OBJID     = 160;
static const int CT_NUMDATA   = 170;
static const int CT_NUMCELLS  = 180;
static const int CT_NAME      = 190;
static const int CT_TS        = 200;
static const int CT_ENDDS     = 210;
static const int CT_RT_JULIAN = 240;
static const int CT_TIMEUNITS = 250;
static const int CT_2D_MESHES = 3;
static const int CT_FLOAT_SIZE = 4;
static const int CF_FLAG_SIZE = 1;
static const int CF_FLAG_INT_SIZE = 4;


static void exit_with_error( MDAL_Status error, const std::string msg )
{
  MDAL::Log::error( error, "BINARY_DAT", msg );
}

static bool read( std::ifstream &in, char *s, int n )
{
  in.read( s, n );
  if ( !in )
    return true; //error
  else
    return false; //OK
}

static bool readIStat( std::ifstream &in, int sflg, char *flag )
{
  if ( sflg == CF_FLAG_SIZE )
  {
    in.read( flag, sflg );
    if ( !in )
      return true; // error
  }
  else
  {
    int istat;
    in.read( reinterpret_cast< char * >( &istat ), sflg );
    if ( !in )
      return true; // error
    else
      *flag = ( istat == 1 );
  }
  return false;
}

MDAL::DriverBinaryDat::DriverBinaryDat():
  Driver( "BINARY_DAT",
          "Binary DAT",
          "*.dat",
          Capability::ReadDatasets | Capability::WriteDatasetsOnVertices
        )
{
}

MDAL::DriverBinaryDat *MDAL::DriverBinaryDat::create()
{
  return new DriverBinaryDat();
}

MDAL::DriverBinaryDat::~DriverBinaryDat() = default;

bool MDAL::DriverBinaryDat::canReadDatasets( const std::string &uri )
{
  std::ifstream in = MDAL::openInputFile( uri, std::ifstream::in | std::ifstream::binary );
  int version;

  if ( read( in, reinterpret_cast< char * >( &version ), 4 ) )
    return false;

  if ( version != CT_VERSION ) // Version should be 3000
    return false;

  return true;
}

/**
 * The DAT format contains "datasets" and each dataset has N-outputs. One output
 * represents data for all vertices/faces for one timestep
 *
 * in TUFLOW results there could be also a special timestep (99999) with maximums
 * we will put it into a separate dataset with name suffixed with "/Maximums"
 *
 * In MDAL we convert one output to one MDAL dataset;
 *
 */
void MDAL::DriverBinaryDat::load( const std::string &datFile, MDAL::Mesh *mesh )
{
  mDatFile = datFile;
  MDAL::Log::resetLastStatus();

  if ( !MDAL::fileExists( mDatFile ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, name(), "File could not be found " + mDatFile );
    return;
  }

  std::ifstream in = MDAL::openInputFile( mDatFile, std::ifstream::in | std::ifstream::binary );

  // implementation based on information from:
  // http://www.xmswiki.com/wiki/SMS:Binary_Dataset_Files_*.dat
  if ( !in ) return exit_with_error( MDAL_Status::Err_FileNotFound, "Couldn't open the file" );

  size_t vertexCount = mesh->verticesCount();
  size_t elemCount = mesh->facesCount();

  int card = 0;
  int version;
  int objecttype;
  int sflt;
  int sflg = 0;
  int vectype;
  int objid;
  int numdata;
  int numcells;
  char groupName[40];
  double referenceTime;
  int timeUnit = 0;
  std::string timeUnitStr;
  char istat;
  float time;

  if ( read( in, reinterpret_cast< char * >( &version ), 4 ) ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "Unable to read version" );

  if ( version != CT_VERSION ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "Invalid version " );

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mesh,
                                          mDatFile
                                        ); // DAT datasets
  group->setDataLocation( MDAL_DataLocation::DataOnVertices );

  // in TUFLOW results there could be also a special timestep (99999) with maximums
  // we will put it into a separate dataset
  std::shared_ptr<DatasetGroup> groupMax = std::make_shared< DatasetGroup >(
        name(),
        mesh,
        mDatFile
      );
  groupMax->setDataLocation( MDAL_DataLocation::DataOnVertices );

  while ( card != CT_ENDDS )
  {
    if ( read( in, reinterpret_cast< char * >( &card ), 4 ) )
    {
      // We've reached the end of the file and there was no ends card
      break;
    }

    switch ( card )
    {
      case CT_OBJTYPE:
        // Object type
        if ( read( in, reinterpret_cast< char * >( &objecttype ), 4 ) || objecttype != CT_2D_MESHES ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "Invalid object type" );
        break;

      case CT_SFLT:
        // Float size
        if ( read( in, reinterpret_cast< char * >( &sflt ), 4 ) || sflt != CT_FLOAT_SIZE ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read float size" );
        break;

      case CT_SFLG:
        // Flag size
        if ( read( in, reinterpret_cast< char * >( &sflg ), 4 ) )
          if ( sflg != CF_FLAG_SIZE && sflg != CF_FLAG_INT_SIZE )
            return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read flag size" );
        break;

      case CT_BEGSCL:
        group->setIsScalar( true );
        groupMax->setIsScalar( true );
        break;

      case CT_BEGVEC:
        group->setIsScalar( false );
        groupMax->setIsScalar( false );
        break;

      case CT_VECTYPE:
        // Vector type
        if ( read( in, reinterpret_cast< char * >( &vectype ), 4 ) || vectype != 0 ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read vector type" );
        break;

      case CT_OBJID:
        // Object id
        if ( read( in, reinterpret_cast< char * >( &objid ), 4 ) ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read object id" );
        break;

      case CT_NUMDATA:
        // Num data
        if ( read( in, reinterpret_cast< char * >( &numdata ), 4 ) ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read num data" );
        if ( numdata != static_cast< int >( vertexCount ) ) return exit_with_error( MDAL_Status::Err_IncompatibleMesh, "invalid num data" );
        break;

      case CT_NUMCELLS:
        // Num data
        if ( read( in, reinterpret_cast< char * >( &numcells ), 4 ) ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read num cells" );
        if ( numcells != static_cast< int >( elemCount ) ) return exit_with_error( MDAL_Status::Err_IncompatibleMesh, "invalid num cells" );
        break;

      case CT_NAME:
        // Name
        if ( read( in, reinterpret_cast< char * >( &groupName ), 40 ) ) return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read name" );
        if ( groupName[39] != 0 )
          groupName[39] = 0;
        group->setName( trim( std::string( groupName ) ) );
        groupMax->setName( group->name() + "/Maximums" );
        break;

      case CT_RT_JULIAN:
        // Reference time
        if ( readIStat( in, sflg, &istat ) )
          return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read reference time" );

        if ( read( in, reinterpret_cast< char * >( &time ), 8 ) )
          return exit_with_error( MDAL_Status::Err_UnknownFormat, "unable to read reference time" );

        referenceTime = static_cast<double>( time );
        group->setReferenceTime( DateTime( referenceTime, DateTime::JulianDay ) );
        break;

      case CT_TIMEUNITS:
        // Time unit
        if ( read( in, reinterpret_cast< char * >( &timeUnit ), 4 ) )
          return exit_with_error( MDAL_Status::Err_UnknownFormat, "Unable to read time units" );

        switch ( timeUnit )
        {
          case 0:
            timeUnitStr = "hours";
            break;
          case 1:
            timeUnitStr = "minutes";
            break;
          case 2:
            timeUnitStr = "seconds";
            break;
          case 4:
            timeUnitStr = "days";
            break;
          default:
            timeUnitStr = "unknown";
            break;
        }
        group->setMetadata( "TIMEUNITS", timeUnitStr );
        break;

      case CT_TS:
        // Time step!
        if ( readIStat( in, sflg, &istat ) )
          return exit_with_error( MDAL_Status::Err_UnknownFormat, "Invalid time step" );

        if ( read( in, reinterpret_cast< char * >( &time ), 4 ) )
          return exit_with_error( MDAL_Status::Err_UnknownFormat, "Invalid time step" );

        double rawTime = static_cast<double>( time );
        MDAL::RelativeTimestamp t( rawTime, MDAL::parseDurationTimeUnit( timeUnitStr ) );

        if ( readVertexTimestep( mesh, group, groupMax, t, istat, sflg, in ) )
          return exit_with_error( MDAL_Status::Err_UnknownFormat, "Unable to read vertex timestep" );

        break;
    }
  }

  if ( !group || group->datasets.size() == 0 )
    return exit_with_error( MDAL_Status::Err_UnknownFormat, "No datasets" );

  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );

  if ( groupMax && groupMax->datasets.size() > 0 )
  {
    groupMax->setStatistics( MDAL::calculateStatistics( groupMax ) );
    mesh->datasetGroups.push_back( groupMax );
  }
}

bool MDAL::DriverBinaryDat::readVertexTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  std::shared_ptr<DatasetGroup> groupMax,
  MDAL::RelativeTimestamp time,
  bool hasStatus,
  int sflg,
  std::ifstream &in )
{
  assert( group && groupMax && ( group->isScalar() == groupMax->isScalar() ) );
  bool isScalar = group->isScalar();

  size_t vertexCount = mesh->verticesCount();
  size_t faceCount = mesh->facesCount();

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MDAL::MemoryDataset2D >( group.get(), hasStatus );

  bool active = true;
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( hasStatus )
    {
      if ( readIStat( in, sflg, reinterpret_cast< char * >( &active ) ) )
        return true; //error
      dataset->setActive( i, active );
    }
  }

  for ( size_t i = 0; i < vertexCount; ++i )
  {
    if ( !isScalar )
    {
      float x, y;

      if ( read( in, reinterpret_cast< char * >( &x ), 4 ) )
        return true; //error
      if ( read( in, reinterpret_cast< char * >( &y ), 4 ) )
        return true; //error

      dataset->setVectorValue( i, static_cast< double >( x ), static_cast< double >( y ) );
    }
    else
    {
      float scalar;

      if ( read( in, reinterpret_cast< char * >( &scalar ), 4 ) )
        return true; //error

      dataset->setScalarValue( i, static_cast< double >( scalar ) );
    }
  }

  if ( MDAL::equals( time.value( MDAL::RelativeTimestamp::hours ), 99999.0 ) ) // Special TUFLOW dataset with maximus
  {
    dataset->setTime( time );
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    groupMax->datasets.push_back( dataset );
  }
  else
  {
    dataset->setTime( time );
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
  return false; //OK
}

// ////////////////////////////////////////////
// WRITE
// ////////////////////////////////////////////

static bool writeRawData( std::ofstream &out, const char *s, int n )
{
  out.write( s, n );
  if ( !out )
    return true; //error
  else
    return false; //OK
}

bool MDAL::DriverBinaryDat::persist( MDAL::DatasetGroup *group )
{
  assert( group->dataLocation() == MDAL_DataLocation::DataOnVertices );

  std::ofstream out = MDAL::openOutputFile( group->uri(), std::ofstream::out | std::ofstream::binary );

  // implementation based on information from:
  // http://www.xmswiki.com/wiki/SMS:Binary_Dataset_Files_*.dat
  if ( !out )
    return true; // Couldn't open the file

  const Mesh *mesh = group->mesh();
  size_t nodeCount = mesh->verticesCount();
  size_t elemCount = mesh->facesCount();

  // version card
  writeRawData( out, reinterpret_cast< const char * >( &CT_VERSION ), 4 );

  // objecttype
  writeRawData( out, reinterpret_cast< const char * >( &CT_OBJTYPE ), 4 );
  writeRawData( out, reinterpret_cast< const char * >( &CT_2D_MESHES ), 4 );

  // float size
  writeRawData( out, reinterpret_cast< const char * >( &CT_SFLT ), 4 );
  writeRawData( out, reinterpret_cast< const char * >( &CT_FLOAT_SIZE ), 4 );

  // Flag size
  writeRawData( out, reinterpret_cast< const char * >( &CT_SFLG ), 4 );
  writeRawData( out, reinterpret_cast< const char * >( &CF_FLAG_SIZE ), 4 );

  // Dataset Group Type
  if ( group->isScalar() )
  {
    writeRawData( out, reinterpret_cast< const char * >( &CT_BEGSCL ), 4 );
  }
  else
  {
    writeRawData( out, reinterpret_cast< const char * >( &CT_BEGVEC ), 4 );
  }

  // Object id (ignored)
  int ignored_val = 1;
  writeRawData( out, reinterpret_cast< const char * >( &CT_OBJID ), 4 );
  writeRawData( out, reinterpret_cast< const char * >( &ignored_val ), 4 );

  // Num nodes
  writeRawData( out, reinterpret_cast< const char * >( &CT_NUMDATA ), 4 );
  writeRawData( out, reinterpret_cast< const char * >( &nodeCount ), 4 );

  // Num cells
  writeRawData( out, reinterpret_cast< const char * >( &CT_NUMCELLS ), 4 );
  writeRawData( out, reinterpret_cast< const char * >( &elemCount ), 4 );

  // Name
  writeRawData( out, reinterpret_cast< const char * >( &CT_NAME ), 4 );
  writeRawData( out, MDAL::leftJustified( group->name(), 39 ).c_str(), 40 );

  // Time steps
  int istat = 1; // include if elements are active

  for ( size_t time_index = 0; time_index < group->datasets.size(); ++ time_index )
  {
    const std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::dynamic_pointer_cast<MDAL::MemoryDataset2D>( group->datasets[time_index] );

    writeRawData( out, reinterpret_cast< const char * >( &CT_TS ), 4 );
    writeRawData( out, reinterpret_cast< const char * >( &istat ), 1 );
    float ftime = static_cast<float>( dataset->time( RelativeTimestamp::hours ) );
    writeRawData( out, reinterpret_cast< const char * >( &ftime ), 4 );

    if ( istat )
    {
      // Write status flags
      for ( size_t i = 0; i < elemCount; i++ )
      {
        bool active = static_cast<bool>( dataset->active( i ) );
        writeRawData( out, reinterpret_cast< const char * >( &active ), 1 );
      }
    }

    for ( size_t i = 0; i < nodeCount; i++ )
    {
      // Read values flags
      if ( !group->isScalar() )
      {
        float x = static_cast<float>( dataset->valueX( i ) );
        float y = static_cast<float>( dataset->valueY( i ) );
        writeRawData( out, reinterpret_cast< const char * >( &x ), 4 );
        writeRawData( out, reinterpret_cast< const char * >( &y ), 4 );
      }
      else
      {
        float val = static_cast<float>( dataset->scalarValue( i ) );
        writeRawData( out, reinterpret_cast< const char * >( &val ), 4 );
      }
    }
  }

  if ( writeRawData( out, reinterpret_cast< const char * >( &CT_ENDDS ), 4 ) ) return true;

  return false;
}

std::string MDAL::DriverBinaryDat::writeDatasetOnFileSuffix() const
{
  return "dat";
}
