/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_3di.hpp"
#include "mdal_logger.hpp"
#include <cmath>
#include <netcdf.h>
#include <assert.h>
#include "mdal_sqlite3.hpp"


MDAL::Driver3Di::Driver3Di()
  : DriverCF(
      "3Di",
      "3Di Results",
      "results_3di.nc",
      Capability::ReadMesh )
{
}

MDAL::Driver3Di *MDAL::Driver3Di::create()
{
  return new Driver3Di();
}

std::string MDAL::Driver3Di::buildUri( const std::string &meshFile )
{
  mNcFile.reset( new NetCDFFile );

  try
  {
    mNcFile->openFile( meshFile );
  }
  catch ( MDAL::Error &err )
  {
    err.setDriver( name() );
    MDAL::Log::error( err );
    return std::string();
  }

  std::vector<std::string> meshNames;

  CFDimensions dims;
  bool sqliteFileExist = check1DConnection( meshFile );
  if ( sqliteFileExist )
  {
    populate1DMeshDimensions( dims );
    if ( dims.size( CFDimensions::Vertex ) > 0 && dims.size( CFDimensions::Edge ) > 0 )
    {
      meshNames.push_back( "Mesh1D" );
    }
  }

  populate2DMeshDimensions( dims );
  if ( dims.size( CFDimensions::Face ) > 0 )
    meshNames.push_back( "Mesh2D" );

  if ( !meshNames.size() )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "No meshes found in file" + meshFile );
    return std::string( "" );
  }

  return MDAL::buildAndMergeMeshUris( meshFile, meshNames, name() );
}

void MDAL::Driver3Di::populate2DMeshDimensions( MDAL::CFDimensions &dims )
{
  size_t count;
  int ncid;

  // 2D Mesh
  mNcFile->getDimension( "nMesh2D_nodes", &count, &ncid );
  dims.setDimension( CFDimensions::Face, count, ncid );

  mNcFile->getDimension( "nCorner_Nodes", &count, &ncid );
  dims.setDimension( CFDimensions::MaxVerticesInFace, count, ncid );

  // Vertices count is populated later in populateElements
  // it is not known from the array dimensions
}

MDAL::CFDimensions MDAL::Driver3Di::populateDimensions( )
{
  CFDimensions dims;
  size_t count;
  int ncid;

  if ( mRequestedMeshName == "Mesh1D" )
  {
    populate1DMeshDimensions( dims );
  }
  else
  {
    // Default loaded mesh is the 2D mesh
    populate2DMeshDimensions( dims );
  }

  // Time
  mNcFile->getDimension( "time", &count, &ncid );
  dims.setDimension( CFDimensions::Time, count, ncid );

  return dims;
}

void MDAL::Driver3Di::populateElements( Vertices &vertices, Edges &edges, Faces &faces )
{
  if ( mRequestedMeshName == "Mesh1D" )
    populateMesh1DElements( vertices, edges );
  else
    populateMesh2DElements( vertices, faces );
}

void MDAL::Driver3Di::populateMesh2DElements( MDAL::Vertices &vertices, MDAL::Faces &faces )
{
  assert( vertices.empty() );
  size_t faceCount = mDimensions.size( CFDimensions::Face );
  faces.resize( faceCount );
  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  size_t arrsize = faceCount * verticesInFace;
  std::map<std::string, size_t> xyToVertex2DId;

  // X coordinate
  int ncidX = mNcFile->getVarId( "Mesh2DContour_x" );
  double fillX = mNcFile->getFillValue( ncidX );
  std::vector<double> faceVerticesX( arrsize );
  if ( nc_get_var_double( mNcFile->handle(), ncidX, faceVerticesX.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

  // Y coordinate
  int ncidY = mNcFile->getVarId( "Mesh2DContour_y" );
  double fillY = mNcFile->getFillValue( ncidY );
  std::vector<double> faceVerticesY( arrsize );
  if ( nc_get_var_double( mNcFile->handle(), ncidY, faceVerticesY.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

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
        vertexId = vertices.size();
        xyToVertex2DId[key] = vertexId;
        vertices.push_back( vertex );
      }
      else
      {
        // existing vertex
        vertexId = it->second;
      }

      face.push_back( vertexId );

    }

    faces[faceId] = face;
  }

  // Only now we have number of vertices, since we identified vertices that
  // are used in multiple faces
  mDimensions.setDimension( CFDimensions::Vertex, vertices.size() );
}

void MDAL::Driver3Di::addBedElevation( MemoryMesh *mesh )
{
  assert( mesh );
  if ( 0 == mesh->facesCount() )
    return;

  size_t faceCount = mesh->facesCount();

  // read Z coordinate of 3di computation nodes centers
  int ncidZ = mNcFile->getVarId( "Mesh2DFace_zcc" );
  double fillZ = mNcFile->getFillValue( ncidZ );
  std::vector<double> coordZ( faceCount );
  if ( nc_get_var_double( mNcFile->handle(), ncidZ, coordZ.data() ) )
    return; //error reading the array


  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mesh,
                                          mesh->uri(),
                                          "Bed Elevation"
                                        );

  group->setDataLocation( MDAL_DataLocation::DataOnFaces );
  group->setIsScalar( true );

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MemoryDataset2D >( group.get() );
  dataset->setTime( MDAL::RelativeTimestamp() );
  for ( size_t i = 0; i < faceCount; ++i )
  {
    dataset->setScalarValue( i, MDAL::safeValue( coordZ[i], fillZ ) );
  }
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  group->datasets.push_back( dataset );
  mesh->datasetGroups.push_back( group );
}

std::string MDAL::Driver3Di::getCoordinateSystemVariableName()
{
  return "projected_coordinate_system";
}

std::string MDAL::Driver3Di::getTimeVariableName() const
{
  return "time";
}

std::set<std::string> MDAL::Driver3Di::ignoreNetCDFVariables()
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

void MDAL::Driver3Di::parseNetCDFVariableMetadata( int varid,
    std::string &variableName,
    std::string &name,
    bool *is_vector,
    bool *isPolar,
    bool *invertedDirection,
    bool *is_x )
{
  *is_vector = false;
  *is_x = true;
  *isPolar = false;
  MDAL_UNUSED( invertedDirection )

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
      variableName = standard_name;
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
    variableName = long_name;
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

std::vector<std::pair<double, double>> MDAL::Driver3Di::parseClassification( int varid ) const
{
  MDAL_UNUSED( varid );
  return std::vector<std::pair<double, double>>();
}

void MDAL::Driver3Di::populate1DMeshDimensions( MDAL::CFDimensions &dims )
{
  size_t count;
  int ncid;

  mNcFile->getDimension( "nMesh1D_nodes", &count, &ncid );
  dims.setDimension( CFDimensions::Vertex, count, ncid );

  mNcFile->getDimension( "nMesh1D_lines", &count, &ncid );
  dims.setDimension( CFDimensions::Edge, count, ncid );
}

void MDAL::Driver3Di::populateMesh1DElements( MDAL::Vertices &vertices, MDAL::Edges &edges )
{
  assert( vertices.empty() && edges.empty() );
  size_t vertexCount = mDimensions.size( CFDimensions::Vertex );
  size_t edgesCount = mDimensions.size( CFDimensions::Edge );
  vertices.resize( vertexCount );
  edges.resize( edgesCount );

  // X coordinate
  int ncidX = mNcFile->getVarId( "Mesh1DNode_xcc" );
  double fillX = mNcFile->getFillValue( ncidX );
  std::vector<double> verticesX( vertexCount );
  if ( nc_get_var_double( mNcFile->handle(), ncidX, verticesX.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

  // Y coordinate
  int ncidY = mNcFile->getVarId( "Mesh1DNode_ycc" );
  double fillY = mNcFile->getFillValue( ncidY );
  std::vector<double> verticesY( vertexCount );
  if ( nc_get_var_double( mNcFile->handle(), ncidY, verticesY.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

  // Z coordinate
  int ncidZ = mNcFile->getVarId( "Mesh1DNode_zcc" );
  double fillZ = mNcFile->getFillValue( ncidZ );
  std::vector<double> verticesZ( vertexCount );
  if ( nc_get_var_double( mNcFile->handle(), ncidZ, verticesZ.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

  // Node Id
  int ncidNodeId = mNcFile->getVarId( "Mesh1DNode_id" );
  std::vector<int> verticesId( vertexCount );
  if ( nc_get_var_int( mNcFile->handle(), ncidNodeId, verticesId.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

  // Edge Id
  int ncidEgeId = mNcFile->getVarId( "Mesh1DLine_id" );
  std::vector<int> edgesId( edgesCount );
  if ( nc_get_var_int( mNcFile->handle(), ncidEgeId, edgesId.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unknown format" );

  for ( size_t i = 0; i < vertexCount; ++i )
  {
    Vertex vertex;
    vertex.x = verticesX[i];
    if ( vertex.x == fillX )
      vertex.x = std::numeric_limits<double>::quiet_NaN();
    vertex.y = verticesY[i];
    if ( vertex.y == fillY )
      vertex.y = std::numeric_limits<double>::quiet_NaN();
    vertex.z = verticesZ[i];
    if ( vertex.z == fillZ )
      vertex.z = std::numeric_limits<double>::quiet_NaN();
    vertices[i] = vertex;
  }

  parse1DConnection( verticesId, edgesId, edges );
}


bool MDAL::Driver3Di::check1DConnection( std::string fileName )
{
  std::string sqliteFile = dirName( fileName ) + "/gridadmin.sqlite";

  if ( !fileExists( sqliteFile ) )
    return false;

  Sqlite3Db sqliteDb;
  return sqliteDb.open( sqliteFile );

}

void MDAL::Driver3Di::parse1DConnection( const std::vector<int> &nodesId,
    const std::vector<int> &edgesId,
    Edges &edges )
{
  std::string sqliteFileName = dirName( mNcFile->getFileName() ) + "/gridadmin.sqlite";

  //construct id map
  std::map<int, size_t> edgeMap;
  std::map<int, size_t> nodeMap;
  for ( size_t edgeIndex = 0; edgeIndex < edges.size(); ++edgeIndex )
    edgeMap[edgesId.at( edgeIndex )] = edgeIndex;

  for ( size_t nodeIndex = 0; nodeIndex < nodesId.size(); ++nodeIndex )
    nodeMap[nodesId.at( nodeIndex )] = nodeIndex;


  Sqlite3Db db;
  if ( !db.open( sqliteFileName ) || !( db.get() ) )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open sqlite database" );

  Sqlite3Statement stmt;

  if ( ! stmt.prepare( &db, "SELECT id, start_node_idx, end_node_idx FROM flowlines" ) )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to read edges connectivity from sqlite database" );

  if ( stmt.columnCount() < 0 || stmt.columnCount() != 3 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Invalid edges connectivity schema in sqlite database" );

  while ( stmt.next() )
  {
    int idEdge, idStartNode, idEndNode;
    idEdge = stmt.getInt( 0 );
    idStartNode = stmt.getInt( 1 );
    idEndNode = stmt.getInt( 2 );

    auto itEdge = edgeMap.find( idEdge );
    auto itStart = nodeMap.find( idStartNode );
    auto itEnd = nodeMap.find( idEndNode );

    if ( itEdge != edgeMap.end() && itStart != nodeMap.end() && itEnd != nodeMap.end() )
    {
      edges[itEdge->second].startVertex = itStart->second;
      edges[itEdge->second].endVertex = itEnd->second;
    }
  }
}
