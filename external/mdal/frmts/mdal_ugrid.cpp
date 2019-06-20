/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_ugrid.hpp"
#include "mdal_utils.hpp"

#include <netcdf.h>
#include <assert.h>

MDAL::DriverUgrid::DriverUgrid()
  : DriverCF(
      "Ugrid",
      "UGRID Results",
      "*.nc" )
{
}

MDAL::DriverUgrid *MDAL::DriverUgrid::create()
{
  return new DriverUgrid();
}

std::string MDAL::DriverUgrid::findMeshName( int dimension, bool optional ) const
{
  const std::vector<std::string> variables = mNcFile.readArrNames();
  for ( const std::string &varName : variables )
  {
    const std::string cf_role = mNcFile.getAttrStr( varName, "cf_role" );
    if ( cf_role == "mesh_topology" )
    {
      int topology_dimension = mNcFile.getAttrInt( varName, "topology_dimension" );
      if ( topology_dimension == dimension )
      {
        return varName;
      }
    }
  }
  if ( optional )
    return "";
  else
    throw MDAL_Status::Err_UnknownFormat;
}

std::string MDAL::DriverUgrid::nodeZVariableName() const
{
  // looks like mesh attributes does not have node_z array name
  // reference
  return mMesh2dName + "_node_z";
}

MDAL::CFDimensions MDAL::DriverUgrid::populateDimensions( )
{
  CFDimensions dims;
  size_t count;
  int ncid;

  mMesh1dName = findMeshName( 1, true ); // optional, may not be present
  mMesh2dName = findMeshName( 2, false ); // force

  // 2D Mesh

  // node_dimension is usually something like nMesh2D_node
  // number of nodes/vertices in the mesh
  const std::string mesh2dNode = mNcFile.getAttrStr( mMesh2dName, "node_dimension" );
  mNcFile.getDimension( mesh2dNode, &count, &ncid );
  dims.setDimension( CFDimensions::Vertex2D, count, ncid );

  // face_dimension is usually something like nMesh2D_face
  // number of faces in the mesh
  const std::string mesh2dFace = mNcFile.getAttrStr( mMesh2dName, "face_dimension" );
  mNcFile.getDimension( mesh2dFace, &count, &ncid );
  dims.setDimension( CFDimensions::Face2D, count, ncid );

  // edge_dimension is usually something like nMesh2D_edge
  // number of edges in the mesh
  const std::string mesh2dEdge = mNcFile.getAttrStr( mMesh2dName, "edge_dimension" );
  mNcFile.getDimension( mesh2dEdge, &count, &ncid );
  dims.setDimension( CFDimensions::Face2DEdge, count, ncid );

  // max_face_nodes_dimension is usually something like max_nMesh2D_face_nodes
  // maximum number of vertices in faces
  const std::string mesh2dMaxNodesInFace = mNcFile.getAttrStr( mMesh2dName, "max_face_nodes_dimension" );
  mNcFile.getDimension( mesh2dMaxNodesInFace, &count, &ncid );
  dims.setDimension( CFDimensions::MaxVerticesInFace, count, ncid );

  // Time
  mNcFile.getDimension( "time", &count, &ncid );
  dims.setDimension( CFDimensions::Time, count, ncid );

  return dims;
}

void MDAL::DriverUgrid::populateFacesAndVertices( Vertices &vertices, Faces &faces )
{
  populateVertices( vertices );
  populateFaces( faces );
}

void MDAL::DriverUgrid::populateVertices( MDAL::Vertices &vertices )
{
  assert( vertices.empty() );
  size_t vertexCount = mDimensions.size( CFDimensions::Vertex2D );
  vertices.resize( vertexCount );
  Vertex *vertexPtr = vertices.data();

  // Parse 2D Mesh
  // node_coordinates should be something like Mesh2D_node_x Mesh2D_node_y
  std::string verticesXName, verticesYName;
  parse2VariablesFromAttribute( mMesh2dName, "node_coordinates", verticesXName, verticesYName, false );
  const std::vector<double> vertices2D_x = mNcFile.readDoubleArr( verticesXName, vertexCount );
  const std::vector<double> vertices2D_y = mNcFile.readDoubleArr( verticesYName, vertexCount );

  std::vector<double> vertices2D_z;
  if ( mNcFile.hasArr( nodeZVariableName() ) )
  {
    vertices2D_z = mNcFile.readDoubleArr( nodeZVariableName(), vertexCount );
  }

  for ( size_t i = 0; i < vertexCount; ++i, ++vertexPtr )
  {
    vertexPtr->x = vertices2D_x[i];
    vertexPtr->y = vertices2D_y[i];
    if ( !vertices2D_z.empty() )
      vertexPtr->z = vertices2D_z[i];
  }
}

void MDAL::DriverUgrid::populateFaces( MDAL::Faces &faces )
{
  assert( faces.empty() );
  size_t faceCount = mDimensions.size( CFDimensions::Face2D );
  faces.resize( faceCount );

  // Parse 2D Mesh
  // face_node_connectivity is usually something like Mesh2D_face_nodes
  const std::string mesh2dFaceNodeConnectivity = mNcFile.getAttrStr( mMesh2dName, "face_node_connectivity" );

  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  int fill_val = mNcFile.getAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" );
  int start_index = mNcFile.getAttrInt( mesh2dFaceNodeConnectivity, "start_index" );
  std::vector<int> face_nodes_conn = mNcFile.readIntArr( mesh2dFaceNodeConnectivity, faceCount * verticesInFace );

  for ( size_t i = 0; i < faceCount; ++i )
  {
    size_t nVertices = verticesInFace;
    std::vector<size_t> idxs;

    for ( size_t j = 0; j < verticesInFace; ++j )
    {
      size_t idx = verticesInFace * i + j;
      int val = face_nodes_conn[idx];

      if ( fill_val == val )
      {
        // found fill val
        nVertices = j;
        assert( nVertices > 1 );
        break;
      }
      else
      {
        idxs.push_back( static_cast<size_t>( val - start_index ) );
      }
    }
    faces[i] = idxs;
  }

}

void MDAL::DriverUgrid::addBedElevation( MDAL::MemoryMesh *mesh )
{
  if ( mNcFile.hasArr( nodeZVariableName() ) ) MDAL::addBedElevationDatasetGroup( mesh, mesh->vertices );
}

std::string MDAL::DriverUgrid::getCoordinateSystemVariableName()
{
  std::string coordinate_system_variable;

  // first try to get the coordinate system variable from grid definition
  if ( mNcFile.hasArr( nodeZVariableName() ) )
  {
    coordinate_system_variable = mNcFile.getAttrStr( nodeZVariableName(), "grid_mapping" );
  }

  // if automatic discovery fails, try to check some hardcoded common variables that store projection
  if ( coordinate_system_variable.empty() )
  {
    if ( mNcFile.hasArr( "projected_coordinate_system" ) )
      coordinate_system_variable = "projected_coordinate_system";
    else if ( mNcFile.hasArr( "wgs84" ) )
      coordinate_system_variable = "wgs84";
  }

  // return, may be empty
  return coordinate_system_variable;
}

std::set<std::string> MDAL::DriverUgrid::ignoreNetCDFVariables()
{
  std::set<std::string> ignore_variables;

  ignore_variables.insert( "projected_coordinate_system" );
  ignore_variables.insert( "time" );
  ignore_variables.insert( "timestep" );

  std::vector<std::string> meshes;
  if ( mNcFile.hasArr( mMesh1dName ) )
    meshes.push_back( mMesh1dName );
  meshes.push_back( mMesh2dName );

  for ( const std::string &mesh : meshes )
  {
    std::string xName, yName;
    ignore_variables.insert( mesh );
    parse2VariablesFromAttribute( mesh, "node_coordinates", xName, yName, true );
    ignore_variables.insert( xName );
    ignore_variables.insert( yName );
    ignore_variables.insert( mNcFile.getAttrStr( mesh, "edge_node_connectivity" ) );
    parse2VariablesFromAttribute( mesh, "edge_coordinates", xName, yName, true );
    if ( !xName.empty() )
    {
      ignore_variables.insert( xName );
      ignore_variables.insert( mNcFile.getAttrStr( xName, "bounds" ) );
    }
    if ( !yName.empty() )
    {
      ignore_variables.insert( yName );
      ignore_variables.insert( mNcFile.getAttrStr( yName, "bounds" ) );
    }
    ignore_variables.insert( mNcFile.getAttrStr( mesh, "face_node_connectivity" ) );
    parse2VariablesFromAttribute( mesh, "face_coordinates", xName, yName, true );
    if ( !xName.empty() )
    {
      ignore_variables.insert( xName );
      ignore_variables.insert( mNcFile.getAttrStr( xName, "bounds" ) );
    }
    if ( !yName.empty() )
    {
      ignore_variables.insert( yName );
      ignore_variables.insert( mNcFile.getAttrStr( yName, "bounds" ) );
    }
    ignore_variables.insert( mNcFile.getAttrStr( mesh, "edge_face_connectivity" ) );
  }

  return ignore_variables;
}

void MDAL::DriverUgrid::parseNetCDFVariableMetadata( int varid, const std::string &variableName, std::string &name, bool *is_vector, bool *is_x )
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
    if ( MDAL::contains( long_name, ", x-component" ) )
    {
      *is_vector = true;
      name = MDAL::replace( long_name, ", x-component", "" );
    }
    else if ( MDAL::contains( long_name, ", y-component" ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( long_name, ", y-component", "" );
    }
    else
    {
      name = long_name;
    }
  }
}

void MDAL::DriverUgrid::parse2VariablesFromAttribute( const std::string &name, const std::string &attr_name,
    std::string &var1, std::string &var2, bool optional ) const
{
  const std::string mesh2dNodeCoordinates = mNcFile.getAttrStr( name, attr_name );
  const std::vector<std::string> chunks = MDAL::split( mesh2dNodeCoordinates, ' ' );

  if ( chunks.size() != 2 )
  {
    if ( optional )
    {
      var1 = "";
      var2 = "";
    }
    else
      throw MDAL_Status::Err_UnknownFormat;
  }
  else
  {
    var1 = chunks[0];
    var2 = chunks[1];
  }
}
