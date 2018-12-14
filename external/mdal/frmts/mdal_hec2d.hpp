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
  class DriverHec2D: public Driver
  {
    public:
      DriverHec2D();
      ~DriverHec2D( ) override = default;
      DriverHec2D *create() override;

      bool canRead( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &resultsFile, MDAL_Status *status ) override;

    private:
      std::unique_ptr< MDAL::MemoryMesh > mMesh;
      std::string mFileName;

      void readFaceOutput( const HdfFile &hdfFile,
                           const HdfGroup &rootGroup,
                           const std::vector<size_t> &areaElemStartIndex,
                           const std::vector<std::string> &flowAreaNames,
                           const std::string rawDatasetName,
                           const std::string datasetName,
                           const std::vector<float> &times );
      void readFaceResults( const HdfFile &hdfFile,
                            const std::vector<size_t> &areaElemStartIndex,
                            const std::vector<std::string> &flowAreaNames );
      std::shared_ptr<MDAL::MemoryDataset> readElemOutput(
        const HdfGroup &rootGroup,
        const std::vector<size_t> &areaElemStartIndex,
        const std::vector<std::string> &flowAreaNames,
        const std::string rawDatasetName,
        const std::string datasetName,
        const std::vector<float> &times,
        std::shared_ptr<MDAL::MemoryDataset> bed_elevation );
      std::shared_ptr<MDAL::MemoryDataset> readBedElevation(
        const HdfGroup &gGeom2DFlowAreas,
        const std::vector<size_t> &areaElemStartIndex,
        const std::vector<std::string> &flowAreaNames );
      void setProjection( HdfFile hdfFile );
      void parseMesh( HdfGroup gGeom2DFlowAreas,
                      std::vector<size_t> &areaElemStartIndex,
                      const std::vector<std::string> &flowAreaNames );
      void readElemResults(
        const HdfFile &hdfFile,
        std::shared_ptr<MDAL::MemoryDataset> bed_elevation,
        const std::vector<size_t> &areaElemStartIndex,
        const std::vector<std::string> &flowAreaNames );
  };

} // namespace MDAL
#endif //MDAL_HEC2D_HPP
