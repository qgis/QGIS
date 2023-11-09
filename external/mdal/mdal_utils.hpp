/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_UTILS_HPP
#define MDAL_UTILS_HPP

#include <string>
#include <vector>
#include <stddef.h>
#include <limits>
#include <sstream>
#include <fstream>
#include <cmath>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

#include <algorithm>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal_datetime.hpp"

// avoid unused variable warnings
#define MDAL_UNUSED(x) (void)x;
#define MDAL_NAN std::numeric_limits<double>::quiet_NaN()

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

namespace MDAL
{

  std::string getEnvVar( const std::string &varname, const std::string &defaultVal = std::string() );

  // endianness
  bool isNativeLittleEndian();

  // numbers
  inline bool equals( double val1, double val2, double eps = std::numeric_limits<double>::epsilon() )
  {
    return fabs( val1 - val2 ) < eps;
  }

  //! returns quiet_NaN if value equals nodata value, otherwise returns val itself
  double safeValue( double val, double nodata, double eps = std::numeric_limits<double>::epsilon() );

  //! Opens the file related to \a inputFileStream
  bool openInputFile( std::ifstream &inputFileStream, const std::string &fileName, std::ifstream::openmode mode = std::ifstream::binary );
  //! Returns a opened input stream file with\a fileName and \a mode
  std::ifstream openInputFile( const std::string &fileName, std::ios_base::openmode mode = std::ifstream::in );

  //! Returns a opened output stream file with\a fileName and \a mode
  std::ofstream openOutputFile( const std::string &fileName, std::ios_base::openmode mode = std::ios_base::in );

  /** Return whether file exists */
  bool fileExists( const std::string &filename );
  std::string baseName( const std::string &filename, bool keepExtension = false );
  std::string fileExtension( const std::string &path );
  std::string dirName( const std::string &filename );
  std::string pathJoin( const std::string &path1, const std::string &path2 );
  std::string readFileToString( const std::string &filename );

  // strings
  enum ContainsBehaviour
  {
    CaseSensitive,
    CaseInsensitive
  };

  bool startsWith( const std::string &str, const std::string &substr, ContainsBehaviour behaviour = CaseSensitive );
  bool endsWith( const std::string &str, const std::string &substr, ContainsBehaviour behaviour = CaseSensitive );
  bool contains( const std::string &str, const std::string &substr, ContainsBehaviour behaviour = CaseSensitive );
  bool contains( const std::vector<std::string> &list, const std::string &str );
  std::string replace( const std::string &str, const std::string &substr, const std::string &replacestr, ContainsBehaviour behaviour = CaseSensitive );
  std::string removeFrom( const std::string &str, const std::string &substr );

  //! left justify and truncate, resulting string will always have width chars
  std::string leftJustified( const std::string &str, size_t width, char fill = ' ' );

  std::string toLower( const std::string &std );

  //! Get a first line from stream clipped to first 100 characters
  bool getHeaderLine( std::ifstream &stream, std::string &line );

  /** Return 0 if not possible to convert */
  size_t toSizeT( const std::string &str );
  size_t toSizeT( const char &str );
  size_t toSizeT( const double value );
  size_t toSizeT( const int value );
  int toInt( const std::string &str );
  int toInt( const size_t value );
  double toDouble( const std::string &str );
  double toDouble( const size_t value );
  bool toBool( const std::string &str );

  //! Returns the string with a adapted format to coordinate
  //! precision is the number of digits after the digital point if fabs(value)>180 (seems to not be a geographic coordinate)
  //! precision+6 is the number of digits after the digital point if fabs(value)<=180 (could be a geographic coordinate)
  std::string coordinateToString( double coordinate, int precision = 2 );

  //! Returns a string with scientific format
  //! precision is the number of signifiant digits
  std::string doubleToString( double value, int precision = 6 );

  /**
   * Splits by deliminer and skips empty parts.
   * Faster than version with std::string
   */
  std::vector<std::string> split( const std::string &str, const char delimiter );

  //! Splits by deliminer and skips empty parts
  std::vector<std::string> split( const std::string &str, const std::string &delimiter );

  std::string join( const std::vector<std::string> parts, const std::string &delimiter );

  //! Right trim
  std::string rtrim(
    const std::string &s,
    const std::string &delimiters = " \f\n\r\t\v" );

  //! Left trim
  std::string ltrim(
    const std::string &s,
    const std::string &delimiters = " \f\n\r\t\v" );

  //! Right and left trim
  std::string trim(
    const std::string &s,
    const std::string &delimiters = " \f\n\r\t\v" );

  //! Returns a string file path encoded consistently with the system from a string encoded with UTF-8
  std::string systemFileName( const std::string &utf8FileName );

  // extent
  BBox computeExtent( const Vertices &vertices );

  // time
  //! Returns a delimiter to get time in hours
  double parseTimeUnits( const std::string &units );
  //! Returns current time stamp in YYYY-mm-ddTHH:MM:SSzoneOffset
  std::string getCurrentTimeStamp();

  // statistics
  void combineStatistics( Statistics &main, const Statistics &other );

  //! Calculates statistics for dataset group
  Statistics calculateStatistics( std::shared_ptr<DatasetGroup> grp );
  Statistics calculateStatistics( DatasetGroup *grp );

  //! Calculates statistics for dataset
  Statistics calculateStatistics( std::shared_ptr<Dataset> dataset );

  // mesh & datasets
  //! Adds bed elevatiom dataset group to mesh
  void addBedElevationDatasetGroup( MDAL::Mesh *mesh, const Vertices &vertices );
  //! Adds a scalar face dataset group with 1 timestep to mesh
  void addFaceScalarDatasetGroup( MDAL::Mesh *mesh, const std::vector<double> &values, const std::string &name );
  //! Adds a scalar vertex dataset group with 1 timestep to mesh
  void addVertexScalarDatasetGroup( MDAL::Mesh *mesh, const std::vector<double> &values, const std::string &name );
  //! Adds a scalar edge dataset group with 1 timestep to mesh
  void addEdgeScalarDatasetGroup( MDAL::Mesh *mesh, const std::vector<double> &values, const std::string &name );

  //! Reads all of type of value. Option to change the endianness is provided
  template<typename T>
  bool readValue( T &value, std::ifstream &in, bool changeEndianness = false )
  {
    char *const p = reinterpret_cast<char *>( &value );

    if ( !in.read( p, sizeof( T ) ) )
      return false;

    if ( changeEndianness )
      std::reverse( p, p + sizeof( T ) );

    return true;
  }

  //! Writes all of type of value. Option to change the endianness is provided
  template<typename T>
  void writeValue( T &value, std::ofstream &out, bool changeEndianness = false )
  {
    T v = value;
    char *const p = reinterpret_cast<char *>( &v );

    if ( changeEndianness )
      std::reverse( p, p + sizeof( T ) );

    out.write( p, sizeof( T ) );
  }

  //! Prepend 0 to string to have n char
  std::string prependZero( const std::string &str, size_t length );

  RelativeTimestamp::Unit parseDurationTimeUnit( const std::string &timeUnit );

  //! parse the time unit in the CF convention string format "XXXX since 2019-01-01 00:00:00"
  //! https://www.unidata.ucar.edu/software/netcdf-java/current/CDM/CalendarDateTime.html
  RelativeTimestamp::Unit parseCFTimeUnit( std::string timeInformation );

  //! parse the reference time in the CF convention string format "XXXX since 2019-01-01 00:00:00"
  //! https://www.unidata.ucar.edu/software/netcdf-java/current/CDM/CalendarDateTime.html
  MDAL::DateTime parseCFReferenceTime( const std::string &timeInformation, const std::string &calendarString );

  /**
    * parses information received from load mesh uri
    * load mesh uri contains: <drivername>:"meshfile":<meshname>
    * drivername and meshname are optional parameters
    */
  void parseDriverAndMeshFromUri( const std::string &uri, std::string &driver, std::string &meshFile, std::string &meshName );

  /**
    * parses only driver information from URI
    */
  void parseDriverFromUri( const std::string &uri, std::string &driver );

  /**
    * parses only meshfile name from URI
    */
  void parseMeshFileFromUri( const std::string &uri, std::string &meshFile );

  /**
    * build uri from provided information about meshfile, driver and meshname
    * only required field is meshFile
    * returns string in formats:
    *  - if both driver and meshname are provided:    <driver>:"meshfile":<meshname>
    *  - if only driver is provided:                  <driver>:"meshfile"
    *  - if only meshname is provided:                "meshfile":meshname
    *  - if neither driver nor meshname is provided:  meshfile
    *  - if not even meshfile is provided:            ""
    */
  std::string buildMeshUri( const std::string &meshFile, const std::string &meshName, const std::string &driver );

  /**
    * helper function to build and merge uris in the same time without the need of cycle
    */
  std::string buildAndMergeMeshUris( const std::string &meshfile, const std::vector<std::string> &meshNames, const std::string &driver = "" );

  struct Error
  {
    Error( MDAL_Status status, std::string message, std::string driverName = "" );
    void setDriver( std::string d );
    MDAL_Status status;
    std::string mssg;
    std::string driver;
  };

  //! Class to handle dynamic library. The loaded library is implicity shared when copying this object
  class Library
  {
    public:
      //! Creater a instance from a library file
      Library( std::string libraryFile );
      ~Library();
      Library( const Library &other );
      Library &operator=( const Library &other );

      //! Returns whether the library is valid after loading the file if needed
      bool isValid();

      //! Returns a list of library file in the folder \a dirPath
      static std::vector<std::string> libraryFilesInDir( const std::string &dirPath );

      /**
       * Returns a function from a symbol name. Caller needs to define what are the types of the arguments and of the returned value :
       * <Type of the returned Value, Type of arg1, Type of arg2, ...>
       */
      template<typename T, typename ... Ts>
      std::function<T( Ts ... args )> getSymbol( const std::string &symbolName )
      {
        if ( !isValid() )
          return std::function<T( Ts ... args )>();
#ifdef _WIN32
        FARPROC proc = GetProcAddress( d->mLibrary, symbolName.c_str() );

        if ( !proc )
          return std::function<T( Ts ... args )>();

        std::function<T( Ts ... args )> symbol = reinterpret_cast<T( * )( Ts ... args )>( proc );
#else
        std::function<T( Ts ... args )> symbol =
          reinterpret_cast<T( * )( Ts ... args )>( dlsym( d->mLibrary, symbolName.c_str() ) );

#if (defined(__APPLE__) && defined(__MACH__))
        /* On mach-o systems, C symbols have a leading underscore and depending
         * on how dlcompat is configured it may or may not add the leading
         * underscore.  If dlsym() fails, add an underscore and try again.
         */
        if ( symbol == nullptr )
        {
          char withUnder[256] = {};
          snprintf( withUnder, sizeof( withUnder ), "_%s", symbolName.c_str() );
          std::function<T( Ts ... args )> symbol = reinterpret_cast<T( * )( Ts ... args )>( dlsym( d->mLibrary, withUnder ) );
        }
#endif
#endif
        return symbol;
      }


    private:
      struct Data
      {
#ifdef _WIN32
        HINSTANCE  mLibrary = nullptr;
#else
        void *mLibrary = nullptr;
#endif
        mutable int mRef = 0;
        std::string mLibraryFile;
      };

      Data *d;

      bool loadLibrary();
  };
} // namespace MDAL
#endif //MDAL_UTILS_HPP
