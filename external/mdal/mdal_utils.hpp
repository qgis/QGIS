/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_UTILS_HPP
#define MDAL_UTILS_HPP

#include <string>
#include <vector>
#include <stddef.h>

namespace MDAL
{

  // debugging
  void debug( const std::string &message );

  /** Return whether file exists */
  bool fileExists( const std::string &filename );
  std::string baseName( const std::string &filename );

  // strings
  bool startsWith( const std::string &str, const std::string &substr );
  bool contains( const std::string &str, const std::string &substr );

  /** Return 0 if not possible to convert */
  size_t toSizeT( const std::string &str );
  double toDouble( const std::string &str );
  bool toBool( const std::string &str );

  enum SplitBehaviour
  {
    SkipEmptyParts,
    KeepEmptyParts
  };
  std::vector<std::string> split( const std::string &str, const std::string &delimiter, SplitBehaviour behaviour );

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

} // namespace MDAL
#endif //MDAL_UTILS_HPP
