/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_MEMORY_DATA_MODEL_HPP
#define MDAL_MEMORY_DATA_MODEL_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "mdal.h"
#include "mdal_data_model.hpp"

namespace MDAL
{
  typedef struct
  {
    double x;
    double y;
    double z; // Bed elevation

  } Vertex;

  typedef std::vector<size_t> Face;
  typedef std::vector<Vertex> Vertices;
  typedef std::vector<Face> Faces;

  /**
   * The MemoryDataset stores all the data in the memory
   */
  class MemoryDataset: public Dataset
  {
    public:
      MemoryDataset( DatasetGroup *grp );
      ~MemoryDataset() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

      /**
       * valid pointer only for for dataset defined on vertices
       */
      int *active();
      double *values();

      const int *constActive() const;
      const double *constValues() const;

    private:
      /**
       * Stores vector2d/scalar data for dataset in form
       * scalars: x1, x2, x3, ..., xN
       * vector2D: x1, y1, x2, y2, x3, y3, .... , xN, yN
       *
       * all values are initialized to std::numerical_limits<double>::quiet_NaN (==NODATA)
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
       * Only make sense for dataset defined on vertices  with size == face count
       * For dataset defined on faces, this is empty vector
       *
       * Values are initialized by default to 1 (active)
       */
      std::vector<int> mActive;
  };

  class MemoryMesh: public Mesh
  {
    public:
      MemoryMesh( const std::string &driverName,
                  size_t verticesCount,
                  size_t facesCount,
                  size_t faceVerticesMaximumCount,
                  BBox extent,
                  const std::string &uri );
      ~MemoryMesh() override;

      std::unique_ptr<MDAL::MeshVertexIterator> readVertices() override;
      std::unique_ptr<MDAL::MeshFaceIterator> readFaces() override;

      Vertices vertices;
      Faces faces;
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
