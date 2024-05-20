/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2023 Lutra Consulting Ltd.
*/

#ifndef MDAL_MIKE21_HPP
#define MDAL_MIKE21_HPP

#include <string>
#include <memory>
#include <regex>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{
  class MeshMike21: public MemoryMesh
  {
    public:
      MeshMike21( size_t faceVerticesMaximumCount,
                  const std::string &uri,
                  const std::map<size_t, size_t> vertexIDtoIndex
                );
      ~MeshMike21() override;

      /**
       * Mike21 may supports gaps in the vertex indexing,
       * but we use continuos array of vertices in MDAL
       * \param vertexID internal index/ID of the vertex that native format uses
       * \returns index of the vertex in the continuous array of vertices we returned by readVertices().
       *          For invalid vertexID it is returned index that is out of vertices array bounds.
       */
      virtual size_t vertexIndex( size_t vertexID ) const;

      /**
       * Returns maximum vertex ID.
       * For meshes without gaps in vertex indexing, it is vertex count - 1
       */
      virtual size_t maximumVertexId() const;

    private:
      /**
       * Mike21 might supports "gaps" in the mesh indexing
       * Store only the indices that have different index and ID
       * https://github.com/lutraconsulting/MDAL/issues/51
       */
      std::map<size_t, size_t> mVertexIDtoIndex;
  };

  /**
   * Mike21 format specification
   * Text file format representing mesh vertices and faces
   * Vertices are stored as - id x y z type_of_vertex (0 for water, 1 for land and above 1 for all other boundaries)
   * The format supports triangles and quads
   * Faces are stored as id 1 2 3 [4]
   *
   * full specification here: https://www.xmswiki.com/wiki/SMS:MIKE_21_*.mesh
   *
   * The format has two special lines in the file:
   * First line - that can have format either:
   *    int int int string - these are type of bathymetry data, units of bathymetry data, number of vertices, CRS
   *    int string - these are number of vertices, CRS
   * Line after the vertices (Element header line):
   *    int int int - number of faces, max number of vertexes per face, mesh type code
   *    [21 for faces with triangles only, 25 for faces with also quadrangular elements]
   *
   * The driver creates at least two Vertex Scalar datasets on the Mesh. "Bed Elevation" for Z coordinate
   * and "VertexType" storing the type of vertex (mentioned above). Besides "NativeVertexIds" and "NativeFaceIds"
   * maybe created if the IDS in input data are not continuous.
   */
  class DriverMike21: public Driver
  {
    public:
      DriverMike21();
      ~DriverMike21() override;
      DriverMike21 *create() override;

      int faceVerticesMaximumCount() const override {return 4;}

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;
      void save( const std::string &fileName, const std::string &, Mesh *mesh ) override;

      std::string saveMeshOnFileSuffix() const override;

    private:
      std::string mMeshFile;
      std::string mCrs;
      std::string mDataType;
      std::string mDataUnit;
      size_t mVertexCount = 0;
      // regex for header line in form of - integer string
      const std::regex mRegexHeader2011 = std::regex( "(\\d+)\\s+(.+)(\\s+)?" );
      // regex for header line in form of - integer integer integer string
      const std::regex mRegexHeader2012 = std::regex( "(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(.+)(\\s+)?" );
      // regex for element header line - integer integer integer(code with two number)
      const std::regex mRegexElementHeader = std::regex( "(\\d+)\\s+(\\d)\\s+(\\d{2})(\\s+)?" );

      bool canReadHeader( const std::string &line );
      void parseHeader( const std::string &line );
  };

} // namespace MDAL
#endif //MDAL_MIKE21_HPP
