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
#include "mdal_logger.hpp"

static HdfFile openHdfFile( const std::string &fileName )
{
  HdfFile file( fileName, HdfFile::ReadOnly );
  if ( !file.isValid() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open Hdf file " + fileName );
  return file;
}

static HdfGroup openHdfGroup( const HdfFile &hdfFile, const std::string &name )
{
  HdfGroup grp = hdfFile.group( name );
  if ( !grp.isValid() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open Hdf group " + name + " from file" );
  return grp;
}

static HdfGroup openHdfGroup( const HdfGroup &hdfGroup, const std::string &name )
{
  HdfGroup grp = hdfGroup.group( name );
  if ( !grp.isValid() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open Hdf group " + name + " from group" );
  return grp;
}


static HdfDataset openHdfDataset( const HdfGroup &hdfGroup, const std::string &name, bool *ok = nullptr )
{
  HdfDataset dsFileType = hdfGroup.dataset( name );
  if ( ok )
    *ok = dsFileType.isValid();
  else if ( !dsFileType.isValid() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open Hdf dataset " + name );

  return dsFileType;
}

static std::string openHdfAttribute( const HdfFile &hdfFile, const std::string &name )
{
  HdfAttribute attr = hdfFile.attribute( name );
  if ( !attr.isValid() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open Hdf attribute " + name + " from file" );
  return attr.readString();
}

static std::string openHdfAttribute( const HdfDataset &hdfDataset, const std::string &name )
{
  HdfAttribute attr = hdfDataset.attribute( name );
  if ( !attr.isValid() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open Hdf group " + name + " from dataset" );
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

static std::string getDataTimeUnit( HdfDataset &dsTime )
{
  // Initially we expect data to be in hours
  std::string dataTimeUnit = "Hours";

  // First we look for the Time attribute
  if ( dsTime.hasAttribute( "Time" ) )
  {
    dataTimeUnit = openHdfAttribute( dsTime, "Time" );
    return dataTimeUnit;
  }

  // Some variants of HEC_RAS have time unit stored in Variables attribute
  // With read values looking like "Time|Days       "
  if ( dsTime.hasAttribute( "Variables" ) )
  {
    dataTimeUnit = openHdfAttribute( dsTime, "Variables" );
    // Removing the "Time|" prefix
    dataTimeUnit = MDAL::replace( dataTimeUnit, "Time|", "" );
  }

  return dataTimeUnit;
}

static std::vector<MDAL::RelativeTimestamp> convertTimeData( std::vector<float> &times, const std::string &originalTimeDataUnit )
{
  std::vector<MDAL::RelativeTimestamp> convertedTime( times.size() );

  MDAL::RelativeTimestamp::Unit unit = MDAL::parseDurationTimeUnit( originalTimeDataUnit );

  for ( size_t i = 0; i < times.size(); i++ )
  {
    convertedTime[i] = MDAL::RelativeTimestamp( double( times[i] ), unit );
  }
  return convertedTime;
}


static MDAL::DateTime convertToDateTime( const std::string strDateTime )
{
  //HECRAS format date is 01JAN2000

  auto data = MDAL::split( strDateTime, " " );
  if ( data.size() < 2 )
    return MDAL::DateTime();

  std::string dateStr = data[0];

  int year = 0;
  int month = 0;
  int day = 0;

  if ( dateStr.size() == 9 )
  {
    day = MDAL::toInt( dateStr.substr( 0, 2 ) );
    std::string monthStr = dateStr.substr( 2, 3 );
    year = MDAL::toInt( dateStr.substr( 5, 4 ) );

    if ( monthStr == "JAN" )
      month = 1;
    else if ( monthStr == "FEB" )
      month = 2;
    else if ( monthStr == "MAR" )
      month = 3;
    else if ( monthStr == "APR" )
      month = 4;
    else if ( monthStr == "MAY" )
      month = 5;
    else if ( monthStr == "JUN" )
      month = 6;
    else if ( monthStr == "JUL" )
      month = 7;
    else if ( monthStr == "AUG" )
      month = 8;
    else if ( monthStr == "SEP" )
      month = 9;
    else if ( monthStr == "OCT" )
      month = 10;
    else if ( monthStr == "NOV" )
      month = 11;
    else if ( monthStr == "DEC" )
      month = 12;
  }

  std::string timeStr = data[1];

  auto timeData = MDAL::split( timeStr, ':' );

  int hours = 0;
  int min = 0;
  double sec = 0;

  if ( timeData.size() == 3 )
  {
    hours = MDAL::toInt( timeData[0] );
    min = MDAL::toInt( timeData[1] );
    sec = MDAL::toDouble( timeData[2] );
  }

  return MDAL::DateTime( year, month, day, hours, min, sec );
}

static MDAL::DateTime readReferenceDateTime( const HdfFile &hdfFile )
{
  std::string refTime;
  HdfGroup gBaseO = getBaseOutputGroup( hdfFile );
  HdfGroup gUnsteadTS = openHdfGroup( gBaseO, "Unsteady Time Series" );
  HdfDataset dsTimeDateStamp = openHdfDataset( gUnsteadTS, "Time Date Stamp" );
  std::vector<std::string> timeStamps = dsTimeDateStamp.readArrayString();

  if ( timeStamps.size() > 0 )
    return convertToDateTime( timeStamps[0] );

  return MDAL::DateTime();
}

static std::vector<MDAL::RelativeTimestamp> readTimes( const HdfFile &hdfFile )
{
  HdfGroup gBaseO = getBaseOutputGroup( hdfFile );
  HdfGroup gUnsteadTS = openHdfGroup( gBaseO, "Unsteady Time Series" );
  HdfDataset dsTime = openHdfDataset( gUnsteadTS, "Time" );
  std::string dataTimeUnits = getDataTimeUnit( dsTime );
  std::vector<float> times = dsTime.readArray();
  return convertTimeData( times, dataTimeUnits );
}

void MDAL::DriverHec2D::readFaceOutput( const HdfFile &hdfFile,
                                        const HdfGroup &rootGroup,
                                        const std::vector<size_t> &areaElemStartIndex,
                                        const std::vector<std::string> &flowAreaNames,
                                        const std::string rawDatasetName,
                                        const std::string datasetName,
                                        const std::vector<RelativeTimestamp> &times,
                                        const DateTime &referenceTime )
{
  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          mFileName,
                                          datasetName
                                        );
  group->setDataLocation( MDAL_DataLocation::DataOnFaces );
  group->setIsScalar( false );
  group->setReferenceTime( referenceTime );

  std::vector<std::shared_ptr<MDAL::MemoryDataset2D>> datasets;

  for ( size_t tidx = 0; tidx < times.size(); ++tidx )
  {
    std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MemoryDataset2D >( group.get() );
    dataset->setTime( times[tidx] );
    datasets.push_back( dataset );
  }

  std::shared_ptr<MDAL::MemoryDataset2D> firstDataset;

  for ( size_t nArea = 0; nArea < flowAreaNames.size(); ++nArea )
  {
    std::string flowAreaName = flowAreaNames[nArea];

    HdfGroup gFlowAreaRes = openHdfGroup( rootGroup, flowAreaName );
    HdfDataset dsVals;
    try
    {
      dsVals = openHdfDataset( gFlowAreaRes, rawDatasetName );
    }
    catch ( MDAL::Error )
    {
      return;
    }

    std::vector<float> vals = dsVals.readArray();

    HdfGroup gGeom = openHdfGroup( hdfFile, "Geometry" );
    HdfGroup gGeom2DFlowAreas = openHdfGroup( gGeom, "2D Flow Areas" );
    HdfGroup gArea = openHdfGroup( gGeom2DFlowAreas, flowAreaName );
    HdfDataset dsCellFaceInfo = openHdfDataset( gArea, "Cells Face and Orientation Info" );
    std::vector<hsize_t> celldims = dsCellFaceInfo.dims();
    std::vector<int> cellFaceInfo = dsCellFaceInfo.readArrayInt();

    HdfDataset dsCellFaceOrValues = openHdfDataset( gArea, "Cells Face and Orientation Values" );
    std::vector<int> cellFaceOrValues = dsCellFaceOrValues.readArrayInt();

    HdfDataset dsFacePointIndex = openHdfDataset( gArea, "Faces FacePoint Indexes" );
    std::vector<hsize_t> fdims = dsFacePointIndex.dims();
    size_t nFaces = static_cast<size_t>( fdims[0] );
    std::vector<int> facePointIndex = dsFacePointIndex.readArrayInt();

    HdfDataset dsCoords = openHdfDataset( gArea, "FacePoints Coordinate" );
    std::vector<hsize_t> cdims = dsCoords.dims();
    std::vector<double> coords = dsCoords.readArrayDouble(); //2xnNodes matrix in array

    for ( size_t tidx = 0; tidx < times.size(); ++tidx )
    {
      std::shared_ptr<MDAL::MemoryDataset2D> dataset = datasets[tidx];
      double *values = dataset->values();

      for ( size_t cell_idx = 0; cell_idx < celldims.at( 0 ); ++cell_idx )
      {
        size_t cell_mdal_idx = cell_idx + areaElemStartIndex[nArea];
        double valx = 0;
        double valy = 0;
        size_t consideredValueCount = 0;
        size_t firstPosition = static_cast<size_t>( cellFaceInfo[cell_idx * 2] );
        size_t faceCount = static_cast<size_t>( cellFaceInfo[cell_idx * 2 + 1] );
        for ( size_t f = 0; f < faceCount; ++f )
        {
          //get face indexes
          size_t faceIndex1 = static_cast<size_t>( cellFaceOrValues[( firstPosition + f ) * 2] );
          size_t faceIndex2 = static_cast<size_t>( cellFaceOrValues[( firstPosition + ( f + 1 ) % faceCount ) * 2] );
          double val1 = static_cast<double>( vals[faceIndex1 + tidx * nFaces] );
          double val2 = static_cast<double>( vals[faceIndex2 + tidx * nFaces] );
          if ( std::isnan( val1 ) || std::isnan( val2 ) )
            continue;

          size_t indexPoint11 = facePointIndex[faceIndex1 * 2];
          size_t indexPoint12 = facePointIndex[faceIndex1 * 2 + 1];
          size_t indexPoint21 = facePointIndex[faceIndex2 * 2];
          size_t indexPoint22 = facePointIndex[faceIndex2 * 2 + 1];
          bool commonIndex = ( indexPoint11 == indexPoint21 ||
                               indexPoint11 == indexPoint22 ||
                               indexPoint12 == indexPoint21 ||
                               indexPoint12 == indexPoint22 );
          if ( !commonIndex )
          {
            // should not happen, but better to prevent
            continue;
          }

          double dx1 = coords[indexPoint11 * 2] - coords[indexPoint12 * 2];
          double dy1 = coords[indexPoint11 * 2 + 1] - coords[indexPoint12 * 2 + 1];
          double dx2 = coords[indexPoint21 * 2] - coords[indexPoint22 * 2];
          double dy2 = coords[indexPoint21 * 2 + 1] - coords[indexPoint22 * 2 + 1];
          double l1 = sqrt( dx1 * dx1 + dy1 * dy1 );
          double l2 = sqrt( dx2 * dx2 + dy2 * dy2 );
          if ( l1 == 0 || l2 == 0 )
          {
            continue;
          }
          double nx1 =   -dy1 / l1;
          double ny1 =  dx1 / l1;
          double nx2 =   -dy2 / l2;
          double ny2 =  dx2 / l2;

          double deter = nx1 * ny2 - nx2 * ny1;
          if ( deter == 0 ) //colinear face, forbidden by hecras, but better to prevent
            continue;
          valx += ( ny2 * val1 - ny1 * val2 ) / deter;
          valy += ( nx1 * val2 - nx2 * val1 ) / deter;
          consideredValueCount++;
        }
        if ( consideredValueCount != 0 )
        {
          valx /= consideredValueCount;
          valy /= consideredValueCount;
          values[cell_mdal_idx * 2] = valx;
          values[cell_mdal_idx * 2 + 1] = valy;
        }
        else
        {
          values[cell_mdal_idx * 2] = std::numeric_limits<double>::quiet_NaN();
          values[cell_mdal_idx * 2 + 1] = std::numeric_limits<double>::quiet_NaN();
        }
      }
    }
  }

  for ( auto &dataset : datasets )
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
  MDAL::DateTime referenceDateTime = readReferenceDateTime( hdfFile );
  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Face Shear Stress", "Shear Stress", mTimes, referenceDateTime );
  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Face Velocity", "Velocity", mTimes, referenceDateTime );

  // SUMMARY
  flowGroup = get2DFlowAreasGroup( hdfFile, "Summary Output" );
  std::vector<MDAL::RelativeTimestamp> dummyTimes( 1, MDAL::RelativeTimestamp() );

  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Face Shear Stress", "Shear Stress/Maximums", dummyTimes, referenceDateTime );
  readFaceOutput( hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Face Velocity", "Velocity/Maximums", dummyTimes, referenceDateTime );
}


std::shared_ptr<MDAL::MemoryDataset2D> MDAL::DriverHec2D::readElemOutput( const HdfGroup &rootGroup,
    const std::vector<size_t> &areaElemStartIndex,
    const std::vector<std::string> &flowAreaNames,
    const std::string rawDatasetName,
    const std::string datasetName,
    const std::vector<RelativeTimestamp> &times,
    std::shared_ptr<MDAL::MemoryDataset2D> bed_elevation,
    const DateTime &referenceTime )
{
  double eps = static_cast<double>( std::numeric_limits<float>::epsilon() ); //hecras use float so comparison needs to use float epsilon

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          mFileName,
                                          datasetName
                                        );
  group->setDataLocation( MDAL_DataLocation::DataOnFaces );
  group->setIsScalar( true );
  group->setReferenceTime( referenceTime );

  std::vector<std::shared_ptr<MDAL::MemoryDataset2D>> datasets;

  for ( size_t tidx = 0; tidx < times.size(); ++tidx )
  {
    std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MemoryDataset2D >( group.get() );
    dataset->setTime( times[tidx] );
    datasets.push_back( dataset );
  }

  for ( size_t nArea = 0; nArea < flowAreaNames.size(); ++nArea )
  {
    size_t nAreaElements = areaElemStartIndex[nArea + 1] - areaElemStartIndex[nArea];
    std::string flowAreaName = flowAreaNames[nArea];
    HdfGroup gFlowAreaRes = openHdfGroup( rootGroup, flowAreaName );

    HdfDataset dsVals;
    try
    {
      dsVals = openHdfDataset( gFlowAreaRes, rawDatasetName );
    }
    catch ( MDAL::Error )
    {
      return nullptr;
    }

    std::vector<float> vals = dsVals.readArray();

    for ( size_t tidx = 0; tidx < times.size(); ++tidx )
    {
      std::shared_ptr<MDAL::MemoryDataset2D> dataset = datasets[tidx];
      double *values = dataset->values();

      for ( size_t i = 0; i < nAreaElements; ++i )
      {
        size_t idx = tidx * nAreaElements + i;
        size_t eInx = areaElemStartIndex[nArea] + i;
        double val =  static_cast<double>( vals[idx] );
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
            else //Water surface
            {
              assert( bed_elevation );
              double bed_elev = bed_elevation->scalarValue( eInx );
              if ( std::isnan( bed_elev ) || fabs( val - bed_elev ) > ( val + bed_elev )*eps )  // change from bed elevation
              {
                values[eInx] = val;
              }
            }
          }
        }
      }
    }
  }

  for ( auto &dataset : datasets )
  {
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mMesh->datasetGroups.push_back( group );

  return datasets[0];
}

std::shared_ptr<MDAL::MemoryDataset2D> MDAL::DriverHec2D::readBedElevation(
  const HdfGroup &gGeom2DFlowAreas,
  const std::vector<size_t> &areaElemStartIndex,
  const std::vector<std::string> &flowAreaNames )
{
  std::vector<MDAL::RelativeTimestamp> times( 1 );
  DateTime referenceTime;

  std::shared_ptr<MDAL::MemoryDataset2D> bedElevation = readElemOutput(
        gGeom2DFlowAreas,
        areaElemStartIndex,
        flowAreaNames,
        "Cells Minimum Elevation",
        "Bed Elevation",
        times,
        std::shared_ptr<MDAL::MemoryDataset2D>(),
        referenceTime
      );

  if ( ! bedElevation ) throw MDAL::Error( MDAL_Status::Err_InvalidData, "Unable to read bed elevation values" );

  return bedElevation;
}

void MDAL::DriverHec2D::readElemResults(
  const HdfFile &hdfFile,
  std::shared_ptr<MDAL::MemoryDataset2D> bed_elevation,
  const std::vector<size_t> &areaElemStartIndex,
  const std::vector<std::string> &flowAreaNames )
{
  // UNSTEADY
  HdfGroup flowGroup = get2DFlowAreasGroup( hdfFile, "Unsteady Time Series" );

  readElemOutput(
    flowGroup,
    areaElemStartIndex,
    flowAreaNames,
    "Water Surface",
    "Water Surface",
    mTimes,
    bed_elevation,
    mReferenceTime );
  readElemOutput(
    flowGroup,
    areaElemStartIndex,
    flowAreaNames,
    "Depth",
    "Depth",
    mTimes,
    bed_elevation,
    mReferenceTime );

  // SUMMARY
  flowGroup = get2DFlowAreasGroup( hdfFile, "Summary Output" );

  std::vector<RelativeTimestamp> dummyTimes( 1, MDAL::RelativeTimestamp() );

  readElemOutput(
    flowGroup,
    areaElemStartIndex,
    flowAreaNames,
    "Maximum Water Surface",
    "Water Surface/Maximums",
    dummyTimes,
    bed_elevation,
    mReferenceTime
  );
}

std::vector<std::string> MDAL::DriverHec2D::read2DFlowAreasNamesFromNameDataset( HdfGroup gGeom2DFlowAreas ) const
{
  bool ok = false;
  HdfDataset dsNames = openHdfDataset( gGeom2DFlowAreas, "Names", &ok );
  std::vector<std::string> names;
  if ( ok )
    names = dsNames.readArrayString();
  return names;
}

/**
 *  For 5.0.5+ format
 *
 *  DATATYPE  H5T_COMPOUND {
 *              H5T_STRING {
 *                 STRSIZE 16;
 *                 STRPAD H5T_STR_NULLTERM;
 *                 CSET H5T_CSET_ASCII;
 *                 CTYPE H5T_C_S1;
 *              } "Name";
 *              H5T_IEEE_F32LE "Mann";
 *              H5T_IEEE_F32LE "Cell Vol Tol";
 *              H5T_IEEE_F32LE "Cell Min Area Fraction";
 *              H5T_IEEE_F32LE "Face Profile Tol";
 *              H5T_IEEE_F32LE "Face Area Tol";
 *              H5T_IEEE_F32LE "Face Conv Ratio";
 *              H5T_IEEE_F32LE "Laminar Depth";
 *              H5T_IEEE_F32LE "Spacing dx";
 *              H5T_IEEE_F32LE "Spacing dy";
 *              H5T_IEEE_F32LE "Shift dx";
 *              H5T_IEEE_F32LE "Shift dy";
 *             H5T_STD_I32LE "Cell Count";
 * }
 */
typedef struct FlowAreasAttribute505
{
  char name[HDF_MAX_NAME];
  float mann;
  float cellVolTol;
  float cellMinAreaFraction;
  float faceProfileTol;
  float faceAreaTol;
  float faceConvRatio;
  float laminarDepth;
  float spacingDx;
  float spacingDy;
  float shifyDx;
  float shifyDy;
  int cellCount;
} FlowAreasAttribute505;


std::vector<std::string> MDAL::DriverHec2D::read2DFlowAreasNamesFromAttributesDataset( HdfGroup gGeom2DFlowAreas ) const
{
  bool ok = false;
  std::vector<std::string> names;
  HdfDataset dsAttributes = openHdfDataset( gGeom2DFlowAreas, "Attributes", &ok );
  if ( !ok )
    return names;
  hid_t attributeHID = H5Tcreate( H5T_COMPOUND, sizeof( FlowAreasAttribute505 ) );
  hid_t stringHID = H5Tcopy( H5T_C_S1 );
  H5Tset_size( stringHID, HDF_MAX_NAME );
  H5Tinsert( attributeHID, "Name", HOFFSET( FlowAreasAttribute505, name ), stringHID );
  H5Tinsert( attributeHID, "Mann", HOFFSET( FlowAreasAttribute505, mann ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Cell Vol Tol", HOFFSET( FlowAreasAttribute505, cellVolTol ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Cell Min Area Fraction", HOFFSET( FlowAreasAttribute505, cellMinAreaFraction ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Face Profile Tol", HOFFSET( FlowAreasAttribute505, faceProfileTol ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Face Area Tol", HOFFSET( FlowAreasAttribute505, faceAreaTol ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Face Conv Ratio", HOFFSET( FlowAreasAttribute505, faceConvRatio ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Laminar Depth", HOFFSET( FlowAreasAttribute505, laminarDepth ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Spacing dx", HOFFSET( FlowAreasAttribute505, spacingDx ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Spacing dy", HOFFSET( FlowAreasAttribute505, spacingDy ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Shift dx", HOFFSET( FlowAreasAttribute505, shifyDx ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Shift dy", HOFFSET( FlowAreasAttribute505, shifyDy ), H5T_NATIVE_FLOAT );
  H5Tinsert( attributeHID, "Cell Count", HOFFSET( FlowAreasAttribute505, cellCount ), H5T_NATIVE_INT );
  std::vector<FlowAreasAttribute505> attributes = dsAttributes.readArray<FlowAreasAttribute505>( attributeHID );
  H5Tclose( attributeHID );
  H5Tclose( stringHID );
  if ( attributes.empty() ) throw MDAL::Error( MDAL_Status::Err_InvalidData, "Unable to read 2D Flow Area Names, no attributes found" );

  for ( const auto &attr : attributes )
  {
    std::string dat = std::string( attr.name );
    names.push_back( MDAL::trim( dat ) );
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
  catch ( MDAL::Error ) { /* projection not set */}
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
      maxVerticesInFace,
      mFileName
    )
  );
  mMesh->setFaces( std::move( faces ) );
  mMesh->setVertices( std::move( vertices ) );
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

bool MDAL::DriverHec2D::canReadMesh( const std::string &uri )
{
  try
  {
    HdfFile hdfFile = openHdfFile( uri );
    std::string fileType = openHdfAttribute( hdfFile, "File Type" );
    return canReadFormat( fileType ) || canReadFormat505( fileType );
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  catch ( MDAL::Error )
  {
    return false;
  }
}

bool MDAL::DriverHec2D::canReadFormat( const std::string &fileType ) const
{
  return fileType == "HEC-RAS Results";
}

bool MDAL::DriverHec2D::canReadFormat505( const std::string &fileType ) const
{
  return fileType == "HEC-RAS Geometry";
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverHec2D::load( const std::string &resultsFile, const std::string & )
{
  mFileName = resultsFile;
  MDAL::Log::resetLastStatus();
  mMesh.reset();

  try
  {
    HdfFile hdfFile = openHdfFile( mFileName );

    HdfGroup gGeom = openHdfGroup( hdfFile, "Geometry" );
    HdfGroup gGeom2DFlowAreas = openHdfGroup( gGeom, "2D Flow Areas" );

    std::vector<std::string> flowAreaNames;

    flowAreaNames = read2DFlowAreasNamesFromNameDataset( gGeom2DFlowAreas );
    if ( flowAreaNames.empty() )
      flowAreaNames = read2DFlowAreasNamesFromAttributesDataset( gGeom2DFlowAreas );

    if ( flowAreaNames.empty() )
      throw MDAL::Error( MDAL_Status::Err_InvalidData, "Unable to read 2D Flow area names, no names found" );

    std::vector<size_t> areaElemStartIndex( flowAreaNames.size() + 1 );

    parseMesh( gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames );
    setProjection( hdfFile );

    mTimes = readTimes( hdfFile );
    mReferenceTime = readReferenceDateTime( hdfFile );

    //Elevation
    std::shared_ptr<MDAL::MemoryDataset2D> bed_elevation = readBedElevation( gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames );

    // Element centered Values
    readElemResults( hdfFile, bed_elevation, areaElemStartIndex, flowAreaNames );

    // Face centered Values
    readFaceResults( hdfFile, areaElemStartIndex, flowAreaNames );

  }
  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "Error occurred while loading file " + resultsFile );
    mMesh.reset();
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
    mMesh.reset();
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}
