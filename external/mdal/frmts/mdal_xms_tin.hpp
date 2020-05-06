/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_TIN_HPP
#define MDAL_TIN_HPP

#include <string>
#include <memory>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"


#define MAX_VERTICES_PER_FACE_TIN 3

namespace MDAL
{
  /**
   * TIN format specification https://www.xmswiki.com/wiki/TIN_Files
   */
  class DriverXmsTin: public Driver
  {
    public:
      DriverXmsTin();
      ~DriverXmsTin() override;
      DriverXmsTin *create() override;

      int faceVerticesMaximumCount() const override;

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;
  };

} // namespace MDAL
#endif //MDAL_TIN_HPP
