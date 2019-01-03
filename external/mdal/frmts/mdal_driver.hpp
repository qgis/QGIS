/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DRIVER_HPP
#define MDAL_DRIVER_HPP

#include <string>
#include "mdal_data_model.hpp"
#include "mdal.h"

namespace MDAL
{
  enum DriverType
  {
    CanReadMeshAndDatasets,
    CanReadOnlyDatasets
  };

  class Driver
  {
    public:
      Driver( const std::string &name,
              const std::string &longName,
              const std::string &filters,
              DriverType type
            );
      virtual ~Driver();

      virtual Driver *create() = 0;

      std::string name() const;
      std::string longName() const;
      std::string filters() const;
      DriverType type() const;

      virtual bool canRead( const std::string &uri ) = 0;

      // loads mesh
      virtual std::unique_ptr< Mesh > load( const std::string &uri, MDAL_Status *status );
      // loads datasets
      virtual void load( const std::string &uri, Mesh *mesh, MDAL_Status *status );

    private:
      std::string mName;
      std::string mLongName;
      std::string mFilters;
      DriverType mType;
  };

} // namespace MDAL
#endif //MDAL_DRIVER_HPP
