/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_GDAL_HPP
#define MDAL_GDAL_HPP

#include "gdal.h"

#include <string>
#include <vector>
#include <map>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{

  class GdalDataset
  {
    public:

      GdalDataset(): mHDataset( nullptr ) {}
      ~GdalDataset()
      {
        if ( mHDataset ) GDALClose( mHDataset );
      }

      void init( const std::string &dsName );

      std::string mDatasetName;
      std::string mProj;
      GDALDatasetH mHDataset;

      unsigned int mNBands; /* number of bands */
      unsigned int mXSize; /* number of x pixels */
      unsigned int mYSize; /* number of y pixels */
      unsigned int mNPoints; /* nodes count */
      unsigned int mNVolumes; /* Faces count */
      double mGT[6]; /* affine transform matrix */

    private:
      void parseParameters();
      void parseProj();
  };

  class DriverGdal: public Driver
  {
    public:
      DriverGdal( const std::string &name,
                  const std::string &description,
                  const std::string &filter,
                  const std::string &gdalDriverName );

      virtual ~DriverGdal() override = default;
      bool canRead( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &fileName, MDAL_Status *status ) override;

    protected:
      typedef std::map<std::string, std::string> metadata_hash; // KEY, VALUE

      /* return true on failure */
      virtual bool parseBandInfo( const GdalDataset *cfGDALDataset,
                                  const metadata_hash &metadata, std::string &band_name, double *time, bool *is_vector, bool *is_x ) = 0;
      virtual double parseMetadataTime( const std::string &time_s );
      virtual std::string GDALFileName( const std::string &fileName ); /* some formats require e.g. adding driver name at the beginning */
      virtual std::vector<std::string> parseDatasetNames( const std::string &fileName );
      virtual void parseGlobals( const metadata_hash &metadata ) {MDAL_UNUSED( metadata );}

      void parseBandIsVector( std::string &band_name, bool *is_vector, bool *is_x );

    private:
      typedef std::map<double, std::vector<GDALRasterBandH> > timestep_map; //TIME (sorted), [X, Y]
      typedef std::map<std::string, timestep_map > data_hash; //Data Type, TIME (sorted), [X, Y]
      typedef std::vector<std::shared_ptr<GdalDataset>> gdal_datasets_vector; //GDAL (Sub)Datasets,

      void registerDriver();

      void initFaces( Vertices &nodes, Faces &Faces, bool is_longitude_shifted );
      bool initVertices( Vertices &vertices ); //returns is_longitude_shifted

      const GdalDataset *meshGDALDataset();

      bool meshes_equals( const GdalDataset *ds1, const GdalDataset *ds2 ) const;

      metadata_hash parseMetadata( GDALMajorObjectH gdalBand, const char *pszDomain = nullptr );
      void addDataToOutput( GDALRasterBandH raster_band, std::shared_ptr<MemoryDataset> tos, bool is_vector, bool is_x );
      bool addSrcProj();
      void activateFaces( std::shared_ptr<MemoryDataset> tos );
      void addDatasetGroups();
      void createMesh();
      void parseRasterBands( const GdalDataset *cfGDALDataset );
      void fixRasterBands();

      std::string mFileName;
      const std::string mGdalDriverName; /* GDAL driver name */
      double *mPafScanline; /* temporary buffer for reading one raster line */
      std::unique_ptr< MemoryMesh > mMesh;
      gdal_datasets_vector gdal_datasets;
      data_hash mBands; /* raster bands GDAL handle */
  };

} // namespace MDAL
#endif // MDAL_GDAL_HPP
