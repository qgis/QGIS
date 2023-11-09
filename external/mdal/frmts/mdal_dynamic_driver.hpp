/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/


#ifndef MDAL_DYNAMIC_DRIVER_H
#define MDAL_DYNAMIC_DRIVER_H

#include "mdal_driver.hpp"
#include "mdal_utils.hpp"
#include "mdal.h"

#include <functional>
#include <set>

namespace MDAL
{
  class DriverDynamic: public Driver
  {

    public:
      ~DriverDynamic() = default;

      Driver *create() override;
      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr<Mesh> load( const std::string &uri, const std::string &meshName ) override;

      //! Creates a dynamic driver from a library file
      static Driver *create( const std::string &libFile );

    private:

      DriverDynamic( const std::string &name,
                     const std::string &longName,
                     const std::string &filters,
                     int capabilityFlags,
                     int maxVertexPerFace,
                     const Library &lib );

      bool loadSymbols();
      Library mLibrary;
      int mCapabilityFlags = 0;
      int mMaxVertexPerFace = std::numeric_limits<int>::max();

      std::set<int> mMeshIds;

      //************************************
      std::function<bool ( const char * )> mCanReadMeshFunction;
      std::function<int ( const char *, const char * )> mOpenMeshFunction;
  };

  class MeshDynamicDriver;

  class MeshVertexIteratorDynamicDriver: public MeshVertexIterator
  {
    public:
      MeshVertexIteratorDynamicDriver( const Library &library, int meshId );

      size_t next( size_t vertexCount, double *coordinates ) override;
    private:
      Library mLibrary;
      int mMeshId;
      int mPosition = 0;

      //************************************
      std::function<int ( int, int, int, double * )> mVerticesFunction;
  };

  class MeshFaceIteratorDynamicDriver: public MeshFaceIterator
  {
    public:
      MeshFaceIteratorDynamicDriver( const Library &library, int meshId );

      size_t next( size_t faceOffsetsBufferLen,
                   int *faceOffsetsBuffer,
                   size_t vertexIndicesBufferLen,
                   int *vertexIndicesBuffer ) override;
    private:
      Library mLibrary;
      int mMeshId;
      int mPosition = 0;

      //************************************
      std::function<int ( int, int, int, int *, int, int * )> mFacesFunction;
  };

  class MeshEdgeIteratorDynamicDriver: public MeshEdgeIterator
  {
    public:
      MeshEdgeIteratorDynamicDriver( const Library &library, int meshId );

      size_t next( size_t edgeCount,
                   int *startVertexIndices,
                   int *endVertexIndices );
    private:
      Library mLibrary;
      int mMeshId;
      int mPosition = 0;

      //************************************
      std::function<int ( int, int, int, int *, int * )> mEdgesFunction;
  };


  class DatasetDynamicDriver
  {
    public:
      DatasetDynamicDriver( int meshId,
                            int groupIndex,
                            int datasetIndex,
                            const Library &library );
      virtual ~DatasetDynamicDriver();

      virtual bool loadSymbol();

      //! Removes stored data in memory (for drivers that support lazy loading)
      void unloadData();

    protected:
      int mMeshId = -1;
      int mGroupIndex = -1;
      int mDatasetIndex = -1;
      Library mLibrary;

      //************************************
      std::function<int ( int, int, int, int, int, double * )> mDataFunction;
      std::function<void( int, int, int )> mUnloadFunction;
  };

  class DatasetDynamicDriver2D: public Dataset2D, public DatasetDynamicDriver
  {
    public:
      DatasetDynamicDriver2D( DatasetGroup *parentGroup,
                              int meshId,
                              int groupIndex,
                              int datasetIndex,
                              const Library &library );
      ~DatasetDynamicDriver2D() override;

      bool loadSymbol() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

    private:

      std::function<int ( int, int, int, int, int, int * )> mActiveFlagsFunction;
  };

  class DatasetDynamicDriver3D: public Dataset3D, public DatasetDynamicDriver
  {
    public:
      DatasetDynamicDriver3D( DatasetGroup *parentGroup,
                              int meshId,
                              int groupIndex,
                              int datasetIndex,
                              size_t volumes,
                              size_t maxVerticalLevelCount,
                              const Library &library );
      ~DatasetDynamicDriver3D() override;
      bool loadSymbol() override;

      size_t verticalLevelCountData( size_t indexStart, size_t count, int *buffer ) override;
      size_t verticalLevelData( size_t indexStart, size_t count, double *buffer ) override;
      size_t faceToVolumeData( size_t indexStart, size_t count, int *buffer ) override;
      size_t scalarVolumesData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorVolumesData( size_t indexStart, size_t count, double *buffer ) override;

    private:

      std::function<int ( int, int, int, int, int, int * )> mVerticalLevelCountDataFunction;
      std::function<int ( int, int, int, int, int, double * )> mVerticalLevelDataFunction;
      std::function<int ( int, int, int, int, int, int * )> mFaceToVolumeDataFunction;

  };

  class MeshDynamicDriver: public Mesh
  {
    public:
      MeshDynamicDriver( const std::string &driverName,
                         size_t faceVerticesMaximumCount,
                         const std::string &uri,
                         const Library &library,
                         int meshId );
      ~MeshDynamicDriver();

      std::unique_ptr<MeshVertexIterator> readVertices() override;
      std::unique_ptr<MeshEdgeIterator> readEdges() override;
      std::unique_ptr<MeshFaceIterator> readFaces() override;
      size_t verticesCount() const override;
      size_t edgesCount() const override;
      size_t facesCount() const override;
      BBox extent() const override;

      //! Set the projection from the source
      void setProjection();

      bool populateDatasetGroups();

      //! Returns whether all the symbols have been loaded
      bool loadSymbol();

    private:
      Library mLibrary;
      int mId = -1;

      //************************************
      std::function<int ( int )> mMeshVertexCountFunction;
      std::function<int ( int )> mMeshFaceCountFunction;
      std::function<int ( int )> mMeshEdgeCountFunction;
      std::function<void ( int, double *, double *, double *, double * )> mMeshExtentFunction;
      std::function<const char *( int )> mMeshProjectionFunction;
      std::function<int ( int )> mMeshDatasetGroupsCountFunction;

      std::function<const char *( int, int )> mDatasetgroupNameFunction;
      std::function<const char *( int, int )> mDatasetGroupReferencetimeFunction;
      std::function<int ( int, int )> mDatasetGroupMetadataCountFunction;
      std::function<const char *( int, int, int )> mDatasetGroupMetadataKeyFunction;
      std::function<const char *( int, int, int )> mDatasetGroupMetadataValueFunction;
      std::function < bool ( int, int, bool *, int *, int * )> mDatasetDescriptionFunction;
      std::function < double( int, int, int, bool * )> mDatasetTimeFunction;
      std::function<bool ( int, int, int )> mDatasetSupportActiveFlagFunction;
      std::function<int ( int, int, int )> mDataset3DMaximumVerticalLevelCount;
      std::function<int ( int, int, int )> mDataset3DVolumeCount;

      std::function<void ( int )> mCloseMeshFunction;
  };
}


#endif // MDAL_DYNAMIC_DRIVER_H
