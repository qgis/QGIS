/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_2DM_HPP
#define MDAL_2DM_HPP

#include <string>
#include <memory>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"


#define MAX_VERTICES_PER_FACE_2DM 4

namespace MDAL
{
  class Mesh2dm: public MemoryMesh
  {
    public:
      Mesh2dm( size_t verticesCount,
               size_t facesCount,
               size_t faceVerticesMaximumCount,
               BBox extent,
               const std::string &uri,
               const std::map<size_t, size_t> vertexIDtoIndex
             );
      ~Mesh2dm() override;


      //! HYDRO_AS-2D supports gaps in the vertex indexing,
      //! but we use continuos array of vertices in MDAL
      //! \param vertexID internal index/ID of the vertex that native format uses
      //! \returns index of the vertex in the continuous array of vertices we returned by readVertices().
      //!          For invalid vertexID it is returned index that is out of vertices array bounds.
      virtual size_t vertexIndex( size_t vertexID ) const;

      //! Returns maximum vertex ID.
      //! For meshes without gaps in vertex indexing, it is vertex count - 1
      virtual size_t maximumVertexId() const;

    private:
      //! 2dm supports "gaps" in the mesh indexing
      //! Store only the indices that have different index and ID
      //! https://github.com/lutraconsulting/MDAL/issues/51
      std::map<size_t, size_t> mVertexIDtoIndex;
  };

  /**
   * 2DM format specification used in TUFLOW, HYDRO_AS-2D and BASEMENET solvers
   * Text file format representing mesh vertices (ND) and faces (E**)
   * ND id x y z
   * Supports lines, triangles and polygons up to 9 vertices (implemented triangles and quads)
   * E3T id 1 2 3 mat_id -> face type, id, vertex indices ..., material index
   *
   * full specification here: https://www.xmswiki.com/wiki/SMS:2D_Mesh_Files_*.2dm
   *
   * Exception for the official specification is for recognition of cell-centered
   * elevation values supported by BASEMENT 3.x releases
   * If face definition has extra column, it is parsed and recognized as
   * elevation, e.g. format for triangle
   * E3T id 1 2 3 mat_id elevation
   * and added automatically as "Bed Elevation (Face)"
   *
   * Note that some 2dm formats do have some extra columns after mat_id column with
   * data with unknown origin/name (e.g. tests/data/2dm/regular_grid.2dm)
   *
   * HYDRO_AS-2D also allows gaps in vertex indexing. In this case we support only files
   * where the vertices are sorted by ID in the source file (limitation of the implementation)
   *
   * Vertex/Face IDs should be indexed from 1. We support indexing from 0 for datasets in xmdf format,
   * but not for ascii dat format (since the loop is from 0 to maximumVertexId() which is in this case
   * numberical_limits<size_t>::max(); (limitation of the implementation)
   */
  class Driver2dm: public Driver
  {
    public:
      Driver2dm();
      ~Driver2dm() override;
      Driver2dm *create() override;

      int faceVerticesMaximumCount() const override
      {return MAX_VERTICES_PER_FACE_2DM;}

      bool canRead( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, MDAL_Status *status ) override;
      void save( const std::string &uri, Mesh *mesh, MDAL_Status *status ) override;

    private:
      std::string mMeshFile;
  };

} // namespace MDAL
#endif //MDAL_2DM_HPP
