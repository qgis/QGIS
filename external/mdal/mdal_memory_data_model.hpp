/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_MEMORY_DATA_MODEL_HPP
#define MDAL_MEMORY_DATA_MODEL_HPP

#include <stddef.h>
#include <assert.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "mdal.h"
#include "mdal_data_model.hpp"

namespace MDAL
{
  class MemoryMesh;

  typedef struct VertexType
  {
    double x = std::numeric_limits<double>::quiet_NaN();
    double y = std::numeric_limits<double>::quiet_NaN();
    double z = 0.0; // Bed elevation

  } Vertex;

  typedef struct
  {
    size_t startVertex;
    size_t endVertex;
  } Edge;

  typedef std::vector<size_t> Face;
  typedef std::vector<Vertex> Vertices;
  typedef std::vector<Edge> Edges;
  typedef std::vector<Face> Faces;

  /**
   * The MemoryDataset stores all the data in the memory
   */
  class MemoryDataset2D: public Dataset2D
  {
    public:
      MemoryDataset2D( DatasetGroup *grp, bool hasActiveFlag = false );
      ~MemoryDataset2D() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;

      //! Returns 0 for datasets that does not support active flags
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

      /**
       * Loop through all faces and activate those which has all 4 values on vertices valid
       * Dataset must support active flags and be defined on vertices
       */
      void activateFaces( MDAL::MemoryMesh *mesh );

      /**
       * Sets active flag for index
       *
       * \param stat 1 for active, 0 for non-active
       * \param index index of the flag
       *
       * Dataset must support active flags
       */
      void setActive( size_t index, int stat )
      {
        assert( supportsActiveFlag() );
        assert( mActive.size() > index );
        mActive[index] = stat;
      }

      void setActive( const int *activeBuffer );

      int active( size_t index ) const
      {
        assert( supportsActiveFlag() );
        assert( mActive.size() > index );
        return mActive[index];
      }

      void setScalarValue( size_t index, double value )
      {
        assert( mValues.size() > index );
        assert( group()->isScalar() );
        mValues[index] = value;
      }

      void setVectorValue( size_t index, double x, double y )
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        mValues[2 * index] = x;
        mValues[2 * index + 1] = y;
      }

      void setValueX( size_t index, double x )
      {
        assert( mValues.size() > 2 * index );
        assert( !group()->isScalar() );

        mValues[2 * index] = x;
      }

      void setValueY( size_t index, double x )
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        mValues[2 * index + 1] = x;
      }

      double valueX( size_t index ) const
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        return mValues[2 * index];
      }

      double valueY( size_t index ) const
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        return mValues[2 * index + 1];
      }

      double scalarValue( size_t index ) const
      {
        assert( mValues.size() > index );
        assert( group()->isScalar() );
        return mValues[index];
      }

      /**
       * Returns pointer to internal buffer with values
       * Never null, already allocated
       * for vector datasets in form x1, y1, ..., xN, yN
       */
      double *values()
      {
        return mValues.data();
      }

    private:
      /**
       * Stores vector2d/scalar data for dataset in form
       * scalars: x1, x2, x3, ..., xN
       * vector2D: x1, y1, x2, y2, x3, y3, .... , xN, yN
       *
       * all values are initialized to std::numerical_limits<double>::quiet_NaN ( == NODATA )
       *
       * size:
       *   - face count if isOnFaces & isScalar
       *   - vertex count if isOnVertices & isScalar
       *   - face count * 2 if isOnFaces & isVector
       *   - vertex count * 2 if isOnVertices & isVector
       */
      std::vector<double> mValues;

      /**
       * Active flag, whether the face is active or not (disabled)
       * Only make sense for dataset defined on vertices
       * For dataset defined on faces, this is empty vector
       *
       * Values are initialized by default to 1 (active)
       */
      std::vector<int> mActive;
  };

  class MemoryDataset3D: public Dataset3D
  {
    public:

      MemoryDataset3D(
        DatasetGroup *grp,
        size_t volumes,
        size_t maxVerticalLevelCount,
        const int *verticalLevelCounts,
        const double *verticalExtrusions
      );

      ~MemoryDataset3D() override;


      void setScalarValue( size_t index, double value )
      {
        assert( mValues.size() > index );
        assert( group()->isScalar() );
        mValues[index] = value;
      }

      void setVectorValue( size_t index, double x, double y )
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        mValues[2 * index] = x;
        mValues[2 * index + 1] = y;
      }

      void setValueX( size_t index, double x )
      {
        assert( mValues.size() > 2 * index );
        assert( !group()->isScalar() );

        mValues[2 * index] = x;
      }

      void setValueY( size_t index, double x )
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        mValues[2 * index + 1] = x;
      }

      double valueX( size_t index ) const
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        return mValues[2 * index];
      }

      double valueY( size_t index ) const
      {
        assert( mValues.size() > 2 * index + 1 );
        assert( !group()->isScalar() );
        return mValues[2 * index + 1];
      }

      double scalarValue( size_t index ) const
      {
        assert( mValues.size() > index );
        assert( group()->isScalar() );
        return mValues[index];
      }

      void updateIndices();

      /**
       * Returns pointer to internal buffer with values
       * Never null, already allocated
       * for vector datasets in form x1, y1, ..., xN, yN
       */
      double *values()
      {
        return mValues.data();
      }

      size_t verticalLevelCountData( size_t indexStart, size_t count, int *buffer ) override;
      size_t verticalLevelData( size_t indexStart, size_t count, double *buffer ) override;
      size_t faceToVolumeData( size_t indexStart, size_t count, int *buffer ) override;
      size_t scalarVolumesData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorVolumesData( size_t indexStart, size_t count, double *buffer ) override;

    private:
      /**
       * Stores vector2d/scalar data for dataset in form
       * scalars: x1, x2, x3, ..., xN
       * vector2D: x1, y1, x2, y2, x3, y3, .... , xN, yN
       *
       * all values are initialized to std::numerical_limits<double>::quiet_NaN ( == NODATA )
       *
       */
      std::vector<double> mValues;

      /**
       * Stores the first index of 3D volume for particular mesh’s face in 3D Stacked Meshes
       *
       * All values are initialised to actual values
       */
      std::vector<int> mFaceToVolume;

      /**
       * Stores number of vertical level for particular mesh’s face in 3D Stacked Meshes.
       *
       * All values are initialised to actual values
       */
      std::vector<int> mVerticalLevelCounts;

      /**
       * Stores vertical level extrusion for particular mesh’s face in 3D Stacked Meshes.
       *
       * All values are initialised to actual values
       */
      std::vector<double> mVerticalExtrusions;
  };

  class MemoryMesh: public Mesh
  {
    public:
      //! Constructs an empty mesh
      MemoryMesh( const std::string &driverName,
                  size_t faceVerticesMaximumCount,
                  const std::string &uri );

      ~MemoryMesh() override;

      std::unique_ptr<MDAL::MeshVertexIterator> readVertices() override;
      std::unique_ptr<MDAL::MeshEdgeIterator> readEdges() override;
      std::unique_ptr<MDAL::MeshFaceIterator> readFaces() override;

      const Vertices &vertices() const {return mVertices;}
      const Faces &faces() const {return mFaces;}
      const Edges &edges() const {return mEdges;}

      //! Sets all vertices using std::move if possible
      void setVertices( Vertices vertices );

      //! Sets all faces using std::move if possible
      void setFaces( Faces faces );

      //! Sets all edges using std::move if possible
      void setEdges( Edges edges );

      size_t verticesCount() const override {return mVertices.size();}
      size_t edgesCount() const override {return mEdges.size();}
      size_t facesCount() const override {return mFaces.size();}
      BBox extent() const override;
      void addVertices( size_t vertexCount, double *coordinates ) override;
      void addFaces( size_t faceCount, size_t driverMaxVerticesPerFace, int *faceSizes, int *vertexIndices ) override;
      void addEdges( size_t edgeCount, int *startVertexIndices, int *endVertexIndices ) override;

      bool isEditable() const override {return true;}

    private:
      BBox mExtent;
      Vertices mVertices;
      Faces mFaces;
      Edges mEdges;
  };

  class MemoryMeshVertexIterator: public MeshVertexIterator
  {
    public:
      MemoryMeshVertexIterator( const MemoryMesh *mesh );
      ~MemoryMeshVertexIterator() override;

      size_t next( size_t vertexCount, double *coordinates ) override;

      const MemoryMesh *mMemoryMesh;
      size_t mLastVertexIndex = 0;
  };

  class MemoryMeshEdgeIterator: public MeshEdgeIterator
  {
    public:
      MemoryMeshEdgeIterator( const MemoryMesh *mesh );
      ~MemoryMeshEdgeIterator() override;

      size_t next( size_t edgeCount,
                   int *startVertexIndices,
                   int *endVertexIndices ) override;

      const MemoryMesh *mMemoryMesh;
      size_t mLastEdgeIndex = 0;
  };

  class MemoryMeshFaceIterator: public MeshFaceIterator
  {
    public:
      MemoryMeshFaceIterator( const MemoryMesh *mesh );
      ~MemoryMeshFaceIterator() override;

      size_t next( size_t faceOffsetsBufferLen,
                   int *faceOffsetsBuffer,
                   size_t vertexIndicesBufferLen,
                   int *vertexIndicesBuffer ) override;

      const MemoryMesh *mMemoryMesh;
      size_t mLastFaceIndex = 0;
  };
} // namespace MDAL
#endif //MDAL_MEMORY_DATA_MODEL_HPP
