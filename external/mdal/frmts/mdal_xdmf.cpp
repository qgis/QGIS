/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <map>
#include <cmath>
#include <limits>

#include "mdal_xdmf.hpp"
#include "mdal_utils.hpp"
#include "mdal_data_model.hpp"
#include "mdal_xml.hpp"
#include "mdal_logger.hpp"

MDAL::XdmfDataset::XdmfDataset( MDAL::DatasetGroup *grp,
                                const MDAL::HyperSlab &slab,
                                const HdfDataset &valuesDs,
                                RelativeTimestamp time )
  : MDAL::Dataset2D( grp )
  , mHdf5DatasetValues( valuesDs )
  , mHyperSlab( slab )
{
  setTime( time );
}

MDAL::XdmfDataset::~XdmfDataset() = default;

std::vector<hsize_t> MDAL::XdmfDataset::offsets( size_t indexStart )
{
  std::vector<hsize_t> ret( 2 );
  ret[0] = mHyperSlab.startX + indexStart;
  ret[1] = mHyperSlab.startY;
  return ret;
}

std::vector<hsize_t> MDAL::XdmfDataset::selections( size_t copyValues )
{
  std::vector<hsize_t> ret( 2 );
  if ( mHyperSlab.countInFirstColumn )
  {
    ret[0] = copyValues;
    ret[1] = mHyperSlab.isScalar ? 1 : 3;
  }
  else
  {
    ret[0] = mHyperSlab.isScalar ? 1 : 3;
    ret[1] = copyValues;
  }
  return ret;
}

size_t MDAL::XdmfDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  assert( mHyperSlab.isScalar );

  size_t nValues = mHyperSlab.count;
  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;
  size_t copyValues = std::min( nValues - indexStart, count );

  std::vector<hsize_t> off = offsets( indexStart );
  std::vector<hsize_t> counts = selections( copyValues );
  std::vector<double> values = mHdf5DatasetValues.readArrayDouble( off, counts );
  if ( values.empty() )
    return 0;

  const double *input = values.data();
  memcpy( buffer, input, copyValues * sizeof( double ) );
  return copyValues;
}


size_t MDAL::XdmfDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  assert( !mHyperSlab.isScalar );

  size_t nValues = mHyperSlab.count;
  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;
  size_t copyValues = std::min( nValues - indexStart, count );

  std::vector<hsize_t> off = offsets( indexStart );
  std::vector<hsize_t> counts = selections( copyValues );
  std::vector<double> values = mHdf5DatasetValues.readArrayDouble( off, counts );
  if ( values.empty() )
    return 0;

  const double *input = values.data();
  for ( size_t j = 0; j < copyValues; ++j )
  {
    buffer[2 * j] = input[3 * j];
    buffer[2 * j + 1] = input[3 * j + 1];
  }
  return copyValues;
}

// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////


MDAL::XdmfFunctionDataset::XdmfFunctionDataset(
  MDAL::DatasetGroup *grp,
  MDAL::XdmfFunctionDataset::FunctionType type,
  const RelativeTimestamp &time )
  : MDAL::Dataset2D( grp )
  , mType( type )
  , mBaseReferenceGroup( "XDMF", grp->mesh(), grp->uri() )
{
  setTime( time );
  mBaseReferenceGroup.setIsScalar( true );
  mBaseReferenceGroup.setDataLocation( grp->dataLocation() );
  mBaseReferenceGroup.setName( "Base group for reference datasets" );
}

MDAL::XdmfFunctionDataset::~XdmfFunctionDataset() = default;

void MDAL::XdmfFunctionDataset::addReferenceDataset(
  const HyperSlab &slab,
  const HdfDataset &hdfDataset,
  const MDAL::RelativeTimestamp &time )
{
  std::shared_ptr<MDAL::XdmfDataset> xdmfDataset = std::make_shared<MDAL::XdmfDataset>(
        &mBaseReferenceGroup,
        slab,
        hdfDataset,
        time
      );
  mReferenceDatasets.push_back( xdmfDataset );
}

void MDAL::XdmfFunctionDataset::swap()
{
  if ( mReferenceDatasets.size() < 2 )
    return;
  std::swap( mReferenceDatasets[0], mReferenceDatasets[1] );
}

size_t MDAL::XdmfFunctionDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  assert( mType != FunctionType::Join );

  if ( mType == FunctionType::Subtract )
    return subtractFunction( indexStart, count, buffer );

  if ( mType == FunctionType::Flow )
    return flowFunction( indexStart, count, buffer );

  return 0;
}

size_t MDAL::XdmfFunctionDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  assert( mType == FunctionType::Join );

  return joinFunction( indexStart, count, buffer );
}

size_t MDAL::XdmfFunctionDataset::subtractFunction( size_t indexStart, size_t count, double *buffer )
{
  std::vector<double> buf( 2 * count, std::numeric_limits<double>::quiet_NaN() );
  size_t copyVals = extractRawData( indexStart, count, 2, buf );
  for ( size_t j = 0; j < copyVals; ++j )
  {
    double x0 = buf[j];
    double x1 = buf[count + j];
    if ( !std::isnan( x0 ) && !std::isnan( x1 ) )
      buffer[j] = x1 - x0;
  }
  return copyVals;
}

size_t MDAL::XdmfFunctionDataset::flowFunction( size_t indexStart, size_t count, double *buffer )
{
  std::vector<double> buf( 4 * count, std::numeric_limits<double>::quiet_NaN() );
  size_t copyVals = extractRawData( indexStart, count, 4, buf );
  for ( size_t j = 0; j < copyVals; ++j )
  {
    double x0 = buf[1 * count + j];
    double x1 = buf[1 * count + j];
    double x2 = buf[2 * count + j];
    double x3 = buf[3 * count + j];

    if ( !std::isnan( x0 ) &&
         !std::isnan( x1 ) &&
         !std::isnan( x2 ) &&
         !MDAL::equals( x2, x3 )
       )
    {
      double diff = x2 - x3;
      buffer[j] = sqrt( ( x0 / diff ) * ( x0 / diff ) + ( x1 / diff ) * ( x1 / diff ) );
    }
  }
  return copyVals;
}

size_t MDAL::XdmfFunctionDataset::joinFunction( size_t indexStart, size_t count, double *buffer )
{
  std::vector<double> buf( 2 * count, std::numeric_limits<double>::quiet_NaN() );
  size_t copyVals = extractRawData( indexStart, count, 2, buf );
  for ( size_t j = 0; j < copyVals; ++j )
  {
    double x = buf[j];
    double y = buf[count + j];
    if ( !std::isnan( x ) && !std::isnan( y ) )
    {
      buffer[2 * j] = x;
      buffer[2 * j + 1] = y;
    }
  }
  return copyVals;
}

size_t MDAL::XdmfFunctionDataset::extractRawData( size_t indexStart, size_t count, size_t nDatasets, std::vector< double > &buf )
{
  assert( buf.size() == nDatasets * count );

  if ( mReferenceDatasets.size() < nDatasets )
    return 0;

  if ( !mReferenceDatasets[0]->group()->isScalar() )
    return 0;

  size_t ret = mReferenceDatasets[0]->scalarData( indexStart, count, buf.data() );
  for ( size_t i = 1; i < nDatasets ; ++i )
  {
    if ( !mReferenceDatasets[i]->group()->isScalar() )
      return 0;
    size_t ret1 = mReferenceDatasets[i]->scalarData( indexStart, count, buf.data() + count * i );
    if ( ret != ret1 )
      return 0;
  }
  return ret;
}

// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////

MDAL::HyperSlab MDAL::DriverXdmf::parseHyperSlab( const std::string &str, size_t dimX )
{
  std::stringstream slabSS( str );
  std::vector<std::vector<size_t>> data( 3, std::vector<size_t>( dimX ) );
  size_t i = 0;
  size_t number;
  while ( slabSS >> number )
  {
    data[i / dimX][i % dimX] = number;
    i++;
  }
  if ( i != 3 * dimX )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "hyperSlab dimensions mismatch" );
  }

  MDAL::HyperSlab slab;
  slab.startX = data[0][0];
  slab.startY = data[0][1];
  size_t countX = data[2][0];
  size_t countY = data[2][1];

  if ( ( data[1][0] != 1 ) || ( data[1][1] != 1 ) )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "only hyperSlab with stride 1 are supported" );
  }

  // sort
  if ( ( countX < countY ) && ( countY != 3 ) )
  {
    std::swap( countX, countY );
    slab.countInFirstColumn = false;
  }
  else
  {
    slab.countInFirstColumn = true;
  }
  slab.count = countX;

  if ( countY == 1 )
  {
    slab.isScalar = true;
  }
  else if ( countY == 3 )
  {
    slab.isScalar = false;
  }
  else
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "hyperSlab dimensions mismatch, not scalar or vector" );
  }

  return slab;
}

MDAL::HyperSlab MDAL::DriverXdmf::parseHyperSlabNode( const XMLFile &xmfFile, xmlNodePtr node )
{
  std::string slabDimS = xmfFile.attribute( node, "Dimensions" );
  std::vector<size_t> slabDim = parseDimensions2D( slabDimS );
  if ( slabDim[0] != 3 || ( slabDim[1] != 2 && slabDim[1] != 3 ) )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Only two-dimensional slab array with dim 3x3 is supported (1)" );
  }

  std::string slabS = xmfFile.content( node );
  HyperSlab slab = parseHyperSlab( slabS, slabDim[1] );
  return slab;
}

HdfDataset MDAL::DriverXdmf::parseHdf5Node( const XMLFile &xmfFile, xmlNodePtr node )
{
  std::string snapshotDimS = xmfFile.attribute( node, "Dimensions" );
  std::vector<size_t> snapshotDim = parseDimensions2D( snapshotDimS );

  std::string hdf5Name, hdf5Path;
  hdf5NamePath( xmfFile.content( node ), hdf5Name, hdf5Path );

  std::shared_ptr<HdfFile> hdfFile;
  if ( mHdfFiles.count( hdf5Name ) == 0 )
  {
    hdfFile = std::make_shared<HdfFile>( hdf5Name, HdfFile::ReadOnly );
    mHdfFiles[hdf5Name] = hdfFile;
  }
  else
  {
    hdfFile = mHdfFiles[hdf5Name];
  }

  if ( !hdfFile->isValid() )
    throw MDAL::Error( MDAL_Status::Err_InvalidData, "invalid or missing file: " + hdf5Name );

  return hdfFile->dataset( hdf5Path );
}

void MDAL::DriverXdmf::hdf5NamePath( const std::string &dataItemPath, std::string &filePath, std::string &hdf5Path )
{
  std::string dirName = MDAL::dirName( mDatFile );
  std::string path( dataItemPath );
  size_t endpos = path.find_last_not_of( " \t\n" );
  if ( std::string::npos != endpos )
  {
    path.erase( endpos + 1 );
  }
  size_t startpos = path.find_first_not_of( " \t\n" );
  if ( std::string::npos != startpos )
  {
    path.erase( 0, startpos );
  }

  std::vector<std::string> chunks = MDAL::split( path, ":" );
  if ( chunks.size() != 2 )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "must be in format fileName:hdfPath" );
  }

  filePath = dirName + "/" + chunks[0];
  hdf5Path = chunks[1];
}

std::vector<size_t> MDAL::DriverXdmf::parseDimensions2D( const std::string &data )
{
  std::stringstream slabDimSS( data );
  std::vector<size_t> slabDim;
  size_t number;
  while ( slabDimSS >> number )
    slabDim.push_back( number );
  if ( slabDim.size() != 2 )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Only two-dimensional slab array is supported" );
  }
  return slabDim;
}

std::pair<HdfDataset, MDAL::HyperSlab> MDAL::DriverXdmf::parseXdmfDataset(
  const XMLFile &xmfFile,
  xmlNodePtr itemNod )
{
  size_t facesCount = mMesh->facesCount();

  size_t dim = xmfFile.querySizeTAttribute( itemNod, "Dimensions" );
  if ( dim != facesCount )
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Dataset dimensions should correspond to the number of mesh elements" );
  }

  xmlNodePtr node1 = xmfFile.getCheckChild( itemNod, "DataItem" );
  xmlNodePtr node2 = xmfFile.getCheckSibling( node1, "DataItem" );

  std::string format1 = xmfFile.attribute( node1, "Format" );
  std::string format2 = xmfFile.attribute( node2, "Format" );

  if ( ( format1 == "XML" ) && ( format2 == "HDF" ) )
  {
    HyperSlab slab = parseHyperSlabNode( xmfFile, node1 );
    HdfDataset hdfDataset = parseHdf5Node( xmfFile, node2 );
    return std::make_pair( hdfDataset, slab );
  }
  else
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Only XML hyperSlab and HDF dataset Format supported" );
  }
}

MDAL::DatasetGroups MDAL::DriverXdmf::parseXdmfXml( )
{
  std::map< std::string, std::shared_ptr<MDAL::DatasetGroup> > groups;
  size_t nTimesteps = 0;

  XMLFile xmfFile;
  xmfFile.openFile( mDatFile );

  xmlNodePtr elem = xmfFile.getCheckRoot( "Xdmf" );
  elem = xmfFile.getCheckChild( elem, "Domain" );
  elem = xmfFile.getCheckChild( elem, "Grid" );

  xmfFile.checkAttribute( elem, "GridType", "Collection", "Expecting Collection Grid Type" );
  xmfFile.checkAttribute( elem, "CollectionType", "Temporal", "Expecting Temporal Collection Type" );

  elem = xmfFile.getCheckChild( elem, "Grid" );

  for ( xmlNodePtr gridNod = elem;
        gridNod != nullptr;
        gridNod = xmfFile.getCheckSibling( gridNod, "Grid", false ) )
  {
    ++nTimesteps;
    xmlNodePtr timeNod = xmfFile.getCheckChild( gridNod, "Time" );
    RelativeTimestamp time( xmfFile.queryDoubleAttribute( timeNod, "Value" ), RelativeTimestamp::hours ); //units, supposed to be hours
    xmlNodePtr scalarNod = xmfFile.getCheckChild( gridNod, "Attribute" );

    for ( ;
          scalarNod != nullptr;
          scalarNod = xmfFile.getCheckSibling( scalarNod, "Attribute", false ) )
    {
      xmfFile.checkAttribute( scalarNod, "Center", "Cell", "Only cell centered data is currently supported" );
      if ( !xmfFile.checkAttribute( scalarNod, "AttributeType", "Scalar" ) &&
           !xmfFile.checkAttribute( scalarNod, "AttributeType", "Vector" ) )
      {
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Only scalar and vector data are currently supported" );
      }
      std::string groupName = xmfFile.attribute( scalarNod, "Name" );
      if ( groupName.empty() )
      {
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Group name cannot be empty" );
      }

      xmlNodePtr itemNod = xmfFile.getCheckChild( scalarNod, "DataItem" );

      if ( xmfFile.checkAttribute( itemNod, "ItemType", "Function" ) )
      {
        std::string function = xmfFile.attribute( itemNod, "Function" );
        function = MDAL::replace( function, " ", "" );
        XdmfFunctionDataset::FunctionType type;
        bool reversed = false;
        bool isScalar = true;
        if ( function == "sqrt($0/($2-$3)*$0/($2-$3)+$1/($2-$3)*$1/($2-$3))" )
        {
          type = XdmfFunctionDataset::Flow;
        }
        else if ( function == "$0-$1" )
        {
          reversed = true;
          type = XdmfFunctionDataset::Subtract;
        }
        else if ( function == "$1-$0" )
        {
          type = XdmfFunctionDataset::Subtract;
        }
        else if ( ( function == "JOIN($0,$1,0*$1)" ) || ( function == "JOIN($0,$1,0)" ) )
        {
          type = XdmfFunctionDataset::Join;
          isScalar = false;
        }

        std::shared_ptr<MDAL::DatasetGroup> group = findGroup( groups, groupName, isScalar );
        std::shared_ptr<MDAL::XdmfFunctionDataset> xdmfFunctionDataset = std::make_shared<MDAL::XdmfFunctionDataset>(
              group.get(),
              type,
              time
            );

        xmlNodePtr dataNod = xmfFile.getCheckChild( itemNod, "DataItem" );
        for ( ;
              dataNod != nullptr;
              dataNod = xmfFile.getCheckSibling( dataNod, "DataItem", false ) )
        {
          if (
            xmfFile.checkAttribute( dataNod, "ItemType", "HyperSlab" ) ||
            xmfFile.checkAttribute( dataNod, "Type", "HyperSlab" ) )
          {
            std::pair<HdfDataset, HyperSlab> data = parseXdmfDataset( xmfFile, dataNod );
            xdmfFunctionDataset->addReferenceDataset( data.second, data.first, time );
          }
          else
          {
            throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Expecting HyperSlab Types under Function" );
          }
        }

        if ( reversed )
        {
          xdmfFunctionDataset->swap();
        }

        // This basically forces to load all data to calculate statistics!
        const MDAL::Statistics stats = MDAL::calculateStatistics( xdmfFunctionDataset );
        xdmfFunctionDataset->setStatistics( stats );
        group->datasets.push_back( xdmfFunctionDataset );
      }
      else if (
        xmfFile.checkAttribute( itemNod, "ItemType", "HyperSlab" ) ||
        xmfFile.checkAttribute( itemNod, "Type", "HyperSlab" ) )
      {
        std::pair<HdfDataset, HyperSlab> data = parseXdmfDataset( xmfFile, itemNod );
        std::shared_ptr<MDAL::DatasetGroup> group = findGroup( groups, groupName, data.second.isScalar );
        std::shared_ptr<MDAL::XdmfDataset> xdmfDataset = std::make_shared<MDAL::XdmfDataset>(
              group.get(),
              data.second,
              data.first,
              time
            );
        // This basically forces to load all data to calculate statistics!
        const MDAL::Statistics stats = MDAL::calculateStatistics( xdmfDataset );
        xdmfDataset->setStatistics( stats );
        group->datasets.push_back( xdmfDataset );
      }
      else
      {
        throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Expecting Function or HyperSlab Type" );
      }
    }
  }

  // check groups
  DatasetGroups ret;
  for ( const auto &group : groups )
  {
    std::shared_ptr<MDAL::DatasetGroup> grp = group.second;
    if ( grp->datasets.size() != nTimesteps )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Invalid group, missing timesteps" );
    }

    const MDAL::Statistics stats = MDAL::calculateStatistics( grp );
    grp->setStatistics( stats );
    // verify the integrity of the dataset
    if ( !std::isnan( stats.minimum ) )
      ret.push_back( grp );
  }

  return ret;
}

std::shared_ptr<MDAL::DatasetGroup> MDAL::DriverXdmf::findGroup( std::map<std::string, std::shared_ptr<MDAL::DatasetGroup> > &groups,
    const std::string &groupName,
    bool isScalar )
{
  std::shared_ptr<MDAL::DatasetGroup> group;
  if ( groups.count( groupName ) == 0 )
  {
    group = std::make_shared<MDAL::DatasetGroup>(
              "XDMF",
              mMesh,
              mDatFile,
              groupName
            );
    group->setIsScalar( isScalar );
    group->setDataLocation( MDAL_DataLocation::DataOnFaces ); //only center-based implemented
    groups[groupName] = group;
  }
  else
  {
    group = groups[groupName];
    if ( group->isScalar() != isScalar )
    {
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat,  "Inconsistent groups" );
    }
  }
  return group;
}


MDAL::DriverXdmf::DriverXdmf()
  : Driver( "XDMF",
            "XDMF",
            "*.xdmf;;*.xmf",
            Capability::ReadDatasets )
{
}

MDAL::DriverXdmf::~DriverXdmf() = default;

MDAL::DriverXdmf *MDAL::DriverXdmf::create()
{
  return new DriverXdmf();
}

bool MDAL::DriverXdmf::canReadDatasets( const std::string &uri )
{
  XMLFile xmfFile;
  try
  {
    xmfFile.openFile( uri );
    xmlNodePtr root = xmfFile.getCheckRoot( "Xdmf" );
    xmfFile.checkAttribute( root, "Version", "2.0", "Invalid version" );
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  catch ( MDAL::Error )
  {
    return false;
  }
  return true;
}

void MDAL::DriverXdmf::load( const std::string &datFile,
                             MDAL::Mesh *mesh )
{
  assert( mesh );

  mDatFile = datFile;
  mMesh = mesh;

  MDAL::Log::resetLastStatus();

  if ( !MDAL::fileExists( mDatFile ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, name(), "File could not be found " + mDatFile );
    return;
  }

  try
  {
    // parse XML
    DatasetGroups groups = parseXdmfXml( );
    // add groups to mesh
    for ( const auto &group : groups )
    {
      mMesh->datasetGroups.push_back( group );
    }
  }
  catch ( MDAL_Status err )
  {
    MDAL::Log::error( err, "Error occurred while loading file " + mDatFile );
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
  }
}
