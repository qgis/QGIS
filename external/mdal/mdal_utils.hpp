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

  /** Return whether file exists */
  bool fileExists( const std::string &filename );

  // strings
  bool startsWith( const std::string &str, const std::string &substr );

  /** Return 0 if not possible to convert */
  size_t toSizeT( const std::string &str );
  double toDouble( const std::string &str );

  enum SplitBehaviour
  {
    SkipEmptyParts,
    KeepEmptyParts
  };
  std::vector<std::string> split( const std::string &str, const std::string &delimiter, SplitBehaviour behaviour );

} // namespace MDAL
#endif //MDAL_UTILS_HPP
