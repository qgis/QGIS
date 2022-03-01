/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2022 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_h2i.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <gdal.h>
#include <cpl_string.h>

#include "mdal_utils.hpp"
#include "mdal_logger.hpp"
#include "mdal_memory_data_model.hpp"


#define DRIVER_NAME "H2I"

MDAL::DriverH2i::DriverH2i():
  Driver( DRIVER_NAME,
          "H2i Mesh File",
          "*.json",
          Capability::ReadMesh )
{}

MDAL::DriverH2i *MDAL::DriverH2i::create()
{
  return new DriverH2i();
}

class GpkgDataset
{
  public:
    GpkgDataset() = default;
    ~GpkgDataset()
    {
      if ( mHDataset ) GDALClose( mHDataset );

    }
    bool open( std::string fileName )
    {
      GDALAllRegister();
      GDALDriverH hDriver = GDALGetDriverByName( "GPKG" );
      if ( !hDriver ) throw MDAL::Error( MDAL_Status::Err_MissingDriver, "No GDAL GPKG driver found, unable to read H2i format" );

      char **papszAllowedDrivers = nullptr;
      papszAllowedDrivers = CSLAddString( papszAllowedDrivers, "GPKG" );
      mHDataset = GDALOpenEx( fileName.c_str(), GDAL_OF_VECTOR, papszAllowedDrivers, nullptr, nullptr );
      CSLDestroy( papszAllowedDrivers );

      return ( mHDataset != nullptr );
    }

    bool hasLayer( std::string layerName )
    {
      OGRLayerH hLayer;
      hLayer = GDALDatasetGetLayerByName( mHDataset, layerName.c_str() );
      return hLayer != nullptr;
    }

    GDALDatasetH mHDataset = nullptr;
};


bool MDAL::DriverH2i::canReadMesh( const std::string &uri )
{
  MetadataH2i metadata;
  if ( !parseJsonFile( uri, metadata ) )
    return false;

  GpkgDataset gridDataset;
  if ( !gridDataset.open( metadata.dirPath + '/' + metadata.gridFile ) )
    return false;

  if ( !gridDataset.hasLayer( metadata.gridlayer ) )
    return false;

  return true;
}

std::string MDAL::DriverH2i::buildUri( const std::string &meshFile )
{
  MetadataH2i metadata;

  if ( !parseJsonFile( meshFile, metadata ) )
    return std::string();

  return MDAL::buildMeshUri( meshFile, metadata.meshName, name() );
}

bool MDAL::DriverH2i::parseJsonFile( const std::string filePath, MetadataH2i &metadata )
{
  using Json = nlohmann::json;

  std::ifstream file = MDAL::openInputFile( filePath );
  if ( !file.is_open() )
    return false;


  std::stringstream stream;
  stream << file.rdbuf();
  std::string jsonString = stream.str();

  try
  {
    Json jsonMeta;
    jsonMeta = Json::parse( jsonString );

    if ( !jsonMeta.contains( "name" ) ||
         !jsonMeta.contains( "crs" ) ||
         !jsonMeta.contains( "mesh" ) ||
         !jsonMeta.contains( "timesteps" )
       )
      return false;

    metadata.meshName = jsonMeta["name"].get<std::string>();
    metadata.crs = jsonMeta["crs"].get<std::string>();
    metadata.gridFile = jsonMeta["mesh"]["nodes_mesh"]["file"].get<std::string>();
    metadata.gridlayer = jsonMeta["mesh"]["nodes_mesh"]["layer"].get<std::string>();
    metadata.referenceTime = jsonMeta["timesteps"]["default"]["start_datetime"].get<std::string>();
    metadata.timeStepFile = jsonMeta["timesteps"]["default"]["timesteps_file"].get<std::string>();

    for ( Json::iterator it = jsonMeta["results"].begin(); it != jsonMeta["results"].end(); ++it )
    {
      MetadataH2iDataset metaDataset;
      metaDataset.layer = ( *it )["layer"];
      metaDataset.file = ( *it )["result_file"];
      metaDataset.type = ( *it )["type"];
      metaDataset.units = ( *it )["units"];
      metaDataset.topology_file = ( *it )["topology_file"];
      metaDataset.isScalar = !( ( *it ).contains( "vector" ) && ( *it )["vector"] );
      metadata.datasetGroups.push_back( metaDataset );
    }

    metadata.metadataFilePath = filePath;
    metadata.dirPath = MDAL::dirName( filePath );

  }
  catch ( Json::exception &e )
  {
    return false;
  }

  return true;
}

struct VertexFactory
{
  VertexFactory( std::vector<MDAL::Vertex> &vertices ):
    verticesRef( vertices )
  {}

  size_t getVertex( double x, double y )
  {
    auto it = createdVertexPosition.find( {x, y} );

    if ( it == createdVertexPosition.end() )
    {
      int vertIndex;
      vertIndex = verticesRef.size();
      verticesRef.push_back( {x, y, 0} );
      createdVertexPosition[ {x, y}] = vertIndex;

      return vertIndex;
    }
    else
    {
      return it->second;
    }
  }

  struct VertexPosition
  {
    double x = 0;
    double y = 0;

    bool operator<( const VertexPosition &other ) const
    {
      double epsX = ( this->x + other.x ) * std::numeric_limits<double>::epsilon();
      if ( other.x - this->x > epsX )
        return true;

      if ( this->x - other.x > epsX )
        return false;

      double epsY = ( this->y + other.y ) * std::numeric_limits<double>::epsilon();
      if ( other.y - this->y > epsY )
        return true;

      return false;
    }
  };

  std::vector<MDAL::Vertex> &verticesRef;
  // this map stores created vertices considering the position store in VertexPosition struct
  std::map<VertexPosition, size_t> createdVertexPosition;
};


std::unique_ptr<MDAL::Mesh> MDAL::DriverH2i::createMeshFrame( const MDAL::DriverH2i::MetadataH2i &metadata )
{
  GpkgDataset gridDataset;
  const std::string gridFile = metadata.dirPath + '/' + metadata.gridFile;
  if ( !gridDataset.open( gridFile ) )
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Unable to open the grid file: " + gridFile );

  OGRLayerH hLayer;
  hLayer = GDALDatasetGetLayerByName( gridDataset.mHDataset, metadata.gridlayer.c_str() );
  if ( hLayer == nullptr )
    throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Unable to find the gpkg layer containing the mesh: " + metadata.gridlayer );

  OGREnvelope layerExtent;
  OGR_L_GetExtent( hLayer, &layerExtent, 1 );

  OGRFeatureDefnH hFDefn = OGR_L_GetLayerDefn( hLayer );
  int nodeFieldIndex = OGR_FD_GetFieldIndex( hFDefn, "number" );
  if ( nodeFieldIndex == -1 )
    throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Unable to find the field \"number\" in the gpkg layer: " + metadata.gridlayer );

  std::vector<Vertex> vertices;
  int maxVerticesCount = 0;
  VertexFactory vertexFactory( vertices );

  size_t facesCount = static_cast<size_t>( OGR_L_GetFeatureCount( hLayer, 1 ) );
  std::vector<Face> faces( facesCount );

  OGRFeatureH hFeature;
  OGR_L_ResetReading( hLayer );
  while ( ( hFeature = OGR_L_GetNextFeature( hLayer ) ) != nullptr )
  {
    size_t faceIndex = static_cast<size_t>( OGR_F_GetFieldAsInteger( hFeature, nodeFieldIndex ) - 1 );
    if ( faceIndex >= facesCount )
      throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Bad face indexing in the gpkg layer: " + metadata.gridlayer );

    OGRGeometryH hGeometry = OGR_F_GetGeometryRef( hFeature );
    if ( hGeometry != nullptr && wkbFlatten( OGR_G_GetGeometryType( hGeometry ) ) == wkbPolygon )
    {
      OGRGeometryH exteriorRing = OGR_G_GetGeometryRef( hGeometry, 0 );

      int pointCount = OGR_G_GetPointCount( exteriorRing );
      Face face;
      for ( int i = 0; i < pointCount - 1; ++i )
      {
        double x = OGR_G_GetX( exteriorRing, i );
        double y = OGR_G_GetY( exteriorRing, i );

        size_t vertexIndex = vertexFactory.getVertex( x, y );
        face.push_back( vertexIndex );
      }
      if ( MDAL::toInt( face.size() ) > maxVerticesCount )
        maxVerticesCount = MDAL::toInt( face.size() );
      faces[faceIndex] = face;
    }
  }

  std::unique_ptr<MemoryMesh> mesh = std::make_unique<MemoryMesh>( name(), maxVerticesCount, metadata.metadataFilePath );

  mesh->setVertices( std::move( vertices ) );
  mesh->setFaces( std::move( faces ) );

  return mesh;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverH2i::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();

  MetadataH2i metadata;
  if ( !parseJsonFile( meshFile, metadata ) )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), meshFile + " could not be opened" );
    return nullptr;
  }

  std::unique_ptr<Mesh> mesh = createMeshFrame( metadata );
  mesh->setSourceCrs( metadata.crs );

  // read time information
  DateTime referenceTime;
  std::vector<RelativeTimestamp> timeSteps;
  parseTime( metadata, referenceTime, timeSteps );

  // build the dataset groups for all temporal quantities.
  const std::vector<MetadataH2iDataset> &metaGroups = metadata.datasetGroups;
  for ( const MetadataH2iDataset &metadatasetGroup : metaGroups )
  {
    std::shared_ptr<DatasetGroup> group =
      std::make_shared<DatasetGroup>( name(), mesh.get(), metadatasetGroup.file, metadatasetGroup.layer );
    std::string datasetGroupFile = metadata.dirPath + '/' + metadatasetGroup.file;
    std::shared_ptr<std::ifstream> in = std::make_shared<std::ifstream>( datasetGroupFile, std::ifstream::binary );

    if ( !in->is_open() )
      continue;

    group->setReferenceTime( referenceTime );
    group->setDataLocation( MDAL_DataLocation::DataOnFaces );
    group->setMetadata( "units", metadatasetGroup.units );
    group->setMetadata( "type", metadatasetGroup.type );
    group->setIsScalar( metadatasetGroup.isScalar );

    if ( metadatasetGroup.topology_file == "2d_nodes" )
    {
      if ( metadatasetGroup.isScalar )
      {
        for ( size_t datasetIndex = 0; datasetIndex < timeSteps.size(); ++datasetIndex )
        {
          std::shared_ptr<DatasetH2iScalar> dataset = std::make_shared<DatasetH2iScalar>( group.get(), in, datasetIndex );
          dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
          group->datasets.push_back( dataset );
          dataset->clear(); // Lazy loading, so we clear the loaded data during statistic calculation
          dataset->setTime( timeSteps.at( datasetIndex ) );
        }
      }
      else
      {
        for ( size_t datasetIndex = 0; datasetIndex < timeSteps.size(); ++datasetIndex )
        {
          std::shared_ptr<DatasetH2iVector> dataset = std::make_shared<DatasetH2iVector>( group.get(), in, datasetIndex );
          dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
          group->datasets.push_back( dataset );
          dataset->clear();
          dataset->setTime( timeSteps.at( datasetIndex ) );
        }
      }

      group->setStatistics( MDAL::calculateStatistics( group ) );
      mesh->datasetGroups.push_back( group );
    }
  }
  return mesh;
}

void MDAL::DriverH2i::parseTime( const MDAL::DriverH2i::MetadataH2i &metadata, MDAL::DateTime &referenceTime, std::vector<MDAL::RelativeTimestamp> &timeSteps )
{
  referenceTime = DateTime( metadata.referenceTime );

  const std::string timeFilePath = metadata.dirPath + '/' + metadata.timeStepFile;
  std::ifstream timeFile = MDAL::openInputFile( timeFilePath );

  if ( !timeFile.is_open() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + timeFilePath );

  timeSteps.clear();
  std::string line;
  while ( std::getline( timeFile, line ) )
  {
    const std::vector<std::string> lineElements = split( line, ' ' );
    if ( lineElements.size() != 2 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + timeFilePath );

    timeSteps.emplace_back( toDouble( lineElements.at( 1 ) ), RelativeTimestamp::seconds );
  }
}

MDAL::DatasetH2iScalar::DatasetH2iScalar( MDAL::DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex )
  : DatasetH2i( grp, in, datasetIndex )
{}

size_t MDAL::DatasetH2iScalar::scalarData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataLoaded )
    loadData();
  size_t nValues = valuesCount();

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mValues.data() + indexStart, copyValues * sizeof( double ) );
  return copyValues;
}

MDAL::DatasetH2i::DatasetH2i( MDAL::DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex ) : Dataset2D( grp )
  , mIn( in )
  , mDatasetIndex( datasetIndex )
{}

void MDAL::DatasetH2i::clear()
{
  mValues.clear();
  mDataLoaded = false;
}

void MDAL::DatasetH2iScalar::loadData()
{
  mIn->seekg( beginingInFile() );
  int datasetSize = 0;
  bool changeEndianness = false;
  MDAL::readValue( datasetSize, *mIn, changeEndianness );

  if ( datasetSize != MDAL::toInt( valuesCount() *sizeof( double ) ) )
  {
    changeEndianness = true;
    mIn->seekg( beginingInFile() );
    MDAL::readValue( datasetSize, *mIn, changeEndianness );
    if ( datasetSize != MDAL::toInt( valuesCount() *sizeof( double ) ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + group()->uri() );
  }

  mValues.resize( valuesCount() );
  for ( size_t i = 0; i < valuesCount(); ++i )
  {
    if ( !readValue( mValues[i], *mIn, changeEndianness ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error when reading file: " + group()->uri() );
  }

  mDataLoaded = true;
}

std::streampos MDAL::DatasetH2iScalar::beginingInFile() const
{
  return ( sizeof( int ) * 2 + sizeof( double ) * valuesCount() ) * mDatasetIndex;
}

MDAL::DatasetH2iVector::DatasetH2iVector( MDAL::DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex )
  : DatasetH2i( grp, in, datasetIndex )
{}

size_t MDAL::DatasetH2iVector::vectorData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataLoaded )
    loadData();
  size_t nValues = mValues.size() / 2;

  if ( ( count < 1 ) || ( indexStart  >= nValues ) )
    return 0;

  size_t copyValues = 2 * std::min( nValues - indexStart, count );
  memcpy( buffer, &mValues.data()[+indexStart * 2], copyValues * sizeof( double ) );
  return copyValues / 2;
}

void MDAL::DatasetH2iVector::loadData()
{
  mIn->seekg( beginingInFile() );
  int datasetSize = 0;
  bool changeEndianness = false;

  // First, magnitude values
  MDAL::readValue( datasetSize, *mIn, changeEndianness );
  if ( datasetSize != MDAL::toInt( valuesCount() *sizeof( double ) ) )
  {
    changeEndianness = true;
    mIn->seekg( beginingInFile() );
    MDAL::readValue( datasetSize, *mIn, changeEndianness );
    if ( datasetSize != MDAL::toInt( 2 * valuesCount() *sizeof( double ) ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + group()->uri() );
  }

  mValues = std::vector<double>( valuesCount() * 2, 0 );

  for ( size_t i = 0; i < valuesCount() ; ++i )
  {
    double magnitude = 0;
    if ( !readValue( magnitude, *mIn, changeEndianness ) )  throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error when reading file: " + group()->uri() );
    mValues[2 * i] = magnitude;
  }

  MDAL::readValue( datasetSize, *mIn, changeEndianness );//last record is again the data count

  // Now, direction values
  MDAL::readValue( datasetSize, *mIn, changeEndianness );
  if ( datasetSize != MDAL::toInt( valuesCount() *sizeof( double ) ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + group()->uri() );

  for ( size_t i = 0; i < valuesCount() ; ++i )
  {
    double direction = 0;
    if ( !readValue( direction, *mIn, changeEndianness ) )  throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error when reading file: " + group()->uri() );
    mValues[2 * i + 1] = mValues[2 * i] * sin( -direction );
    mValues[2 * i] = mValues[2 * i] * cos( -direction );

  }
  mDataLoaded = true;
}

std::streampos MDAL::DatasetH2iVector::beginingInFile() const
{
  return ( sizeof( int ) * 4 + sizeof( double ) * valuesCount() * 2 ) * mDatasetIndex;
}
