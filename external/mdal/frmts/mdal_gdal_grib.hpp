/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_GDAL_GRIB_HPP
#define MDAL_GDAL_GRIB_HPP

#include "mdal_gdal.hpp"
#include "mdal_data_model.hpp"
#include "mdal.h"
#include <string>

namespace MDAL
{

  class LoaderGdalGrib: public LoaderGdal
  {
    public:
      LoaderGdalGrib( const std::string &gribFile );
    private:
      bool parseBandInfo( const MDAL::GdalDataset *cfGDALDataset,
                          const metadata_hash &metadata, std::string &band_name,
                          double *time, bool *is_vector, bool *is_x
                        ) override;

      /**
       * ref time (UTC sec)
       *
       * parsed only once, because
       * some GRIB files do not use FORECAST_SEC, but VALID_TIME
       * metadata, so ref time varies with dataset-to-dataset
       */
      double mRefTime;
  };

} // namespace MDAL
#endif // MDAL_GDAL_GRIB_HPP
