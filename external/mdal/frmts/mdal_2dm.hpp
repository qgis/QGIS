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


      //! Some formats supports gaps in the vertex indexing, but we return continuos array from MDAL
      //! for most of the formats this returns
      //! \param vertexID internal index/ID of the vertex that native format uses
      //! \returns index of the vertex in the continuous array of vertices we returned by readVertices()
      virtual size_t vertexIndex( size_t vertexID ) const;

    private:
      // 2dm supports "gaps" in the mesh indexing
      // Store only the indices that have different index and ID
      // https://github.com/lutraconsulting/MDAL/issues/51
      std::map<size_t, size_t> mVertexIDtoIndex;
  };

  class Loader2dm
  {
    public:
      Loader2dm( const std::string &meshFile );
      std::unique_ptr< Mesh > load( MDAL_Status *status );

    private:
      std::string mMeshFile;
  };

} // namespace MDAL
#endif //MDAL_2DM_HPP
