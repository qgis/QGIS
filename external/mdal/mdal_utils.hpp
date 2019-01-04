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

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"

// avoid unused variable warnings
#define MDAL_UNUSED(x) (void)x;
#define MDAL_NAN std::numeric_limits<double>::quiet_NaN()

namespace MDAL
{
  // numbers
  bool equals( double val1, double val2, double eps = std::numeric_limits<double>::epsilon() );

  //! returns quiet_NaN if value equals nodata value, otherwise returns val itself
  double safeValue( double val, double nodata, double eps = std::numeric_limits<double>::epsilon() );

  // debugging
  void debug( const std::string &message );

  /** Return whether file exists */
  bool fileExists( const std::string &filename );
  std::string baseName( const std::string &filename );
  std::string dirName( const std::string &filename );
  std::string pathJoin( const std::string &path1, const std::string &path2 );

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
  //! left justify and truncate, resulting string will always have width chars
  std::string leftJustified( const std::string &str, size_t width, char fill = ' ' );

  std::string toLower( const std::string &std );

  /** Return 0 if not possible to convert */
  size_t toSizeT( const std::string &str );
  int toInt( const std::string &str );
  double toDouble( const std::string &str );
  bool toBool( const std::string &str );
  bool isNumber( const std::string &str );

  enum SplitBehaviour
  {
    SkipEmptyParts,
    KeepEmptyParts
  };
  std::vector<std::string> split( const std::string &str, const std::string &delimiter, SplitBehaviour behaviour );
  std::string join( const std::vector<std::string> parts, const std::string &delimiter );

  // http://www.cplusplus.com/faq/sequences/strings/trim/
  inline std::string rtrim(
    const std::string &s,
    const std::string &delimiters = " \f\n\r\t\v" )
  {
    return s.substr( 0, s.find_last_not_of( delimiters ) + 1 );
  }

  // http://www.cplusplus.com/faq/sequences/strings/trim/
  inline std::string ltrim(
    const std::string &s,
    const std::string &delimiters = " \f\n\r\t\v" )
  {
    return s.substr( s.find_first_not_of( delimiters ) );
  }

  // http://www.cplusplus.com/faq/sequences/strings/trim/
  inline std::string trim(
    const std::string &s,
    const std::string &delimiters = " \f\n\r\t\v" )
  {
    return ltrim( rtrim( s, delimiters ), delimiters );
  }

  // extent
  BBox computeExtent( const Vertices &vertices );

  // time
  //! Returns a delimiter to get time in hours
  double parseTimeUnits( const std::string &units );

  // statistics
  void combineStatistics( Statistics &main, const Statistics &other );

  //! Calculates statistics for dataset group
  Statistics calculateStatistics( std::shared_ptr<DatasetGroup> grp );
  Statistics calculateStatistics( DatasetGroup *grp );

  //! Calculates statistics for dataset
  Statistics calculateStatistics( std::shared_ptr<Dataset> dataset );

  // mesh & datasets
  //! Add bed elevatiom dataset group to mesh
  void addBedElevationDatasetGroup( MDAL::Mesh *mesh, const Vertices &vertices );

} // namespace MDAL
#endif //MDAL_UTILS_HPP
