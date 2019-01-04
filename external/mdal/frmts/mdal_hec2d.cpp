/*
 MDAL - mMesh Data Abstraction Library (MIT License)
 Copyright (C) 2016 Lutra Consulting
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <iterator>
#include "assert.h"

#include "mdal_hec2d.hpp"
#include "mdal_hdf5.hpp"
#include "mdal_utils.hpp"

static HdfFile openHdfFile( const std::string &fileName )
{
  HdfFile file( fileName );
  if ( !file.isValid() )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return file;
}

static HdfGroup openHdfGroup( const HdfFile &hdfFile, const std::string &name )
{
  HdfGroup grp = hdfFile.group( name );
  if ( !grp.isValid() )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return grp;
}

static HdfGroup openHdfGroup( const HdfGroup &hdfGroup, const std::string &name )
{
  HdfGroup grp = hdfGroup.group( name );
  if ( !grp.isValid() )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return grp;
}

static HdfDataset openHdfDataset( const HdfGroup &hdfGroup, const std::string &name )
{
  HdfDataset dsFileType = hdfGroup.dataset( name );
  if ( !dsFileType.isValid() )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return dsFileType;
}


static std::string openHdfAttribute( const HdfFile &hdfFile, const std::string &name )
{
  HdfAttribute attr = hdfFile.attribute( name );
  if ( !attr.isValid() )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return attr.readString();
}

static HdfGroup getBaseOutputGroup( const HdfFile &hdfFile )
{
  HdfGroup gResults = openHdfGroup( hdfFile, "Results" );
  HdfGroup gUnsteady = openHdfGroup( gResults, "Unsteady" );
  HdfGroup gOutput = openHdfGroup( gUnsteady, "Output" );
  HdfGroup gOBlocks = openHdfGroup( gOutput, "Output Blocks" );
  HdfGroup gBaseO = openHdfGroup( gOBlocks, "Base Output" );
  return gBaseO;
}

static HdfGroup get2DFlowAreasGroup( const HdfFile &hdfFile, const std::string loc )
{
  HdfGroup gBaseO = getBaseOutputGroup( hdfFile );
  HdfGroup gLoc = openHdfGroup( gBaseO, loc );
  HdfGroup g2DFlowRes = openHdfGroup( gLoc, "2D Flow Areas" );
  return g2DFlowRes;
}

static std::vector<float> readTimes( const HdfFile &hdfFile )
{
  HdfGroup gBaseO = getBaseOutputGroup( hdfFile );
  HdfGroup gUnsteadTS = openHdfGroup( gBaseO, "Unsteady Time Series" );
  HdfDataset dsTimes = openHdfDataset( gUnsteadTS, "Time" );
  std::vector<float> times = dsTimes.readArray();
  return times;
}

static std::vector<int> readFace2Cells( const HdfFile &hdfFile, const std::string &flowAreaName, size_t *nFaces )
{
  // First read face to node mapping
  HdfGroup gGeom = openHdfGroup( hdfFile, "Geometry" );
  HdfGroup gGeom2DFlowAreas = openHdfGroup( gGeom, "2D Flow Areas" );
  HdfGroup gArea = openHdfGroup( gGeom2DFlowAreas, flowAreaName );
  HdfDataset dsFace2Cells = openHdfDataset( gArea, "Faces Cell Indexes" );

  std::vector<hsize_t> fdims = dsFace2Cells.dims();
  std::vector<int> face2Cells = dsFace2Cells.readArrayInt(); //2x nFaces

  *nFaces = fdims[0];
  return face2Cells;
}

void MDAL::DriverHec2D::readFaceOutput( const HdfFile &hdfFile,
                                        const HdfGroup &rootGroup,
                                        const std::vector<size_t> &areaElemStartIndex,
                                        const std::vector<std::string> &flowAreaNames,
                                        const std::string rawDatasetName,
                                        const std::string datasetName,
                                        const std::vector<float> &times )
{
  double eps = std::numeric_limits<double>::min();

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          mFileName,
                                          datasetName
                                        );
  group->setIsOnVertices( false );
  group->setIsScalar( true );

  std::vector<std::shared_ptr<MDAL::MemoryDataset>> datasets;

  for ( size_t tidx = 0; tidx < times.size(); ++tidx )
  {
    std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MemoryDataset >( group.get() );
    double time = static_cast<double>( times[tidx] );
    dataset->setTime( time );
    datasets.push_back( dataset );
  }

  std::shared_ptr<MDAL::MemoryDataset> firstDataset;

  for ( size_t nArea = 0; nArea < flowAreaNames.size(); ++nArea )
  {
    std::string flowAreaName = flowAreaNames[nArea];

    size_t nFaces;
    std::vector<int> face2Cells = readFace2Cells( hdfFile, flowAreaName, &nFaces );

    HdfGroup gFlowAreaRes = openHdfGroup( rootGroup, flowAreaName );
    HdfDataset dsVals = openHdfDataset( gFlowAreaRes, rawDatasetName );
    std::vector<float> vals = dsVals.readArray();

    for ( size_t tidx = 0; tidx < times.size(); ++tidx )
    {
      std::shared_ptr<MDAL::MemoryDataset> dataset = datasets[tidx];
      double *values = dataset->values();

      for ( size_t i = 0; i < nFaces; ++i )
      {
        size_t idx = tidx * nFaces + i;
        double val = static_cast<double>( vals[idx] ); // This is value on face!

        if ( !std::isnan( val ) && fabs( val ) > eps ) //not nan and not 0
        {
          for ( size_t c = 0; c < 2; ++c )
          {
            size_t cell_idx = face2Cells[2 * i + c] + areaElemStartIndex[nArea];
            // Take just maximum
            if ( std::isnan( values[cell_idx] ) || values[cell_idx] < val )
            {
              values[cell_idx] = val;
            }
          }
        }
      }
    }
  }

  for ( auto dataset : datasets )
  {
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mMesh->datasetGroups.push_back( group );
}

void MDAL::DriverHec2D::readFaceResults( const HdfFile &hdfFile,
    const std::vector<size_t> &areaElemStartIndex,
    const std::vector<std::string> &flowAreaNames )
{
  // UNSTEADY
  HdfGroup flowGroup = get2DFlowAreasGroup( hdfFile, "Unsteady Time Series" );
  std::vector<float> times = readTimes( hdfFile );

  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Face Shear Stress", "Face Shear Stress", times );
  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Face Velocity", "Face Velocity", times );

  // SUMMARY
  flowGroup = get2DFlowAreasGroup( hdfFile, "Summary Output" );
  times.clear();
  times.push_back( 0.0f );

  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Face Shear Stress", "Face Shear Stress/Maximums", times );
  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Face Velocity", "Face Velocity/Maximums", times );
}


std::shared_ptr<MDAL::MemoryDataset> MDAL::DriverHec2D::readElemOutput( const HdfGroup &rootGroup,
    const std::vector<size_t> &areaElemStartIndex,
    const std::vector<std::string> &flowAreaNames,
    const std::string rawDatasetName,
    const std::string datasetName,
    const std::vector<float> &times,
    std::shared_ptr<MDAL::MemoryDataset> bed_elevation )
{
  double eps = std::numeric_limits<double>::min();

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          mFileName,
                                          datasetName
                                        );
  group->setIsOnVertices( false );
  group->setIsScalar( true );

  std::vector<std::shared_ptr<MDAL::MemoryDataset>> datasets;

  for ( size_t tidx = 0; tidx < times.size(); ++tidx )
  {
    std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MemoryDataset >( group.get() );
    double time = static_cast<double>( times[tidx] );
    dataset->setTime( time );
    datasets.push_back( dataset );
  }

  for ( size_t nArea = 0; nArea < flowAreaNames.size(); ++nArea )
  {
    size_t nAreaElements = areaElemStartIndex[nArea + 1] - areaElemStartIndex[nArea];
    std::string flowAreaName = flowAreaNames[nArea];
    HdfGroup gFlowAreaRes = openHdfGroup( rootGroup, flowAreaName );

    HdfDataset dsVals = openHdfDataset( gFlowAreaRes, rawDatasetName );
    std::vector<float> vals = dsVals.readArray();

    for ( size_t tidx = 0; tidx < times.size(); ++tidx )
    {
      std::shared_ptr<MDAL::MemoryDataset> dataset = datasets[tidx];
      double *values = dataset->values();

      for ( size_t i = 0; i < nAreaElements; ++i )
      {
        size_t idx = tidx * nAreaElements + i;
        size_t eInx = areaElemStartIndex[nArea] + i;
        double val = static_cast<double>( vals[idx] );
        if ( !std::isnan( val ) )
        {
          if ( !bed_elevation )
          {
            // we are populating bed elevation dataset
            values[eInx] = val;
          }
          else
          {
            if ( datasetName == "Depth" )
            {
              if ( fabs( val ) > eps ) // 0 Depth is no-data
              {
                values[eInx] = val;
              }
            }
            else     //Water surface
            {
              assert( bed_elevation );
              double bed_elev = bed_elevation->values()[eInx];
              if ( std::isnan( bed_elev ) || fabs( bed_elev - val ) > eps ) // change from bed elevation
              {
                values[eInx] = val;
              }
            }
          }
        }
      }
    }
  }

  for ( auto dataset : datasets )
  {
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mMesh->datasetGroups.push_back( group );

  return datasets[0];
}

std::shared_ptr<MDAL::MemoryDataset> MDAL::DriverHec2D::readBedElevation(
  const HdfGroup &gGeom2DFlowAreas,
  const std::vector<size_t> &areaElemStartIndex,
  const std::vector<std::string> &flowAreaNames )
{
  std::vector<float> times( 1, 0.0f );
  return readElemOutput(
           gGeom2DFlowAreas,
           areaElemStartIndex,
           flowAreaNames,
           "Cells Minimum Elevation",
           "Bed Elevation",
           times,
           std::shared_ptr<MDAL::MemoryDataset>()
         );
}

void MDAL::DriverHec2D::readElemResults(
  const HdfFile &hdfFile,
  std::shared_ptr<MDAL::MemoryDataset> bed_elevation,
  const std::vector<size_t> &areaElemStartIndex,
  const std::vector<std::string> &flowAreaNames )
{
  // UNSTEADY
  HdfGroup flowGroup = get2DFlowAreasGroup( hdfFile, "Unsteady Time Series" );
  std::vector<float> times = readTimes( hdfFile );

  readElemOutput(
    flowGroup,
    areaElemStartIndex,
    flowAreaNames,
    "Water Surface",
    "Water Surface",
    times,
    bed_elevation );
  readElemOutput(
    flowGroup,
    areaElemStartIndex,
    flowAreaNames,
    "Depth",
    "Depth",
    times,
    bed_elevation );

  // SUMMARY
  flowGroup = get2DFlowAreasGroup( hdfFile, "Summary Output" );
  times.clear();
  times.push_back( 0.0f );

  readElemOutput(
    flowGroup,
    areaElemStartIndex,
    flowAreaNames,
    "Maximum Water Surface",
    "Water Surface/Maximums",
    times,
    bed_elevation
  );
}

std::vector<std::string> read2DFlowAreasNames( HdfGroup gGeom2DFlowAreas )
{
  HdfDataset dsNames = openHdfDataset( gGeom2DFlowAreas, "Names" );
  std::vector<std::string> names = dsNames.readArrayString();
  if ( names.empty() )
  {
    throw MDAL_Status::Err_InvalidData;
  }
  return names;
}

void MDAL::DriverHec2D::setProjection( HdfFile hdfFile )
{
  try
  {
    std::string proj_wkt = openHdfAttribute( hdfFile, "Projection" );
    mMesh->setSourceCrsFromWKT( proj_wkt );
  }
  catch ( MDAL_Status ) { /* projection not set */}
}

void MDAL::DriverHec2D::parseMesh(
  HdfGroup gGeom2DFlowAreas,
  std::vector<size_t> &areaElemStartIndex,
  const std::vector<std::string> &flowAreaNames )
{
  Faces faces;
  Vertices vertices;

  size_t maxVerticesInFace = 0;

  for ( size_t nArea = 0; nArea < flowAreaNames.size(); ++nArea )
  {
    std::string flowAreaName = flowAreaNames[nArea];

    HdfGroup gArea = openHdfGroup( gGeom2DFlowAreas, flowAreaName );

    HdfDataset dsCoords = openHdfDataset( gArea, "FacePoints Coordinate" );
    std::vector<hsize_t> cdims = dsCoords.dims();
    std::vector<double> coords = dsCoords.readArrayDouble(); //2xnNodes matrix in array
    size_t nNodes = cdims[0];
    size_t areaNodeStartIndex = vertices.size();
    vertices.resize( areaNodeStartIndex + nNodes );
    for ( size_t n = 0; n < nNodes; ++n )
    {
      size_t nIdx = areaNodeStartIndex + n;
      vertices[nIdx].x = coords[cdims[1] * n];
      vertices[nIdx].y = coords[cdims[1] * n + 1];
    }

    HdfDataset dsElems = openHdfDataset( gArea, "Cells FacePoint Indexes" );
    std::vector<hsize_t> edims = dsElems.dims();
    size_t nElems = edims[0];
    size_t maxFaces = edims[1]; // elems have up to 8 faces, but sometimes the table has less than 8 columns
    std::vector<int> elem_nodes = dsElems.readArrayInt(); //maxFacesxnElements matrix in array
    areaElemStartIndex[nArea] = faces.size();
    faces.resize( faces.size() + nElems );
    for ( size_t e = 0; e < nElems; ++e )
    {
      size_t eIdx = areaElemStartIndex[nArea] + e;
      std::vector<size_t> idx( maxFaces );
      size_t nValidVertexes = maxFaces;
      for ( size_t fi = 0; fi < maxFaces; ++fi )
      {
        int elem_node_idx = elem_nodes[edims[1] * e + fi];

        if ( elem_node_idx == -1 )
        {
          nValidVertexes = fi;
          break;
        }
        else
        {
          idx[fi] = areaNodeStartIndex + static_cast<size_t>( elem_node_idx ); // shift by this area start node index
        }
      }
      if ( nValidVertexes > 0 )
        faces[eIdx].assign( idx.begin(), std::next( idx.begin(), nValidVertexes ) );

      if ( nValidVertexes > maxVerticesInFace )
        maxVerticesInFace = nValidVertexes;
    }
  }
  areaElemStartIndex[flowAreaNames.size()] = faces.size();

  mMesh.reset(
    new MemoryMesh(
      name(),
      vertices.size(),
      faces.size(),
      maxVerticesInFace,
      computeExtent( vertices ),
      mFileName
    )
  );
  mMesh->faces = faces;
  mMesh->vertices = vertices;
}

MDAL::DriverHec2D::DriverHec2D()
  : Driver( "HEC2D",
            "HEC-RAS 2D",
            "*.hdf",
            Capability::ReadMesh )
{
}

MDAL::DriverHec2D *MDAL::DriverHec2D::create()
{
  return new DriverHec2D();
}

bool MDAL::DriverHec2D::canRead( const std::string &uri )
{
  try
  {
    HdfFile hdfFile = openHdfFile( uri );
    std::string fileType = openHdfAttribute( hdfFile, "File Type" );
    if ( fileType != "HEC-RAS Results" )
    {
      return false;
    }
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverHec2D::load( const std::string &resultsFile, MDAL_Status *status )
{
  mFileName = resultsFile;
  if ( status ) *status = MDAL_Status::None;
  mMesh.reset();

  try
  {
    HdfFile hdfFile = openHdfFile( mFileName );

    // Verify it is correct file
    std::string fileType = openHdfAttribute( hdfFile, "File Type" );
    if ( fileType != "HEC-RAS Results" )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }

    HdfGroup gGeom = openHdfGroup( hdfFile, "Geometry" );
    HdfGroup gGeom2DFlowAreas = openHdfGroup( gGeom, "2D Flow Areas" );

    std::vector<std::string> flowAreaNames = read2DFlowAreasNames( gGeom2DFlowAreas );
    std::vector<size_t> areaElemStartIndex( flowAreaNames.size() + 1 );

    parseMesh( gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames );
    setProjection( hdfFile );

    //Elevation
    std::shared_ptr<MDAL::MemoryDataset> bed_elevation = readBedElevation( gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames );

    // Element centered Values
    readElemResults( hdfFile, bed_elevation, areaElemStartIndex, flowAreaNames );

    // Face centered Values
    readFaceResults( hdfFile, areaElemStartIndex, flowAreaNames );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    mMesh.reset();
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}
