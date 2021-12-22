/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_esri_tin.hpp"
#include "mdal_logger.hpp"

MDAL::DriverEsriTin::DriverEsriTin(): Driver( "ESRI_TIN",
      "Esri TIN",
      "*.adf",
      Capability::ReadMesh )
{}

MDAL::Driver *MDAL::DriverEsriTin::create()
{
  return new DriverEsriTin();
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverEsriTin::load( const std::string &uri, const std::string & )
{
  MDAL::Log::resetLastStatus();

  try
  {
    std::list<int> superpointIndexes;
    Faces faces;
    bool isNativeLittleEndian = MDAL::isNativeLittleEndian();

    //read the total number of vertices (including superpoints and isolated vertices)
    int32_t totalIndexesCount32;

    std::ifstream inDenv = MDAL::openInputFile( denvFile( uri ), std::ios_base::in | std::ios_base::binary );

    if ( !inDenv.is_open() )
    {
      inDenv = MDAL::openInputFile( denv9File( uri ), std::ifstream::in | std::ifstream::binary );
      if ( !inDenv.is_open() )
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + uri + " as denv file" );
    }
    readValue( totalIndexesCount32, inDenv, isNativeLittleEndian );
    size_t totalIndexesCount = static_cast<size_t>( totalIndexesCount32 );

    /* Round 1 :populates faces with raw indexes from the file
     * rawAndCorrectedIndexesMap is used to map raw indexes from the files and corrected indexes
     * Corrected indexes take into account the unwanted vertexes (superpoints, isolated vertices)
     * Wanted vertices are associated with the corrected index
     * Unwanted vertices are associated with the totalIndexesCount value
     */
    std::vector<size_t> rawAndCorrectedIndexesMap( totalIndexesCount, totalIndexesCount );
    std::ifstream inFaces = MDAL::openInputFile( faceFile( uri ), std::ifstream::in | std::ifstream::binary );
    std::ifstream inMsk = MDAL::openInputFile( mskFile( uri ), std::ifstream::in | std::ifstream::binary );
    std::ifstream inMsx = MDAL::openInputFile( msxFile( uri ), std::ifstream::in | std::ifstream::binary );

    if ( ! inFaces.is_open() )
      throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not open file " + uri + " as faces file" );
    if ( ! inMsk.is_open() )
      throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not open file " + uri + " as msk file" );
    if ( ! inMsx.is_open() )
      throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not open file " + uri + " as msx file" );

    //Find the beginning of data in mskFile
    inMsx.seekg( -4, std::ios::end );
    int32_t mskBegin;
    if ( ! readValue( mskBegin, inMsx, true ) )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to find the beginning of data in tmsx.adf file" );

    //read information in mskFile
    inMsk.seekg( -mskBegin * 2, std::ios::end );
    int32_t maskIntergerCount;
    if ( ! readValue( maskIntergerCount, inMsk, true ) )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to read information in tmsk.adf file" );
    inMsk.ignore( 4 ); //unused 4 bytes
    int32_t maskBitsCount;
    if ( ! readValue( maskBitsCount, inMsk, true ) )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to read information in tmsk.adf file" );

    int c = 0;
    int32_t maskInt = 0;
    while ( true )
    {
      //read mask file
      if ( c % 32 == 0 && c < maskBitsCount ) //first bit in the mask array have to be used-->read next maskInt
        if ( ! readValue( maskInt, inMsk, true ) )
          throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to read information in tmsk.adf file" );

      Face f;
      for ( int i = 0; i < 3; ++i )
      {
        int32_t index;
        if ( ! readValue( index, inFaces, isNativeLittleEndian ) )
          break;

        f.push_back( static_cast<size_t>( index - 1 ) );
      }

      if ( f.size() == 0 ) //that's mean this is the end of the file
        break;

      if ( f.size() < 3 ) //that's mean the face is not complete
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to read information in tnod.adf file, face is not complete" );

      //exclude masked face
      if ( !( maskInt & 0x01 ) )
      {
        faces.push_back( f );
        //fill raw indexes
        for ( auto ri : f )
        {
          if ( ri >= totalIndexesCount )
            throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "More raw indexes than total number of indexes" );
          rawAndCorrectedIndexesMap[ri] = 1;
        }
      }

      c++;
      maskInt = maskInt >> 1;
    }

    inFaces.close();
    inMsk.close();
    inMsx.close();

    //Round 2 :count the number of wanted indexes and fill the rawIndexes value with the correctedIndex
    size_t correctedIndexCount = 0;
    for ( size_t i = 0; i < rawAndCorrectedIndexesMap.size(); ++i )
    {
      if ( rawAndCorrectedIndexesMap.at( i ) < totalIndexesCount )
      {
        rawAndCorrectedIndexesMap[i] = correctedIndexCount;
        correctedIndexCount++;
      }
    }

    //Round 3: populate vertices
    Vertices vertices( correctedIndexCount );
    std::ifstream inXY = MDAL::openInputFile( xyFile( uri ), std::ifstream::in | std::ifstream::binary );
    std::ifstream inZ = MDAL::openInputFile( zFile( uri ), std::ifstream::in | std::ifstream::binary );

    if ( ! inXY.is_open() )
      throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not open file " + uri + " as xyFile type" );

    if ( ! inZ.is_open() )
      throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Could not open file " + uri + " as zFile type" );

    size_t rawIndex = 0;
    while ( rawIndex < rawAndCorrectedIndexesMap.size() )
    {
      Vertex vert;

      if ( !readValue( vert.x, inXY, isNativeLittleEndian ) )
        break;

      if ( !readValue( vert.y, inXY, isNativeLittleEndian ) )
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read value from xy file" );

      float zValue;
      if ( !readValue( zValue, inZ, isNativeLittleEndian ) )
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read value from z file" );
      vert.z = double( zValue );

      // store the vertex only if it is a wanted index
      if ( rawAndCorrectedIndexesMap[rawIndex] < totalIndexesCount )
        vertices[rawAndCorrectedIndexesMap[rawIndex]] = vert ;

      rawIndex++;
    }

    inXY.close();
    inZ.close();

    //Round 4 :apply correction to the face's indexes
    for ( auto &face : faces )
      for ( auto &fi : face )
        fi = rawAndCorrectedIndexesMap[fi];

    //create the memory mesh
    std::unique_ptr< MemoryMesh > mesh(
      new MemoryMesh(
        name(),
        3,
        uri
      )
    );

    //move the faces and the vertices in the mesh
    mesh->setFaces( std::move( faces ) );
    mesh->setVertices( std::move( vertices ) );

    //create the "Altitude" dataset
    addBedElevationDatasetGroup( mesh.get(), mesh->vertices() );
    mesh->datasetGroups.back()->setName( "Altitude" );

    std::string crs = getCrsWkt( uri );
    if ( ! crs.empty() )
      mesh->setSourceCrsFromWKT( crs );

    return std::unique_ptr<Mesh>( mesh.release() );
  }
  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "error while loading file " + uri );
    return std::unique_ptr<Mesh>();
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
    return std::unique_ptr<Mesh>();
  }
}

bool MDAL::DriverEsriTin::canReadMesh( const std::string &uri )
{

  std::string zFileName = zFile( uri );
  std::string faceFileName = faceFile( uri );

  std::ifstream xyIn = MDAL::openInputFile( xyFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! xyIn.is_open() )
    return false;

  std::ifstream zIn = MDAL::openInputFile( zFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! zIn.is_open() )
    return false;

  std::ifstream faceIn = MDAL::openInputFile( faceFile( uri ), std::ifstream::in | std::ifstream::binary );
  if ( ! faceIn.is_open() )
    return false;

  std::ifstream hullIn = MDAL::openInputFile( hullFile( uri ), std::ifstream::in | std::ifstream::binary );
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

std::string MDAL::DriverEsriTin::denvFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tdenv.adf" );
}

std::string MDAL::DriverEsriTin::denv9File( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "tdenv9.adf" );
}

std::string MDAL::DriverEsriTin::crsFile( const std::string &uri ) const
{
  return pathJoin( dirName( uri ), "prj.adf" );
}

/*
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
*/

std::string MDAL::DriverEsriTin::getCrsWkt( const std::string &uri ) const
{
  std::ifstream inCRS = MDAL::openInputFile( crsFile( uri ) );
  if ( ! inCRS.is_open() )
    return std::string();

  std::string crsWkt;
  std::getline( inCRS, crsWkt );

  if ( crsWkt == "{B286C06B-0879-11D2-AACA-00C04FA33C20}" ) //com class id of the esri UnknownCoordinateSystem class
    crsWkt = "";

  return crsWkt;
}

