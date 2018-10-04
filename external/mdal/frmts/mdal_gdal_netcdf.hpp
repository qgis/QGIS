/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_GDAL_NETCDF_HPP
#define MDAL_GDAL_NETCDF_HPP

#include "mdal_gdal.hpp"
#include "mdal_data_model.hpp"
#include "mdal.h"
#include <string>

namespace MDAL
{

  class LoaderGdalNetCDF: public LoaderGdal
  {
    public:
      LoaderGdalNetCDF( const std::string &netCDFFile );
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
