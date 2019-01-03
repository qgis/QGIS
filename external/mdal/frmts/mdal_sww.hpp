/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_SWW_HPP
#define MDAL_SWW_HPP

#include <string>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{
  // AnuGA format with extension .SWW
  class DriverSWW: public Driver
  {
    public:
      DriverSWW();
      ~DriverSWW( ) override = default;
      DriverSWW *create() override;

      std::unique_ptr< Mesh > load( const std::string &resultsFile, MDAL_Status *status ) override;
      bool canRead( const std::string &uri ) override;
    private:
      std::string mFileName;
  };
} // namespace MDAL
#endif //MDAL_SWW_HPP
