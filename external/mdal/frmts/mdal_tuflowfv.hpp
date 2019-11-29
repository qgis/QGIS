/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_TUFLOWFV_HPP
#define MDAL_TUFLOWFV_HPP

#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"
#include "mdal_cf.hpp"

namespace MDAL
{
  class TuflowFVDataset3D: public Dataset3D
  {
    public:
      TuflowFVDataset3D(
        DatasetGroup *parent,
        int ncid_x,
        int ncid_y,
        size_t timesteps,
        size_t volumesCount,
        size_t facesCount,
        size_t levelFacesCount,
        size_t ts,
        size_t maximumLevelsCount,
        std::shared_ptr<NetCDFFile> ncFile
      );
      virtual ~TuflowFVDataset3D() override;

      size_t verticalLevelCountData( size_t indexStart, size_t count, int *buffer ) override;
      size_t verticalLevelData( size_t indexStart, size_t count, double *buffer ) override;
      size_t faceToVolumeData( size_t indexStart, size_t count, int *buffer ) override;
      size_t scalarVolumesData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorVolumesData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeVolumesData( size_t indexStart, size_t count, int *buffer ) override;

    private:
      int mNcidX; //!< NetCDF variable id
      int mNcidY; //!< NetCDF variable id
      size_t mTimesteps;
      size_t mFacesCount;
      size_t mLevelFacesCount;
      size_t mTs;
      std::shared_ptr<NetCDFFile> mNcFile;

      int mNcidVerticalLevels = -1; //! variable id of int NL(NumCells2D) ;
      int mNcidVerticalLevelsZ = -1; //! variable id of float layerface_Z(Time, NumLayerFaces3D) ;
      int mNcidActive2D = -1; //! variable id of int stat(Time, NumCells2D) ;
      int mNcid3DTo2D = -1; //! variable id of int idx2(NumCells3D) ;
      int mNcid2DTo3D = -1; //! variable id of int idx3(NumCells2D) ;
  };

  /**
   * TUFLOW FV format
   *
   * Binary NetCDF format with structure similar to UGRID stored as
   * 3D Layered Mesh (https://github.com/qgis/QGIS-Enhancement-Proposals/issues/158)
   *
   * Both mesh and dataset is stored in single file.
   */
  class DriverTuflowFV: public DriverCF
  {
    public:
      DriverTuflowFV();
      ~DriverTuflowFV() override;
      DriverTuflowFV *create() override;

    private:
      CFDimensions populateDimensions( ) override;
      void populateFacesAndVertices( Vertices &vertices, Faces &faces ) override;
      void addBedElevation( MemoryMesh *mesh ) override;
      std::string getCoordinateSystemVariableName() override;
      std::set<std::string> ignoreNetCDFVariables() override;
      void parseNetCDFVariableMetadata( int varid, const std::string &variableName,
                                        std::string &name, bool *is_vector, bool *is_x ) override;
      std::string getTimeVariableName() const override;

      // TODO 2d dataset with active flag
      // TODO CRS from prj file

      std::shared_ptr<MDAL::Dataset> create3DDataset(
        std::shared_ptr<MDAL::DatasetGroup> group,
        size_t ts,
        const MDAL::CFDatasetGroupInfo &dsi,
        double fill_val_x, double fill_val_y ) override;

      void addBedElevationDatasetOnFaces();
      void populateVertices( MDAL::Vertices &vertices );
      void populateFaces( MDAL::Faces &faces );

      void calculateMaximumLevelCount();
      int mMaximumLevelsCount = -1;
  };

} // namespace MDAL
#endif //MDAL_TUFLOWFV_HPP
