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
#include <string.h>
#include <stdio.h>
#include <ctime>
#include <stdlib.h>

#if defined _WIN32
#define UNICODE
#include <locale>
#include <codecvt>
#endif

std::string MDAL::getEnvVar( const std::string &varname, const std::string &defaultVal )
{
  if ( varname.empty() )
    return std::string();

  char *envVarC = getenv( varname.c_str() );

  if ( !envVarC )
    return defaultVal;
  else
    return std::string( envVarC );
}

bool MDAL::openInputFile( std::ifstream &inputFileStream, const std::string &fileName, std::ios_base::openmode mode )
{
#if defined _MSC_VER
  std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
  std::wstring wStr = converter.from_bytes( fileName );
  inputFileStream.open( wStr, std::ifstream::in | mode );
#else
  inputFileStream.open( fileName, std::ifstream::in | mode );
#endif

  return inputFileStream.is_open();
}

std::ifstream MDAL::openInputFile( const std::string &fileName, std::ios_base::openmode mode )
{
  std::ifstream ret;

#if defined _MSC_VER
  std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
  std::wstring wStr = converter.from_bytes( fileName );
  ret.open( wStr, mode );
#else
  ret.open( fileName, mode );
#endif

  return ret;
}

std::ofstream MDAL::openOutputFile( const std::string &fileName, std::ios_base::openmode mode )
{
  std::ofstream ret;

#if defined _MSC_VER
  std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
  std::wstring wStr = converter.from_bytes( fileName );
  ret.open( wStr, mode );
#else
  ret.open( fileName, mode );
#endif

  return ret;
}

bool MDAL::fileExists( const std::string &filename )
{
  std::ifstream in;

  if ( !openInputFile( in, filename ) )
    return false;

  return in.good();
}

std::string MDAL::readFileToString( const std::string &filename )
{
  if ( MDAL::fileExists( filename ) )
  {
    std::ifstream t = openInputFile( filename );

    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
  }
  return "";
}

bool MDAL::startsWith( const std::string &str, const std::string &substr, ContainsBehaviour behaviour )
{
  if ( ( str.size() < substr.size() ) || substr.empty() )
    return false;

  if ( behaviour == ContainsBehaviour::CaseSensitive )
    return str.rfind( substr, 0 ) == 0;
  else
    return startsWith( toLower( str ), toLower( substr ), ContainsBehaviour::CaseSensitive );
}

bool MDAL::endsWith( const std::string &str, const std::string &substr, ContainsBehaviour behaviour )
{
  if ( ( str.size() < substr.size() ) || substr.empty() )
    return false;

  if ( behaviour == ContainsBehaviour::CaseSensitive )
    return str.rfind( substr ) == ( str.size() - substr.size() );
  else
    return endsWith( toLower( str ), toLower( substr ), ContainsBehaviour::CaseSensitive );
}

std::vector<std::string> MDAL::split( const std::string &str,
                                      const char delimiter
                                    )
{
  std::vector<std::string> list;
  std::string::const_iterator start = str.begin();
  std::string::const_iterator end = str.end();
  std::string::const_iterator next;
  std::string token;
  do
  {
    next = std::find( start, end, delimiter );
    token = std::string( start, next );
    if ( !token.empty() )
      list.push_back( token );

    if ( next == end )
      break;
    else
      start = next + 1;
  }
  while ( true );
  return list;
}


std::vector<std::string> MDAL::split( const std::string &str,
                                      const std::string &delimiter )
{
  std::vector<std::string> list;
  std::string::size_type start = 0;
  std::string::size_type next;
  std::string token;
  do
  {
    next = str.find( delimiter, start );
    if ( next == std::string::npos )
      token = str.substr( start ); // rest of the string
    else
      token = str.substr( start, next - start ); // part of the string
    if ( !token.empty() )
      list.push_back( token );
    start = next + delimiter.size();
  }
  while ( next != std::string::npos );
  return list;
}

size_t MDAL::toSizeT( const std::string &str )
{
  int i = atoi( str.c_str() );
  if ( i < 0 ) // consistent with atoi return
    i = 0;
  return static_cast< size_t >( i );
}

size_t MDAL::toSizeT( const char &str )
{
  int i = atoi( &str );
  if ( i < 0 ) // consistent with atoi return
    i = 0;
  return static_cast< size_t >( i );
}

size_t MDAL::toSizeT( const double value )
{
  return static_cast<size_t>( value );
}

int MDAL::toInt( const size_t value )
{
  if ( value > std::numeric_limits<int>::max() )
    throw std::runtime_error( "Invalid cast" );
  return static_cast< int >( value );
}

double MDAL::toDouble( const size_t value )
{
  return static_cast< double >( value );
}

double MDAL::toDouble( const std::string &str )
{
  return atof( str.c_str() );
}

int MDAL::toInt( const std::string &str )
{
  return atoi( str.c_str() );
}

std::string MDAL::baseName( const std::string &filename, bool keepExtension )
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

  if ( !keepExtension )
  {
    // Remove extension if present.
    const size_t period_idx = fname.rfind( '.' );
    if ( std::string::npos != period_idx )
    {
      fname.erase( period_idx );
    }
  }
  return fname;
}

std::string MDAL::fileExtension( const std::string &path )
{
  std::string filename = MDAL::baseName( path, true );

  const size_t lastDotIx = filename.find_last_of( "." );
  if ( std::string::npos == lastDotIx )
  {
    return std::string();
  }

  std::string extension = filename.substr( lastDotIx );

  return extension;
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

std::string MDAL::removeFrom( const std::string &str, const std::string &substr )
{
  std::string res( str );
  size_t pos = res.rfind( substr );
  if ( pos != std::string::npos )
  {
    res = res.substr( 0, pos );
  }
  return res;

}

// http://www.cplusplus.com/faq/sequences/strings/trim/
std::string MDAL::trim( const std::string &s, const std::string &delimiters )
{
  if ( s.empty() )
    return s;

  return ltrim( rtrim( s, delimiters ), delimiters );
}

// http://www.cplusplus.com/faq/sequences/strings/trim/
std::string MDAL::ltrim( const std::string &s, const std::string &delimiters )
{
  if ( s.empty() )
    return s;

  size_t found = s.find_first_not_of( delimiters );

  if ( found == std::string::npos )
  {
    return "";
  }
  else
  {
    return s.substr( found );
  }
}

// http://www.cplusplus.com/faq/sequences/strings/trim/
std::string MDAL::rtrim( const std::string &s, const std::string &delimiters )
{
  if ( s.empty() )
    return s;

  size_t found = s.find_last_not_of( delimiters );
  if ( found == std::string::npos )
  {
    return "";
  }
  else
  {
    return s.substr( 0, found + 1 );
  }
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

double MDAL::safeValue( double val, double nodata, double eps )
{
  if ( std::isnan( val ) )
    return val;

  if ( std::isnan( nodata ) )
    return val;

  if ( equals( val, nodata, eps ) )
    return std::numeric_limits<double>::quiet_NaN();

  return val;
}

double MDAL::parseTimeUnits( const std::string &units )
{
  double divBy = 1;
  // We are trying to parse strings like
  //
  // "seconds since 2001-05-05 00:00:00"
  // "hours since 1900-01-01 00:00:0.0"
  // "days since 1961-01-01 00:00:00"
  //
  // or simply
  // hours, days, seconds, ...

  const std::vector<std::string> units_list = MDAL::split( units, " since " );
  std::string unit_definition = units;
  if ( !units_list.empty() )
  {
    unit_definition = units_list[0];
  }

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

  return divBy;
}

std::string MDAL::getCurrentTimeStamp()
{
  time_t t ;
  struct tm *tmp ;
  char MY_TIME[50];
  time( &t );
  tmp = localtime( &t );
  strftime( MY_TIME, sizeof( MY_TIME ), "%Y-%m-%dT%H:%M:%S%z", tmp );
  std::string s = MDAL::trim( MY_TIME );
  return s;
}

MDAL::Statistics _calculateStatistics( const std::vector<double> &values, size_t count, bool isVector, const std::vector<int> &active )
{
  MDAL::Statistics ret;

  double min = std::numeric_limits<double>::quiet_NaN();
  double max = std::numeric_limits<double>::quiet_NaN();
  bool firstIteration = true;

  for ( size_t i = 0; i < count; ++i )
  {
    if ( !active.empty() && active.at( i ) == 0 )
      continue;

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

  for ( std::shared_ptr<Dataset> &ds : grp->datasets )
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
  bool is3D = dataset->group()->dataLocation() == MDAL_DataLocation::DataOnVolumes;
  size_t bufLen = 2000;
  std::vector<double> buffer( isVector ? bufLen * 2 : bufLen );
  std::vector<int> activeBuffer;
  bool activeFaceFlag = dataset->group()->dataLocation() == MDAL_DataLocation::DataOnFaces && dataset->supportsActiveFlag();

  if ( activeFaceFlag )
    activeBuffer.resize( bufLen );

  size_t i = 0;
  while ( i < dataset->valuesCount() )
  {
    size_t valsRead;
    if ( is3D )
    {
      if ( isVector )
      {
        valsRead = dataset->vectorVolumesData( i, bufLen, buffer.data() );
      }
      else
      {
        valsRead = dataset->scalarVolumesData( i, bufLen, buffer.data() );
      }
    }
    else
    {
      if ( isVector )
      {
        valsRead = dataset->vectorData( i, bufLen, buffer.data() );
      }
      else
      {
        valsRead = dataset->scalarData( i, bufLen, buffer.data() );
      }

      if ( activeFaceFlag )
        dataset->activeData( i, bufLen, activeBuffer.data() );
    }
    if ( valsRead == 0 )
      return ret;

    MDAL::Statistics dsStats = _calculateStatistics( buffer, valsRead, isVector, activeBuffer );
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
  std::vector<double> values( mesh->verticesCount() );
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    values[i] = vertices[i].z;
  }
  addVertexScalarDatasetGroup( mesh, values, "Bed Elevation" );
}

static void _addScalarDatasetGroup( MDAL::Mesh *mesh,
                                    const std::vector<double> &values,
                                    const std::string &name,
                                    MDAL_DataLocation location
                                  )
{
  if ( !mesh )
    return;

  size_t maxCount = 0;
  switch ( location )
  {
    case MDAL_DataLocation::DataOnVertices: maxCount = mesh->verticesCount(); break;
    case MDAL_DataLocation::DataOnFaces: maxCount = mesh->facesCount(); break;
    case MDAL_DataLocation::DataOnEdges: maxCount = mesh->edgesCount(); break;
    default:
      assert( false );
  }

  if ( values.empty() )
    return;

  if ( maxCount == 0 )
    return;

  assert( values.size() ==  maxCount );

  std::shared_ptr<MDAL::DatasetGroup> group = std::make_shared< MDAL::DatasetGroup >(
        mesh->driverName(),
        mesh,
        mesh->uri(),
        name
      );
  group->setDataLocation( location );
  group->setIsScalar( true );

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MDAL::MemoryDataset2D >( group.get() );
  dataset->setTime( 0.0 );
  memcpy( dataset->values(), values.data(), sizeof( double )*values.size() );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
}


void MDAL::addFaceScalarDatasetGroup( MDAL::Mesh *mesh,
                                      const std::vector<double> &values,
                                      const std::string &name )
{
  _addScalarDatasetGroup( mesh, values, name, MDAL_DataLocation::DataOnFaces );
}

void MDAL::addVertexScalarDatasetGroup( MDAL::Mesh *mesh, const std::vector<double> &values, const std::string &name )
{
  _addScalarDatasetGroup( mesh, values, name, MDAL_DataLocation::DataOnVertices );
}

void MDAL::addEdgeScalarDatasetGroup( MDAL::Mesh *mesh, const std::vector<double> &values, const std::string &name )
{
  _addScalarDatasetGroup( mesh, values, name, MDAL_DataLocation::DataOnEdges );
}

bool MDAL::isNativeLittleEndian()
{
  // https://stackoverflow.com/a/4181991/2838364
  int n = 1;
  return ( *( char * )&n == 1 );
}

std::string MDAL::coordinateToString( double coordinate, int precision )
{
  std::ostringstream oss;
  oss.setf( std::ios::fixed );
  if ( fabs( coordinate ) > 180 )
    oss.precision( precision ); //seems to not be a geographic coordinate, so 'precision' digits after the digital point
  else
    oss.precision( 6 + precision ); //could be a geographic coordinate, so 'precision'+6 digits after the digital point

  oss << coordinate;

  std::string returnString = oss.str();

  //remove unnecessary '0' or '.'
  if ( returnString.size() > 0 )
  {
    while ( '0' == returnString.back() )
    {
      returnString.pop_back();
    }

    if ( '.' == returnString.back() )
      returnString.pop_back();
  }

  return returnString;
}

std::string MDAL::doubleToString( double value, int precision )
{
  std::ostringstream oss;
  oss.precision( precision );
  oss << value;
  return oss.str();
}

std::string MDAL::prependZero( const std::string &str, size_t length )
{
  if ( length <= str.size() )
    return  str;

  return std::string( length - str.size(), '0' ).append( str );
}

MDAL::RelativeTimestamp::Unit MDAL::parseDurationTimeUnit( const std::string &timeUnit )
{
  MDAL::RelativeTimestamp::Unit unit = MDAL::RelativeTimestamp::hours; //default unit

  if ( timeUnit == "millisec" ||
       timeUnit == "msec" ||
       timeUnit == "millisecs" ||
       timeUnit == "msecs"
     )
  {
    unit = MDAL::RelativeTimestamp::milliseconds;
  }
  else if ( timeUnit == "second" ||
            timeUnit == "seconds" ||
            timeUnit == "Seconds" ||
            timeUnit == "sec" ||
            timeUnit == "secs" ||
            timeUnit == "s" ||
            timeUnit == "se" || // ascii_dat format
            timeUnit == "2" )  // ascii_dat format
  {
    unit = MDAL::RelativeTimestamp::seconds;
  }
  else if ( timeUnit == "minute" ||
            timeUnit == "minutes" ||
            timeUnit == "Minutes" ||
            timeUnit == "min" ||
            timeUnit == "mins" ||
            timeUnit == "mi" || // ascii_dat format
            timeUnit == "1" ) // ascii_dat format
  {
    unit = MDAL::RelativeTimestamp::minutes;
  }
  else if ( timeUnit == "day" ||
            timeUnit == "days" ||
            timeUnit == "Days" )
  {
    unit = MDAL::RelativeTimestamp::days;
  }
  else if ( timeUnit == "week" ||
            timeUnit == "weeks" )
  {
    unit = MDAL::RelativeTimestamp::weeks;
  }


  return unit;
}

MDAL::RelativeTimestamp::Unit MDAL::parseCFTimeUnit( std::string timeInformation )
{
  auto strings = MDAL::split( timeInformation, ' ' );
  if ( strings.size() < 3 )
    return MDAL::RelativeTimestamp::hours; //default value

  if ( strings[1] == "since" )
  {
    std::string timeUnit = strings[0];
    if ( timeUnit == "month" ||
         timeUnit == "months" ||
         timeUnit == "mon" ||
         timeUnit == "mons" )
    {
      return MDAL::RelativeTimestamp::months_CF;
    }
    else if ( timeUnit == "year" ||
              timeUnit == "years" ||
              timeUnit == "yr" ||
              timeUnit == "yrs" )
    {
      return MDAL::RelativeTimestamp::exact_years;
    }

    return MDAL::parseDurationTimeUnit( strings[0] );
  }

  return MDAL::RelativeTimestamp::hours;//default value
}

MDAL::DateTime MDAL::parseCFReferenceTime( const std::string &timeInformation, const std::string &calendarString )
{
  auto strings = MDAL::split( timeInformation, ' ' );
  if ( strings.size() < 3 )
    return MDAL::DateTime();

  if ( strings[1] != "since" )
    return MDAL::DateTime();

  std::string dateString = strings[2];

  auto dateStringValues = MDAL::split( dateString, '-' );
  if ( dateStringValues.size() != 3 )
    return MDAL::DateTime();

  int year = MDAL::toInt( dateStringValues[0] );
  int month = MDAL::toInt( dateStringValues[1] );
  int day = MDAL::toInt( dateStringValues[2] );

  int hours = 0;
  int minutes = 0;
  double seconds = 0;

  if ( strings.size() > 3 )
  {
    std::string timeString = strings[3];
    auto timeStringsValue = MDAL::split( timeString, ":" );
    if ( timeStringsValue.size() == 3 )
    {
      hours = MDAL::toInt( timeStringsValue[0] );
      minutes = MDAL::toInt( timeStringsValue[1] );
      seconds = MDAL::toDouble( timeStringsValue[2] );
    }
  }

  MDAL::DateTime::Calendar calendar;
  if ( calendarString == "gregorian" || calendarString == "standard" || calendarString.empty() )
    calendar = MDAL::DateTime::Gregorian;
  else if ( calendarString == "proleptic_gregorian" )
    calendar = MDAL::DateTime::ProlepticGregorian;
  else if ( calendarString == "julian" )
    calendar = MDAL::DateTime::Julian;
  else
    return MDAL::DateTime();

  return MDAL::DateTime( year, month, day, hours, minutes, seconds, calendar );
}

bool MDAL::getHeaderLine( std::ifstream &stream, std::string &line )
{
  if ( !stream.is_open() ) return false;
  char b[100] = "";
  if ( ! stream.get( b, sizeof( b ) - 1, '\n' ) ) return false;
  line = std::string( b );
  return true;
}

MDAL::Error::Error( MDAL_Status status, std::string message, std::string driverName ): status( status ), mssg( message ), driver( driverName ) {}

void MDAL::Error::setDriver( std::string driverName )
{
  driver = driverName;
}

void MDAL::parseDriverFromUri( const std::string &uri, std::string &driver )
{
  bool hasDriverSet = ( uri.find( ":\"" ) != std::string::npos );
  driver = "";

  if ( !hasDriverSet )
    return;

  driver = MDAL::split( uri, ":\"" )[0];
}

void MDAL::parseMeshFileFromUri( const std::string &uri, std::string &meshFile )
{
  bool hasDriverSet = ( uri.find( ":\"" ) != std::string::npos );
  bool hasSpecificMeshSet = ( uri.find( "\":" ) != std::string::npos );
  meshFile = "";

  if ( !hasDriverSet && !hasSpecificMeshSet )
    meshFile = MDAL::trim( uri, "\"" );
  else if ( hasDriverSet && hasSpecificMeshSet )
  {
    std::string token = MDAL::split( uri, ":\"" )[1]; // split from driver
    token = MDAL::split( token, "\":" )[0]; // split from specific mesh
    meshFile = MDAL::trim( token, "\"" );
  }
  else if ( hasDriverSet )
  {
    std::string token = MDAL::split( uri, ":\"" )[1]; // split from driver
    meshFile = MDAL::trim( token, "\"" );
  }
  else if ( hasSpecificMeshSet )
  {
    std::string token = MDAL::split( uri, "\":" )[0]; // split from specific mesh
    meshFile = MDAL::trim( token, "\"" );
  }
}

void parseSpecificMeshFromUri( const std::string &uri, std::string &meshName )
{
  bool hasSpecificMeshSet = ( uri.find( "\":" ) != std::string::npos );
  meshName = "";

  if ( !hasSpecificMeshSet )
    return;

  std::vector<std::string> tokens = MDAL::split( uri, "\":" );
  if ( tokens.size() > 1 )
  {
    meshName = MDAL::trim( tokens.at( 1 ), "\"" );
  }
}

void MDAL::parseDriverAndMeshFromUri( const std::string &uri, std::string &driver, std::string &meshFile, std::string &meshName )
{
  parseDriverFromUri( uri, driver );
  parseMeshFileFromUri( uri, meshFile );
  parseSpecificMeshFromUri( uri, meshName );
}

std::string MDAL::buildMeshUri( const std::string &meshFile, const std::string &meshName, const std::string &driver )
{
  if ( meshFile.empty() )
    return std::string();

  std::string uri( "" );

  bool hasDriverName = !driver.empty();
  bool hasMeshName = !meshName.empty();

  if ( hasDriverName && hasMeshName )
    uri = driver + ":\"" + meshFile + "\":" + meshName;
  else if ( !hasDriverName && !hasMeshName )
    uri = meshFile;
  else if ( hasDriverName ) // only driver
    uri = driver + ":\"" + meshFile + "\"";
  else if ( hasMeshName ) // only mesh name
    uri = "\"" + meshFile + "\":" + meshName;

  return uri;
}

std::string MDAL::buildAndMergeMeshUris( const std::string &meshFile, const std::vector<std::string> &meshNames, const std::string &driver )
{
  std::string mergedUris;
  size_t meshNamesCount = meshNames.size();

  for ( size_t i = 0; i < meshNamesCount; ++i )
  {
    mergedUris += buildMeshUri( meshFile, meshNames.at( i ), driver );

    if ( ( i + 1 ) < meshNamesCount ) // If this is not the last mesh in array, add separator
      mergedUris += ";;";
  }

  if ( meshNamesCount == 0 )
    mergedUris = buildMeshUri( meshFile, "", driver );

  return mergedUris;
}

MDAL::Library::Library( std::string libraryFile )
{
  d = new Data;
  d->mLibraryFile = libraryFile;
  d->mRef++;
}

MDAL::Library::~Library()
{
  d->mRef--;
#ifdef WIN32
  if ( d->mLibrary &&  d->mRef == 0 )
    FreeLibrary( d->mLibrary );
#else
  if ( d->mLibrary &&  d->mRef == 0 )
    dlclose( d->mLibrary );
#endif
}

MDAL::Library::Library( const MDAL::Library &other )
{
  *this = other;
  d->mRef++;
}

MDAL::Library &MDAL::Library::operator=( const MDAL::Library &other )
{
  d = other.d;
  d->mRef++;

  return ( *this );
}

bool MDAL::Library::isValid()
{
  if ( !d->mLibrary )
    loadLibrary();

  return d->mLibrary != nullptr;
}

std::vector<std::string> MDAL::Library::libraryFilesInDir( const std::string &dirPath )
{
  std::vector<std::string> filesList;
#if defined(WIN32)
  WIN32_FIND_DATA data;
  HANDLE hFind;
  std::string pattern = dirPath;
  pattern.push_back( '*' );

  hFind = FindFirstFile( pattern.c_str(), &data );

  if ( hFind == INVALID_HANDLE_VALUE )
    return filesList;

  do
  {
    std::string fileName( data.cFileName );
    if ( !fileName.empty() && fileExtension( fileName ) == ".dll" )
      filesList.push_back( fileName );
  }
  while ( FindNextFile( hFind, &data ) != 0 );

  FindClose( hFind );
#else
  DIR *dir = opendir( dirPath.c_str() );
  struct dirent *de = readdir( dir );
  while ( de != nullptr )
  {
    std::string fileName( de->d_name );
    if ( !fileName.empty() )
    {
      std::string extentsion = fileExtension( fileName );
      if ( extentsion == ".so" || extentsion == ".dylib" )
        filesList.push_back( fileName );
    }
    de = readdir( dir );
  }

  closedir( dir );
#endif
  return filesList;
}

bool MDAL::Library::loadLibrary()
{
  //should we allow only one successful loading?
  if ( d->mLibrary )
    return false;
#ifdef WIN32
  UINT uOldErrorMode =
    SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS );
  d->mLibrary = LoadLibrary( d->mLibraryFile.c_str() );
  SetErrorMode( uOldErrorMode );
#else
  d->mLibrary = dlopen( d->mLibraryFile.c_str(), RTLD_LAZY );
#endif

  return d->mLibrary != nullptr;
}

