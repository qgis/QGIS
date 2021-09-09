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
    None                      = 0,
    ReadMesh                  = 1 << 0, //!< Can read mesh and all datasets stored in the mesh file
    SaveMesh                  = 1 << 1, //!< Can save the mesh
    ReadDatasets              = 1 << 2, //!< Can read only datasets (groups) from existing mesh
    WriteDatasetsOnVertices   = 1 << 3, //!< Can write datasets (groups) on MDAL_DataLocation::DataOnVertices
    WriteDatasetsOnFaces      = 1 << 4, //!< Can write datasets (groups) on MDAL_DataLocation::DataOnFaces
    WriteDatasetsOnVolumes    = 1 << 5, //!< Can write datasets (groups) on MDAL_DataLocation::DataOnVolumes
    WriteDatasetsOnEdges      = 1 << 6, //!< Can write datasets (groups) on MDAL_DataLocation::DataOnEdges
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
      bool hasWriteDatasetCapability( MDAL_DataLocation location ) const;

      virtual std::string writeDatasetOnFileSuffix() const;
      virtual std::string saveMeshOnFileSuffix() const;

      virtual bool canReadMesh( const std::string &uri );
      virtual bool canReadDatasets( const std::string &uri );

      //! returns the maximum vertices per face
      virtual int faceVerticesMaximumCount() const;

      // constructs loading uri / uris
      virtual std::string buildUri( const std::string &meshFile );
      // loads mesh
      virtual std::unique_ptr< Mesh > load( const std::string &uri, const std::string &meshName = "" );
      // loads datasets
      virtual void load( const std::string &uri, Mesh *mesh );
      // save mesh
      virtual void save( const std::string &fileName, const std::string &meshName, Mesh *mesh );

      // create new dataset group
      virtual void createDatasetGroup(
        Mesh *mesh,
        const std::string &groupName,
        MDAL_DataLocation dataLocation,
        bool hasScalarData,
        const std::string &datasetGroupFile );

      // create new 2D dataset from array
      virtual void createDataset( DatasetGroup *group,
                                  RelativeTimestamp time,
                                  const double *values,
                                  const int *active );

      // create new 3D dataset from array
      virtual void createDataset( DatasetGroup *group,
                                  RelativeTimestamp time,
                                  const double *values,
                                  const int *verticalLevelCount,
                                  const double *verticalExtrusion );

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
