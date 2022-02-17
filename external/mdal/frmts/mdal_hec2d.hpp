/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_HEC2D_HPP
#define MDAL_HEC2D_HPP

#include <string>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_hdf5.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{
  /**
   * HEC-RAS 2D format.
   *
   * This is a HDF5-based format to store mesh and datasets
   * in a single file. The format supports meshes is multiple
   * (disconnected) areas.
   *
   * There is a small change in the format in HEC-RAS 5.0.5+, where
   *    - Header File Type is different (HEC-RAS Results vs HEC-RAS Geometry)
   *    - Names or areas are stored in different place (names array vs attributes array)
   *
   * Time data unit should be present in Time dataset and Time or Variable attribute for given dataset root,
   * Since MDAL API is reporting times in float hours, the original values need to be corrected
   * based on value found in the Time attribute.
   *
   * All reference times can be found in Time Data Stamp dataset.
   * First value in the dataset is reported by MDAL as reference time
   *
   */
  class DriverHec2D: public Driver
  {
    public:
      DriverHec2D();
      ~DriverHec2D( ) override = default;
      DriverHec2D *create() override;

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &resultsFile, const std::string &meshName = "" ) override;

    private:
      std::unique_ptr< MDAL::MemoryMesh > mMesh;
      std::string mFileName;

      std::vector<MDAL::RelativeTimestamp> mTimes ;
      DateTime mReferenceTime;

      // Regular format
      bool canReadFormat( const std::string &fileType ) const;

      // 5.0.5 format
      bool canReadFormat505( const std::string &fileType ) const;

      std::vector<std::string> read2DFlowAreasNamesFromNameDataset( HdfGroup gGeom2DFlowAreas ) const;
      std::vector<std::string> read2DFlowAreasNamesFromAttributesDataset( HdfGroup gGeom2DFlowAreas ) const;

      // Common functions
      void readFaceOutput( const HdfFile &hdfFile,
                           const HdfGroup &rootGroup,
                           const std::vector<size_t> &areaElemStartIndex,
                           const std::vector<std::string> &flowAreaNames,
                           const std::string rawDatasetName,
                           const std::string datasetName,
                           const std::vector<MDAL::RelativeTimestamp> &times,
                           const DateTime &referenceTime );

      void readFaceResults( const HdfFile &hdfFile,
                            const std::vector<size_t> &areaElemStartIndex,
                            const std::vector<std::string> &flowAreaNames );

      std::shared_ptr<MDAL::MemoryDataset2D> readElemOutput(
        const HdfGroup &rootGroup,
        const std::vector<size_t> &areaElemStartIndex,
        const std::vector<std::string> &flowAreaNames,
        const std::string rawDatasetName,
        const std::string datasetName,
        const std::vector<MDAL::RelativeTimestamp> &times,
        std::shared_ptr<MDAL::MemoryDataset2D> bed_elevation,
        const DateTime &referenceTime );

      std::shared_ptr<MDAL::MemoryDataset2D> readBedElevation(
        const HdfGroup &gGeom2DFlowAreas,
        const std::vector<size_t> &areaElemStartIndex,
        const std::vector<std::string> &flowAreaNames );

      void setProjection( HdfFile hdfFile );

      void parseMesh( HdfGroup gGeom2DFlowAreas,
                      std::vector<size_t> &areaElemStartIndex,
                      const std::vector<std::string> &flowAreaNames );

      void readElemResults(
        const HdfFile &hdfFile,
        std::shared_ptr<MDAL::MemoryDataset2D> bed_elevation,
        const std::vector<size_t> &areaElemStartIndex,
        const std::vector<std::string> &flowAreaNames );
  };

} // namespace MDAL
#endif //MDAL_HEC2D_HPP
