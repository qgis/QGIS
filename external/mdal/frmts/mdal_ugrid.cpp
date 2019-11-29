/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_ugrid.hpp"
#include "mdal_utils.hpp"

#include <netcdf.h>
#include <assert.h>
#include <algorithm>
#include <cmath>

MDAL::DriverUgrid::DriverUgrid()
  : DriverCF(
      "Ugrid",
      "UGRID Results",
      "*.nc",
      Capability::ReadMesh | Capability::SaveMesh )
{

}

MDAL::DriverUgrid *MDAL::DriverUgrid::create()
{
  return new DriverUgrid();
}

std::string MDAL::DriverUgrid::findMeshName( int dimension, bool optional ) const
{
  const std::vector<std::string> variables = mNcFile->readArrNames();
  for ( const std::string &varName : variables )
  {
    const std::string cf_role = mNcFile->getAttrStr( varName, "cf_role" );
    if ( cf_role == "mesh_topology" )
    {
      int topology_dimension = mNcFile->getAttrInt( varName, "topology_dimension" );
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
  const std::vector<std::string> variables = mNcFile->readArrNames();
  for ( const std::string &varName : variables )
  {
    const std::string stdName = mNcFile->getAttrStr( varName, "standard_name" );
    const std::string meshName = mNcFile->getAttrStr( varName, "mesh" );
    const std::string location = mNcFile->getAttrStr( varName, "location" );

    if ( stdName == "altitude" && meshName == mMesh2dName && location == "node" )
    {
      return varName;
    }
  }

  // not found, the file in non UGRID standard conforming,
  // but lets try the common name
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

  //node dimension location is retrieved from the node variable

  std::vector<std::string> nodeVariablesName = MDAL::split( mNcFile->getAttrStr( mMesh2dName, "node_coordinates" ), ' ' );
  if ( nodeVariablesName.size() < 2 )
    throw MDAL_Status::Err_UnknownFormat;

  std::vector<size_t> nodeDimension;
  std::vector<int> nodeDimensionId;
  mNcFile->getDimensions( nodeVariablesName.at( 0 ), nodeDimension, nodeDimensionId );
  if ( nodeDimension.size() != 1 )
    throw MDAL_Status::Err_UnknownFormat;

  dims.setDimension( CFDimensions::Vertex2D, nodeDimension.at( 0 ), nodeDimensionId.at( 0 ) );


  //face dimension location is retrieved from the face_node_connectivity variable
  //if face_dimension is defined as attribute, the dimension at this location help to desambiguate vertex per faces and number of faces

  std::string faceConnectivityVariablesName = mNcFile->getAttrStr( mMesh2dName, "face_node_connectivity" );
  std::string faceDimensionLocation = mNcFile->getAttrStr( mMesh2dName, "face_dimension" );
  if ( faceConnectivityVariablesName == "" )
    throw MDAL_Status::Err_UnknownFormat;

  size_t facesCount;
  size_t maxVerticesPerFace;

  std::vector<size_t> faceDimension;
  std::vector<int> faceDimensionId;
  int facesIndexDimensionId;
  int maxVerticesPerFaceDimensionId;
  mNcFile->getDimensions( faceConnectivityVariablesName, faceDimension, faceDimensionId );
  if ( faceDimension.size() != 2 )
    throw MDAL_Status::Err_UnknownFormat;

  if ( faceDimensionLocation != "" )
  {
    mNcFile->getDimension( faceDimensionLocation, &facesCount, &ncid );
    if ( facesCount == faceDimension.at( 0 ) )
    {
      facesIndexDimensionId = faceDimensionId.at( 0 );
      maxVerticesPerFaceDimensionId = faceDimensionId.at( 1 );
      maxVerticesPerFace = faceDimension.at( 1 );
    }
    else
    {
      facesIndexDimensionId = faceDimensionId.at( 1 );
      maxVerticesPerFaceDimensionId = faceDimensionId.at( 0 );
      maxVerticesPerFace = faceDimension.at( 0 );
    }
  }
  else
  {
    facesIndexDimensionId = faceDimensionId.at( 0 );
    facesCount = faceDimension.at( 0 );
    maxVerticesPerFaceDimensionId = faceDimensionId.at( 1 );
    maxVerticesPerFace = faceDimension.at( 1 );
  }

  dims.setDimension( CFDimensions::Face2D, facesCount, facesIndexDimensionId );
  dims.setDimension( CFDimensions::MaxVerticesInFace, maxVerticesPerFace, maxVerticesPerFaceDimensionId );



  // number of edges in the mesh, not required for UGRID format
  const std::string mesh2dEdge = mNcFile->getAttrStr( mMesh2dName, "edge_dimension" );
  if ( mNcFile->hasDimension( mesh2dEdge ) )
  {
    mNcFile->getDimension( mesh2dEdge, &count, &ncid );
    dims.setDimension( CFDimensions::Face2DEdge, count, ncid );
  }
  else
  {
    dims.setDimension( CFDimensions::Face2DEdge, 0, -1 );
  }

  // Time not required for UGRID format
  if ( mNcFile->hasDimension( "time" ) )
  {
    mNcFile->getDimension( "time", &count, &ncid );
    dims.setDimension( CFDimensions::Time, count, ncid );
  }
  else
  {
    dims.setDimension( CFDimensions::Time, 0, -1 );
  }

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
  const std::vector<double> vertices2D_x = mNcFile->readDoubleArr( verticesXName, vertexCount );
  const std::vector<double> vertices2D_y = mNcFile->readDoubleArr( verticesYName, vertexCount );

  std::vector<double> vertices2D_z;
  if ( mNcFile->hasArr( nodeZVariableName() ) )
  {
    vertices2D_z = mNcFile->readDoubleArr( nodeZVariableName(), vertexCount );
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
  const std::string mesh2dFaceNodeConnectivity = mNcFile->getAttrStr( mMesh2dName, "face_node_connectivity" );

  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  int fill_val = -1;
  if ( mNcFile->hasAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" ) )
    fill_val = mNcFile->getAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" );
  int start_index = mNcFile->getAttrInt( mesh2dFaceNodeConnectivity, "start_index" );
  std::vector<int> face_nodes_conn = mNcFile->readIntArr( mesh2dFaceNodeConnectivity, faceCount * verticesInFace );

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
  if ( mNcFile->hasArr( nodeZVariableName() ) ) MDAL::addBedElevationDatasetGroup( mesh, mesh->vertices );
}

std::string MDAL::DriverUgrid::getCoordinateSystemVariableName()
{
  std::string coordinate_system_variable;

  // first try to get the coordinate system variable from grid definition
  std::vector<std::string> nodeVariablesName = MDAL::split( mNcFile->getAttrStr( mMesh2dName, "node_coordinates" ), ' ' );
  if ( nodeVariablesName.size() > 1 )
  {
    if ( mNcFile->hasArr( nodeVariablesName[0] ) )
    {
      coordinate_system_variable = mNcFile->getAttrStr( nodeVariablesName[0], "grid_mapping" );
    }
  }


  // if automatic discovery fails, try to check some hardcoded common variables that store projection
  if ( coordinate_system_variable.empty() )
  {
    if ( mNcFile->hasArr( "projected_coordinate_system" ) )
      coordinate_system_variable = "projected_coordinate_system";
    else if ( mNcFile->hasArr( "wgs84" ) )
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
  if ( mNcFile->hasArr( mMesh1dName ) )
    meshes.push_back( mMesh1dName );
  meshes.push_back( mMesh2dName );

  for ( const std::string &mesh : meshes )
  {
    std::string xName, yName;
    ignore_variables.insert( mesh );
    parse2VariablesFromAttribute( mesh, "node_coordinates", xName, yName, true );
    ignore_variables.insert( xName );
    ignore_variables.insert( yName );
    ignore_variables.insert( mNcFile->getAttrStr( mesh, "edge_node_connectivity" ) );
    parse2VariablesFromAttribute( mesh, "edge_coordinates", xName, yName, true );
    if ( !xName.empty() )
    {
      ignore_variables.insert( xName );
      ignore_variables.insert( mNcFile->getAttrStr( xName, "bounds" ) );
    }
    if ( !yName.empty() )
    {
      ignore_variables.insert( yName );
      ignore_variables.insert( mNcFile->getAttrStr( yName, "bounds" ) );
    }
    ignore_variables.insert( mNcFile->getAttrStr( mesh, "face_node_connectivity" ) );
    parse2VariablesFromAttribute( mesh, "face_coordinates", xName, yName, true );
    if ( !xName.empty() )
    {
      ignore_variables.insert( xName );
      ignore_variables.insert( mNcFile->getAttrStr( xName, "bounds" ) );
    }
    if ( !yName.empty() )
    {
      ignore_variables.insert( yName );
      ignore_variables.insert( mNcFile->getAttrStr( yName, "bounds" ) );
    }
    ignore_variables.insert( mNcFile->getAttrStr( mesh, "edge_face_connectivity" ) );
  }

  return ignore_variables;
}

void MDAL::DriverUgrid::parseNetCDFVariableMetadata( int varid, const std::string &variableName, std::string &name, bool *is_vector, bool *is_x )
{
  *is_vector = false;
  *is_x = true;

  std::string long_name = mNcFile->getAttrStr( "long_name", varid );
  if ( long_name.empty() )
  {
    std::string standard_name = mNcFile->getAttrStr( "standard_name", varid );
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
    if ( MDAL::contains( long_name, ", x-component" ) || MDAL::contains( long_name, "u component of " ) )
    {
      *is_vector = true;
      name = MDAL::replace( long_name, ", x-component", "" );
      name = MDAL::replace( name, "u component of ", "" );
    }
    else if ( MDAL::contains( long_name, ", y-component" ) || MDAL::contains( long_name, "v component of " ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( long_name, ", y-component", "" );
      name = MDAL::replace( name, "v component of ", "" );
    }
    else
    {
      name = long_name;
    }
  }
}

std::string MDAL::DriverUgrid::getTimeVariableName() const
{
  return "time";
}

void MDAL::DriverUgrid::parse2VariablesFromAttribute( const std::string &name, const std::string &attr_name,
    std::string &var1, std::string &var2, bool optional ) const
{
  const std::string mesh2dNodeCoordinates = mNcFile->getAttrStr( name, attr_name );
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

void MDAL::DriverUgrid::save( const std::string &uri, MDAL::Mesh *mesh, MDAL_Status *status )
{
  mFileName = uri;

  try
  {
    // Create file
    mNcFile.reset( new NetCDFFile );
    mNcFile->createFile( mFileName );

    // Write globals
    writeGlobals( );

    // Write variables
    writeVariables( mesh );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
  }
}


void MDAL::DriverUgrid::writeVariables( MDAL::Mesh *mesh )
{
  // Global dimensions
  int dimNodeCountId = mNcFile->defineDimension( "nmesh2d_node", mesh->verticesCount() );
  int dimFaceCountId = mNcFile->defineDimension( "nmesh2d_face", mesh->facesCount() );
  mNcFile->defineDimension( "nmesh2d_edge", 1 ); // no data on edges, cannot be 0, since 0==NC_UNLIMITED
  int dimTimeId = mNcFile->defineDimension( "time", NC_UNLIMITED );
  int dimMaxNodesPerFaceId = mNcFile->defineDimension( "max_nmesh2d_face_nodes",
                             mesh->faceVerticesMaximumCount() );

  // Mesh 2D Definition
  int mesh2dId = mNcFile->defineVar( "mesh2d", NC_INT, 0, nullptr );
  mNcFile->putAttrStr( mesh2dId, "cf_role", "mesh_topology" );
  mNcFile->putAttrStr( mesh2dId, "long_name", "Topology data of 2D network" );
  mNcFile->putAttrInt( mesh2dId, "topology_dimension", 2 );
  mNcFile->putAttrStr( mesh2dId, "node_coordinates", "mesh2d_node_x mesh2d_node_y" );
  mNcFile->putAttrStr( mesh2dId, "node_dimension", "nmesh2d_node" );
  mNcFile->putAttrStr( mesh2dId, "edge_dimension", "nmesh2d_edge" );
  mNcFile->putAttrStr( mesh2dId, "max_face_nodes_dimension", "max_nmesh2d_face_nodes" );
  mNcFile->putAttrStr( mesh2dId, "face_node_connectivity", "mesh2d_face_nodes" );
  mNcFile->putAttrStr( mesh2dId, "face_dimension", "nmesh2d_face" );

  // Nodes X coordinate
  int mesh2dNodeXId = mNcFile->defineVar( "mesh2d_node_x", NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeXId, "standard_name", "projection_x_coordinate" );
  mNcFile->putAttrStr( mesh2dNodeXId, "long_name", "x-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeXId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2dNodeXId, "location", "node" );

  // Nodes Y coordinate
  int mesh2dNodeYId = mNcFile->defineVar( "mesh2d_node_y", NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeYId, "standard_name", "projection_y_coordinate" );
  mNcFile->putAttrStr( mesh2dNodeYId, "long_name", "y-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeYId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2dNodeYId, "location", "node" );

  // Nodes Z coordinate
  int mesh2dNodeZId = mNcFile->defineVar( "mesh2d_node_z", NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeZId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2dNodeZId, "location", "node" );
  mNcFile->putAttrStr( mesh2dNodeZId, "coordinates", "mesh2d_node_x mesh2d_node_y" );
  mNcFile->putAttrStr( mesh2dNodeZId, "standard_name", "altitude" );
  mNcFile->putAttrStr( mesh2dNodeZId, "long_name", "z-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeZId, "grid_mapping", "projected_coordinate_system" );
  double fillNodeZCoodVal = -999.0;
  mNcFile->putAttrDouble( mesh2dNodeZId, "_FillValue", fillNodeZCoodVal );

  // Faces 2D Variable
  int mesh2FaceNodesId_dimIds [] { dimFaceCountId, dimMaxNodesPerFaceId };
  int mesh2FaceNodesId = mNcFile->defineVar( "mesh2d_face_nodes", NC_INT, 2, mesh2FaceNodesId_dimIds );
  mNcFile->putAttrStr( mesh2FaceNodesId, "cf_role", "face_node_connectivity" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "location", "face" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "long_name", "Mapping from every face to its corner nodes (counterclockwise)" );
  mNcFile->putAttrInt( mesh2FaceNodesId, "start_index", 0 );
  int fillFace2DVertexValue = -999;
  mNcFile->putAttrInt( mesh2FaceNodesId, "_FillValue", fillFace2DVertexValue );

  // Projected Coordinate System
  int pcsId = mNcFile->defineVar( "projected_coordinate_system", NC_INT, 0, nullptr );

  if ( mesh->crs() == "" )
  {
    mNcFile->putAttrInt( pcsId, "epsg", 0 );
    mNcFile->putAttrStr( pcsId, "EPSG_code", "epgs:0" );
  }
  else
  {
    std::vector<std::string> words = MDAL::split( mesh->crs(), ":" );

    if ( words[0] == "EPSG" && words.size() > 1 )
    {
      mNcFile->putAttrInt( pcsId, "epsg", std::stoi( words[1] ) );
      mNcFile->putAttrStr( pcsId, "EPSG_code", mesh->crs() );
    }
    else
    {
      mNcFile->putAttrStr( pcsId, "wkt", mesh->crs() );
    }
  }

  // Time array
  int timeId = mNcFile->defineVar( "time", NC_DOUBLE, 1, &dimTimeId );
  mNcFile->putAttrStr( timeId, "units", "hours since 2000-01-01 00:00:00" );

  // Turning off define mode - allows data write
  nc_enddef( mNcFile->handle() );

  // Write vertices

  const size_t maxBufferSize = 1000;
  const size_t bufferSize = std::min( mesh->verticesCount(), maxBufferSize );
  const size_t verticesCoordCount = bufferSize * 3;

  std::vector<double> verticesCoordinates( verticesCoordCount );
  std::unique_ptr<MDAL::MeshVertexIterator> vertexIterator = mesh->readVertices();

  size_t vertexIndex = 0;
  size_t vertexFileIndex = 0;
  while ( vertexIndex < mesh->verticesCount() )
  {
    size_t verticesRead = vertexIterator->next( bufferSize, verticesCoordinates.data() );
    if ( verticesRead == 0 )
      break;

    for ( size_t i = 0; i < verticesRead; i++ )
    {
      mNcFile->putDataDouble( mesh2dNodeXId, vertexFileIndex, verticesCoordinates[3 * i] );
      mNcFile->putDataDouble( mesh2dNodeYId, vertexFileIndex, verticesCoordinates[3 * i + 1] );
      if ( std::isnan( verticesCoordinates[3 * i + 2] ) )
        mNcFile->putDataDouble( mesh2dNodeZId, vertexFileIndex, fillNodeZCoodVal );
      else
        mNcFile->putDataDouble( mesh2dNodeZId, vertexFileIndex, verticesCoordinates[3 * i + 2] );
      vertexFileIndex++;
    }
    vertexIndex += verticesRead;
  }

  // Write faces
  std::unique_ptr<MDAL::MeshFaceIterator> faceIterator = mesh->readFaces();
  const size_t faceVerticesMax = mesh->faceVerticesMaximumCount();
  const size_t facesCount = mesh->facesCount();
  const size_t faceOffsetsBufferLen = std::min( facesCount, maxBufferSize );
  const size_t vertexIndicesBufferLen = faceOffsetsBufferLen * faceVerticesMax;

  std::vector<int> faceOffsetsBuffer( faceOffsetsBufferLen );
  std::vector<int> vertexIndicesBuffer( vertexIndicesBufferLen );

  size_t faceIndex = 0;
  while ( faceIndex < facesCount )
  {
    size_t facesRead = faceIterator->next(
                         faceOffsetsBufferLen,
                         faceOffsetsBuffer.data(),
                         vertexIndicesBufferLen,
                         vertexIndicesBuffer.data() );
    if ( facesRead == 0 )
      break;

    for ( size_t i = 0; i < facesRead; i++ )
    {
      std::vector<int> verticesFaceData( faceVerticesMax, fillFace2DVertexValue );
      int startIndex = 0;
      if ( i > 0 )
        startIndex = faceOffsetsBuffer[ i - 1 ];
      int endIndex = faceOffsetsBuffer[ i ];

      size_t k = 0;
      for ( int j = startIndex; j < endIndex; ++j )
      {
        int vertexIndex = vertexIndicesBuffer[ static_cast<size_t>( j ) ];
        verticesFaceData[k++] = vertexIndex;
      }
      mNcFile->putDataArrayInt( mesh2FaceNodesId, faceIndex + i, faceVerticesMax, verticesFaceData.data() );
    }
    faceIndex += facesRead;
  }

  // Time values (not implemented)
  mNcFile->putDataDouble( timeId, 0, 0.0 );

  // Turning on define mode
  nc_redef( mNcFile->handle() );
}

void MDAL::DriverUgrid::writeGlobals()
{
  mNcFile->putAttrStr( NC_GLOBAL, "source", "MDAL " + std::string( MDAL_Version() ) );
  mNcFile->putAttrStr( NC_GLOBAL, "date_created", MDAL::getCurrentTimeStamp() );
  mNcFile->putAttrStr( NC_GLOBAL, "Conventions", "CF-1.6 UGRID-1.0" );
}
