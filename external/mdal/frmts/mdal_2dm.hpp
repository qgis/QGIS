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

namespace MDAL
{
  class Mesh2dm: public MemoryMesh
  {
    public:
      Mesh2dm( size_t faceVerticesMaximumCount,
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
   * The format supports lines, triangles and quads, where the elememts could be
   * stored with some intermediate points (e.g. triangle defined by 6 vertices, vertices
   * and the middle of the edges) We do support only simple definition, so E6T, E8Q and E9Q
   * are not supported.
   * E3T id 1 2 3 mat_id -> face type, id, vertex indices ..., material index
   *
   * full specification here: https://www.xmswiki.com/wiki/SMS:2D_Mesh_Files_*.2dm
   *
   * This will process as many material IDs as promised by the NUM_MATERIALS_PER_ELEM tag and add them as face
   * dataset groups. The naming for these groups is "Material ID" for the first, "Bed Elevation (Face)" for the
   * second, and finally "Auxiliary Material ID <X>" for any subsequent materials, X being a counter to ensure
   * unique group names:
   * E** id 1 2 3 [Material ID] [Bed Elevation (Face)] [Auxiliary Material ID 1] [Auxiliary Material ID 2] ...
   * If the NUM_MATERIALS_PER_ELEM tag is not provided, a fallback mode is used that will only check for the
   * second MATID column and add it under the name "Bed Elevation (Face)" if found.
   * Noe that this is purely a compatibility mode for BASEMENT 3.x releases; NUM_MATERIALS_... is a required
   * tag according to the 2DM specification.
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

      int faceVerticesMaximumCount() const override {return 6;}

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;
      void save( const std::string &fileName, const std::string &, Mesh *mesh ) override;

      std::string saveMeshOnFileSuffix() const override;

    private:
      std::string mMeshFile;
  };

} // namespace MDAL
#endif //MDAL_2DM_HPP
