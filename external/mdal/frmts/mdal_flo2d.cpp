/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2016 Lutra Consulting
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_flo2d.hpp"
#include <vector>
#include <map>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>
#include <assert.h>

#include "mdal_utils.hpp"
#include "mdal_hdf5.hpp"
#include "mdal_logger.hpp"

#define FLO2D_NAN 0.0

#define INVALID_INDEX std::numeric_limits<size_t>::max()

static std::string fileNameFromDir( const std::string &mainFileName, const std::string &name )
{
  std::string dir = MDAL::dirName( mainFileName );
  return MDAL::pathJoin( dir, name );
}

static double getDouble( double val )
{
  if ( MDAL::equals( val, FLO2D_NAN, 1e-8 ) )
  {
    return MDAL_NAN;
  }
  else
  {
    return val;
  }
}

static double toFlo2DDouble( double val )
{
  if ( std::isnan( val ) )
  {
    return FLO2D_NAN;
  }
  else
  {
    return val;
  }
}

static double getDouble( const std::string &val )
{
  double valF = MDAL::toDouble( val );
  return getDouble( valF );
}

void MDAL::DriverFlo2D::addStaticDataset(
  std::vector<double> &vals,
  const std::string &groupName,
  const std::string &datFileName )
{
  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          datFileName,
                                          groupName
                                        );
  group->setDataLocation( MDAL_DataLocation::DataOnFaces );
  group->setIsScalar( true );

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MemoryDataset2D >( group.get() );
  assert( vals.size() == dataset->valuesCount() );
  dataset->setTime( MDAL::RelativeTimestamp() );
  double *values = dataset->values();
  memcpy( values, vals.data(), vals.size() * sizeof( double ) );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mMesh->datasetGroups.push_back( group );
}


void MDAL::DriverFlo2D::parseCADPTSFile( const std::string &datFileName, std::vector<CellCenter> &cells, MDAL::BBox &cellCenterExtent )
{
  std::string cadptsFile( fileNameFromDir( datFileName, "CADPTS.DAT" ) );
  if ( !MDAL::fileExists( cadptsFile ) )
  {
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not find file " + cadptsFile );
  }

  std::ifstream cadptsStream = MDAL::openInputFile( cadptsFile, std::ifstream::in );
  std::string line;
  // CADPTS.DAT - COORDINATES OF CELL CENTERS (ELEM NUM, X, Y)
  while ( std::getline( cadptsStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 3 )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading CADPTS file, wrong lineparts count (3)" );
    }
    CellCenter cc;
    cc.id = MDAL::toSizeT( lineParts[0] ) - 1; //numbered from 1
    cc.x = MDAL::toDouble( lineParts[1] );
    cc.y = MDAL::toDouble( lineParts[2] );
    cells.push_back( cc );

    if ( cc.x > cellCenterExtent.maxX )
      cellCenterExtent.maxX = cc.x;
    if ( cc.x < cellCenterExtent.minX )
      cellCenterExtent.minX = cc.x;
    if ( cc.y > cellCenterExtent.maxY )
      cellCenterExtent.maxY = cc.y;
    if ( cc.y < cellCenterExtent.minY )
      cellCenterExtent.minY = cc.y;
  }
}

void MDAL::DriverFlo2D::parseCHANBANKFile( const std::string &datFileName,
    std::map<size_t, size_t> &cellIdToVertices,
    std::map<size_t, std::vector<size_t>> &duplicatedRightBankToVertex,
    size_t &verticesCount )
{
  std::string chanBankFile( fileNameFromDir( datFileName, "CHANBANK.DAT" ) );
  if ( !MDAL::fileExists( chanBankFile ) )
  {
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not find file " + chanBankFile );
  }
  std::ifstream chanBankStream = MDAL::openInputFile( chanBankFile, std::ifstream::in );
  std::string line;
  // CHANBANK.DAT - Cell id of each bank (Left Bank id , Right Bank id), if right bank id is 0, channel is only on left cell
  size_t vertexIndex = 0;
  while ( std::getline( chanBankStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 2 )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading CHANBANK file, wrong lineparts count (2)" );
    }
    int leftBank = MDAL::toInt( MDAL::toSizeT( lineParts[0] ) ) - 1;  //numbered from 1
    int rightBank = MDAL::toInt( MDAL::toSizeT( lineParts[1] ) ) - 1;

    std::map<size_t, size_t>::const_iterator it = cellIdToVertices.find( rightBank );
    if ( it != cellIdToVertices.end() )
    {
      // sometimes a right bank can be associated several times to a left bank (inverse is not possible)
      std::map<size_t, std::vector<size_t>>::iterator itDupplicated = duplicatedRightBankToVertex.find( rightBank );
      if ( itDupplicated == duplicatedRightBankToVertex.end() )
        duplicatedRightBankToVertex[rightBank] = std::vector<size_t>( 1, vertexIndex );
      else
        itDupplicated->second.push_back( vertexIndex );
    }
    else if ( rightBank >= 0 )
    {
      cellIdToVertices[rightBank] = vertexIndex;
    }

    cellIdToVertices[leftBank] = vertexIndex;

    vertexIndex++;
  }
  verticesCount = vertexIndex;
}

void MDAL::DriverFlo2D::parseCHANFile( const std::string &datFileName, const std::map<size_t, size_t> &cellIdToVertices, std::vector<Edge> &edges )
{
  std::string chanFile( fileNameFromDir( datFileName, "CHAN.DAT" ) );
  if ( !MDAL::fileExists( chanFile ) )
  {
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not find file " + chanFile );
  }
  std::ifstream chanStream = MDAL::openInputFile( chanFile, std::ifstream::in );
  std::string line;
  // CHAN.DAT - each reachs are represented by following line beginning by R, V,T or N
  // Confluences are represented by line beginning by C
  // other line are no used by MDAL
  int previousCellId = -1;
  while ( std::getline( chanStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.empty() )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading CHAN file, empty line" );
    }
    std::string firstChar = lineParts[0];
    if ( firstChar == "R" || firstChar == "V" || firstChar == "T" || firstChar == "N" ) //chanel node
    {
      if ( lineParts.size() < 2 )
      {
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading CHAN file, wrong chanel element line" );
      }
      int currentCellId = MDAL::toInt( MDAL::toSizeT( lineParts[1] ) ) - 1;
      if ( previousCellId >= 0 )
      {
        std::map<size_t, size_t>::const_iterator it1 = cellIdToVertices.find( previousCellId );
        std::map<size_t, size_t>::const_iterator it2 = cellIdToVertices.find( currentCellId );
        if ( it1 != cellIdToVertices.end() && it2 != cellIdToVertices.end() )
          edges.push_back( { it1->second, it2->second } );
      }

      previousCellId = currentCellId;
    }
    else
    {
      previousCellId = -1;
      if ( firstChar == "C" ) //confluence
      {
        if ( lineParts.size() != 3 )
        {
          throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading CHAN file, wrong confluence line:" );
        }
        std::map<size_t, size_t>::const_iterator it1 = cellIdToVertices.find( MDAL::toSizeT( lineParts[1] ) - 1 );
        std::map<size_t, size_t>::const_iterator it2 = cellIdToVertices.find( MDAL::toSizeT( lineParts[2] ) - 1 );
        if ( it1 != cellIdToVertices.end() && it2 != cellIdToVertices.end() )
          edges.push_back( {it1->second, it2->second} );
      }
    }
  }
}

static bool parseHYCHANBlock( std::ifstream &fileStream, int &cellId, std::vector<std::vector<double>> &data, size_t variableCount )
{
  std::string line;
  cellId = -1;
  while ( std::getline( fileStream, line ) )
  {
    std::vector<std::string> lineParts = MDAL::split( line, "  " );
    for ( size_t i = 0; i < lineParts.size(); ++i )
      lineParts[i] = MDAL::trim( lineParts.at( i ) );

    if ( lineParts.size() > 1 && lineParts[0] == "CHANNEL HYDROGRAPH FOR ELEMENT NO:" )
    {
      cellId = MDAL::toInt( lineParts[1] ) - 1;
      break;
    }
  }

  if ( cellId == -1 )
    return false;

  while ( std::getline( fileStream, line ) )
  {
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() > 1 && lineParts[0] == "TIME" )
      break;
  }

  // pass two lines
  std::getline( fileStream, line );
  std::getline( fileStream, line );

  //start to store the data
  if ( fileStream.eof() )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading HYCHAN file, wrong format" );
  }

  size_t timeStep = 0;
  while ( std::getline( fileStream, line ) )
  {
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != variableCount + 1 )
      break;

    if ( timeStep >= data.size() )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading HYCHAN file, wrong format" );

    if ( lineParts.size() - 1 > variableCount )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading HYCHAN file, wrong format" );

    std::vector<double> valuesLine( variableCount );
    for ( size_t i = 0; i < lineParts.size() - 1; ++i )
      valuesLine[i] = MDAL::toDouble( lineParts[i + 1] );

    data[timeStep] = std::move( valuesLine );
    timeStep++;
  }
  return true;
}

void MDAL::DriverFlo2D::parseHYCHANFile( const std::string &datFileName, const std::map<size_t, size_t> &cellIdToVertices )
{
  std::string hyChanFile( fileNameFromDir( datFileName, "HYCHAN.OUT" ) );
  if ( !MDAL::fileExists( hyChanFile ) )
  {
    return;
  }
  std::ifstream hyChanStream = MDAL::openInputFile( hyChanFile, std::ifstream::in );
  std::string line;

  std::vector<std::string> variablesName;

  // first, search for the variables name
  while ( std::getline( hyChanStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, "  " );
    for ( size_t i = 0; i < lineParts.size(); ++i )
      lineParts[i] = MDAL::trim( lineParts.at( i ) );

    if ( lineParts.size() > 1 && lineParts[0] == "TIME" )
    {
      for ( size_t i = 1; i < lineParts.size(); ++i )
        variablesName.push_back( lineParts.at( i ) );
      break;
    }
  }

  // parse first block to have time step
  // pass two lines
  std::getline( hyChanStream, line );
  std::getline( hyChanStream, line );

  if ( hyChanStream.eof() )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading HYCHAN file, wrong format" );
  }

  std::vector<double> timeStep;
  while ( std::getline( hyChanStream, line ) )
  {
    const std::vector<std::string> lineParts = MDAL::split( line, ' ' );

    if ( lineParts.size() != variablesName.size() + 1 )
      break;

    timeStep.push_back( MDAL::toDouble( lineParts[0] ) );
  }

  std::vector<std::shared_ptr<DatasetGroup>> datasetGroups;
  for ( size_t i = 0; i < variablesName.size(); ++i )
  {
    datasetGroups.push_back( std::make_shared<DatasetGroup>( name(), mMesh.get(), datFileName, variablesName.at( i ) ) );
    for ( size_t j = 0; j < timeStep.size(); ++j )
    {
      datasetGroups[i]->datasets.push_back( std::make_shared<MemoryDataset2D>( datasetGroups.back().get(), false ) );
      datasetGroups[i]->datasets.back()->setTime( timeStep[j] );
    }
  }

  hyChanStream.seekg( 0, hyChanStream.beg );
  std::vector<std::vector<double>> data( timeStep.size(), std::vector<double>( variablesName.size() ) );
  int cellId;

  // parse a bloc of HYCHAN file and fill corresponding value in the dataset groups
  while ( parseHYCHANBlock( hyChanStream, cellId, data, variablesName.size() ) )
  {
    std::map<size_t, size_t>::const_iterator it = cellIdToVertices.find( cellId );

    if ( it != cellIdToVertices.end() && it->second < mMesh->verticesCount() )
    {
      for ( size_t i = 0; i < data.size(); ++i )
      {
        if ( data[i].size() != datasetGroups.size() )
          throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while reading HYCHAN file, wrong format" );

        for ( size_t j = 0; j < data[i].size(); ++j )
        {
          if ( i >= datasetGroups[j]->datasets.size() )
            throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while reading HYCHAN file, wrong format" );

          static_cast<MDAL::MemoryDataset2D *>( datasetGroups[j]->datasets[i].get() )->setScalarValue( it->second, data[i][j] );
        }
      }
    }
  }

  for ( std::shared_ptr<DatasetGroup> datasetGroup : datasetGroups )
  {
    for ( std::shared_ptr<Dataset> dataset : datasetGroup->datasets )
      dataset->setStatistics( MDAL::calculateStatistics( dataset ) );

    datasetGroup->setStatistics( MDAL::calculateStatistics( datasetGroup ) );
    mMesh->datasetGroups.push_back( datasetGroup );
  }
}


void MDAL::DriverFlo2D::createMesh1d( const std::string &datFileName, const std::vector<MDAL::DriverFlo2D::CellCenter> &cells, std::map<size_t, size_t> &cellsIdToVertex )
{
  //    In flow 2D, each 1D node can be defined by one or two cell id, corresponding to the left bank and the right bank.
  //    If there is only one id associated with the node, the cell id is the left bank, and the right bank has id cell equal to 0.
  //    Consequences for the (X, Y) position of the node/vertex :
  //    - if only one cell (the left bank id cell), the position of the node is this left bank cell position.
  //    - if two cells, left bank and right bank, the position of the node is the centre of the segment [left bank, right bank].
  //
  //    parseCHANBANKFile() creates a map between cells and 1D vertex (in MDAL sens):  cellsIdToVertex.
  //    Each cell id (key), left bank and right bank if it exists, is mapped with a node/vertex (value is vertex index).
  //    Vertex indexes are the position of the node in the CHANBANK file. As right bank cell could be associated with several nodes,
  //    when consecutive edges are not colinear (seen in the test data file, for example, line 57-58 and 61-62 of the CHANBANK.DAT test file).
  //    In this case, those duplicated right banks are stored in another map: duplicatedRightBankToVertex (key: right bank cell, value: vector of indexes of associated vertex).
  //
  //    Once the links between left/right bank cells and vertices are defined, the following step is to calculate the effective (X, Y) position of each vertex.
  //    For that, all the cells are iterated :
  //    If the id cell is contained in the cellsIdToVertex map and the vertex has not already has been its coordinates filled (vertices are initiated with NaN coordinate),
  //    then this vertex takes the coordinate of the corresponding cell.
  //    If the id cell is contained in the cellsIdToVertex map and the vertex coordinates are not NaN, that means that the vertex is associated with two banks,
  //    so add the second coordinates to the first and multiply by factor 0.5 to average the coordinates.
  //
  //    As the id cells could correspond to a cell that is a right bank associated with severals vertex,
  //    do the same with duplicatedRightBankToVertex that contain the vertices associated with the duplicated right bank.
  //
  //    How the cellsIdToVertex and duplicatedRightBankToVertex maps are created, each vertex could not be present more than twice in those maps.

  std::map<size_t, std::vector<size_t>> duplicatedRightBankToVertex;
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  size_t verticesCount;

  parseCHANBANKFile( datFileName, cellsIdToVertex, duplicatedRightBankToVertex, verticesCount );
  vertices = std::vector<Vertex>( verticesCount );

  for ( const MDAL::DriverFlo2D::CellCenter &cell : cells )
  {
    std::map<size_t, size_t>::const_iterator it = cellsIdToVertex.find( cell.id );
    if ( it != cellsIdToVertex.end() )
    {
      if ( it->second < vertices.size() )
      {
        Vertex &vertex = vertices[it->second];
        //if coordinates of vertex is not nan, those coordinates are the first bank, so add the second and use factor 0.5
        if ( std::isnan( vertex.x ) )
        {
          vertex.x = cell.x;
          vertex.y = cell.y;
        }
        else
        {
          vertex.x += cell.x;
          vertex.x *= 0.5;
          vertex.y += cell.y;
          vertex.y *= 0.5;
        }
      }
    }

    std::map<size_t, std::vector<size_t>>::const_iterator itDuplicated = duplicatedRightBankToVertex.find( cell.id );
    if ( itDuplicated != duplicatedRightBankToVertex.end() )
    {
      const std::vector<size_t> &indexes = itDuplicated->second;
      for ( size_t index : indexes )
      {
        if ( index < vertices.size() )
        {
          Vertex &vertex = vertices[index];
          if ( std::isnan( vertex.x ) )
          {
            vertex.x = cell.x;
            vertex.y = cell.y;
          }
          else
          {
            vertex.x += cell.x;
            vertex.x *= 0.5;
            vertex.y += cell.y;
            vertex.y *= 0.5;
          }
        }
      }
    }
  }

  parseCHANFile( datFileName, cellsIdToVertex, edges );
  mMesh.reset( new MemoryMesh( name(), 0, mDatFileName ) );
  mMesh->setVertices( std::move( vertices ) );
  mMesh->setEdges( std::move( edges ) );
}

void MDAL::DriverFlo2D::parseFPLAINFile( std::vector<double> &elevations,
    const std::string &datFileName,
    std::vector<CellCenter> &cells,
    double &cellSize )
{
  elevations.clear();
  // FPLAIN.DAT - CONNECTIVITY (ELEM NUM, ELEM N, ELEM E, ELEM S, ELEM W, MANNING-N, BED ELEVATION)
  std::string fplainFile( fileNameFromDir( datFileName, "FPLAIN.DAT" ) );
  if ( !MDAL::fileExists( fplainFile ) )
  {
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not find file " + fplainFile );
  }

  std::ifstream fplainStream = MDAL::openInputFile( fplainFile, std::ifstream::in );
  std::string line;

  bool cellSizeCalculated = false;

  while ( std::getline( fplainStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 7 )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading FPLAIN.DAT file, wrong lineparts count (7)" );
    }

    if ( !cellSizeCalculated )
    {
      size_t cc_i = MDAL::toSizeT( lineParts[0] ) - 1; //numbered from 1
      for ( int i = 1; i < 5; ++i )  //search the first cell that have a neighbor to calculate cell size
      {
        int neighborCell = MDAL::toInt( lineParts[i] );
        if ( neighborCell != 0 )
        {
          if ( i % 2 == 1 ) //North or South neighbor
            cellSize = fabs( cells[neighborCell - 1].y - cells[cc_i].y );
          else // East or West
            cellSize = fabs( cells[neighborCell - 1].x - cells[cc_i].x );

          cellSizeCalculated = true;
          break;
        }
      }
    }

    elevations.push_back( MDAL::toDouble( lineParts[6] ) );
  }

  if ( !cellSizeCalculated )
    throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Only isolated cell(s), not possible to calculate cell size" );
}

static void addDatasetToGroup( std::shared_ptr<MDAL::DatasetGroup> group, std::shared_ptr<MDAL::MemoryDataset2D> dataset )
{
  if ( group && dataset && dataset->valuesCount() > 0 )
  {
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
}

void MDAL::DriverFlo2D::parseTIMDEPFile( const std::string &datFileName, const std::vector<double> &elevations )
{
  // TIMDEP.OUT
  // this file is optional, so if not present, reading is skipped
  // time (separate line)
  // For every Vertex:
  // FLO2D: ELEM NUMber (indexed from 1), depth, velocity, velocity x, velocity y
  // FLO2DPro: ELEM NUMber (indexed from 1), depth, velocity, velocity x, velocity y, water surface elevation
  std::string inFile( fileNameFromDir( datFileName, "TIMDEP.OUT" ) );
  if ( !MDAL::fileExists( inFile ) )
  {
    return;
  }

  std::ifstream inStream = MDAL::openInputFile( inFile, std::ifstream::in );
  std::string line;

  size_t nVertexs = mMesh->verticesCount();
  size_t ntimes = 0;

  RelativeTimestamp time = RelativeTimestamp();
  size_t face_idx = 0;

  std::shared_ptr<DatasetGroup> depthDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Depth"
      );
  depthDsGroup->setDataLocation( MDAL_DataLocation::DataOnFaces );
  depthDsGroup->setIsScalar( true );


  std::shared_ptr<DatasetGroup> waterLevelDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Water Level"
      );
  waterLevelDsGroup->setDataLocation( MDAL_DataLocation::DataOnFaces );
  waterLevelDsGroup->setIsScalar( true );

  std::shared_ptr<DatasetGroup> flowDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Velocity"
      );
  flowDsGroup->setDataLocation( MDAL_DataLocation::DataOnFaces );
  flowDsGroup->setIsScalar( false );

  std::shared_ptr<MDAL::MemoryDataset2D> flowDataset;
  std::shared_ptr<MDAL::MemoryDataset2D> depthDataset;
  std::shared_ptr<MDAL::MemoryDataset2D> waterLevelDataset;

  while ( std::getline( inStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() == 1 )
    {
      time = RelativeTimestamp( MDAL::toDouble( line ), RelativeTimestamp::hours );
      ntimes++;

      if ( depthDataset ) addDatasetToGroup( depthDsGroup, depthDataset );
      if ( flowDataset ) addDatasetToGroup( flowDsGroup, flowDataset );
      if ( waterLevelDataset ) addDatasetToGroup( waterLevelDsGroup, waterLevelDataset );

      depthDataset  = std::make_shared< MemoryDataset2D >( depthDsGroup.get() );
      flowDataset = std::make_shared< MemoryDataset2D >( flowDsGroup.get() );
      waterLevelDataset = std::make_shared< MemoryDataset2D >( waterLevelDsGroup.get() );

      depthDataset->setTime( time );
      flowDataset->setTime( time );
      waterLevelDataset->setTime( time );

      face_idx = 0;

    }
    else if ( ( lineParts.size() == 5 ) || ( lineParts.size() == 6 ) )
    {
      // new Vertex for time
      if ( !depthDataset || !flowDataset || !waterLevelDataset ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Depth, flow or water level dataset is not valid (null)" );
      if ( face_idx == nVertexs ) throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Invalid face id" );

      // this is magnitude: getDouble(lineParts[2]);
      flowDataset->setVectorValue( face_idx, getDouble( lineParts[3] ),  getDouble( lineParts[4] ) );

      double depth = getDouble( lineParts[1] );
      depthDataset->setScalarValue( face_idx, depth );

      if ( !std::isnan( depth ) ) depth += elevations[face_idx];
      waterLevelDataset->setScalarValue( face_idx, depth );

      face_idx ++;
    }
    else
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to load TIMDEP file, found unknown format" );
    }
  }

  if ( depthDataset ) addDatasetToGroup( depthDsGroup, depthDataset );
  if ( flowDataset ) addDatasetToGroup( flowDsGroup, flowDataset );
  if ( waterLevelDataset ) addDatasetToGroup( waterLevelDsGroup, waterLevelDataset );

  depthDsGroup->setStatistics( MDAL::calculateStatistics( depthDsGroup ) );
  flowDsGroup->setStatistics( MDAL::calculateStatistics( flowDsGroup ) );
  waterLevelDsGroup->setStatistics( MDAL::calculateStatistics( waterLevelDsGroup ) );

  mMesh->datasetGroups.push_back( depthDsGroup );
  mMesh->datasetGroups.push_back( flowDsGroup );
  mMesh->datasetGroups.push_back( waterLevelDsGroup );
}


void MDAL::DriverFlo2D::parseDEPTHFile( const std::string &datFileName, const std::vector<double> &elevations )
{
  // this file is optional, so if not present, reading is skipped
  std::string depthFile( fileNameFromDir( datFileName, "DEPTH.OUT" ) );
  if ( !MDAL::fileExists( depthFile ) )
  {
    return; //optional file
  }

  std::ifstream depthStream = MDAL::openInputFile( depthFile, std::ifstream::in );
  std::string line;

  size_t nFaces = mMesh->facesCount();
  std::vector<double> maxDepth( nFaces );
  std::vector<double> maxWaterLevel( nFaces );

  size_t vertex_idx = 0;

  // DEPTH.OUT - COORDINATES (ELEM NUM, X, Y, MAX DEPTH)
  while ( std::getline( depthStream, line ) )
  {
    line = MDAL::rtrim( line );
    if ( vertex_idx == nFaces ) throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Error while loading DEPTH file, invalid vertex index" );

    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 4 )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading DEPTH file, wrong lineparts count (4)" );
    }

    double val = getDouble( lineParts[3] );
    maxDepth[vertex_idx] = val;

    //water level
    if ( !std::isnan( val ) ) val += elevations[vertex_idx];
    maxWaterLevel[vertex_idx] = val;


    vertex_idx++;
  }

  addStaticDataset( maxDepth, "Depth/Maximums", datFileName );
  addStaticDataset( maxWaterLevel, "Water Level/Maximums", datFileName );
}


void MDAL::DriverFlo2D::parseVELFPVELOCFile( const std::string &datFileName )
{
  // these files are optional, so if not present, reading is skipped
  size_t nFaces = mMesh->facesCount();
  std::vector<double> maxVel( nFaces );

  {
    std::string velocityFile( fileNameFromDir( datFileName, "VELFP.OUT" ) );
    if ( !MDAL::fileExists( velocityFile ) )
    {
      return; //optional file
    }

    std::ifstream velocityStream = MDAL::openInputFile( velocityFile, std::ifstream::in );
    std::string line;

    size_t vertex_idx = 0;

    // VELFP.OUT - COORDINATES (ELEM NUM, X, Y, MAX VEL) - Maximum floodplain flow velocity;
    while ( std::getline( velocityStream, line ) )
    {
      if ( vertex_idx == nFaces ) throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Error while loading VELFP file, invalid vertex index" );

      line = MDAL::rtrim( line );
      std::vector<std::string> lineParts = MDAL::split( line, ' ' );
      if ( lineParts.size() != 4 )
      {
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading VELFP file, wrong lineparts count (4)" );
      }

      double val = getDouble( lineParts[3] );
      maxVel[vertex_idx] = val;

      vertex_idx++;
    }
  }

  {
    std::string velocityFile( fileNameFromDir( datFileName, "VELOC.OUT" ) );
    if ( !MDAL::fileExists( velocityFile ) )
    {
      return; //optional file
    }

    std::ifstream velocityStream = MDAL::openInputFile( velocityFile, std::ifstream::in );
    std::string line;

    size_t vertex_idx = 0;

    // VELOC.OUT - COORDINATES (ELEM NUM, X, Y, MAX VEL)  - Maximum channel flow velocity
    while ( std::getline( velocityStream, line ) )
    {
      if ( vertex_idx == nFaces ) throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Error while loading VELOC file, invalid vertex index" );

      line = MDAL::rtrim( line );
      std::vector<std::string> lineParts = MDAL::split( line, ' ' );
      if ( lineParts.size() != 4 )
      {
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while loading VELOC file, wrong lineparts count (4)" );
      }

      double val = getDouble( lineParts[3] );
      if ( !std::isnan( val ) )  // overwrite value from VELFP if it is not 0
      {
        maxVel[vertex_idx] = val;
      }

      vertex_idx++;
    }
  }

  addStaticDataset( maxVel, "Velocity/Maximums", datFileName );
}

MDAL::Vertex MDAL::DriverFlo2D::createVertex( size_t position, double half_cell_size, const CellCenter &cell )
{
  MDAL::Vertex n;
  n.x = cell.x;
  n.y = cell.y;

  switch ( position )
  {
    case 0:
      n.x += half_cell_size;
      n.y -= half_cell_size;
      break;

    case 1:
      n.x += half_cell_size;
      n.y += half_cell_size;
      break;

    case 2:
      n.x -= half_cell_size;
      n.y += half_cell_size;
      break;

    case 3:
      n.x -= half_cell_size;
      n.y -= half_cell_size;
      break;
  }

  return n;
}

void MDAL::DriverFlo2D::createMesh2d( const std::vector<CellCenter> &cells, const BBox &cellCenterExtent, double cell_size )
{
  // Create all Faces from cell centers.
  // Vertexs must be also created, they are not stored in FLO-2D files
  // try to reuse Vertexs already created for other Faces by usage of unique_Vertexs set.

  double half_cell_size = cell_size / 2;
  Faces faces( cells.size(), Face( 4 ) );

  BBox vertexExtent( cellCenterExtent.minX - half_cell_size,
                     cellCenterExtent.maxX + half_cell_size,
                     cellCenterExtent.minY - half_cell_size,
                     cellCenterExtent.maxY + half_cell_size );

  size_t width = MDAL::toSizeT( ( vertexExtent.maxX - vertexExtent.minX ) / cell_size + 1.5 );
  size_t heigh = MDAL::toSizeT( ( vertexExtent.maxY - vertexExtent.minY ) / cell_size + 1.5 );
  std::vector<std::vector<size_t>> vertexGrid( width, std::vector<size_t>( heigh, INVALID_INDEX ) );

  Vertices vertices;

  for ( size_t i = 0; i < cells.size(); ++i )
  {
    Face &e = faces[i];

    size_t xVertexIdx = MDAL::toSizeT( ( cells[i].x - vertexExtent.minX ) / cell_size );
    size_t yVertexIdx = MDAL::toSizeT( ( cells[i].y - vertexExtent.minY ) / cell_size );

    for ( size_t position = 0; position < 4; ++position )
    {
      size_t xPos = 0;
      size_t yPos = 0;

      switch ( position )
      {
        case 0:
          xPos = 1;
          yPos = 0;
          break;

        case 1:
          xPos = 1;
          yPos = 1;
          break;

        case 2:
          xPos = 0;
          yPos = 1;
          break;

        case 3:
          xPos = 0;
          yPos = 0;
          break;
      }

      if ( vertexGrid[xVertexIdx + xPos][yVertexIdx + yPos] == INVALID_INDEX )
      {
        vertices.push_back( createVertex( position, half_cell_size, cells.at( i ) ) );
        vertexGrid[xVertexIdx + xPos][yVertexIdx + yPos] = vertices.size() - 1;
      }

      e[position] = vertexGrid[xVertexIdx + xPos][yVertexIdx + yPos];
    }
  }

  mMesh.reset(
    new MemoryMesh(
      name(),
      4, //maximum quads
      mDatFileName
    )
  );
  mMesh->setFaces( std::move( faces ) );
  mMesh->setVertices( std::move( vertices ) );
}

bool MDAL::DriverFlo2D::parseHDF5Datasets( MemoryMesh *mesh, const std::string &timedepFileName )
{
  //return true on error

  size_t nFaces =  mesh->facesCount();
  if ( !fileExists( timedepFileName ) ) return true;

  HdfFile file( timedepFileName, HdfFile::ReadOnly );
  if ( !file.isValid() ) return true;

  HdfGroup timedataGroup = file.group( "TIMDEP NETCDF OUTPUT RESULTS" );
  if ( !timedataGroup.isValid() ) return true;

  std::vector<std::string> groupNames = timedataGroup.groups();

  for ( const std::string &grpName : groupNames )
  {
    HdfGroup grp = timedataGroup.group( grpName );
    if ( !grp.isValid() ) return true;

    HdfAttribute groupType = grp.attribute( "Grouptype" );
    if ( !groupType.isValid() ) return true;

    HdfAttribute timeUnitAttribute = grp.attribute( "TimeUnits" );
    std::string timeUnitString = timeUnitAttribute.readString();

    /* Min and Max arrays in TIMDEP.HDF5 files have dimensions 1xntimesteps .
        HdfDataset minDs = grp.dataset("Mins");
        if (!minDs.isValid()) return true;

        HdfDataset maxDs = grp.dataset("Maxs");
        if (!maxDs.isValid()) return true;
    */

    HdfDataset timesDs = grp.dataset( "Times" );
    if ( !timesDs.isValid() ) return true;
    size_t timesteps = timesDs.elementCount();

    HdfDataset valuesDs = grp.dataset( "Values" );
    if ( !valuesDs.isValid() ) return true;

    bool isVector = MDAL::contains( groupType.readString(), "vector", ContainsBehaviour::CaseInsensitive );

    // Some sanity checks
    size_t expectedSize = mesh->facesCount() * timesteps;
    if ( isVector ) expectedSize *= 2;
    if ( valuesDs.elementCount() != expectedSize ) return true;

    // Read data
    std::vector<double> times = timesDs.readArrayDouble();
    std::vector<float> values = valuesDs.readArray();

    // Create dataset now
    std::shared_ptr<DatasetGroup> ds = std::make_shared< DatasetGroup >(
                                         name(),
                                         mesh,
                                         timedepFileName,
                                         grpName
                                       );
    ds->setDataLocation( MDAL_DataLocation::DataOnFaces );
    ds->setIsScalar( !isVector );

    for ( size_t ts = 0; ts < timesteps; ++ts )
    {
      std::shared_ptr< MemoryDataset2D > output = std::make_shared< MemoryDataset2D >( ds.get() );
      output->setTime( times[ts], parseDurationTimeUnit( timeUnitString ) );

      if ( isVector )
      {
        // vector
        for ( size_t i = 0; i < nFaces; ++i )
        {
          size_t idx = 2 * ( ts * nFaces + i );
          double x = getDouble( static_cast<double>( values[idx] ) );
          double y = getDouble( static_cast<double>( values[idx + 1] ) );
          output->setVectorValue( i, x, y );
        }
      }
      else
      {
        // scalar
        for ( size_t i = 0; i < nFaces; ++i )
        {
          size_t idx = ts * nFaces + i;
          double val = getDouble( static_cast<double>( values[idx] ) );
          output->setScalarValue( i, val );
        }
      }
      addDatasetToGroup( ds, output );
    }

    // TODO use mins & maxs arrays
    ds->setStatistics( MDAL::calculateStatistics( ds ) );
    mesh->datasetGroups.push_back( ds );

  }

  return false;
}

void MDAL::DriverFlo2D::parseOUTDatasets( const std::string &datFileName, const std::vector<double> &elevations )
{
  // Create Depth and Velocity datasets Time varying datasets
  parseTIMDEPFile( datFileName, elevations );

  // Maximum Depth and Water Level
  parseDEPTHFile( datFileName, elevations );

  // Maximum Velocity
  parseVELFPVELOCFile( datFileName );
}

MDAL::DriverFlo2D::DriverFlo2D()
  : Driver(
      "FLO2D",
      "Flo2D",
      "*.nc;;*.DAT;;*.OUT",
      Capability::ReadMesh | Capability::ReadDatasets | Capability::WriteDatasetsOnFaces )
{

}

MDAL::DriverFlo2D *MDAL::DriverFlo2D::create()
{
  return new DriverFlo2D();
}

bool MDAL::DriverFlo2D::canReadMesh( const std::string &uri )
{
  std::string cadptsFile( fileNameFromDir( uri, "CADPTS.DAT" ) );
  if ( !MDAL::fileExists( cadptsFile ) )
  {
    return false;
  }

  std::string fplainFile( fileNameFromDir( uri, "FPLAIN.DAT" ) );
  std::string chanFile( fileNameFromDir( uri, "CHAN.DAT" ) );
  std::string chanBankFile( fileNameFromDir( uri, "CHANBANK.DAT" ) );

  return MDAL::fileExists( fplainFile ) || ( MDAL::fileExists( chanFile ) && MDAL::fileExists( chanBankFile ) );
}

bool MDAL::DriverFlo2D::canReadDatasets( const std::string &uri )
{
  if ( !fileExists( uri ) ) return false;

  HdfFile file( uri, HdfFile::ReadOnly );
  if ( !file.isValid() ) return false;

  HdfGroup timedataGroup = file.group( "TIMDEP NETCDF OUTPUT RESULTS" );
  if ( !timedataGroup.isValid() ) return false;

  return true;
}

std::string MDAL::DriverFlo2D::buildUri( const std::string &meshFile )
{
  std::vector<std::string> meshNames;

  std::string mesh1DTopologyFile( fileNameFromDir( meshFile, "CHAN.DAT" ) );
  std::string mesh1DVerticesPosition( fileNameFromDir( meshFile, "CHANBANK.DAT" ) );
  std::string mesh2DTopologyFile( fileNameFromDir( meshFile, "FPLAIN.DAT" ) );

  if ( fileExists( mesh1DTopologyFile ) && fileExists( mesh1DVerticesPosition ) )
    meshNames.push_back( "mesh1d" );

  if ( fileExists( mesh2DTopologyFile ) )
    meshNames.push_back( "mesh2d" );

  return MDAL::buildAndMergeMeshUris( meshFile, meshNames, name() );
}

void MDAL::DriverFlo2D::load( const std::string &uri, MDAL::Mesh *mesh )
{
  MDAL::Log::resetLastStatus();

  MDAL::MemoryMesh *memoryMesh = dynamic_cast<MDAL::MemoryMesh *>( mesh );
  if ( !memoryMesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), "Mesh is not valid (null)" );
    return;
  }

  if ( !MDAL::fileExists( uri ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, name(), "Could not find file " + uri );
    return;
  }

  bool err = parseHDF5Datasets( memoryMesh, uri );
  if ( err )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "Could not parse HDF5 datasets" );
  }
}

std::unique_ptr< MDAL::Mesh > MDAL::DriverFlo2D::load( const std::string &resultsFile, const std::string &meshName )
{
  MDAL::Log::resetLastStatus();
  mDatFileName = resultsFile;
  std::string mesh2DTopologyFile( fileNameFromDir( resultsFile, "FPLAIN.DAT" ) );
  std::string mesh1DTopologyFile( fileNameFromDir( resultsFile, "CHAN.DAT" ) );

  if ( meshName == "mesh2d" || ( meshName.empty() && fileExists( mesh2DTopologyFile ) ) )
    return loadMesh2d();

  if ( meshName == "mesh1d" || fileExists( mesh1DTopologyFile ) )
    return loadMesh1d();

  return nullptr;
}


std::unique_ptr<MDAL::Mesh> MDAL::DriverFlo2D::loadMesh1d()
{
  std::vector<CellCenter> cells;
  std::map<size_t, size_t> cellsIdToVertex;
  MDAL::BBox cellCenterExtent;
  try
  {
    // Parse cells position
    parseCADPTSFile( mDatFileName, cells, cellCenterExtent );
    createMesh1d( mDatFileName, cells, cellsIdToVertex );
    parseHYCHANFile( mDatFileName, cellsIdToVertex );
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}


std::unique_ptr<MDAL::Mesh> MDAL::DriverFlo2D::loadMesh2d()
{
  mMesh.reset();
  std::vector<CellCenter> cells;

  try
  {
    // Parse mMesh info
    MDAL::BBox cellCenterExtent;
    parseCADPTSFile( mDatFileName, cells, cellCenterExtent );
    std::vector<double> elevations;
    double cell_size;
    parseFPLAINFile( elevations, mDatFileName, cells, cell_size );

    // Create mMesh
    createMesh2d( cells, cellCenterExtent, cell_size );

    // create output for bed elevation
    addStaticDataset( elevations, "Bed Elevation", mDatFileName );

    // check if we have HDF5 file
    std::string TIMDEPFileName = fileNameFromDir( mDatFileName, "TIMDEP.HDF5" );
    if ( parseHDF5Datasets( mMesh.get(), TIMDEPFileName ) )
    {
      // some problem with HDF5 data, try text files
      parseOUTDatasets( mDatFileName, elevations );
    }
  }

  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "error occurred while loading file " + mDatFileName );
    mMesh.reset();
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}

bool MDAL::DriverFlo2D::addToHDF5File( DatasetGroup *group )
{
  assert( MDAL::fileExists( group->uri() ) );
  HdfFile file( group->uri(), HdfFile::ReadWrite );
  if ( !file.isValid() ) return true;

  HdfGroup timedataGroup = file.group( "TIMDEP NETCDF OUTPUT RESULTS" );
  if ( !timedataGroup.isValid() ) return true;
  return appendGroup( file, group, timedataGroup );
}


bool MDAL::DriverFlo2D::saveNewHDF5File( DatasetGroup *dsGroup )
{
  // Create file
  HdfFile file( dsGroup->uri(), HdfFile::Create );
  // Unable to create
  if ( !file.isValid() ) return true;

  // Create float dataset File Version
  HdfDataset dsFileVersion( file.id(), "/File Version", H5T_NATIVE_FLOAT );
  dsFileVersion.write( 1.0f );

  // Create string dataset File Type
  HdfDataset dsFileType( file.id(), "/File Type", HdfDataType::createString() );
  dsFileType.write( "Xmdf" );

  // Create group TIMDEP NETCDF OUTPUT RESULTS
  HdfGroup groupTNOR = HdfGroup::create( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS" );

  // Create attribute
  HdfAttribute attTNORGrouptype( groupTNOR.id(), "Grouptype", HdfDataType::createString() );
  // Write string value to attribute
  attTNORGrouptype.write( "Generic" );

  return appendGroup( file, dsGroup, groupTNOR );
}

bool MDAL::DriverFlo2D::appendGroup( HdfFile &file, MDAL::DatasetGroup *dsGroup, HdfGroup &groupTNOR )
{
  assert( dsGroup->dataLocation() == MDAL_DataLocation::DataOnFaces );

  HdfDataType dtMaxString = HdfDataType::createString();
  std::string dsGroupName = dsGroup->name();
  const size_t timesCount = dsGroup->datasets.size();
  const size_t facesCount = dsGroup->mesh()->facesCount();
  size_t valCount = facesCount;
  HdfDataspace dscValues;
  if ( dsGroup->isScalar() )
  {
    std::vector<hsize_t> dimsForScalarValues = { timesCount, facesCount };
    dscValues = HdfDataspace( dimsForScalarValues );
  }
  else
  {
    valCount *= 2;
    std::vector<hsize_t> dimsForVectorValues = { timesCount, facesCount, 2 };
    dscValues =  HdfDataspace( dimsForVectorValues );
  }

  std::vector<hsize_t> timesCountVec = {timesCount};
  HdfDataspace dscTimes( timesCountVec );

  std::vector<float> maximums( timesCount );
  std::vector<float> minimums( timesCount );
  std::vector<double> times( timesCount );
  std::vector<float> values( timesCount * valCount );

  // prepare data
  for ( size_t i = 0; i < dsGroup->datasets.size(); i++ )
  {
    const std::shared_ptr<Dataset> &dataset = dsGroup->datasets.at( i );
    std::vector<double> singleRowValues( valCount );

    if ( dsGroup->isScalar() )
    {
      dataset->scalarData( 0, facesCount, singleRowValues.data() );
    }
    else
    {
      dataset->vectorData( 0, facesCount, singleRowValues.data() );
    }

    for ( size_t j = 0; j < valCount; j++ )
    {
      double doubleValue = toFlo2DDouble( singleRowValues[j] );
      values[i * valCount + j] = static_cast<float>( doubleValue );
    }

    const Statistics st = dataset->statistics();
    maximums[i] = static_cast<float>( st.maximum );
    minimums[i] = static_cast<float>( st.minimum );
    times[i] = dataset->time( RelativeTimestamp::hours ) ;
  }

  // store data
  int i = 0;
  while ( file.pathExists( "/TIMDEP NETCDF OUTPUT RESULTS/" + dsGroupName ) )
  {
    dsGroupName = dsGroup->name() + "_" + std::to_string( i ); // make sure we have unique group name
  }
  HdfGroup group = HdfGroup::create( groupTNOR.id(), "/TIMDEP NETCDF OUTPUT RESULTS/" + dsGroupName );

  HdfAttribute attDataType( group.id(), "Data Type", H5T_NATIVE_INT );
  attDataType.write( 0 );

  HdfAttribute attDatasetCompression( group.id(), "DatasetCompression", H5T_NATIVE_INT );
  attDatasetCompression.write( -1 );

  /*
  HdfDataspace dscDatasetUnits( dimsSingle );
  HdfAttribute attDatasetUnits( group.id(), "DatasetUnits", true );
  attDatasetUnits.writeString( dscDatasetUnits.id(), "unknown" );
  */

  HdfAttribute attGrouptype( group.id(), "Grouptype", dtMaxString );
  if ( dsGroup->isScalar() )
    attGrouptype.write( "DATASET SCALAR" );
  else
    attGrouptype.write( "DATASET VECTOR" );

  HdfAttribute attTimeUnits( group.id(), "TimeUnits", dtMaxString );
  attTimeUnits.write( "Hours" );

  HdfDataset dsMaxs( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/" + dsGroupName + "/Maxs", H5T_NATIVE_FLOAT, timesCountVec );
  dsMaxs.write( maximums );

  HdfDataset dsMins( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/" + dsGroupName + "/Mins", H5T_NATIVE_FLOAT, timesCountVec );
  dsMins.write( minimums );

  HdfDataset dsTimes( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/" + dsGroupName + "/Times", H5T_NATIVE_DOUBLE, timesCountVec );
  dsTimes.write( times );

  HdfDataset dsValues( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/" + dsGroupName + "/Values", H5T_NATIVE_FLOAT, dscValues );
  dsValues.write( values );

  return false; //OK
}

bool MDAL::DriverFlo2D::persist( DatasetGroup *group )
{
  if ( !group || ( group->dataLocation() != MDAL_DataLocation::DataOnFaces ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, name(), "flo-2d can store only 2D face datasets" );
    return true;
  }

  try
  {
    // Return true on error
    if ( MDAL::fileExists( group->uri() ) )
    {
      // Add dataset to a existing file
      return addToHDF5File( group );
    }
    else
    {
      // Create new HDF5 file with Flow2D structure
      return saveNewHDF5File( group );
    }
  }
  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "error occurred" );
    return true;
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
    return true;
  }
}
