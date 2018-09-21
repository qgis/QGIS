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

std::string MDAL::toLower( const std::string &std )
{
  std::string res( std );
  std::transform( res.begin(), res.end(), res.begin(), ::tolower );
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

std::string MDAL::removeLastChar( const std::string &str )
{
  std::string ret( str );
  ret.pop_back();
  return ret;
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
