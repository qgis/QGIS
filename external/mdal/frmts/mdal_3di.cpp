/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_3di.hpp"

MDAL::Loader3Di::Loader3Di( const std::string &fileName )
  : LoaderCF( fileName )
{
}

MDAL::CFDimensions MDAL::Loader3Di::populateDimensions()
{
  CFDimensions dims;
  size_t count;
  int ncid;

  // 2D Mesh
  mNcFile.getDimension( "nMesh2D_nodes", &count, &ncid );
  dims.setDimension( CFDimensions::Face2D, count, ncid );

  mNcFile.getDimension( "nCorner_Nodes", &count, &ncid );
  dims.setDimension( CFDimensions::MaxVerticesInFace, count, ncid );

  // Vertices count is populated later in populateFacesAndVertices
  // it is not known from the array dimensions

  // Time
  mNcFile.getDimension( "time", &count, &ncid );
  dims.setDimension( CFDimensions::Time, count, ncid );

  return dims;
}

void MDAL::Loader3Di::populateFacesAndVertices( MDAL::Mesh *mesh )
{
  assert( mesh );
  size_t faceCount = mDimensions.size( CFDimensions::Face2D );
  mesh->faces.resize( faceCount );
  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  size_t arrsize = faceCount * verticesInFace;
  std::map<std::string, size_t> xyToVertex2DId;

  // X coordinate
  int ncidX = mNcFile.getVarId( "Mesh2DContour_x" );
  double fillX = mNcFile.getFillValue( ncidX );
  std::vector<double> faceVerticesX( arrsize );
  if ( nc_get_var_double( mNcFile.handle(), ncidX, faceVerticesX.data() ) ) throw MDAL_Status::Err_UnknownFormat;

  // Y coordinate
  int ncidY = mNcFile.getVarId( "Mesh2DContour_y" );
  double fillY = mNcFile.getFillValue( ncidY );
  std::vector<double> faceVerticesY( arrsize );
  if ( nc_get_var_double( mNcFile.handle(), ncidY, faceVerticesY.data() ) ) throw MDAL_Status::Err_UnknownFormat;

  // now populate create faces and backtrack which vertices
  // are used in multiple faces
  for ( size_t faceId = 0; faceId < faceCount; ++faceId )
  {
    Face face;

    for ( size_t faceVertexId = 0; faceVertexId < verticesInFace; ++faceVertexId )
    {
      size_t arrId = faceId * verticesInFace + faceVertexId;
      Vertex vertex;
      vertex.x = faceVerticesX[arrId];
      vertex.y = faceVerticesY[arrId];
      vertex.z = 0;

      if ( MDAL::equals( vertex.x, fillX ) || MDAL::equals( vertex.y, fillY ) )
        break;


      size_t vertexId;

      std::string key = std::to_string( vertex.x ) + "," + std::to_string( vertex.y );
      const auto it = xyToVertex2DId.find( key );
      if ( it == xyToVertex2DId.end() )
      {
        // new vertex
        vertexId = mesh->vertices.size();
        xyToVertex2DId[key] = vertexId;
        mesh->vertices.push_back( vertex );
      }
      else
      {
        // existing vertex
        vertexId = it->second;
      }

      face.push_back( vertexId );

    }

    mesh->faces[faceId] = face;
  }

  // Only now we have number of vertices, since we identified vertices that
  // are used in multiple faces
  mDimensions.setDimension( CFDimensions::Vertex2D, mesh->vertices.size() );
}

void MDAL::Loader3Di::addBedElevation( MDAL::Mesh *mesh )
{
  assert( mesh );
  if ( mesh->faces.empty() )
    return;

  size_t faceCount = mesh->faces.size();

  // read Z coordinate of 3di computation nodes centers
  int ncidZ = mNcFile.getVarId( "Mesh2DFace_zcc" );
  double fillZ = mNcFile.getFillValue( ncidZ );
  std::vector<double> coordZ( faceCount );
  if ( nc_get_var_double( mNcFile.handle(), ncidZ, coordZ.data() ) )
    return; //error reading the array


  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >();
  group->isOnVertices = false;
  group->isScalar = true;
  group->setName( "Bed Elevation" );
  group->uri = mesh->uri;
  std::shared_ptr<MDAL::Dataset> dataset = std::make_shared< Dataset >();
  dataset->time = 0.0;
  dataset->values.resize( faceCount );
  dataset->active.resize( faceCount );
  dataset->parent = group.get();
  for ( size_t i = 0; i < faceCount; ++i )
  {
    dataset->values[i].x = MDAL::safeValue( coordZ[i], fillZ );
  }
  group->datasets.push_back( dataset );
  mesh->datasetGroups.push_back( group );
}

std::string MDAL::Loader3Di::getCoordinateSystemVariableName()
{
  return "projected_coordinate_system";
}

std::set<std::string> MDAL::Loader3Di::ignoreNetCDFVariables()
{
  std::set<std::string> ignore_variables;

  ignore_variables.insert( "projected_coordinate_system" );
  ignore_variables.insert( "time" );

  std::vector<std::string> meshes;
  meshes.push_back( "Mesh1D" );
  meshes.push_back( "Mesh2D" );

  for ( const std::string &mesh : meshes )
  {
    ignore_variables.insert( mesh );
    ignore_variables.insert( mesh + "Node_id" );
    ignore_variables.insert( mesh + "Node_type" );

    ignore_variables.insert( mesh + "Face_xcc" );
    ignore_variables.insert( mesh + "Face_ycc" );
    ignore_variables.insert( mesh + "Face_zcc" );
    ignore_variables.insert( mesh + "Contour_x" );
    ignore_variables.insert( mesh + "Contour_y" );
    ignore_variables.insert( mesh + "Face_sumax" );

    ignore_variables.insert( mesh + "Line_id" );
    ignore_variables.insert( mesh + "Line_xcc" );
    ignore_variables.insert( mesh + "Line_ycc" );
    ignore_variables.insert( mesh + "Line_zcc" );
    ignore_variables.insert( mesh + "Line_type" );
  }

  return ignore_variables;
}

std::string MDAL::Loader3Di::nameSuffix( MDAL::CFDimensions::Type type )
{
  MDAL_UNUSED( type );
  return "";
}

void MDAL::Loader3Di::parseNetCDFVariableMetadata( int varid, const std::string &variableName, std::string &name, bool *is_vector, bool *is_x )
{
  *is_vector = false;
  *is_x = true;

  std::string long_name = mNcFile.getAttrStr( "long_name", varid );
  if ( long_name.empty() )
  {
    std::string standard_name = mNcFile.getAttrStr( "standard_name", varid );
    if ( standard_name.empty() )
    {
      name = variableName;
    }
    else
    {
      if ( MDAL::contains( standard_name, "_x_" ) )
      {
        *is_vector = true;
        name = MDAL::replace( standard_name, "_x_", "" );
      }
      else if ( MDAL::contains( standard_name, "_y_" ) )
      {
        *is_vector = true;
        *is_x = false;
        name = MDAL::replace( standard_name, "_y_", "" );
      }
      else
      {
        name = standard_name;
      }
    }
  }
  else
  {
    if ( MDAL::contains( long_name, " in x direction" ) )
    {
      *is_vector = true;
      name = MDAL::replace( long_name, " in x direction", "" );
    }
    else if ( MDAL::contains( long_name, " in y direction" ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( long_name, " in y direction", "" );
    }
    else
    {
      name = long_name;
    }
  }
}
