/***************************************************************************
                             qgsgdalutils.cpp
                             ----------------
    begin                : September 2018
    copyright            : (C) 2018 Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalutils.h"

#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsrasterblock.h"
#include "qgssettings.h"

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include "gdal.h"
#include "gdalwarper.h"
#include "cpl_string.h"

#include <QNetworkProxy>
#include <QString>
#include <QImage>
#include <QFileInfo>
#include <mutex>


QgsGdalOption QgsGdalOption::fromXmlNode( const CPLXMLNode *node )
{
  if ( node->eType != CXT_Element || !EQUAL( node->pszValue, "Option" ) )
    return {};

  const QString optionName( CPLGetXMLValue( node, "name", nullptr ) );
  if ( optionName.isEmpty() )
    return {};

  QgsGdalOption option;
  option.name = optionName;

  option.description = QString( CPLGetXMLValue( node, "description", nullptr ) );
  option.scope = QString( CPLGetXMLValue( node, "scope", nullptr ) );

  option.type = QgsGdalOption::Type::Text;

  const char *pszType = CPLGetXMLValue( node, "type", nullptr );
  const char *pszDefault = CPLGetXMLValue( node, "default", nullptr );
  if ( pszType && EQUAL( pszType, "string-select" ) )
  {
    option.type = QgsGdalOption::Type::Select;
    for ( auto psOption = node->psChild; psOption != nullptr; psOption = psOption->psNext )
    {
      if ( psOption->eType != CXT_Element ||
           !EQUAL( psOption->pszValue, "Value" ) ||
           !psOption->psChild )
      {
        continue;
      }
      option.options << psOption->psChild->pszValue;
    }
    option.defaultValue = pszDefault ? QString( pszDefault ) : option.options.value( 0 );
    return option;
  }
  else if ( pszType && EQUAL( pszType, "boolean" ) )
  {
    option.type = QgsGdalOption::Type::Boolean;
    option.defaultValue = pszDefault ? QString( pszDefault ) : u"YES"_s;
    return option;
  }
  else if ( pszType && EQUAL( pszType, "string" ) )
  {
    option.type = QgsGdalOption::Type::Text;
    if ( pszDefault )
      option.defaultValue = QString( pszDefault );
    return option;
  }
  else if ( pszType && ( EQUAL( pszType, "int" ) || EQUAL( pszType, "integer" ) ) )
  {
    option.type = QgsGdalOption::Type::Int;
    if ( pszDefault )
    {
      bool ok = false;
      const int defaultInt = QString( pszDefault ).toInt( &ok );
      if ( ok )
        option.defaultValue = defaultInt;
    }

    if ( const char *pszMin = CPLGetXMLValue( node, "min", nullptr ) )
    {
      bool ok = false;
      const int minInt = QString( pszMin ).toInt( &ok );
      if ( ok )
        option.minimum = minInt;
    }
    if ( const char *pszMax = CPLGetXMLValue( node, "max", nullptr ) )
    {
      bool ok = false;
      const int maxInt = QString( pszMax ).toInt( &ok );
      if ( ok )
        option.maximum = maxInt;
    }
    return option;
  }
  else if ( pszType && ( EQUAL( pszType, "double" ) || EQUAL( pszType, "float" ) ) )
  {
    option.type = QgsGdalOption::Type::Double;
    if ( pszDefault )
    {
      bool ok = false;
      const double defaultDouble = QString( pszDefault ).toDouble( &ok );
      if ( ok )
        option.defaultValue = defaultDouble;
    }

    if ( const char *pszMin = CPLGetXMLValue( node, "min", nullptr ) )
    {
      bool ok = false;
      const double minDouble = QString( pszMin ).toDouble( &ok );
      if ( ok )
        option.minimum = minDouble;
    }
    if ( const char *pszMax = CPLGetXMLValue( node, "max", nullptr ) )
    {
      bool ok = false;
      const double maxDouble = QString( pszMax ).toDouble( &ok );
      if ( ok )
        option.maximum = maxDouble;
    }
    return option;
  }

  QgsDebugError( u"Unhandled GDAL option type: %1"_s.arg( pszType ) );
  return {};
}

QList<QgsGdalOption> QgsGdalOption::optionsFromXml( const CPLXMLNode *node )
{
  QList< QgsGdalOption > options;
  for ( auto psItem = node->psChild; psItem != nullptr; psItem = psItem->psNext )
  {
    const QgsGdalOption option = fromXmlNode( psItem );
    if ( option.type == QgsGdalOption::Type::Invalid )
      continue;

    options << option;
  }
  return options;
}


//
// QgsGdalUtils
//

bool QgsGdalUtils::supportsRasterCreate( GDALDriverH driver )
{
  const QString driverShortName = GDALGetDriverShortName( driver );
  if ( driverShortName == "SQLite"_L1 ||
       driverShortName == "PDF"_L1 )
  {
    // it supports Create() but only for vector side
    return false;
  }
  return GDALGetMetadataItem( driver, GDAL_DCAP_CREATE, nullptr ) &&
         GDALGetMetadataItem( driver, GDAL_DCAP_RASTER, nullptr );
}

gdal::dataset_unique_ptr QgsGdalUtils::createSingleBandMemoryDataset( GDALDataType dataType, const QgsRectangle &extent, int width, int height, const QgsCoordinateReferenceSystem &crs )
{
  return createMultiBandMemoryDataset( dataType, 1, extent, width, height, crs );
}

gdal::dataset_unique_ptr QgsGdalUtils::createMultiBandMemoryDataset( GDALDataType dataType, int bands, const QgsRectangle &extent, int width, int height, const QgsCoordinateReferenceSystem &crs )
{
  GDALDriverH hDriverMem = GDALGetDriverByName( "MEM" );
  if ( !hDriverMem )
  {
    return gdal::dataset_unique_ptr();
  }

  gdal::dataset_unique_ptr hSrcDS( GDALCreate( hDriverMem, "", width, height, bands, dataType, nullptr ) );

  const double cellSizeX = extent.width() / width;
  const double cellSizeY = extent.height() / height;
  double geoTransform[6];
  geoTransform[0] = extent.xMinimum();
  geoTransform[1] = cellSizeX;
  geoTransform[2] = 0;
  geoTransform[3] = extent.yMinimum() + ( cellSizeY * height );
  geoTransform[4] = 0;
  geoTransform[5] = -cellSizeY;

  GDALSetProjection( hSrcDS.get(), crs.toWkt( Qgis::CrsWktVariant::PreferredGdal ).toLatin1().constData() );
  GDALSetGeoTransform( hSrcDS.get(), geoTransform );
  return hSrcDS;
}

gdal::dataset_unique_ptr QgsGdalUtils::createSingleBandTiffDataset( const QString &filename, GDALDataType dataType, const QgsRectangle &extent, int width, int height, const QgsCoordinateReferenceSystem &crs )
{
  const double cellSizeX = extent.width() / width;
  const double cellSizeY = extent.height() / height;
  double geoTransform[6];
  geoTransform[0] = extent.xMinimum();
  geoTransform[1] = cellSizeX;
  geoTransform[2] = 0;
  geoTransform[3] = extent.yMinimum() + ( cellSizeY * height );
  geoTransform[4] = 0;
  geoTransform[5] = -cellSizeY;

  GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
  if ( !hDriver )
  {
    return gdal::dataset_unique_ptr();
  }

  // Create the output file.
  gdal::dataset_unique_ptr hDstDS( GDALCreate( hDriver, filename.toUtf8().constData(), width, height, 1, dataType, nullptr ) );
  if ( !hDstDS )
  {
    return gdal::dataset_unique_ptr();
  }

  // Write out the projection definition.
  GDALSetProjection( hDstDS.get(), crs.toWkt( Qgis::CrsWktVariant::PreferredGdal ).toLatin1().constData() );
  GDALSetGeoTransform( hDstDS.get(), geoTransform );
  return hDstDS;
}

gdal::dataset_unique_ptr QgsGdalUtils::imageToMemoryDataset( const QImage &image )
{
  if ( image.isNull() )
    return nullptr;

  const QRgb *rgb = reinterpret_cast<const QRgb *>( image.constBits() );
  GDALDriverH hDriverMem = GDALGetDriverByName( "MEM" );
  if ( !hDriverMem )
  {
    return nullptr;
  }
  gdal::dataset_unique_ptr hSrcDS( GDALCreate( hDriverMem, "",  image.width(), image.height(), 0, GDT_Byte, nullptr ) );

  char **papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                        << u"PIXELOFFSET=%1"_s.arg( sizeof( QRgb ) )
                        << u"LINEOFFSET=%1"_s.arg( image.bytesPerLine() )
                        << u"DATAPOINTER=%1"_s.arg( reinterpret_cast< qulonglong >( rgb ) + 2 ) );
  GDALAddBand( hSrcDS.get(), GDT_Byte, papszOptions );
  CSLDestroy( papszOptions );

  papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                 << u"PIXELOFFSET=%1"_s.arg( sizeof( QRgb ) )
                 << u"LINEOFFSET=%1"_s.arg( image.bytesPerLine() )
                 << u"DATAPOINTER=%1"_s.arg( reinterpret_cast< qulonglong >( rgb ) + 1 ) );
  GDALAddBand( hSrcDS.get(), GDT_Byte, papszOptions );
  CSLDestroy( papszOptions );

  papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                 << u"PIXELOFFSET=%1"_s.arg( sizeof( QRgb ) )
                 << u"LINEOFFSET=%1"_s.arg( image.bytesPerLine() )
                 << u"DATAPOINTER=%1"_s.arg( reinterpret_cast< qulonglong >( rgb ) ) );
  GDALAddBand( hSrcDS.get(), GDT_Byte, papszOptions );
  CSLDestroy( papszOptions );

  papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                 << u"PIXELOFFSET=%1"_s.arg( sizeof( QRgb ) )
                 << u"LINEOFFSET=%1"_s.arg( image.bytesPerLine() )
                 << u"DATAPOINTER=%1"_s.arg( reinterpret_cast< qulonglong >( rgb ) + 3 ) );
  GDALAddBand( hSrcDS.get(), GDT_Byte, papszOptions );
  CSLDestroy( papszOptions );

  return hSrcDS;
}

gdal::dataset_unique_ptr QgsGdalUtils::blockToSingleBandMemoryDataset( int pixelWidth, int pixelHeight, const QgsRectangle &extent, void *block,  GDALDataType dataType )
{
  if ( !block )
    return nullptr;

  GDALDriverH hDriverMem = GDALGetDriverByName( "MEM" );
  if ( !hDriverMem )
    return nullptr;

  const double cellSizeX = extent.width() / pixelWidth;
  const double cellSizeY = extent.height() / pixelHeight;
  double geoTransform[6];
  geoTransform[0] = extent.xMinimum();
  geoTransform[1] = cellSizeX;
  geoTransform[2] = 0;
  geoTransform[3] = extent.yMinimum() + ( cellSizeY * pixelHeight );
  geoTransform[4] = 0;
  geoTransform[5] = -cellSizeY;

  gdal::dataset_unique_ptr hDstDS( GDALCreate( hDriverMem, "", pixelWidth, pixelHeight, 0, dataType, nullptr ) );

  int dataTypeSize = GDALGetDataTypeSizeBytes( dataType );
  char **papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                        << u"PIXELOFFSET=%1"_s.arg( dataTypeSize )
                        << u"LINEOFFSET=%1"_s.arg( pixelWidth * dataTypeSize )
                        << u"DATAPOINTER=%1"_s.arg( reinterpret_cast< qulonglong >( block ) ) );
  GDALAddBand( hDstDS.get(), dataType, papszOptions );
  CSLDestroy( papszOptions );

  GDALSetGeoTransform( hDstDS.get(), geoTransform );

  return hDstDS;
}

gdal::dataset_unique_ptr QgsGdalUtils::blockToSingleBandMemoryDataset( const QgsRectangle &extent, QgsRasterBlock *block )
{
  if ( !block )
    return nullptr;

  gdal::dataset_unique_ptr ret = blockToSingleBandMemoryDataset( block->width(), block->height(), extent, block->bits(), gdalDataTypeFromQgisDataType( block->dataType() ) );
  if ( ret )
  {
    GDALRasterBandH band = GDALGetRasterBand( ret.get(), 1 );
    if ( band )
      GDALSetRasterNoDataValue( band, block->noDataValue() );
  }

  return ret;
}



gdal::dataset_unique_ptr QgsGdalUtils::blockToSingleBandMemoryDataset( double rotation,
    const QgsPointXY &origin,
    double gridXSize,
    double gridYSize,
    QgsRasterBlock *block )
{
  if ( !block )
    return nullptr;

  GDALDriverH hDriverMem = GDALGetDriverByName( "MEM" );
  if ( !hDriverMem )
    return nullptr;

  const double cellSizeX = gridXSize / block->width();
  const double cellSizeY = gridYSize / block->height();
  double geoTransform[6];
  geoTransform[0] = origin.x();
  geoTransform[1] = cellSizeX * std::cos( rotation );
  geoTransform[2] = cellSizeY * std::sin( rotation );
  geoTransform[3] = origin.y();
  geoTransform[4] = cellSizeX * std::sin( rotation );
  geoTransform[5] = -cellSizeY * std::cos( rotation );

  GDALDataType dataType = gdalDataTypeFromQgisDataType( block->dataType() );
  gdal::dataset_unique_ptr hDstDS( GDALCreate( hDriverMem, "", block->width(), block->height(), 0, dataType, nullptr ) );

  int dataTypeSize = GDALGetDataTypeSizeBytes( dataType );
  char **papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                        << u"PIXELOFFSET=%1"_s.arg( dataTypeSize )
                        << u"LINEOFFSET=%1"_s.arg( block->width() * dataTypeSize )
                        << u"DATAPOINTER=%1"_s.arg( reinterpret_cast< qulonglong >( block->bits() ) ) );
  GDALAddBand( hDstDS.get(), dataType, papszOptions );
  CSLDestroy( papszOptions );

  GDALSetGeoTransform( hDstDS.get(), geoTransform );

  GDALRasterBandH band = GDALGetRasterBand( hDstDS.get(), 1 );
  if ( band )
    GDALSetRasterNoDataValue( band, block->noDataValue() );

  return hDstDS;
}

static bool resampleSingleBandRasterStatic( GDALDatasetH hSrcDS, GDALDatasetH hDstDS, GDALResampleAlg resampleAlg, char **papszOptions )
{
  gdal::warp_options_unique_ptr psWarpOptions( GDALCreateWarpOptions() );
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;

  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands = reinterpret_cast< int * >( CPLMalloc( sizeof( int ) * 1 ) );
  psWarpOptions->panDstBands = reinterpret_cast< int * >( CPLMalloc( sizeof( int ) * 1 ) );
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands[0] = 1;
  double noDataValue = GDALGetRasterNoDataValue( GDALGetRasterBand( hDstDS, 1 ), nullptr );
  psWarpOptions->padfDstNoDataReal = reinterpret_cast< double * >( CPLMalloc( sizeof( double ) * 1 ) );
  psWarpOptions->padfDstNoDataReal[0] = noDataValue;
  psWarpOptions->padfSrcNoDataReal = reinterpret_cast< double * >( CPLMalloc( sizeof( double ) * 1 ) );
  psWarpOptions->padfSrcNoDataReal[0] = noDataValue;
  psWarpOptions->eResampleAlg = resampleAlg;

  // Establish reprojection transformer.
  psWarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer2( hSrcDS, hDstDS, papszOptions );

  if ( ! psWarpOptions->pTransformerArg )
  {
    return false;
  }

  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
  psWarpOptions->papszWarpOptions = CSLSetNameValue( psWarpOptions-> papszWarpOptions, "INIT_DEST", "NO_DATA" );

  // Initialize and execute the warp operation.
  bool retVal = false;
  GDALWarpOperation oOperation;
  CPLErr initResult = oOperation.Initialize( psWarpOptions.get() );
  if ( initResult != CE_Failure )
    retVal =  oOperation.ChunkAndWarpImage( 0, 0, GDALGetRasterXSize( hDstDS ), GDALGetRasterYSize( hDstDS ) ) == CE_None;
  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  return retVal;
}

bool QgsGdalUtils::resampleSingleBandRaster( GDALDatasetH hSrcDS, GDALDatasetH hDstDS, GDALResampleAlg resampleAlg, const char *pszCoordinateOperation )
{
  char **papszOptions = nullptr;
  if ( pszCoordinateOperation )
    papszOptions = CSLSetNameValue( papszOptions, "COORDINATE_OPERATION", pszCoordinateOperation );

  bool result = resampleSingleBandRasterStatic( hSrcDS, hDstDS, resampleAlg, papszOptions );
  CSLDestroy( papszOptions );
  return result;
}

bool QgsGdalUtils::resampleSingleBandRaster( GDALDatasetH hSrcDS,
    GDALDatasetH hDstDS,
    GDALResampleAlg resampleAlg,
    const QgsCoordinateReferenceSystem &sourceCrs,
    const QgsCoordinateReferenceSystem &destinationCrs )
{
  char **papszOptions = nullptr;

  papszOptions = CSLSetNameValue( papszOptions, "SRC_SRS", sourceCrs.toWkt( Qgis::CrsWktVariant::PreferredGdal ).toUtf8().constData() );
  papszOptions = CSLSetNameValue( papszOptions, "DST_SRS", destinationCrs.toWkt( Qgis::CrsWktVariant::PreferredGdal ).toUtf8().constData() );

  bool result = resampleSingleBandRasterStatic( hSrcDS, hDstDS, resampleAlg, papszOptions );
  CSLDestroy( papszOptions );
  return result;
}

QImage QgsGdalUtils::resampleImage( const QImage &image, QSize outputSize, GDALRIOResampleAlg resampleAlg )
{
  const gdal::dataset_unique_ptr srcDS = QgsGdalUtils::imageToMemoryDataset( image );
  if ( !srcDS )
    return QImage();

  GDALRasterIOExtraArg extra;
  INIT_RASTERIO_EXTRA_ARG( extra );
  extra.eResampleAlg = resampleAlg;

  QImage res( outputSize, image.format() );
  if ( res.isNull() )
    return QImage();

  GByte *rgb = reinterpret_cast<GByte *>( res.bits() );

  CPLErr err = GDALRasterIOEx( GDALGetRasterBand( srcDS.get(), 1 ), GF_Read, 0, 0, image.width(), image.height(), rgb + 2, outputSize.width(),
                               outputSize.height(), GDT_Byte, sizeof( QRgb ), res.bytesPerLine(), &extra );
  if ( err != CE_None )
  {
    QgsDebugError( u"failed to read red band"_s );
    return QImage();
  }

  err = GDALRasterIOEx( GDALGetRasterBand( srcDS.get(), 2 ), GF_Read, 0, 0, image.width(), image.height(), rgb + 1, outputSize.width(),
                        outputSize.height(), GDT_Byte, sizeof( QRgb ), res.bytesPerLine(), &extra );
  if ( err != CE_None )
  {
    QgsDebugError( u"failed to read green band"_s );
    return QImage();
  }

  err = GDALRasterIOEx( GDALGetRasterBand( srcDS.get(), 3 ), GF_Read, 0, 0, image.width(), image.height(), rgb, outputSize.width(),
                        outputSize.height(), GDT_Byte, sizeof( QRgb ), res.bytesPerLine(), &extra );
  if ( err != CE_None )
  {
    QgsDebugError( u"failed to read blue band"_s );
    return QImage();
  }

  err = GDALRasterIOEx( GDALGetRasterBand( srcDS.get(), 4 ), GF_Read, 0, 0, image.width(), image.height(), rgb + 3, outputSize.width(),
                        outputSize.height(), GDT_Byte, sizeof( QRgb ), res.bytesPerLine(), &extra );
  if ( err != CE_None )
  {
    QgsDebugError( u"failed to read alpha band"_s );
    return QImage();
  }

  return res;
}

QString QgsGdalUtils::helpCreationOptionsFormat( const QString &format )
{
  QString message;
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  if ( myGdalDriver )
  {
    // first report details and help page
    char **GDALmetadata = GDALGetMetadata( myGdalDriver, nullptr );
    message += "Format Details:\n"_L1;
    message += u"  Extension: %1\n"_s.arg( CSLFetchNameValue( GDALmetadata, GDAL_DMD_EXTENSION ) );
    message += u"  Short Name: %1"_s.arg( GDALGetDriverShortName( myGdalDriver ) );
    message += u"  /  Long Name: %1\n"_s.arg( GDALGetDriverLongName( myGdalDriver ) );
    const QString helpUrl = gdalDocumentationUrlForDriver( myGdalDriver );
    if ( !helpUrl.isEmpty() )
      message += u"  Help page:  %1\n\n"_s.arg( helpUrl );

    // next get creation options
    // need to serialize xml to get newlines, should we make the basic xml prettier?
    CPLXMLNode *psCOL = CPLParseXMLString( GDALGetMetadataItem( myGdalDriver,
                                           GDAL_DMD_CREATIONOPTIONLIST, "" ) );
    char *pszFormattedXML = CPLSerializeXMLTree( psCOL );
    if ( pszFormattedXML )
      message += QString( pszFormattedXML );
    if ( psCOL )
      CPLDestroyXMLNode( psCOL );
    if ( pszFormattedXML )
      CPLFree( pszFormattedXML );
  }
  return message;
}

char **QgsGdalUtils::papszFromStringList( const QStringList &list )
{
  char **papszRetList = nullptr;
  const auto constList = list;
  for ( const QString &elem : constList )
  {
    papszRetList = CSLAddString( papszRetList, elem.toLocal8Bit().constData() );
  }
  return papszRetList;
}

QString QgsGdalUtils::validateCreationOptionsFormat( const QStringList &creationOptions, const QString &format )
{
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  if ( ! myGdalDriver )
    return u"invalid GDAL driver"_s;

  char **papszOptions = papszFromStringList( creationOptions );
  // get error string?
  const int ok = GDALValidateCreationOptions( myGdalDriver, papszOptions );
  CSLDestroy( papszOptions );

  if ( !ok )
    return u"Failed GDALValidateCreationOptions() test"_s;
  return QString();
}

static void setRPCTransformerOptions( GDALDatasetH hSrcDS, char  ***opts )
{
  if ( GDALGetMetadata( hSrcDS, "RPC" ) )
  {
    // Some RPC may contain a HEIGHT_DEFAULT entry, that must be used as the
    // best default for RPC_HEIGHT. See https://github.com/OSGeo/gdal/pull/11989
    const char *heightStr = GDALGetMetadataItem( hSrcDS, "HEIGHT_DEFAULT", "RPC" );
    if ( !heightStr )
    {
      // Otherwise well-behaved RPC should have height offset which is also
      // a resaonable default for RPC_HEIGHT.
      heightStr = GDALGetMetadataItem( hSrcDS, "HEIGHT_OFF", "RPC" );
    }
    if ( heightStr )
      *opts = CSLAddNameValue( *opts, "RPC_HEIGHT", heightStr );
  }
}

GDALDatasetH QgsGdalUtils::rpcAwareAutoCreateWarpedVrt(
  GDALDatasetH hSrcDS,
  const char *pszSrcWKT,
  const char *pszDstWKT,
  GDALResampleAlg eResampleAlg,
  double dfMaxError,
  const GDALWarpOptions *psOptionsIn )
{
  char **opts = nullptr;
  setRPCTransformerOptions( hSrcDS, &opts );
  GDALDatasetH hRetDS = GDALAutoCreateWarpedVRTEx( hSrcDS, pszSrcWKT, pszDstWKT, eResampleAlg, dfMaxError, psOptionsIn, opts );
  CSLDestroy( opts );
  return hRetDS;
}

void *QgsGdalUtils::rpcAwareCreateTransformer( GDALDatasetH hSrcDS, GDALDatasetH hDstDS, char **papszOptions )
{
  char **opts = CSLDuplicate( papszOptions );
  setRPCTransformerOptions( hSrcDS, &opts );
  void *transformer = GDALCreateGenImgProjTransformer2( hSrcDS, hDstDS, opts );
  CSLDestroy( opts );
  return transformer;
}

GDALDataType QgsGdalUtils::gdalDataTypeFromQgisDataType( Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::DataType::UnknownDataType:
      return GDALDataType::GDT_Unknown;
      break;
    case Qgis::DataType::Byte:
      return GDALDataType::GDT_Byte;
      break;
    case Qgis::DataType::Int8:
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
      return GDALDataType::GDT_Int8;
#else
      return GDALDataType::GDT_Unknown;
#endif
      break;
    case Qgis::DataType::UInt16:
      return GDALDataType::GDT_UInt16;
      break;
    case Qgis::DataType::Int16:
      return GDALDataType::GDT_Int16;
      break;
    case Qgis::DataType::UInt32:
      return GDALDataType::GDT_UInt32;
      break;
    case Qgis::DataType::Int32:
      return GDALDataType::GDT_Int32;
      break;
    case Qgis::DataType::Float32:
      return GDALDataType::GDT_Float32;
      break;
    case Qgis::DataType::Float64:
      return GDALDataType::GDT_Float64;
      break;
    case Qgis::DataType::CInt16:
      return GDALDataType::GDT_CInt16;
      break;
    case Qgis::DataType::CInt32:
      return GDALDataType::GDT_CInt32;
      break;
    case Qgis::DataType::CFloat32:
      return GDALDataType::GDT_CFloat32;
      break;
    case Qgis::DataType::CFloat64:
      return GDALDataType::GDT_CFloat64;
      break;
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
      return GDALDataType::GDT_Unknown;
      break;
  };

  return GDALDataType::GDT_Unknown;
}

GDALResampleAlg QgsGdalUtils::gdalResamplingAlgorithm( Qgis::RasterResamplingMethod method )
{
  GDALResampleAlg eResampleAlg = GRA_NearestNeighbour;
  switch ( method )
  {
    case Qgis::RasterResamplingMethod::Nearest:
    case Qgis::RasterResamplingMethod::Gauss: // Gauss not available in GDALResampleAlg
      eResampleAlg = GRA_NearestNeighbour;
      break;

    case Qgis::RasterResamplingMethod::Bilinear:
      eResampleAlg = GRA_Bilinear;
      break;

    case Qgis::RasterResamplingMethod::Cubic:
      eResampleAlg = GRA_Cubic;
      break;

    case Qgis::RasterResamplingMethod::CubicSpline:
      eResampleAlg = GRA_CubicSpline;
      break;

    case Qgis::RasterResamplingMethod::Lanczos:
      eResampleAlg = GRA_Lanczos;
      break;

    case Qgis::RasterResamplingMethod::Average:
      eResampleAlg = GRA_Average;
      break;

    case Qgis::RasterResamplingMethod::Mode:
      eResampleAlg = GRA_Mode;
      break;
  }

  return eResampleAlg;
}

#ifndef QT_NO_NETWORKPROXY
void QgsGdalUtils::setupProxy()
{
  // Check proxy configuration, they are application level but
  // instead of adding an API and complex signal/slot connections
  // given the limited cost of checking them on every provider instantiation
  // we can do it here so that new settings are applied whenever a new layer
  // is created.
  const QgsSettings settings;
  // Check that proxy is enabled
  if ( settings.value( u"proxy/proxyEnabled"_s, false ).toBool() )
  {
    // Get the first configured proxy
    QList<QNetworkProxy> proxies( QgsNetworkAccessManager::instance()->proxyFactory()->queryProxy( ) );
    if ( ! proxies.isEmpty() )
    {
      const QNetworkProxy proxy( proxies.first() );
      // TODO/FIXME: check excludes (the GDAL config options are global, we need a per-connection config option)
      //QStringList excludes;
      //excludes = settings.value( u"proxy/proxyExcludedUrls"_s, "" ).toStringList();

      const QString proxyHost( proxy.hostName() );
      const quint16 proxyPort( proxy.port() );

      const QString proxyUser( proxy.user() );
      const QString proxyPassword( proxy.password() );

      if ( ! proxyHost.isEmpty() )
      {
        QString connection( proxyHost );
        if ( proxyPort )
        {
          connection += ':' +  QString::number( proxyPort );
        }
        CPLSetConfigOption( "GDAL_HTTP_PROXY", connection.toUtf8() );
        if ( !  proxyUser.isEmpty( ) )
        {
          QString credentials( proxyUser );
          if ( !  proxyPassword.isEmpty( ) )
          {
            credentials += ':' + proxyPassword;
          }
          CPLSetConfigOption( "GDAL_HTTP_PROXYUSERPWD", credentials.toUtf8() );
        }
      }
    }
  }
}

bool QgsGdalUtils::pathIsCheapToOpen( const QString &path, int smallFileSizeLimit )
{
  const QFileInfo info( path );
  const long long size = info.size();

  // if size could not be determined, safest to flag path as expensive
  if ( size == 0 )
    return false;

  const QString suffix = info.suffix().toLower();
  static const QStringList sFileSizeDependentExtensions
  {
    u"xlsx"_s,
    u"ods"_s,
    u"csv"_s
  };
  if ( sFileSizeDependentExtensions.contains( suffix ) )
  {
    // path corresponds to a file type which is only cheap to open for small files
    return size < smallFileSizeLimit;
  }

  // treat all other formats as expensive.
  // TODO -- flag formats which only require a quick header parse as cheap
  return false;
}

QStringList QgsGdalUtils::multiLayerFileExtensions()
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  // get supported extensions
  static std::once_flag initialized;
  static QStringList SUPPORTED_DB_LAYERS_EXTENSIONS;
  std::call_once( initialized, []
  {
    // iterate through all of the supported drivers, adding the corresponding file extensions for
    // types which advertise multilayer support
    GDALDriverH driver = nullptr;

    QSet< QString > extensions;

    for ( int i = 0; i < GDALGetDriverCount(); ++i )
    {
      driver = GDALGetDriver( i );
      if ( !driver )
      {
        QgsLogger::warning( "unable to get driver " + QString::number( i ) );
        continue;
      }

      bool isMultiLayer = false;
      if ( QString( GDALGetMetadataItem( driver, GDAL_DCAP_RASTER, nullptr ) ) == "YES"_L1 )
      {
        if ( GDALGetMetadataItem( driver, GDAL_DMD_SUBDATASETS, nullptr ) )
        {
          isMultiLayer = true;
        }
      }
      if ( !isMultiLayer && QString( GDALGetMetadataItem( driver, GDAL_DCAP_VECTOR, nullptr ) ) == "YES"_L1 )
      {
        if ( GDALGetMetadataItem( driver, GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, nullptr ) )
        {
          isMultiLayer = true;
        }
      }

      if ( !isMultiLayer )
        continue;

      const QString driverExtensions = GDALGetMetadataItem( driver, GDAL_DMD_EXTENSIONS, "" );
      if ( driverExtensions.isEmpty() )
        continue;

      const QStringList splitExtensions = driverExtensions.split( ' ', Qt::SkipEmptyParts );

      for ( const QString &ext : splitExtensions )
      {
        // maintain older behavior -- don't always expose tiff files as containers
        if ( ext == "tif"_L1 || ext == "tiff"_L1 )
          continue;

        extensions.insert( ext );
      }
    }

    SUPPORTED_DB_LAYERS_EXTENSIONS = QStringList( extensions.constBegin(), extensions.constEnd() );
  } );
  return SUPPORTED_DB_LAYERS_EXTENSIONS;

#else
  static const QStringList SUPPORTED_DB_LAYERS_EXTENSIONS
  {
    u"gpkg"_s,
    u"sqlite"_s,
    u"db"_s,
    u"gdb"_s,
    u"kml"_s,
    u"kmz"_s,
    u"osm"_s,
    u"mdb"_s,
    u"accdb"_s,
    u"xls"_s,
    u"xlsx"_s,
    u"ods"_s,
    u"gpx"_s,
    u"pdf"_s,
    u"pbf"_s,
    u"vrt"_s,
    u"nc"_s,
    u"dxf"_s,
    u"shp.zip"_s,
    u"gdb.zip"_s };
  return SUPPORTED_DB_LAYERS_EXTENSIONS;
#endif
}

QString QgsGdalUtils::vsiPrefixForPath( const QString &path )
{
  const QStringList vsiPrefixes = QgsGdalUtils::vsiArchivePrefixes();

  const thread_local QRegularExpression vsiRx( u"^(/vsi.+?/)"_s, QRegularExpression::PatternOption::CaseInsensitiveOption );
  const QRegularExpressionMatch vsiMatch = vsiRx.match( path );
  if ( vsiMatch.hasMatch() )
    return vsiMatch.captured( 1 );

  if ( path.endsWith( ".shp.zip"_L1, Qt::CaseInsensitive ) ||
       path.endsWith( ".gdb.zip"_L1, Qt::CaseInsensitive ) )
  {
    // GDAL 3.1 Shapefile/OpenFileGDB drivers directly handle .shp.zip and .gdb.zip files
    if ( GDALIdentifyDriverEx( path.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr ) )
      return QString();
    return u"/vsizip/"_s;
  }
  else if ( path.endsWith( ".zip"_L1, Qt::CaseInsensitive ) )
  {
    // GTFS driver directly handles .zip files
    const char *const apszAllowedDrivers[] = { "GTFS", nullptr };
    if ( GDALIdentifyDriverEx( path.toUtf8().constData(), GDAL_OF_VECTOR, apszAllowedDrivers, nullptr ) )
      return QString();
    return u"/vsizip/"_s;
  }
  else if ( path.endsWith( ".tar"_L1, Qt::CaseInsensitive ) ||
            path.endsWith( ".tar.gz"_L1, Qt::CaseInsensitive ) ||
            path.endsWith( ".tgz"_L1, Qt::CaseInsensitive ) )
    return u"/vsitar/"_s;
  else if ( path.endsWith( ".gz"_L1, Qt::CaseInsensitive ) )
    return u"/vsigzip/"_s;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  else if ( vsiPrefixes.contains( u"/vsi7z/"_s ) &&
            ( path.endsWith( ".7z"_L1, Qt::CaseInsensitive ) ||
              path.endsWith( ".lpk"_L1, Qt::CaseInsensitive ) ||
              path.endsWith( ".lpkx"_L1, Qt::CaseInsensitive ) ||
              path.endsWith( ".mpk"_L1, Qt::CaseInsensitive ) ||
              path.endsWith( ".mpkx"_L1, Qt::CaseInsensitive ) ) )
    return u"/vsi7z/"_s;
  else if ( vsiPrefixes.contains( u"/vsirar/"_s ) &&
            path.endsWith( ".rar"_L1, Qt::CaseInsensitive ) )
    return u"/vsirar/"_s;
#endif

  return QString();
}

QStringList QgsGdalUtils::vsiArchivePrefixes()
{
  QStringList res { u"/vsizip/"_s,
                    u"/vsitar/"_s,
                    u"/vsigzip/"_s,
                  };
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  res.append( u"/vsi7z/"_s );
  res.append( u"/vsirar/"_s );
#endif
  return res;
}

QList<QgsGdalUtils::VsiNetworkFileSystemDetails> QgsGdalUtils::vsiNetworkFileSystems()
{
  // get supported extensions
  static std::once_flag initialized;
  static QList<QgsGdalUtils::VsiNetworkFileSystemDetails> VSI_FILE_SYSTEM_DETAILS;
  std::call_once( initialized, []
  {
    if ( char **papszPrefixes = VSIGetFileSystemsPrefixes() )
    {
      for ( int i = 0; papszPrefixes[i]; i++ )
      {
        QgsGdalUtils::VsiNetworkFileSystemDetails details;
        details.identifier = QString( papszPrefixes[i] );
        if ( details.identifier.startsWith( '/' ) )
          details.identifier = details.identifier.mid( 1 );
        if ( details.identifier.endsWith( '/' ) )
          details.identifier.chop( 1 );

        if ( details.identifier == "vsicurl"_L1 )
          details.name = QObject::tr( "HTTP/HTTPS/FTP" );
        else if ( details.identifier == "vsis3"_L1 )
          details.name = QObject::tr( "AWS S3" );
        else if ( details.identifier == "vsigs"_L1 )
          details.name = QObject::tr( "Google Cloud Storage" );
        else if ( details.identifier == "vsiaz"_L1 )
          details.name = QObject::tr( "Microsoft Azure Blob" );
        else if ( details.identifier == "vsiadls"_L1 )
          details.name = QObject::tr( "Microsoft Azure Data Lake Storage" );
        else if ( details.identifier == "vsioss"_L1 )
          details.name = QObject::tr( "Alibaba Cloud OSS" );
        else if ( details.identifier == "vsiswift"_L1 )
          details.name = QObject::tr( "OpenStack Swift Object Storage" );
        else if ( details.identifier == "vsihdfs"_L1 )
          details.name = QObject::tr( "Hadoop File System" );
        else
          continue;
        VSI_FILE_SYSTEM_DETAILS.append( details );
      }

      CSLDestroy( papszPrefixes );
    }
  } );

  return VSI_FILE_SYSTEM_DETAILS;
}

bool QgsGdalUtils::isVsiArchivePrefix( const QString &prefix )
{
  const QStringList prefixes = vsiArchivePrefixes();
  for ( const QString &archivePrefix : prefixes )
  {
    // catch chained prefixes, eg "/vsizip/vsicurl"
    if ( prefix.contains( archivePrefix ) )
      return true;
  }
  return false;
}

QStringList QgsGdalUtils::vsiArchiveFileExtensions()
{
  QStringList res { u".zip"_s,
                    u".tar"_s,
                    u".tar.gz"_s,
                    u".tgz"_s,
                    u".gz"_s,
                  };
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  res.append( { u".7z"_s,
                u".lpk"_s,
                u".lpkx"_s,
                u".mpk"_s,
                u".mpkx"_s,
                u".rar"_s
              } );
#endif
  return res;
}

bool QgsGdalUtils::isVsiArchiveFileExtension( const QString &extension )
{
  const QString extWithDot = extension.startsWith( '.' ) ? extension : ( '.' + extension );
  return vsiArchiveFileExtensions().contains( extWithDot.toLower() );
}

Qgis::VsiHandlerType QgsGdalUtils::vsiHandlerType( const QString &prefix )
{
  if ( prefix.isEmpty() )
    return Qgis::VsiHandlerType::Invalid;

  QString vsiPrefix = prefix;
  if ( vsiPrefix.startsWith( '/' ) )
    vsiPrefix = vsiPrefix.mid( 1 );
  if ( vsiPrefix.endsWith( '/' ) )
    vsiPrefix.chop( 1 );

  if ( !vsiPrefix.startsWith( "vsi"_L1 ) )
    return Qgis::VsiHandlerType::Invalid;

  if ( vsiPrefix == "vsizip"_L1 ||
       vsiPrefix == "vsigzip"_L1 ||
       vsiPrefix == "vsitar"_L1 ||
       vsiPrefix == "vsi7z"_L1 ||
       vsiPrefix == "vsirar"_L1 )
    return Qgis::VsiHandlerType::Archive;

  else if ( vsiPrefix == "vsicurl"_L1 ||
            vsiPrefix == "vsicurl_streaming"_L1 )
    return Qgis::VsiHandlerType::Network;

  else if ( vsiPrefix == "vsis3"_L1 ||
            vsiPrefix == "vsicurl_streaming"_L1 ||
            vsiPrefix == "vsigs"_L1 ||
            vsiPrefix == "vsigs_streaming"_L1 ||
            vsiPrefix == "vsiaz"_L1 ||
            vsiPrefix == "vsiaz_streaming"_L1 ||
            vsiPrefix == "vsiadls"_L1 ||
            vsiPrefix == "vsioss"_L1 ||
            vsiPrefix == "vsioss_streaming"_L1 ||
            vsiPrefix == "vsiswift"_L1 ||
            vsiPrefix == "vsiswift_streaming"_L1 ||
            vsiPrefix == "vsihdfs"_L1 ||
            vsiPrefix == "vsiwebhdfs"_L1 )
    return Qgis::VsiHandlerType::Cloud;

  else if ( vsiPrefix == "vsimem"_L1 )
    return Qgis::VsiHandlerType::Memory;

  return Qgis::VsiHandlerType::Other;
}

bool QgsGdalUtils::vrtMatchesLayerType( const QString &vrtPath, Qgis::LayerType type )
{
  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  GDALDriverH hDriver = nullptr;

  switch ( type )
  {
    case Qgis::LayerType::Vector:
      hDriver = GDALIdentifyDriverEx( vrtPath.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
      break;

    case Qgis::LayerType::Raster:
      hDriver = GDALIdentifyDriverEx( vrtPath.toUtf8().constData(), GDAL_OF_RASTER, nullptr, nullptr );
      break;

    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }

  CPLPopErrorHandler();
  return static_cast< bool >( hDriver );
}

QString QgsGdalUtils::gdalDocumentationUrlForDriver( GDALDriverH hDriver )
{
  if ( hDriver )
  {
    const QString gdalDriverHelpTopic = GDALGetMetadataItem( hDriver, GDAL_DMD_HELPTOPIC, nullptr );  // e.g. "drivers/vector/ili.html"
    if ( !gdalDriverHelpTopic.isEmpty() )
      return u"https://gdal.org/%1"_s.arg( gdalDriverHelpTopic );
  }
  return QString();
}

bool QgsGdalUtils::applyVsiCredentialOptions( const QString &prefix, const QString &path, const QVariantMap &options )
{
  QString vsiPrefix = prefix;
  if ( !vsiPrefix.startsWith( '/' ) )
    vsiPrefix.prepend( '/' );
  if ( !vsiPrefix.endsWith( '/' ) )
    vsiPrefix.append( '/' );

  QString vsiPath = path;
  if ( vsiPath.endsWith( '/' ) )
    vsiPath.chop( 1 );

  const QString bucket = vsiPrefix + vsiPath;

  for ( auto it = options.constBegin(); it != options.constEnd(); ++it )
  {
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 6, 0)
    VSISetPathSpecificOption( bucket.toUtf8().constData(), it.key().toUtf8().constData(), it.value().toString().toUtf8().constData() );
#elif GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 5, 0)
    VSISetCredential( bucket.toUtf8().constData(), it.key().toUtf8().constData(), it.value().toString().toUtf8().constData() );
#else
    ( void )bucket;
    QgsMessageLog::logMessage( QObject::tr( "Cannot use VSI credential options on GDAL versions earlier than 3.5" ), u"GDAL"_s, Qgis::MessageLevel::Critical );
    return false;
#endif
  }
  return true;
}
#endif

QgsGdalProgressAdapter::QgsGdalProgressAdapter( QgsFeedback *feedback, double startPercentage, double endPercentage ): mFeedback( feedback ), mStartPercentage( startPercentage ), mEndPercentage( endPercentage )
{
}

/* static */
int CPL_STDCALL QgsGdalProgressAdapter::progressCallback( double dfComplete,
    const char * /* pszMessage */,
    void *pProgressArg )
{
  const QgsGdalProgressAdapter *adapter = static_cast<QgsGdalProgressAdapter *>( pProgressArg );

  if ( QgsFeedback *feedback = adapter->mFeedback )
  {
    feedback->setProgress( adapter->mStartPercentage +
                           ( adapter->mEndPercentage - adapter->mStartPercentage ) * dfComplete );

    if ( feedback->isCanceled() )
      return false;
  }

  return true;
}
