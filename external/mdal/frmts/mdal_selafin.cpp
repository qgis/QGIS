/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include <stdio.h>
#include <string.h>
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
#include <algorithm>

#include "mdal_selafin.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include <math.h>

MDAL::SerafinStreamReader::SerafinStreamReader() = default;

void MDAL::SerafinStreamReader::initialize( const std::string &fileName )
{
  mFileName = fileName;
  if ( !MDAL::fileExists( mFileName ) )
  {
    throw MDAL_Status::Err_FileNotFound;
  }

  mIn = std::ifstream( mFileName, std::ifstream::in | std::ifstream::binary );
  if ( !mIn )
    throw MDAL_Status::Err_FileNotFound; // Couldn't open the file

  // get length of file:
  mIn.seekg( 0, mIn.end );
  mFileSize = mIn.tellg();
  mIn.seekg( 0, mIn.beg );

  mStreamInFloatPrecision = getStreamPrecision();
  mIsNativeLittleEndian = MDAL::isNativeLittleEndian();
}

bool MDAL::SerafinStreamReader::getStreamPrecision( )
{
  ignore_array_length( );
  ignore( 72 );
  std::string varType = read_string_without_length( 8 );
  bool ret;
  if ( varType == "SERAFIN" )
  {
    ret = true;
  }
  else if ( varType == "SERAFIND" )
  {
    ret = false;
  }
  else
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  ignore_array_length( );
  return ret;
}

std::string MDAL::SerafinStreamReader::read_string( size_t len )
{
  size_t length = read_sizet();
  if ( length != len ) throw MDAL_Status::Err_UnknownFormat;
  std::string ret = read_string_without_length( len );
  ignore_array_length();
  return ret;
}

std::vector<double> MDAL::SerafinStreamReader::read_double_arr( size_t len )
{
  size_t length = read_sizet();
  if ( mStreamInFloatPrecision )
  {
    if ( length != len * 4 ) throw MDAL_Status::Err_UnknownFormat;
  }
  else
  {
    if ( length != len * 8 ) throw MDAL_Status::Err_UnknownFormat;
  }
  std::vector<double> ret( len );
  for ( size_t i = 0; i < len; ++i )
  {
    ret[i] = read_double();
  }
  ignore_array_length();
  return ret;
}

std::vector<int> MDAL::SerafinStreamReader::read_int_arr( size_t len )
{
  size_t length = read_sizet();
  if ( length != len * 4 ) throw MDAL_Status::Err_UnknownFormat;
  std::vector<int> ret( len );
  for ( size_t i = 0; i < len; ++i )
  {
    ret[i] = read_int();
  }
  ignore_array_length();
  return ret;
}

std::vector<size_t> MDAL::SerafinStreamReader::read_size_t_arr( size_t len )
{
  size_t length = read_sizet();
  if ( length != len * 4 ) throw MDAL_Status::Err_UnknownFormat;
  std::vector<size_t> ret( len );
  for ( size_t i = 0; i < len; ++i )
  {
    ret[i] = read_sizet();
  }
  ignore_array_length();
  return ret;
}

std::string MDAL::SerafinStreamReader::read_string_without_length( size_t len )
{
  std::vector<char> ptr( len );
  mIn.read( ptr.data(), static_cast<int>( len ) );
  if ( !mIn )
    throw MDAL_Status::Err_UnknownFormat;

  size_t str_length = 0;
  for ( size_t i = len; i > 0; --i )
  {
    if ( ptr[i - 1] != 0x20 )
    {
      str_length = i;
      break;
    }
  }
  std::string ret( ptr.data(), str_length );
  return ret;
}

double MDAL::SerafinStreamReader::read_double( )
{
  double ret;

  if ( mStreamInFloatPrecision )
  {
    float ret_f;
    if ( !readValue( ret_f, mIn, mIsNativeLittleEndian ) )
      throw MDAL_Status::Err_UnknownFormat;
    ret = static_cast<double>( ret_f );
  }
  else
  {
    if ( !readValue( ret, mIn, mIsNativeLittleEndian ) )
      throw MDAL_Status::Err_UnknownFormat;
  }
  return ret;
}


int MDAL::SerafinStreamReader::read_int( )
{
  unsigned char data[4];

  if ( mIn.read( reinterpret_cast< char * >( &data ), 4 ) )
    if ( !mIn )
      throw MDAL_Status::Err_UnknownFormat;
  if ( mIsNativeLittleEndian )
  {
    std::reverse( reinterpret_cast< char * >( &data ), reinterpret_cast< char * >( &data ) + 4 );
  }
  int var;
  memcpy( reinterpret_cast< char * >( &var ),
          reinterpret_cast< char * >( &data ),
          4 );
  return var;
}

size_t MDAL::SerafinStreamReader::read_sizet()
{
  int var = read_int( );
  return static_cast<size_t>( var );
}

size_t MDAL::SerafinStreamReader::remainingBytes()
{
  return static_cast<size_t>( mFileSize - mIn.tellg() );
}

void MDAL::SerafinStreamReader::ignore( int len )
{
  mIn.ignore( len );
  if ( !mIn )
    throw MDAL_Status::Err_UnknownFormat;
}

void MDAL::SerafinStreamReader::ignore_array_length( )
{
  ignore( 4 );
}

// //////////////////////////////
// DRIVER
// //////////////////////////////
MDAL::DriverSelafin::DriverSelafin():
  Driver( "SELAFIN",
          "Selafin File",
          "*.slf",
          Capability::ReadMesh
        )
{
}

MDAL::DriverSelafin *MDAL::DriverSelafin::create()
{
  return new DriverSelafin();
}

MDAL::DriverSelafin::~DriverSelafin() = default;


void MDAL::DriverSelafin::parseFile( std::vector<std::string> &var_names,
                                     double *xOrigin,
                                     double *yOrigin,
                                     size_t *nElem,
                                     size_t *nPoint,
                                     size_t *nPointsPerElem,
                                     std::vector<size_t> &ikle,
                                     std::vector<double> &x,
                                     std::vector<double> &y,
                                     std::vector<timestep_map> &data )
{

  /* 1 record containing the title of the study (72 characters) and a 8 characters
  string indicating the type of format (SERAFIN or SERAFIND)
  */
  mReader.initialize( mFileName );

  /* 1 record containing the two integers NBV(1) and NBV(2) (number of linear
     and quadratic variables, NBV(2) with the value of 0 for Telemac, as
     quadratic values are not saved so far), numbered from 1 in docs
  */
  std::vector<size_t> nbv = mReader.read_size_t_arr( 2 );

  /* NBV(1) records containing the names and units of each variab
     le (over 32 characters)
  */
  for ( size_t i = 0; i < nbv[0]; ++i )
  {
    var_names.push_back( mReader.read_string( 32 ) );
  }

  /* 1 record containing the integers table IPARAM (10 integers, of which only
      the 6 are currently being used), numbered from 1 in docs

      - if IPARAM (3) != 0: the value corresponds to the x-coordinate of the
      origin of the mesh,

      - if IPARAM (4) != 0: the value corresponds to the y-coordinate of the
      origin of the mesh,

      - if IPARAM (7) != 0: the value corresponds to the number of  planes
      on the vertical (3D computation),

      - if IPARAM (8) != 0: the value corresponds to the number of
      boundary points (in parallel),

      - if IPARAM (9) != 0: the value corresponds to the number of interface
      points (in parallel),

      - if IPARAM(8) or IPARAM(9) != 0: the array IPOBO below is replaced
      by the array KNOLG (total initial number of points). All the other
      numbers are local to the sub-domain, including IKLE
  */
  std::vector<int> params = mReader.read_int_arr( 10 );
  *xOrigin = static_cast<double>( params[2] );
  *yOrigin = static_cast<double>( params[3] );

  if ( params[6] != 0 )
  {
    // would need additional parsing
    throw MDAL_Status::Err_MissingDriver;
  }

  /*
    if IPARAM (10) = 1: a record containing the computation starting date
  */

  if ( params[9] == 1 )
  {
    std::vector<int> datetime = mReader.read_int_arr( 6 );
    MDAL_UNUSED( datetime )
  }

  /* 1 record containing the integers NELEM,NPOIN,NDP,1 (number of
     elements, number of points, number of points per element and the value 1)
   */
  std::vector<size_t> numbers = mReader.read_size_t_arr( 4 );
  *nElem = numbers[0];
  *nPoint = numbers[1];
  *nPointsPerElem = numbers[2];

  /* 1 record containing table IKLE (integer array
     of dimension (NDP,NELEM)
     which is the connectivity table.

     Attention: in TELEMAC-2D, the dimensions of this array are (NELEM,NDP))
  */
  ikle = mReader.read_size_t_arr( ( *nElem ) * ( *nPointsPerElem ) );
  for ( size_t i = 0; i < ikle.size(); ++i )
  {
    -- ikle[i];  //numbered from 1
  }

  /* 1 record containing table IPOBO (integer array of dimension NPOIN); the
     value of one element is 0 for an internal point, and
     gives the numbering of boundary points for the others
  */
  std::vector<int> iPointBoundary = mReader.read_int_arr( *nPoint );
  MDAL_UNUSED( iPointBoundary );

  /* 1 record containing table X (real array of dimension NPOIN containing the
     abscissae of the points)
  */
  x = mReader.read_double_arr( *nPoint );

  /* 1 record containing table Y (real array of dimension NPOIN containing the
     abscissae of the points)
  */
  y = mReader.read_double_arr( *nPoint );


  /* Next, for each time step, the following are found:
     - 1 record containing time T (real),
     - NBV(1)+NBV(2) records containing the results tables for each variable at time
  */
  data.resize( var_names.size() );

  size_t nTimesteps = mReader.remainingBytes() / ( 12 + ( 4 + ( *nPoint ) * 4 + 4 ) * var_names.size() );
  for ( size_t nT = 0; nT < nTimesteps; ++nT )
  {
    std::vector<double> times = mReader.read_double_arr( 1 );
    double time = times[0];

    for ( size_t i = 0; i < var_names.size(); ++i )
    {
      timestep_map &datait = data[i];
      std::vector<double> datavals = mReader.read_double_arr( *nPoint );
      datait[time] = datavals;
    }
  }
}

void MDAL::DriverSelafin::createMesh(
  double xOrigin,
  double yOrigin,
  size_t nElems,
  size_t nPoints,
  size_t nPointsPerElem,
  std::vector<size_t> &ikle,
  std::vector<double> &x,
  std::vector<double> &y )
{
  Vertices nodes( nPoints );
  Vertex *nodesPtr = nodes.data();
  for ( size_t n = 0; n < nPoints; ++n, ++nodesPtr )
  {
    nodesPtr->x = xOrigin + x[n];
    nodesPtr->y = yOrigin + y[n];
  }

  Faces elements( nElems );
  for ( size_t e = 0; e < nElems; ++e )
  {
    if ( nPointsPerElem != 3 )
    {
      throw MDAL_Status::Err_IncompatibleMesh; //we can add it, but it is uncommon for this format
    }

    // elemPtr->setId(e);
    elements[e].resize( 3 );
    for ( size_t p = 0; p < 3; p++ )
    {
      size_t val = ikle[e * 3 + p];
      if ( val > nPoints - 1 )
      {
        elements[e][p] = 0;
      }
      else
      {
        elements[e][p] = val;
      }
    }
  }

  mMesh.reset(
    new MemoryMesh(
      "SELAFIN",
      nodes.size(),
      elements.size(),
      3, //Triangles
      computeExtent( nodes ),
      mFileName
    )
  );
  mMesh->faces = elements;
  mMesh->vertices = nodes;
}

void MDAL::DriverSelafin::addData( const std::vector<std::string> &var_names, const std::vector<timestep_map> &data, size_t nPoints )
{
  for ( size_t nName = 0; nName < var_names.size(); ++nName )
  {
    std::string var_name( var_names[nName] );
    var_name = MDAL::toLower( MDAL::trim( var_name ) );
    var_name = MDAL::replace( var_name, "/", "" ); // slash is represented as sub-dataset group but we do not have such subdatasets groups in Selafin

    bool is_vector = false;
    bool is_x = true;

    if ( MDAL::contains( var_name, "velocity u" ) || MDAL::contains( var_name, "along x" ) )
    {
      is_vector = true;
      var_name = MDAL::replace( var_name, "velocity u", "velocity" );
      var_name = MDAL::replace( var_name, "along x", "" );
    }
    else if ( MDAL::contains( var_name, "velocity v" ) || MDAL::contains( var_name, "along y" ) )
    {
      is_vector = true;
      is_x =  false;
      var_name =  MDAL::replace( var_name, "velocity v", "velocity" );
      var_name =  MDAL::replace( var_name, "along y", "" );
    }

    if ( MDAL::contains( var_name, "vitesse u" ) || MDAL::contains( var_name, "suivant x" ) )
    {
      is_vector = true;
      var_name = MDAL::replace( var_name, "vitesse u", "vitesse" );
      var_name = MDAL::replace( var_name, "suivant x", "" );
    }
    else if ( MDAL::contains( var_name, "vitesse v" ) || MDAL::contains( var_name, "suivant y" ) )
    {
      is_vector = true;
      is_x =  false;
      var_name =  MDAL::replace( var_name, "vitesse v", "vitesse" );
      var_name =  MDAL::replace( var_name, "suivant y", "" );
    }

    std::shared_ptr<DatasetGroup> group = mMesh->group( var_name );
    if ( !group )
    {
      group = std::make_shared< DatasetGroup >(
                mMesh->driverName(),
                mMesh.get(),
                mMesh->uri(),
                var_name
              );
      group->setIsOnVertices( true );
      group->setIsScalar( !is_vector );

      mMesh->datasetGroups.push_back( group );
    }

    size_t i = 0;
    for ( timestep_map::const_iterator it = data[nName].begin(); it != data[nName].end(); ++it, ++i )
    {
      std::shared_ptr<MDAL::MemoryDataset> dataset;
      if ( group->datasets.size() > i )
      {
        dataset = std::dynamic_pointer_cast<MDAL::MemoryDataset>( group->datasets[i] );
      }
      else
      {
        dataset = std::make_shared< MemoryDataset >( group.get() );
        dataset->setTime( it->first );
        group->datasets.push_back( dataset );
      }
      double *values = dataset->values();

      for ( size_t nP = 0; nP < nPoints; nP++ )
      {
        double val = it->second.at( nP );
        if ( MDAL::equals( val, 0 ) )
        {
          val = std::numeric_limits<double>::quiet_NaN();
        }
        if ( is_vector )
        {
          if ( is_x )
          {
            values[2 * nP] = val;
          }
          else
          {
            values[2 * nP + 1] = val;
          }
        }
        else
        {
          values[nP] = val;
        }
      }
    }
  }

  // now activate faces and calculate statistics
  for ( auto group : mMesh->datasetGroups )
  {
    for ( auto dataset : group->datasets )
    {
      std::shared_ptr<MDAL::MemoryDataset> dts = std::dynamic_pointer_cast<MDAL::MemoryDataset>( dataset );
      MDAL::activateFaces( mMesh.get(), dts );

      MDAL::Statistics stats = MDAL::calculateStatistics( dataset );
      dataset->setStatistics( stats );
    }

    MDAL::Statistics stats = MDAL::calculateStatistics( group );
    group->setStatistics( stats );
  }
}

bool MDAL::DriverSelafin::canRead( const std::string &uri )
{
  if ( !MDAL::fileExists( uri ) ) return false;

  std::ifstream in( uri, std::ifstream::in | std::ifstream::binary );
  if ( !in ) return false;

  // The first four bytes of the file should contain the values (in hexadecimal): 00 00 00 50.
  // This actually indicates the start of a string of length 80 in the file.
  // At position 84 in the file, the eight next bytes should read (in hexadecimal): 00 00 00 50 00 00 00 04.
  unsigned char data[ 92 ];
  in.read( reinterpret_cast< char * >( &data ), 92 );
  if ( !in )
    return false;

  if ( data[0] != 0 || data[1] != 0 ||
       data[2] != 0 || data[3] != 0x50 )
    return false;

  if ( data[84 + 0] != 0 || data[84 + 1] != 0 ||
       data[84 + 2] != 0 || data[84 + 3] != 0x50 ||
       data[84 + 4] != 0 || data[84 + 5] != 0 ||
       data[84 + 6] != 0 || data[84 + 7] != 8 )
    return false;

  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverSelafin::load( const std::string &meshFile, MDAL_Status *status )
{
  if ( status ) *status = MDAL_Status::None;
  mFileName = meshFile;
  mMesh.reset();

  std::vector<std::string> var_names;
  double xOrigin;
  double yOrigin;
  size_t nElems;
  size_t nPoints;
  size_t nPointsPerElem;
  std::vector<size_t> ikle;
  std::vector<double> x;
  std::vector<double> y;
  std::vector<timestep_map> data;

  try
  {
    parseFile( var_names,
               &xOrigin,
               &yOrigin,
               &nElems,
               &nPoints,
               &nPointsPerElem,
               ikle,
               x,
               y,
               data );

    createMesh( xOrigin,
                yOrigin,
                nElems,
                nPoints,
                nPointsPerElem,
                ikle,
                x,
                y );

    addData( var_names, data, nPoints );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    mMesh.reset();
  }
  return std::unique_ptr<Mesh>( mMesh.release() );
}
