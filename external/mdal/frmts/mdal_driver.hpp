/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DRIVER_HPP
#define MDAL_DRIVER_HPP

#include <string>
#include "mdal_data_model.hpp"
#include "mdal.h"

namespace MDAL
{
  enum Capability
  {
    None          = 0,
    ReadMesh      = 1 << 0, //! Can read mesh and all datasets stored in the mesh file
    ReadDatasets  = 1 << 1, //! Can read only datasets (groups) from existing mesh
    WriteDatasets = 1 << 2, //! Can write datasets (groups)
    SaveMesh      = 1 << 3, //! Can save the mesh
  };

  class Driver
  {
    public:
      Driver( const std::string &name,
              const std::string &longName,
              const std::string &filters,
              int capabilityFlags
            );
      virtual ~Driver();

      virtual Driver *create() = 0;

      std::string name() const;
      std::string longName() const;
      std::string filters() const;
      bool hasCapability( Capability capability ) const;

      virtual bool canRead( const std::string &uri ) = 0;

      //! returns the maximum vertices per face
      virtual int faceVerticesMaximumCount() const;

      // loads mesh
      virtual std::unique_ptr< Mesh > load( const std::string &uri, MDAL_Status *status );
      // loads datasets
      virtual void load( const std::string &uri, Mesh *mesh, MDAL_Status *status );

      // save mesh
      virtual void save( const std::string &uri, Mesh *mesh, MDAL_Status *status );

      // create new dataset group
      virtual void createDatasetGroup(
        Mesh *mesh,
        const std::string &groupName,
        bool isOnVertices,
        bool hasScalarData,
        const std::string &datasetGroupFile );

      // create new dataset from array
      virtual void createDataset( DatasetGroup *group,
                                  double time,
                                  const double *values,
                                  const int *active );

      // persist to the file
      // returns true on error, false on success
      virtual bool persist( DatasetGroup *group );

    private:
      std::string mName;
      std::string mLongName;
      std::string mFilters;
      int mCapabilityFlags;
  };

} // namespace MDAL
#endif //MDAL_DRIVER_HPP
