/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_esri_tin.hpp"

MDAL::DriverEsriTin::DriverEsriTin(): Driver( "ESRI_TIN",
      "Esri TIN",
      "*.adf",
      Capability::ReadMesh )
{}

MDAL::Driver *MDAL::DriverEsriTin::create()
{
  return new DriverEsriTin();
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverEsriTin::load( const std::string &uri, MDAL_Status *status )
{
  if ( status ) *status = MDAL_Status::None;

  try
  {
    std::list<int> superpointIndexes;
    Vertices vertices;
    Faces faces;

    readSuperpoints( uri, superpointIndexes );
    populateVertices( uri, vertices, superpointIndexes );
    populateFaces( uri, faces, superpointIndexes );

    std::unique_ptr< MemoryMesh > mesh(
      new MemoryMesh(
        name(),
        vertices.size(),
        faces.size(),
        3,
        computeExtent( vertices ),
        uri
      )
    );

    mesh->faces = std::move( faces );
    mesh->vertices = std::move( vertices );

    addBedElevationDatasetGroup( mesh.get(), mesh->vertices );
    mesh->datasetGroups.back()->setName( "Altitude" );

    std::string crs = getCrsWkt( uri );
    if ( ! crs.empty() )
      mesh->setSourceCrsFromWKT( crs );

    return std::unique_ptr<Mesh>( mesh.release() );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    return std::unique_ptr<Mesh>();
  }
}

bool MDAL::DriverEsriTin::canRead( const std::string &uri )
{

  std::string zFileName = zFile( uri );
  std::string faceFileName = faceFile( uri );

  std::ifstream xyIn( xyFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! xyIn.is_open() )
    return false;

  std::ifstream zIn( zFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! zIn.is_open() )
    return false;

  std::ifstream faceIn( faceFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! faceIn.is_open() )
    return false;

  std::ifstream hullIn( hullFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! hullIn.is_open() )
    return false;

  return true;
}

std::string MDAL::DriverEsriTin::xyFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tnxy.adf" );
}

std::string MDAL::DriverEsriTin::zFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tnz.adf" );
}

std::string MDAL::DriverEsriTin::faceFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tnod.adf" );
}

std::string MDAL::DriverEsriTin::mskFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tmsk.adf" );
}

std::string MDAL::DriverEsriTin::msxFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tmsx.adf" );
}

std::string MDAL::DriverEsriTin::hullFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "thul.adf" );
}

std::string MDAL::DriverEsriTin::crsFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "prj.adf" );
}

void MDAL::DriverEsriTin::readSuperpoints( const std::string &uri, std::list<int> &superpointsIndexes ) const
{
  superpointsIndexes.clear();
  bool isNativeLittleEndian = MDAL::isNativeLittleEndian();
  std::ifstream inHull( hullFile( uri ), std::ifstream::in | std::ifstream::binary );

  int32_t index;
  while ( readValue( index, inHull, isNativeLittleEndian ) && index != -1 )
    superpointsIndexes.push_back( index );

  superpointsIndexes.sort();
}

std::string MDAL::DriverEsriTin::getTinName( const std::string &uri ) const
{
  std::string tinName = uri;
  size_t last_slash_idx = tinName.find_last_of( "\\/" );

  if ( last_slash_idx == std::string::npos )
    return "";

  tinName.erase( last_slash_idx, tinName.size() - last_slash_idx );
  last_slash_idx = tinName.find_last_of( "\\/" );

  if ( last_slash_idx == std::string::npos )
    return "";

  tinName.erase( 0, last_slash_idx + 1 );

  return tinName;
}

void MDAL::DriverEsriTin::populateVertices( const std::string &uri, MDAL::Vertices &vertices, const std::list<int> &superpointIndexes ) const
{
  //first, read the (x,y) coordinates

  bool isNativeLittleEndian = MDAL::isNativeLittleEndian();
  std::ifstream inXY( xyFile( uri ), std::ifstream::in | std::ifstream::binary );
  std::ifstream inZ( zFile( uri ), std::ifstream::in | std::ifstream::binary );

  if ( ! inXY.is_open() )
    throw MDAL_Status::Err_FileNotFound;

  if ( ! inZ.is_open() )
    throw MDAL_Status::Err_FileNotFound;

  int fileIndex = 1;
  while ( true )
  {
    Vertex vert;

    if ( !readValue( vert.x, inXY, isNativeLittleEndian ) )
      break;

    if ( !readValue( vert.y, inXY, isNativeLittleEndian ) )
      throw MDAL_Status::Err_UnknownFormat;

    float zValue;
    if ( !readValue( zValue, inZ, isNativeLittleEndian ) )
      throw MDAL_Status::Err_UnknownFormat;
    vert.z = double( zValue );

    if ( correctedIndex( fileIndex, superpointIndexes ) >= 0 ) //store the vertex only if it is not a superpoint
      vertices.push_back( vert );
    fileIndex++;
  }

  inXY.close();
  inZ.close();
}

void MDAL::DriverEsriTin::populateFaces( const std::string &uri, MDAL::Faces &faces, const std::list<int> &superpointIndexes ) const
{
  bool isNativeLittleEndian = MDAL::isNativeLittleEndian();
  std::ifstream inFaces( faceFile( uri ), std::ifstream::in | std::ifstream::binary );
  std::ifstream inMsk( mskFile( uri ), std::ifstream::in | std::ifstream::binary );
  std::ifstream inMsx( msxFile( uri ), std::ifstream::in | std::ifstream::binary );

  if ( ! inFaces.is_open() )
    throw MDAL_Status::Err_FileNotFound;
  if ( ! inMsk.is_open() )
    throw MDAL_Status::Err_FileNotFound;
  if ( ! inMsx.is_open() )
    throw MDAL_Status::Err_FileNotFound;

  //Find the beginning of data in mskFile
  inMsx.seekg( -4, std::ios::end );
  int32_t mskBegin;
  if ( ! readValue( mskBegin, inMsx, true ) )
    throw  MDAL_Status::Err_UnknownFormat;

  //read information in mskFile
  inMsk.seekg( -mskBegin * 2, std::ios::end );
  int32_t maskIntergerCount;
  if ( ! readValue( maskIntergerCount, inMsk, true ) )
    throw MDAL_Status::Err_UnknownFormat;
  inMsk.ignore( 4 ); //unused 4 bytes
  int32_t maskBitsCount;
  if ( ! readValue( maskBitsCount, inMsk, true ) )
    throw MDAL_Status::Err_UnknownFormat;

  int c = 0;
  int32_t maskInt = 0;
  while ( true )
  {
    //read mask file
    if ( c % 32 == 0 && c < maskBitsCount ) //first bit in the mask array have to be used-->read next maskInt
      if ( ! readValue( maskInt, inMsk, true ) )
        throw MDAL_Status::Err_UnknownFormat;

    Face f;
    for ( int i = 0; i < 3; ++i )
    {
      int32_t index;
      if ( ! readValue( index, inFaces, isNativeLittleEndian ) )
        break;

      index = correctedIndex( index, superpointIndexes );

      f.push_back( static_cast<size_t>( index ) );
    }

    if ( f.size() == 0 ) //that's mean this is the end of the file
      break;

    if ( f.size() < 3 ) //that's mean the face is not complete
      throw MDAL_Status::Err_UnknownFormat;

    //exclude masked face
    if ( !( maskInt & 0x01 ) )
      faces.push_back( f );

    c++;
    maskInt = maskInt >> 1;
  }

  inFaces.close();
  inMsk.close();
  inMsx.close();
}

std::string MDAL::DriverEsriTin::getCrsWkt( const std::string &uri ) const
{
  std::ifstream inCRS( crsFile( uri ), std::ifstream::in );
  if ( ! inCRS.is_open() )
    return std::string();

  std::string crsWkt;
  std::getline( inCRS, crsWkt );

  if ( crsWkt == "{B286C06B-0879-11D2-AACA-00C04FA33C20}" ) //com class id of the esri UnknownCoordinateSystem class
    crsWkt = "";

  return crsWkt;
}

int MDAL::DriverEsriTin::correctedIndex( int i, const std::list<int> &superPointsIndexes ) const
{
  //usualy i is bigger than the biggest superpoint's index
  if ( i > superPointsIndexes.back() )
    return i - int( superPointsIndexes.size() ) - 1;

  //correction of index depending of the position of the superpoints
  int correctedIndex = i - 1;
  for ( auto si : superPointsIndexes )
  {
    if ( i == si )
      return -1; //return -1 if the index is associated with superpoint
    else if ( i > si )
    {
      correctedIndex--;
    }
    else
      break;
  }

  return correctedIndex;
}
