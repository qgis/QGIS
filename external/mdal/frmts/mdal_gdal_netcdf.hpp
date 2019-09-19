/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_GDAL_NETCDF_HPP
#define MDAL_GDAL_NETCDF_HPP

#include <string>

#include "mdal_gdal.hpp"
#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{

  class DriverGdalNetCDF: public DriverGdal
  {
    public:
      DriverGdalNetCDF();
      ~DriverGdalNetCDF( ) override = default;
      DriverGdalNetCDF *create() override;

    private:
      std::string GDALFileName( const std::string &fileName ) override;
      bool parseBandInfo( const MDAL::GdalDataset *cfGDALDataset,
                          const metadata_hash &metadata, std::string &band_name,
                          double *time, bool *is_vector, bool *is_x
                        ) override;
      void parseGlobals( const metadata_hash &metadata ) override;

      //! delimiter to get time in hours
      double mTimeDiv;
  };

} // namespace MDAL
#endif // MDAL_GDAL_NETCDF_HPP
