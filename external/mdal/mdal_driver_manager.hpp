/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DRIVER_MANAGER_HPP
#define MDAL_DRIVER_MANAGER_HPP

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "mdal.h"
#include "mdal_data_model.hpp"
#include "mdal_logger.hpp"
#include "frmts/mdal_driver.hpp"

namespace MDAL
{

  class DriverManager
  {
    public:
      static DriverManager &instance()
      {
        static DriverManager sInstance;
        return sInstance;
      }
      DriverManager( DriverManager const & )   = delete;
      void operator=( DriverManager const & )  = delete;

      std::string getUris( const std::string &file, const std::string &driverName = "" ) const;

      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName ) const;
      std::unique_ptr< Mesh > load( const std::string &driverName,
                                    const std::string &meshFile,
                                    const std::string &meshName ) const;
      void loadDatasets( Mesh *mesh, const std::string &datasetFile ) const;

      void save( Mesh *mesh, const std::string &uri ) const;

      size_t driversCount() const;
      std::shared_ptr<MDAL::Driver> driver( const std::string &driverName ) const;
      std::shared_ptr<MDAL::Driver> driver( size_t index ) const;

      void loadDynamicDrivers();

    private:
      DriverManager();

      std::vector<std::shared_ptr<MDAL::Driver>> mDrivers;
  };

} // namespace MDAL
#endif //MDAL_DRIVER_MANAGER_HPP
