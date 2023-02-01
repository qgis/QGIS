/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_ugrid.hpp"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

#include <netcdf.h>
#include <assert.h>
#include <algorithm>
#include <cmath>

#define FILL_COORDINATES_VALUE -999.0
#define FILL_FACE2D_VALUE -999

MDAL::DriverUgrid::DriverUgrid()
  : DriverCF(
      "Ugrid",
      "UGRID",
      "*.nc",
      Capability::ReadMesh |
      Capability::SaveMesh |
      Capability::WriteDatasetsOnVertices |
      Capability::WriteDatasetsOnFaces )
{}

MDAL::DriverUgrid *MDAL::DriverUgrid::create()
{
  return new DriverUgrid();
}

std::vector<std::string> MDAL::DriverUgrid::findMeshesNames() const
{
  std::vector<std::string> meshesInFile;

  const std::vector<std::string> variables = mNcFile->readArrNames();
  for ( const std::string &var : variables )
  {
    bool isMeshTopology = mNcFile->getAttrStr( var, "cf_role" ) == "mesh_topology";
    if ( isMeshTopology )
    {
      // file can include more meshes
      meshesInFile.push_back( var );
    }
  }

  return meshesInFile;
}

std::string MDAL::DriverUgrid::buildUri( const std::string &meshFile )
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

  std::vector<std::string> meshNames = findMeshesNames();
  if ( !meshNames.size() )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "No meshes found in file" + meshFile );
    return std::string( "" );
  }

  // ignore network variable
  std::vector<std::string>::iterator position = std::find( meshNames.begin(), meshNames.end(), "network" );
  if ( position != meshNames.end() )
    meshNames.erase( position );

  return MDAL::buildAndMergeMeshUris( meshFile, meshNames, name() );
}

std::string MDAL::DriverUgrid::nodeZVariableName() const
{
  const std::vector<std::string> variables = mNcFile->readArrNames();
  for ( const std::string &varName : variables )
  {
    const std::string stdName = mNcFile->getAttrStr( varName, "standard_name" );
    const std::string meshName = mNcFile->getAttrStr( varName, "mesh" );
    const std::string location = mNcFile->getAttrStr( varName, "location" );

    if ( stdName == "altitude" && meshName == mMeshName && location == "node" )
    {
      return varName;
    }
  }

  // not found, the file in non UGRID standard conforming,
  // but lets try the common name
  return mMeshName + "_node_z";
}

MDAL::CFDimensions MDAL::DriverUgrid::populateDimensions( )
{
  CFDimensions dims;
  size_t count;
  int ncid;

  mAllMeshNames = findMeshesNames();

  if ( mAllMeshNames.empty() )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, name(), "File " + mFileName + " does not contain any valid mesh definition" );

  if ( !mRequestedMeshName.empty() )
  {
    if ( std::find( std::begin( mAllMeshNames ), std::end( mAllMeshNames ), mRequestedMeshName ) != std::end( mAllMeshNames ) )
      mMeshName = mRequestedMeshName;
    else
      throw MDAL::Error( MDAL_Status::Err_InvalidData, "No such mesh with name: " + mRequestedMeshName, name() );
  }
  else
  {
    if ( mAllMeshNames.size() == 1 )
      mMeshName = mAllMeshNames.at( 0 );
    else // there are more meshes in file
    {
      if ( MDAL::contains( mAllMeshNames.at( 0 ), "network" ) ) // ignore the network variable for a moment
        mMeshName = mAllMeshNames.at( 1 );
      else
        mMeshName = mAllMeshNames.at( 0 );

      MDAL::Log::warning( MDAL_Status::Warn_MultipleMeshesInFile, name(), "Found multiple meshes in file, working with: " + mMeshName );
    }
  }

  if ( mMeshName.empty() ) throw MDAL::Error( MDAL_Status::Err_InvalidData, "Unable to parse mesh name from file" );

  mMeshDimension = mNcFile->getAttrInt( mMeshName, "topology_dimension" );

  if ( ( mMeshDimension < 1 ) || ( mMeshDimension > 2 ) )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, name(), "Unable to parse topology dimension from mesh or mesh is 3D" );

  MDAL::Log::info( "Parsing " + std::to_string( mMeshDimension ) + "D mesh with name: " + mMeshName );

  std::string nodeXVariable, nodeYVariable;
  if ( mMeshDimension == 1 )
    parseCoordinatesFrom1DMesh( mMeshName, "node_coordinates", nodeXVariable, nodeYVariable );
  else
    parse2VariablesFromAttribute( mMeshName, "node_coordinates", nodeXVariable, nodeYVariable, false );

  std::vector<size_t> nodeDimension;
  std::vector<int> nodeDimensionId;
  mNcFile->getDimensions( nodeXVariable, nodeDimension, nodeDimensionId );

  if ( nodeDimension.size() != 1 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while parsing dimensions" );

  dims.setDimension( CFDimensions::Vertex, nodeDimension.at( 0 ), nodeDimensionId.at( 0 ) );

  /* continue parsing dimension dependent variables */
  if ( mMeshDimension == 1 )
    populate1DMeshDimensions( dims );
  else
    populate2DMeshDimensions( dims, ncid );

  /* Time variable - not required for UGRID format */
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

void MDAL::DriverUgrid::populate1DMeshDimensions( MDAL::CFDimensions &dims ) const
{
  /* Parse number of edges ( dimension ) from mesh */
  std::string edgeConnectivityVariableName = mNcFile->getAttrStr( mMeshName, "edge_node_connectivity" );
  if ( edgeConnectivityVariableName.empty() )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Did not find edge node connectivity attribute" );

  std::vector<size_t> edgeDimension;
  std::vector<int> edgeDimensionId;
  mNcFile->getDimensions( edgeConnectivityVariableName, edgeDimension, edgeDimensionId );
  if ( edgeDimension.size() != 2 )
    throw MDAL::Error( MDAL_Status::Err_InvalidData, name(), "Unable to parse dimensions for edge_nodes_connectivity variable" );

  size_t edgesCount = edgeDimension.at( 0 ); // Only interested in first value, edge will always have only 2 nodes
  int edgesCountId = edgeDimensionId.at( 0 );

  dims.setDimension( CFDimensions::Edge, edgesCount, edgesCountId );
}

void MDAL::DriverUgrid::populate2DMeshDimensions( MDAL::CFDimensions &dims, int &ncid ) const
{
  // face dimension location is retrieved from the face_node_connectivity variable
  // if face_dimension is defined as attribute, the dimension at this location help to desambiguate vertex per faces and number of faces
  std::string faceConnectivityVariablesName = mNcFile->getAttrStr( mMeshName, "face_node_connectivity" );
  std::string faceDimensionLocation = mNcFile->getAttrStr( mMeshName, "face_dimension" );
  if ( faceConnectivityVariablesName == "" )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Did not find face connectivity attribute" );

  size_t facesCount;
  size_t maxVerticesPerFace;
  size_t count;

  std::vector<size_t> faceDimension;
  std::vector<int> faceDimensionId;
  int facesIndexDimensionId;
  int maxVerticesPerFaceDimensionId;
  mNcFile->getDimensions( faceConnectivityVariablesName, faceDimension, faceDimensionId );
  if ( faceDimension.size() != 2 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Face dimension is 2D" );

  // if face_dimension is not present in file, get it from dimension element
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

  dims.setDimension( CFDimensions::Face, facesCount, facesIndexDimensionId );
  dims.setDimension( CFDimensions::MaxVerticesInFace, maxVerticesPerFace, maxVerticesPerFaceDimensionId );

  // number of edges in the mesh, not required for UGRID format
  const std::string mesh2dEdge = mNcFile->getAttrStr( mMeshName, "edge_dimension" );
  if ( mNcFile->hasDimension( mesh2dEdge ) )
  {
    mNcFile->getDimension( mesh2dEdge, &count, &ncid );
    dims.setDimension( CFDimensions::Face2DEdge, count, ncid );
  }
  else
  {
    dims.setDimension( CFDimensions::Face2DEdge, 0, -1 );
  }
}

void MDAL::DriverUgrid::populateElements( Vertices &vertices, Edges &edges, Faces &faces )
{
  populateVertices( vertices );

  if ( mMeshDimension == 1 )
    populateEdges( edges ); // 1D mesh
  else
    populateFaces( faces ); // 2D mesh
}

void MDAL::DriverUgrid::populateVertices( MDAL::Vertices &vertices )
{
  assert( vertices.empty() );
  size_t vertexCount = mDimensions.size( CFDimensions::Vertex );
  vertices.resize( vertexCount );
  Vertex *vertexPtr = vertices.data();

  // node_coordinates should be something like Mesh2D_node_x Mesh2D_node_y
  std::string verticesXName, verticesYName;
  if ( mMeshDimension == 1 )
    parseCoordinatesFrom1DMesh( mMeshName, "node_coordinates", verticesXName, verticesYName );
  else
    parse2VariablesFromAttribute( mMeshName, "node_coordinates", verticesXName, verticesYName, false );

  const std::vector<double> verticesX = mNcFile->readDoubleArr( verticesXName, vertexCount );
  const std::vector<double> verticesY = mNcFile->readDoubleArr( verticesYName, vertexCount );

  std::vector<double> verticesZ;
  if ( mNcFile->hasArr( nodeZVariableName() ) )
  {
    verticesZ = mNcFile->readDoubleArr( nodeZVariableName(), vertexCount );
  }

  if ( verticesX.size() == 1 &&
       verticesY.size() == 1 &&
       verticesZ.size() == 1 &&
       verticesX.at( 0 ) == FILL_COORDINATES_VALUE &&
       verticesY.at( 0 ) == FILL_COORDINATES_VALUE &&
       verticesZ.at( 0 ) == FILL_COORDINATES_VALUE )
  {
    vertexCount = 0;
    vertices.clear();
  }


  for ( size_t i = 0; i < vertexCount; ++i, ++vertexPtr )
  {
    vertexPtr->x = verticesX[i];
    vertexPtr->y = verticesY[i];
    if ( !verticesZ.empty() )
      vertexPtr->z = verticesZ[i];
  }
}

void MDAL::DriverUgrid::populateEdges( MDAL::Edges &edges )
{
  assert( edges.empty() );

  // number of edges
  size_t edgesCount = mDimensions.size( CFDimensions::Edge );
  edges.resize( edgesCount );

  const std::string edgeNodeConnectivityVar = mNcFile->getAttrStr( mMeshName, "edge_node_connectivity" );
  if ( edgeNodeConnectivityVar.empty() )
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Unable to find edge_node_connectivity attribute of " + mMeshName );

  // load edges
  std::vector<int> edgeNodesIdxs = mNcFile->readIntArr( edgeNodeConnectivityVar, edgesCount * 2 ); // two nodes per edge
  int startIndex = mNcFile->getAttrInt( edgeNodeConnectivityVar, "start_index" );

  // iterate over all edge_nodes coordinates - those are indexes for nodes
  for ( size_t i = 0; i < edgesCount; ++i )
  {

    int startEdgeIx = MDAL::toInt( i ) * 2;
    int endEdgeIx = MDAL::toInt( i ) * 2 + 1;

    edges[i].startVertex = edgeNodesIdxs[startEdgeIx] - startIndex;
    edges[i].endVertex = edgeNodesIdxs[endEdgeIx] - startIndex;
  }
}

void MDAL::DriverUgrid::populateFaces( MDAL::Faces &faces )
{
  assert( faces.empty() );
  size_t faceCount = mDimensions.size( CFDimensions::Face );
  faces.resize( faceCount );

  // Parse 2D Mesh
  // face_node_connectivity is usually something like Mesh2D_face_nodes
  const std::string mesh2dFaceNodeConnectivity = mNcFile->getAttrStr( mMeshName, "face_node_connectivity" );

  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  int fillVal = -1;
  if ( mNcFile->hasAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" ) )
    fillVal = mNcFile->getAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" );
  int startIndex = mNcFile->getAttrInt( mesh2dFaceNodeConnectivity, "start_index" );
  std::vector<int> faceNodesConn = mNcFile->readIntArr( mesh2dFaceNodeConnectivity, faceCount * verticesInFace );

  for ( size_t i = 0; i < faceCount; ++i )
  {
    std::vector<size_t> idxs;

    for ( size_t j = 0; j < verticesInFace; ++j )
    {
      size_t idx = verticesInFace * i + j;
      int val = faceNodesConn[idx];

      if ( fillVal == val )
      {
        // found fill val
        break;
      }
      else
      {
        idxs.push_back( static_cast<size_t>( val - startIndex ) );
      }
    }
    faces[i] = idxs;
  }

  if ( faces.size() == 1 && faces.at( 0 ).size() == 0 )
    faces.clear();
}

void MDAL::DriverUgrid::addBedElevation( MDAL::MemoryMesh *mesh )
{
  if ( mNcFile->hasArr( nodeZVariableName() ) ) MDAL::addBedElevationDatasetGroup( mesh, mesh->vertices() );
}

std::string MDAL::DriverUgrid::getCoordinateSystemVariableName()
{
  std::string coordinateSystemVariable;

  // first try to get the coordinate system variable from grid definition
  std::vector<std::string> nodeVariablesName = MDAL::split( mNcFile->getAttrStr( mMeshName, "node_coordinates" ), ' ' );
  if ( nodeVariablesName.size() > 1 )
  {
    if ( mNcFile->hasArr( nodeVariablesName[0] ) )
    {
      coordinateSystemVariable = mNcFile->getAttrStr( nodeVariablesName[0], "grid_mapping" );
    }
  }


  // if automatic discovery fails, try to check some hardcoded common variables that store projection
  if ( coordinateSystemVariable.empty() )
  {
    if ( mNcFile->hasArr( "projected_coordinate_system" ) )
      coordinateSystemVariable = "projected_coordinate_system";
    else if ( mNcFile->hasArr( "wgs84" ) )
      coordinateSystemVariable = "wgs84";
  }

  // return, may be empty
  return coordinateSystemVariable;
}

std::set<std::string> MDAL::DriverUgrid::ignoreNetCDFVariables()
{
  std::set<std::string> ignoreVariables;

  ignoreVariables.insert( "projected_coordinate_system" );
  ignoreVariables.insert( "time" );
  ignoreVariables.insert( "timestep" );

  for ( const std::string &mesh : mAllMeshNames )
  {
    ignoreVariables.insert( mesh );

    int dim = mNcFile->getAttrInt( mesh, "topology_dimension" );
    if ( dim == 1 )
      ignore1DMeshVariables( mesh, ignoreVariables );
    else
      ignore2DMeshVariables( mesh, ignoreVariables );
  }

  return ignoreVariables;
}

void MDAL::DriverUgrid::ignore1DMeshVariables( const std::string &mesh, std::set<std::string> &ignoreVariables )
{
  // ignore all variables with network in name
  // network topology does not contain any data
  if ( MDAL::contains( mesh, "network" ) )
  {
    std::vector<std::string> variables = mNcFile->readArrNames();
    for ( const std::string &var : variables )
    {
      if ( MDAL::contains( var, "network" ) )
        ignoreVariables.insert( var );
    }
    return;
  }

  ignoreVariables.insert( mNcFile->getAttrStr( mesh, "edge_node_connectivity" ) );
  ignoreVariables.insert( mNcFile->getAttrStr( mesh, "node_id" ) );
  ignoreVariables.insert( mNcFile->getAttrStr( mesh, "node_long_name" ) );

  std::vector<std::string> coordinateVarsToIgnore {"node_coordinates", "edge_coordinates"};

  for ( const std::string &coordinateIt : coordinateVarsToIgnore )
  {
    std::string coordinatesVar = mNcFile->getAttrStr( mesh, coordinateIt );
    std::vector<std::string> allCoords = MDAL::split( coordinatesVar, " " );

    for ( const std::string &var : allCoords )
    {
      ignoreVariables.insert( var );
      ignoreVariables.insert( mNcFile->getAttrStr( var, "bounds" ) );
    }
  }
}

void MDAL::DriverUgrid::ignore2DMeshVariables( const std::string &mesh, std::set<std::string> &ignoreVariables )
{
  std::string xName, yName;
  parse2VariablesFromAttribute( mesh, "node_coordinates", xName, yName, true );
  ignoreVariables.insert( xName );
  ignoreVariables.insert( yName );
  ignoreVariables.insert( nodeZVariableName() );
  ignoreVariables.insert( mNcFile->getAttrStr( mesh, "edge_node_connectivity" ) );
  parse2VariablesFromAttribute( mesh, "edge_coordinates", xName, yName, true );

  if ( !xName.empty() )
  {
    ignoreVariables.insert( xName );
    ignoreVariables.insert( mNcFile->getAttrStr( xName, "bounds" ) );
  }
  if ( !yName.empty() )
  {
    ignoreVariables.insert( yName );
    ignoreVariables.insert( mNcFile->getAttrStr( yName, "bounds" ) );
  }

  ignoreVariables.insert( mNcFile->getAttrStr( mesh, "face_node_connectivity" ) );
  parse2VariablesFromAttribute( mesh, "face_coordinates", xName, yName, true );
  if ( !xName.empty() )
  {
    ignoreVariables.insert( xName );
    ignoreVariables.insert( mNcFile->getAttrStr( xName, "bounds" ) );
  }
  if ( !yName.empty() )
  {
    ignoreVariables.insert( yName );
    ignoreVariables.insert( mNcFile->getAttrStr( yName, "bounds" ) );
  }
  ignoreVariables.insert( mNcFile->getAttrStr( mesh, "edge_face_connectivity" ) );
}

void MDAL::DriverUgrid::parseNetCDFVariableMetadata( int varid,
    std::string &variableName,
    std::string &name,
    bool *isVector,
    bool *isPolar,
    bool *invertedDirection,
    bool *isX )
{
  *isVector = false;
  *isX = true;
  *isPolar = false;
  *invertedDirection = false;

  std::string longName = mNcFile->getAttrStr( "long_name", varid );
  if ( longName.empty() )
  {
    std::string standardName = mNcFile->getAttrStr( "standard_name", varid );
    if ( standardName.empty() )
    {
      name = variableName;
    }
    else
    {
      variableName = standardName;
      if ( MDAL::contains( standardName, "_x_" ) )
      {
        *isVector = true;
        name = MDAL::replace( standardName, "_x_", "" );
      }
      else if ( MDAL::contains( standardName, "_y_" ) )
      {
        *isVector = true;
        *isX = false;
        name = MDAL::replace( standardName, "_y_", "" );
      }
      else if ( MDAL::contains( standardName, "_from_direction" ) )
      {
        *isVector = true;
        *isPolar = true;
        *isX = false;
        *invertedDirection = true;
        name = MDAL::replace( standardName, "_speed", "_velocity" );
        name = MDAL::replace( name, "_from_direction", "" );
      }
      else if ( MDAL::contains( standardName, "_to_direction" ) )
      {
        *isVector = true;
        *isPolar = true;
        *isX = false;
        name = MDAL::replace( standardName, "_speed", "_velocity" );
        name = MDAL::replace( name, "_to_direction", "" );
      }
      else
      {
        name = standardName;
      }
    }
  }
  else
  {
    variableName = longName;
    if ( MDAL::contains( longName, ", x-component" ) || MDAL::contains( longName, "u component of " ) )
    {
      *isVector = true;
      name = MDAL::replace( longName, ", x-component", "" );
      name = MDAL::replace( name, "u component of ", "" );
    }
    else if ( MDAL::contains( longName, ", y-component" ) || MDAL::contains( longName, "v component of " ) )
    {
      *isVector = true;
      *isX = false;
      name = MDAL::replace( longName, ", y-component", "" );
      name = MDAL::replace( name, "v component of ", "" );
    }
    else if ( MDAL::contains( longName, " magnitude" ) )
    {
      *isVector = true;
      *isPolar = true;
      *isX = true;
      name = MDAL::replace( longName, "speed", "velocity" );
      name = MDAL::removeFrom( name, " magnitude" );
    }
    else if ( MDAL::contains( longName, "direction" ) )
    {
      *isVector = true;
      *isPolar = true;
      *isX = false;

      // check from_/to_direction in standard_name
      std::string standardName = mNcFile->getAttrStr( "standard_name", varid );
      *invertedDirection = MDAL::contains( longName, "from direction" );

      name = MDAL::replace( longName, "speed", "velocity" );
      name = MDAL::removeFrom( name, " from direction" );
      name = MDAL::removeFrom( name, " to direction" );
      name = MDAL::removeFrom( name, " direction" );
    }
    else
    {
      name = longName;
    }
  }
}

std::string MDAL::DriverUgrid::getTimeVariableName() const
{
  return "time";
}

void MDAL::DriverUgrid::parseCoordinatesFrom1DMesh( const std::string &meshName, const std::string &attr_name, std::string &var1, std::string &var2 )
{
  std::vector<std::string> nodeVariablesName = MDAL::split( mNcFile->getAttrStr( meshName, attr_name ), ' ' );

  if ( nodeVariablesName.size() < 2 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while parsing node coordinates" );
  else if ( nodeVariablesName.size() > 3 )
  {
    // format does not follow UGRID convention and have extra variables in coordinate attribute
    // tring to parse coordinate variables, it usually ends with _x and _y, e.g. mesh1d_node_x, mesh1d_node_y

    MDAL::Log::warning( MDAL_Status::Warn_InvalidElements, name(),
                        "Node coordinates consists of more than 3 variables, taking variable with _x in name by default" );

    for ( const auto &nodeVar : nodeVariablesName )
    {
      if ( MDAL::contains( nodeVar, "_x" ) )
      {
        var1 = nodeVar;
      }
      else if ( MDAL::contains( nodeVar, "_y" ) )
      {
        var2 = nodeVar;
      }
    }
    if ( var1.empty() || var2.empty() )
      throw MDAL::Error( MDAL_Status::Err_InvalidData, name(), "Could not parse node coordinates from mesh" );
  }
  else // 2 variables as node coordinates
  {
    var1 = nodeVariablesName.at( 0 );
    var2 = nodeVariablesName.at( 1 );
  }
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
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to parse variables from attribute" );
  }
  else
  {
    var1 = chunks[0];
    var2 = chunks[1];
  }
}

void MDAL::DriverUgrid::save( const std::string &fileName, const std::string &meshName, MDAL::Mesh *mesh )
{
  mFileName = fileName;

  std::string effectiveMeshName = meshName;
  if ( effectiveMeshName.empty() )
    effectiveMeshName = "mesh2d";

  try
  {
    // Create file
    mNcFile.reset( new NetCDFFile );
    mNcFile->createFile( mFileName );

    // Write globals
    writeGlobals( );

    // Write variables
    writeVariables( mesh, effectiveMeshName );
  }
  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "could not save file " + fileName );
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
  }

  mNcFile.reset();
}

std::string MDAL::DriverUgrid::writeDatasetOnFileSuffix() const
{
  return "nc";
}

std::string MDAL::DriverUgrid::saveMeshOnFileSuffix() const
{
  return "nc";
}

bool MDAL::DriverUgrid::writeDatasetGroup( MDAL::DatasetGroup *group, const std::string &fileName, const std::string &meshName )
{
  mNcFile.reset( new NetCDFFile );

  try
  {
    mNcFile->openFile( fileName, true );
  }
  catch ( MDAL::Error &err )
  {
    err.setDriver( name() );
    MDAL::Log::error( err );
    return true;
  }

  mRequestedMeshName = meshName;

  mDimensions = populateDimensions();

  bool needAddingTime = mDimensions.size( CFDimensions::Time ) == 0;

  if ( ! needAddingTime &&  group->datasets.size() != 1 ) // existing and new dataset group are not static
  {
    std::vector<MDAL::RelativeTimestamp> times;
    MDAL::DateTime referenceTime = parseTime( times );
    if ( times.size() != group->datasets.size() )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Existing time steps count is incompatible with new dataset count", name() );

    for ( size_t i = 0; i < times.size(); ++i )
      if ( referenceTime + times.at( i )  != group->referenceTime() + group->datasets.at( i )->timestamp() )
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "At least one new time is incompatible with existing dataset time", name() );
  }

  CFDimensions::Type type;

  std::string elementType;
  switch ( group->dataLocation() )
  {
    case DataInvalidLocation:
      throw MDAL::Error( MDAL_Status::Err_UnsupportedElement, "Unable to write unknown dataset location", name() );
      break;
    case DataOnVertices:
      type = CFDimensions::Vertex;
      elementType = "node";
      break;
    case DataOnFaces:
      type = CFDimensions::Face ;
      elementType = "face";
      break;
    case DataOnVolumes:
      throw MDAL::Error( MDAL_Status::Err_UnsupportedElement, "Writing dataset on volume not supported", name() );
      break;
    case DataOnEdges:
      throw MDAL::Error( MDAL_Status::Err_UnsupportedElement, "Writing dataset on edge not supported", name() );
      break;
  }

  int dimElemId = mDimensions.netCfdId( type );
  size_t elementCount = mDimensions.size( type );

  nc_redef( mNcFile->handle() );

  std::vector<double> timeSteps;
  int dimTimeId = -1;
  int timeId = -1;
  if ( needAddingTime )
  {
    dimTimeId = mNcFile->defineDimension( "time", NC_UNLIMITED );
    timeId = mNcFile->defineVar( "time", NC_DOUBLE, 1, &dimTimeId );
    std::string refTimeString = group->referenceTime().toStandardCalendarISO8601();
    refTimeString = replace( refTimeString, "T", " " );
    mNcFile->putAttrStr( timeId, "units", "hours since " + refTimeString );
    timeSteps.resize( group->datasets.size() );
  }
  else
    dimTimeId = mDimensions.netCfdId( CFDimensions::Time );

  std::vector<int> writeDim( {dimTimeId, dimElemId} );

  if ( group->isScalar() )
  {
    int groupId = mNcFile->defineVar( mMeshName + "_" + replace( group->name(), " ", "_" ), NC_DOUBLE, 2, writeDim.data() );
    mNcFile->putAttrStr( groupId, "standard_name", group->name() );
    mNcFile->putAttrStr( groupId, "units", group->getMetadata( "units" ) );
    mNcFile->putAttrStr( groupId, "location", elementType );
    mNcFile->putAttrStr( groupId, "coordinates", mMeshName + "_face_x " + mMeshName + "_face_y" );
    mNcFile->setFillValue( groupId, NC_FILL_DOUBLE );

    nc_enddef( mNcFile->handle() );

    for ( size_t di = 0; di < group->datasets.size(); ++di )
    {
      std::vector<double> values( elementCount );
      size_t valueCount = group->datasets.at( di )->scalarData( 0, elementCount, values.data() );
      if ( valueCount != elementCount )
        throw MDAL::Error( MDAL_Status::Err_IncompatibleDataset, "Wrong dataset values count", name() );

      for ( size_t i = 0; i < values.size(); ++i )
        if ( std::isnan( values.at( i ) ) )
          values[i] = NC_FILL_DOUBLE;

      mNcFile->putDataArrayDouble( groupId, di, values );
      if ( needAddingTime )
        mNcFile->putDataDouble( timeId, di, group->datasets.at( di )->time( RelativeTimestamp::hours ) );
    }
  }
  else
  {
    std::string nameX = group->name() + "_x_";
    std::string nameY = group->name() + "_y_";
    int groupIdX = mNcFile->defineVar( mMeshName + "_" + replace( nameX, " ", "_" ), NC_DOUBLE, 2, writeDim.data() );
    mNcFile->putAttrStr( groupIdX, "standard_name", nameX );
    mNcFile->putAttrStr( groupIdX, "units", group->getMetadata( "units" ) );
    mNcFile->putAttrStr( groupIdX, "location", elementType );
    mNcFile->putAttrStr( groupIdX, "coordinates", mMeshName + "_face_x " + mMeshName + "_face_y" );
    mNcFile->setFillValue( groupIdX, NC_FILL_DOUBLE );

    int groupIdY = mNcFile->defineVar( mMeshName + "_" + replace( nameY, " ", "_" ), NC_DOUBLE, 2, writeDim.data() );
    mNcFile->putAttrStr( groupIdY, "standard_name", nameY );
    mNcFile->putAttrStr( groupIdY, "units", group->getMetadata( "units" ) );
    mNcFile->putAttrStr( groupIdY, "location", elementType );
    mNcFile->putAttrStr( groupIdY, "coordinates", mMeshName + "_face_x " + mMeshName + "_face_y" );
    mNcFile->setFillValue( groupIdY, NC_FILL_DOUBLE );

    nc_enddef( mNcFile->handle() );

    for ( size_t di = 0; di < group->datasets.size(); ++di )
    {
      std::vector<double> values( elementCount * 2 );
      std::vector<double> valuesX( elementCount );
      std::vector<double> valuesY( elementCount );
      size_t valueCount = group->datasets.at( di )->vectorData( 0, elementCount, values.data() );
      if ( valueCount != elementCount )
        throw MDAL::Error( MDAL_Status::Err_IncompatibleDataset, "Wrong dataset values count", name() );

      for ( size_t i = 0; i < elementCount; ++i )
      {
        valuesX[i] = values.at( i * 2 );
        valuesY[i] = values.at( i * 2 + 1 );

        if ( std::isnan( valuesX.at( i ) ) )
          valuesX[i] = NC_FILL_DOUBLE;

        if ( std::isnan( valuesY.at( i ) ) )
          valuesY[i] = NC_FILL_DOUBLE;
      }

      mNcFile->putDataArrayDouble( groupIdX, di, valuesX );
      mNcFile->putDataArrayDouble( groupIdY, di, valuesY );
      if ( needAddingTime )
        mNcFile->putDataDouble( timeId, di, group->datasets.at( di )->time( RelativeTimestamp::hours ) );
    }
  }

  return false;
}

bool MDAL::DriverUgrid::persist( MDAL::DatasetGroup *group )
{
  if ( !group ||
       ( group->dataLocation() != MDAL_DataLocation::DataOnVertices  &&
         group->dataLocation() != MDAL_DataLocation::DataOnFaces ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, name(), "Ugrid can store only 2D vertices datasets or 2D faces datasets" );
    return true;
  }

  mNcFile.reset();

  try
  {
    std::string fileName;
    std::string driver;
    std::string meshName;
    parseDriverAndMeshFromUri( group->uri(), driver, fileName, meshName );

    if ( ! MDAL::fileExists( fileName ) )
    {
      if ( meshName.empty() )
        meshName = "mesh2d";
      else
        meshName = replace( meshName, " ", "_" );

      //create a new mesh file
      save( fileName, meshName, group->mesh() );
      if ( ! MDAL::fileExists( fileName ) )
        throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Unable to create new file" );
    }

    return writeDatasetGroup( group, fileName, meshName );
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
    return true;
  }
}

void MDAL::DriverUgrid::writeVariables( MDAL::Mesh *mesh, const std::string &meshName )
{
  // Mesh 2D Definition
  const std::string dimNodeName = "n" + meshName + "_node";
  const std::string dimFaceName = "n" + meshName + "_face";
  const std::string dimEdgeName = "n" + meshName + "_edge";
  const std::string dimMaxFaceNodesName = "max_n" + meshName + "_face_nodes";

  // Global dimensions
  int dimNodeCountId = mNcFile->defineDimension( dimNodeName, mesh->verticesCount() == 0 ? 1 : mesh->verticesCount() ); //if no vertices, set 1 since 0==NC_UNLIMITED
  int dimFaceCountId = mNcFile->defineDimension( dimFaceName, mesh->facesCount() == 0 ? 1 : mesh->facesCount() ); //if no vertices, set 1 since 0==NC_UNLIMITED
  mNcFile->defineDimension( dimEdgeName, 1 ); // no data on edges, cannot be 0, since 0==NC_UNLIMITED
  int dimMaxNodesPerFaceId = mNcFile->defineDimension( dimMaxFaceNodesName,
                             mesh->faceVerticesMaximumCount() == 0 ? 1 : mesh->faceVerticesMaximumCount() ); //if 0, set 1 since 0==NC_UNLIMITED

  const std::string varNodeXName = meshName + "_node_x";
  const std::string varNodeYName = meshName + "_node_y";
  const std::string varNodeZName = meshName + "_node_z";
  const std::string varConnectivityName = meshName + "_face_nodes";

  int mesh2dId = mNcFile->defineVar( meshName, NC_INT, 0, nullptr );
  mNcFile->putAttrStr( mesh2dId, "cf_role", "mesh_topology" );
  mNcFile->putAttrStr( mesh2dId, "long_name", "Topology data of 2D network" );
  mNcFile->putAttrInt( mesh2dId, "topology_dimension", 2 );
  mNcFile->putAttrStr( mesh2dId, "node_coordinates", varNodeXName + " " + varNodeYName );
  mNcFile->putAttrStr( mesh2dId, "node_dimension", dimNodeName );
  mNcFile->putAttrStr( mesh2dId, "edge_dimension", dimEdgeName );
  mNcFile->putAttrStr( mesh2dId, "max_face_nodes_dimension", dimMaxFaceNodesName );
  mNcFile->putAttrStr( mesh2dId, "face_node_connectivity", varConnectivityName );
  mNcFile->putAttrStr( mesh2dId, "face_dimension", dimFaceName );

  // Nodes X coordinate
  int mesh2dNodeXId = mNcFile->defineVar( varNodeXName, NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeXId, "standard_name", "projection_x_coordinate" );
  mNcFile->putAttrStr( mesh2dNodeXId, "long_name", "x-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeXId, "mesh", meshName );
  mNcFile->putAttrStr( mesh2dNodeXId, "location", "node" );

  // Nodes Y coordinate
  int mesh2dNodeYId = mNcFile->defineVar( varNodeYName, NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeYId, "standard_name", "projection_y_coordinate" );
  mNcFile->putAttrStr( mesh2dNodeYId, "long_name", "y-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeYId, "mesh", meshName );
  mNcFile->putAttrStr( mesh2dNodeYId, "location", "node" );

  // Nodes Z coordinate
  int mesh2dNodeZId = mNcFile->defineVar( varNodeZName, NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeZId, "mesh", meshName );
  mNcFile->putAttrStr( mesh2dNodeZId, "location", "node" );
  mNcFile->putAttrStr( mesh2dNodeZId, "coordinates", varNodeXName + " " + varNodeYName );
  mNcFile->putAttrStr( mesh2dNodeZId, "standard_name", "altitude" );
  mNcFile->putAttrStr( mesh2dNodeZId, "long_name", "z-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeZId, "grid_mapping", "projected_coordinate_system" );
  mNcFile->putAttrDouble( mesh2dNodeZId, "_FillValue", FILL_COORDINATES_VALUE );

  // Faces 2D Variable
  int mesh2FaceNodesId_dimIds [] { dimFaceCountId, dimMaxNodesPerFaceId };
  int mesh2FaceNodesId = mNcFile->defineVar( varConnectivityName, NC_INT, 2, mesh2FaceNodesId_dimIds );
  mNcFile->putAttrStr( mesh2FaceNodesId, "cf_role", "face_node_connectivity" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "mesh", meshName );
  mNcFile->putAttrStr( mesh2FaceNodesId, "location", "face" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "long_name", "Mapping from every face to its corner nodes (counterclockwise)" );
  mNcFile->putAttrInt( mesh2FaceNodesId, "start_index", 0 );
  mNcFile->putAttrInt( mesh2FaceNodesId, "_FillValue", FILL_FACE2D_VALUE );

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

  // Turning off define mode - allows data write
  nc_enddef( mNcFile->handle() );

  // Write vertices

  const size_t maxBufferSize = 1000;
  const size_t bufferSize = std::min( mesh->verticesCount(), maxBufferSize );
  const size_t verticesCoordCount = bufferSize * 3;

  std::vector<double> verticesCoordinates( verticesCoordCount );
  std::unique_ptr<MDAL::MeshVertexIterator> vertexIterator = mesh->readVertices();

  if ( mesh->verticesCount() == 0 )
  {
    // if there is no vertices fill the first fake vertex, see global dimension
    mNcFile->putDataDouble( mesh2dNodeXId, 0, FILL_COORDINATES_VALUE );
    mNcFile->putDataDouble( mesh2dNodeYId, 0, FILL_COORDINATES_VALUE );
    mNcFile->putDataDouble( mesh2dNodeZId, 0, FILL_COORDINATES_VALUE );
  }
  else
  {
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
          mNcFile->putDataDouble( mesh2dNodeZId, vertexFileIndex, FILL_COORDINATES_VALUE );
        else
          mNcFile->putDataDouble( mesh2dNodeZId, vertexFileIndex, verticesCoordinates[3 * i + 2] );
        vertexFileIndex++;
      }
      vertexIndex += verticesRead;
    }
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

  if ( facesCount == 0 )
  {
    // if there is no vertices fill the first fake vertex, see global dimension
    int fillValue = FILL_FACE2D_VALUE;
    mNcFile->putDataArrayInt( 0, 0, 1, &fillValue );
  }
  else
  {
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
        std::vector<int> verticesFaceData( faceVerticesMax, FILL_FACE2D_VALUE );
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
  }
}

void MDAL::DriverUgrid::writeGlobals()
{
  mNcFile->putAttrStr( NC_GLOBAL, "source", "MDAL " + std::string( MDAL_Version() ) );
  mNcFile->putAttrStr( NC_GLOBAL, "date_created", MDAL::getCurrentTimeStamp() );
  mNcFile->putAttrStr( NC_GLOBAL, "Conventions", "CF-1.6 UGRID-1.0" );
}

std::vector<std::pair<double, double>> MDAL::DriverUgrid::parseClassification( int varid ) const
{
  std::vector<std::pair<double, double>> classes;
  std::string flagBoundVarName = mNcFile->getAttrStr( "flag_bounds", varid );
  if ( !flagBoundVarName.empty() )
  {
    try
    {
      int boundsVarId = mNcFile->getVarId( flagBoundVarName );
      std::vector<size_t> classDims;
      std::vector<int> classDimIds;
      mNcFile->getDimensions( flagBoundVarName, classDims, classDimIds );
      std::vector<double> boundValues = mNcFile->readDoubleArr( boundsVarId, 0, 0, classDims[0], classDims[1] );

      if ( classDims[1] != 2 || classDims[0] <= 0 )
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Invalid classification dimension" );

      std::pair<std::string, std::string> classificationMeta;
      classificationMeta.first = "classification";
      std::string classification;
      for ( size_t i = 0; i < classDims[0]; ++i )
      {
        std::pair<double, double> classBound;
        classBound.first = boundValues[i * 2];
        classBound.second = boundValues[i * 2 + 1];
        classes.push_back( classBound );
      }
    }
    catch ( MDAL::Error &err )
    {
      MDAL::Log::warning( err.status, err.driver, "Error when parsing class bounds: " + err.mssg + ", classification ignored" );
    }
  }

  return classes;
}
