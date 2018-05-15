/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DEFINES_HPP
#define MDAL_DEFINES_HPP

#include <stddef.h>
#include <vector>

namespace MDAL
{

  typedef struct
  {
    double x;
    double y;
  } Vertex;

  typedef std::vector<size_t> Face;

  struct Mesh
  {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
  };

} // namespace MDAL
#endif //MDAL_DEFINES_HPP

