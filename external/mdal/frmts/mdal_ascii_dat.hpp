/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_ASCII_DAT_HPP
#define MDAL_ASCII_DAT_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{

  /**
   * ASCII Dat format is used by various solvers and the output
   * from various solvers can have slightly different header.
   * The format is used by TUFLOW, BASEMENT and HYDRO_AS-2D solvers.
   *
   * The most frequent form is based on official SMS documentation
   * https://www.xmswiki.com/wiki/SMS:ASCII_Dataset_Files_*.dat
   * The official format only supports data defined on vertices,
   * both scalar vector. The new format is recognized by keyword
   * "DATASET" on the first line of the file.
   *
   * BASEMENT solver also stores data defined on elements. Element is
   * either face or edge. To recognize
   * such dataset, the dataset name contains "_els_" substring
   * (e.g. depth_els_1.dat). We do not support reading element data for
   * meshes with combined 2D (faces) and 1D elements (edges)
   *
   * HYDRO_AS-2D solver can have mesh that has numbering gaps, but
   * speficies values for even missing indexes in dataset file
   *
   * In one file, there is always one dataset group stored.
   *
   * Sometime the "older" datasets may have some part of the
   * header missing, e.g. the file starts with SCALAR or VECTOR or TS
   * keyword. The older format does not have "active" flags for faces
   * and does not recognize most of the keywords. Old format data
   * are always defined on vertices
   */
  class DriverAsciiDat: public Driver
  {
    public:
      DriverAsciiDat();
      ~DriverAsciiDat( ) override;
      DriverAsciiDat *create() override;

      bool canReadDatasets( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh ) override;
      bool persist( DatasetGroup *group ) override;

      std::string writeDatasetOnFileSuffix() const override;

    private:
      bool canReadOldFormat( const std::string &line ) const;
      bool canReadNewFormat( const std::string &line ) const;

      void loadOldFormat( std::ifstream &in, Mesh *mesh ) const;
      void loadNewFormat( std::ifstream &in, Mesh *mesh ) const;

      //! Gets maximum (native) index.
      //! For meshes without indexing gap it is vertexCount - 1
      //! For some HYDRO_AS-2D meshes with indexing gaps, it returns
      //! maximum native index of the vertex in defined in the mesh
      size_t maximumId( const Mesh *mesh ) const;

      void readVertexTimestep( const Mesh *mesh,
                               std::shared_ptr<DatasetGroup> group,
                               RelativeTimestamp t,
                               bool isVector,
                               bool hasStatus,
                               std::ifstream &stream ) const;

      void readElementTimestep( const Mesh *mesh,
                                std::shared_ptr<DatasetGroup> group,
                                RelativeTimestamp t,
                                bool isVector,
                                std::ifstream &stream ) const;

      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
