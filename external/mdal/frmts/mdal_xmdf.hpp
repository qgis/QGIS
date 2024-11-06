/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_XMDF_HPP
#define MDAL_XMDF_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_hdf5.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{

  /**
   * The XmdfDataset reads the data directly from HDF5 file
   * by usage of hyperslabs retrieval. This format is used by TUFLOW and HYDRO_AS-2D
   * basically all (timesteps) data for one particular dataset groups
   * are stored in single
   *   3D arrays (time, x, y) for vector datasets
   *   2D arrays (time, x) for scalar datasets
   *   2D arrays (time, active) for active flags (optional, supported by default)
   *
   * For TUFLOW, the dataset groups are structured with a tree starting from a unique group and datasets support active flag value.
   *
   * For HYDRO_AS-2D, all the groups are on the root of the file and the datasets don't support active flag value.
   *
   */
  class XmdfDataset: public Dataset2D
  {
    public:
      XmdfDataset( DatasetGroup *grp,
                   const HdfDataset &valuesDs,
                   const HdfDataset &activeDs,
                   hsize_t timeIndex );
      ~XmdfDataset() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

      const HdfDataset &dsValues() const;
      const HdfDataset &dsActive() const;
      hsize_t timeIndex() const;

    private:
      HdfDataset mHdf5DatasetValues;
      HdfDataset mHdf5DatasetActive;
      // index or row where the data for this timestep begins
      hsize_t mTimeIndex;
  };

  class DriverXmdf: public Driver
  {
    public:
      /**
       * Driver for XMDF Files
       *
       * Structure of the TUFLOW file. Groups are optional since it depends
       * on tools which groups are created.
       * - root
       *   - Temporal
       *     - Depth
       *     - Velocity
       *     - ..
       *   - Maximums
       *     - Depth
       *     - Velocity
       *     - ..
       *   - Difference (res_to_res.exe TUFLOW utility tool)
       *     - ..
       *   - Times (e.g. time of peak velocity)
       *     - ..
       *   - ...
       */
      DriverXmdf();
      ~DriverXmdf( ) override = default;
      DriverXmdf *create() override;

      bool canReadDatasets( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh ) override;

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;

    private:
      MDAL::Mesh *mMesh = nullptr;
      std::string mDatFile;
      std::shared_ptr<MDAL::DatasetGroup> readXmdfGroupAsDatasetGroup(
        const HdfGroup &rootGroup,
        const std::string &groupName,
        size_t vertexCount,
        size_t faceCount ) const;

      void addDatasetGroupsFromXmdfGroup(
        DatasetGroups &groups,
        const HdfGroup &rootGroup,
        const std::string &nameSuffix,
        size_t vertexCount,
        size_t faceCount ) const;

      void readGroupsTree( HdfFile &file,
                           const std::string &name,
                           MDAL::DatasetGroups &groups,
                           size_t vertexCount,
                           size_t faceCount ) const;

      std::string buildUri( const std::string &meshFile ) override;
      std::vector<std::string> findMeshesNames() const;

      std::vector<std::string> meshGroupPaths( const HdfGroup &group ) const;
      std::vector<std::string> meshGroupPaths( const HdfFile &file ) const;
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
