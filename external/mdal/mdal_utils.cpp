/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_utils.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <assert.h>
#include <cmath>
#include <string.h>
#include <stdio.h>

bool MDAL::fileExists( const std::string &filename )
{
  std::ifstream in( filename );
  return in.good();
}


bool MDAL::startsWith( const std::string &str, const std::string &substr, ContainsBehaviour behaviour )
{
  if ( behaviour == ContainsBehaviour::CaseSensitive )
    return str.rfind( substr, 0 ) == 0;
  else
    return startsWith( toLower( str ), toLower( substr ), ContainsBehaviour::CaseSensitive );
}

bool MDAL::endsWith( const std::string &str, const std::string &substr, ContainsBehaviour behaviour )
{
  if ( behaviour == ContainsBehaviour::CaseSensitive )
    return str.rfind( substr ) == ( str.size() - substr.size() );
  else
    return endsWith( toLower( str ), toLower( substr ), ContainsBehaviour::CaseSensitive );
}

std::vector<std::string> MDAL::split( const std::string &str, const std::string &delimiter, SplitBehaviour behaviour )
{
  std::string remaining( str );
  std::vector<std::string> list;
  size_t pos = 0;
  std::string token;
  while ( ( pos = remaining.find( delimiter ) ) != std::string::npos )
  {
    token = remaining.substr( 0, pos );

    if ( behaviour == SplitBehaviour::SkipEmptyParts )
    {
      if ( !token.empty() )
        list.push_back( token );
    }
    else
      list.push_back( token );

    remaining.erase( 0, pos + delimiter.length() );
  }
  list.push_back( remaining );
  return list;
}

size_t MDAL::toSizeT( const std::string &str )
{
  int i = atoi( str.c_str() );
  if ( i < 0 ) // consistent with atoi return
    i = 0;
  return static_cast< size_t >( i );
}

double MDAL::toDouble( const std::string &str )
{
  return atof( str.c_str() );
}

bool MDAL::isNumber( const std::string &str )
{
  // https://stackoverflow.com/a/16465826/2838364
  return ( strspn( str.c_str(), "-.0123456789" ) == str.size() );
}

int MDAL::toInt( const std::string &str )
{
  return atoi( str.c_str() );
}

std::string MDAL::baseName( const std::string &filename )
{
  // https://stackoverflow.com/a/8520815/2838364
  std::string fname( filename );

  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  const size_t last_slash_idx = fname.find_last_of( "\\/" );
  if ( std::string::npos != last_slash_idx )
  {
    fname.erase( 0, last_slash_idx + 1 );
  }

  // Remove extension if present.
  const size_t period_idx = fname.rfind( '.' );
  if ( std::string::npos != period_idx )
  {
    fname.erase( period_idx );
  }
  return fname;
}

std::string MDAL::pathJoin( const std::string &path1, const std::string &path2 )
{
//https://stackoverflow.com/questions/6297738/how-to-build-a-full-path-string-safely-from-separate-strings#6297807
#ifdef _MSC_VER
  return path1 + "\\" + path2;
#else
  return path1 + "/" + path2;
#endif
}

std::string MDAL::dirName( const std::string &filename )
{
  std::string dname( filename );
  const size_t last_slash_idx = dname.find_last_of( "\\/" );
  if ( std::string::npos != last_slash_idx )
  {
    dname.erase( last_slash_idx, dname.size() - last_slash_idx );
  }
  return dname;
}

bool MDAL::contains( const std::string &str, const std::string &substr, ContainsBehaviour behaviour )
{
  if ( behaviour == ContainsBehaviour::CaseSensitive )
    return str.find( substr ) != std::string::npos;
  else
  {
    auto it = std::search(
                str.begin(), str.end(),
                substr.begin(),   substr.end(),
                []( char ch1, char ch2 )
    {
#ifdef _MSC_VER
      return toupper( ch1 ) == toupper( ch2 );
#else
      return std::toupper( ch1 ) == std::toupper( ch2 );
#endif
    }
              );
    return ( it != str.end() );
  }
}

void MDAL::debug( const std::string &message )
{
#ifdef NDEBUG
  MDAL_UNUSED( message );
#else
  std::cout << message << std::endl;
#endif
}

bool MDAL::toBool( const std::string &str )
{
  int i = atoi( str.c_str() );
  return i != 0;
}

bool MDAL::contains( const std::vector<std::string> &list, const std::string &str )
{
  return std::find( list.begin(), list.end(), str ) != list.end();
}

std::string MDAL::join( const std::vector<std::string> parts, const std::string &delimiter )
{
  std::stringstream res;
  for ( auto iter = parts.begin(); iter != parts.end(); iter++ )
  {
    if ( iter != parts.begin() ) res << delimiter;
    res << *iter;
  }
  return res.str();
}

std::string MDAL::leftJustified( const std::string &str, size_t width, char fill )
{
  std::string ret( str );
  if ( ret.size() > width )
  {
    ret = ret.substr( 0, width );
  }
  else
  {
    ret = ret + std::string( width - ret.size(), fill );
  }
  assert( ret.size() == width );
  return ret;
}

std::string MDAL::toLower( const std::string &std )
{
  std::string res( std );
#ifdef WIN32
  //silence algorithm(1443): warning C4244: '=': conversion from 'int' to 'char'
  std::transform( res.begin(), res.end(), res.begin(),
  []( char c ) {return static_cast<char>( ::tolower( c ) );} );
#else
  std::transform( res.begin(), res.end(), res.begin(), ::tolower );
#endif
  return res;
}

std::string MDAL::replace( const std::string &str, const std::string &substr, const std::string &replacestr, MDAL::ContainsBehaviour behaviour )
{
  std::string res( str );
  if ( behaviour == ContainsBehaviour::CaseSensitive )
  {
    while ( res.find( substr ) != std::string::npos )
    {
      res.replace( res.find( substr ), substr.size(), replacestr );
    }
  }
  else
  {
    // https://stackoverflow.com/a/40577390/2838364
    std::string lower_s = toLower( str );
    std::string lower_substring = toLower( substr );

    auto position = lower_s.find( lower_substring );
    while ( position != std::string::npos )
    {
      res.replace( position, lower_substring.size(), replacestr );
      lower_s.replace( position, lower_substring.size(), replacestr );
      position = lower_s.find( lower_substring );
    }
  }
  return res;
}

MDAL::BBox MDAL::computeExtent( const MDAL::Vertices &vertices )
{
  BBox b;

  if ( vertices.empty() )
    return b;

  b.minX = vertices[0].x;
  b.maxX = vertices[0].x;
  b.minY = vertices[0].y;
  b.maxY = vertices[0].y;

  for ( Vertices::size_type i = 0; i < vertices.size(); i++ )
  {
    const Vertex &n = vertices[i];
    if ( n.x > b.maxX ) b.maxX = n.x;
    if ( n.x < b.minX ) b.minX = n.x;
    if ( n.y > b.maxY ) b.maxY = n.y;
    if ( n.y < b.minY ) b.minY = n.y;
  }
  return b;
}

bool MDAL::equals( double val1, double val2, double eps )
{
  return fabs( val1 - val2 ) < eps;
}

double MDAL::safeValue( double val, double nodata, double eps )
{
  if ( equals( val, nodata, eps ) )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    return val;
  }
}

double MDAL::parseTimeUnits( const std::string &units )
{
  double divBy = 1;
  // We are trying to parse strings like
  // "seconds since 2001-05-05 00:00:00"
  // "hours since 1900-01-01 00:00:0.0"
  // "days since 1961-01-01 00:00:00"
  const std::vector<std::string> units_list = MDAL::split( units, " since ", SkipEmptyParts );
  if ( units_list.size() == 2 )
  {
    // Give me hours
    if ( units_list[0] == "seconds" )
    {
      divBy = 3600.0;
    }
    else if ( units_list[0] == "minutes" )
    {
      divBy = 60.0;
    }
    else if ( units_list[0] == "days" )
    {
      divBy = 1.0 / 24.0;
    }
  }

  return divBy;
}

MDAL::Statistics _calculateStatistics( const std::vector<double> &values, size_t count, bool isVector )
{
  MDAL::Statistics ret;

  double min = std::numeric_limits<double>::quiet_NaN();
  double max = std::numeric_limits<double>::quiet_NaN();
  bool firstIteration = true;

  for ( size_t i = 0; i < count; ++i )
  {
    double magnitude;
    if ( isVector )
    {
      double x = values[2 * i];
      double y = values[2 * i + 1];
      if ( std::isnan( x ) || std::isnan( y ) )
        continue;
      magnitude = sqrt( x * x + y * y );
    }
    else
    {
      double x = values[i];
      if ( std::isnan( x ) )
        continue;
      magnitude = x;
    }

    if ( firstIteration )
    {
      firstIteration = false;
      min = magnitude;
      max = magnitude;
    }
    else
    {
      if ( magnitude < min )
      {
        min = magnitude;
      }
      if ( magnitude > max )
      {
        max = magnitude;
      }
    }
  }

  ret.minimum = min;
  ret.maximum = max;
  return ret;
}

MDAL::Statistics MDAL::calculateStatistics( std::shared_ptr<MDAL::DatasetGroup> grp )
{
  return calculateStatistics( grp.get() );
}

MDAL::Statistics MDAL::calculateStatistics( DatasetGroup *grp )
{
  Statistics ret;
  if ( !grp )
    return ret;

  for ( std::shared_ptr<Dataset> ds : grp->datasets )
  {
    MDAL::Statistics dsStats = ds->statistics();
    combineStatistics( ret, dsStats );
  }
  return ret;
}

MDAL::Statistics MDAL::calculateStatistics( std::shared_ptr<Dataset> dataset )
{
  Statistics ret;
  if ( !dataset )
    return ret;

  bool isVector = !dataset->group()->isScalar();
  size_t bufLen = 2000;
  std::vector<double> buffer( isVector ? bufLen * 2 : bufLen );

  size_t i = 0;
  while ( i < dataset->valuesCount() )
  {
    size_t valsRead;
    if ( isVector )
    {
      valsRead = dataset->vectorData( i, bufLen, buffer.data() );
    }
    else
    {
      valsRead = dataset->scalarData( i, bufLen, buffer.data() );
    }
    MDAL::Statistics dsStats = _calculateStatistics( buffer, valsRead, isVector );
    combineStatistics( ret, dsStats );
    i += valsRead;
  }

  return ret;
}

void MDAL::combineStatistics( MDAL::Statistics &main, const MDAL::Statistics &other )
{
  if ( std::isnan( main.minimum ) ||
       ( !std::isnan( other.minimum ) && ( main.minimum > other.minimum ) ) )
  {
    main.minimum = other.minimum;
  }

  if ( std::isnan( main.maximum ) ||
       ( !std::isnan( other.maximum ) && ( main.maximum < other.maximum ) ) )
  {
    main.maximum = other.maximum;
  }
}

void MDAL::addBedElevationDatasetGroup( MDAL::Mesh *mesh, const Vertices &vertices )
{
  if ( !mesh )
    return;

  if ( 0 == mesh->facesCount() )
    return;

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          mesh->driverName(),
                                          mesh,
                                          mesh->uri(),
                                          "Bed Elevation"
                                        );
  group->setIsOnVertices( true );
  group->setIsScalar( true );

  std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MemoryDataset >( group.get() );
  dataset->setTime( 0.0 );
  double *vals = dataset->values();
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    vals[i] = vertices[i].z;
  }
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
}
