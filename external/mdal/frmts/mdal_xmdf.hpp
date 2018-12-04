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

namespace MDAL
{

  /**
   * The XmdfDataset reads the data directly from HDF5 file
   * by usage of hyperslabs retrieval
   *
   * basically all (timesteps) data for one particular dataset groups
   * are stored in single
   *   3D arrays (time, x, y) for vector datasets
   *   2D arrays (time, x) for scalar datasets
   *   2D arrays (time, active) for active flags
   */
  class XmdfDataset: public Dataset
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

  class LoaderXmdf
  {
    public:
      LoaderXmdf( const std::string &datFile );
      void load( Mesh *mesh, MDAL_Status *status );

    private:
      MDAL::Mesh *mMesh = nullptr;
      std::string mDatFile;
      std::shared_ptr<MDAL::DatasetGroup> readXmdfGroupAsDatasetGroup(
        const HdfGroup &rootGroup,
        const std::string &name,
        size_t vertexCount,
        size_t faceCount );

      void addDatasetGroupsFromXmdfGroup(
        DatasetGroups &groups,
        const HdfGroup &rootGroup,
        size_t vertexCount,
        size_t faceCount );
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
