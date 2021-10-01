/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Runette Software Ltd
*/

#ifndef MDAL_PLY_HPP
#define MDAL_PLY_HPP

#include <string>
#include <memory>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"


namespace MDAL
{
  /**
   * PLY format specification : http://gamma.cs.unc.edu/POWERPLANT/papers/ply.pdf
   */
  class DriverPly : public Driver
  {
    public:
      DriverPly();
      ~DriverPly() override;
      DriverPly *create() override;

      bool canReadMesh( const std::string &uri ) override;
      int faceVerticesMaximumCount() const override {return 100;}

      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;
      void save( const std::string &fileName, const std::string &meshName, Mesh *mesh ) override;
      bool persist( DatasetGroup *group ) override;

      std::string saveMeshOnFileSuffix() const override;

    private:
      std::shared_ptr<DatasetGroup> addDatasetGroup( MDAL::Mesh *mesh, const std::string &name, const MDAL_DataLocation location, bool isScalar );
      void addDataset2D( MDAL::DatasetGroup *group, const std::vector<double> &values );
      void addDataset3D( MDAL::DatasetGroup *group,
                         const std::vector<double> &values,
                         const std::vector<int> &valueIndexes,
                         const std::vector<double> &levels,
                         const std::vector<int> &levelIndexes );

  };

} // namespace MDAL
#endif //MDAL_PLY_HPP
