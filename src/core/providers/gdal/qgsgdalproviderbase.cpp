/***************************************************************************
  qgsgdalproviderbase.cpp  - Common base class for GDAL and WCS provider
                             -------------------
    begin                : November, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalproviderbase.h"
///@cond PRIVATE

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include <cpl_conv.h>
#include <cpl_string.h>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsgdalproviderbase.h"
#include "qgssettings.h"

#include <mutex>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QgsGdalProviderBase::QgsGdalProviderBase()
{

  // first get the GDAL driver manager
  QgsGdalProviderBase::registerGdalDrivers();
}

/**
 * \param bandNumber the number of the band for which you want a color table
 * \param list a pointer the object that will hold the color table
 * \return TRUE of a color table was able to be read, FALSE otherwise
 */
QList<QgsColorRampShader::ColorRampItem> QgsGdalProviderBase::colorTable( GDALDatasetH gdalDataset, int bandNumber )const
{
  QList<QgsColorRampShader::ColorRampItem> ct;

  //Invalid band number, segfault prevention
  if ( 0 >= bandNumber )
  {
    QgsDebugMsg( QStringLiteral( "Invalid parameter" ) );
    return ct;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( gdalDataset, bandNumber );
  GDALColorTableH myGdalColorTable = GDALGetRasterColorTable( myGdalBand );

  if ( myGdalColorTable )
  {
    QgsDebugMsgLevel( QStringLiteral( "Color table found" ), 2 );

    // load category labels
    char **categoryNames = GDALGetRasterCategoryNames( myGdalBand );
    QVector<QString> labels;
    if ( categoryNames )
    {
      int i = 0;
      while ( categoryNames[i] )
      {
        labels.append( QString( categoryNames[i] ) );
        i++;
      }
    }

    const int myEntryCount = GDALGetColorEntryCount( myGdalColorTable );
    const GDALColorInterp myColorInterpretation = GDALGetRasterColorInterpretation( myGdalBand );
    QgsDebugMsgLevel( "Color Interpretation: " + QString::number( static_cast< int >( myColorInterpretation ) ), 2 );
    const GDALPaletteInterp myPaletteInterpretation  = GDALGetPaletteInterpretation( myGdalColorTable );
    QgsDebugMsgLevel( "Palette Interpretation: " + QString::number( static_cast< int >( myPaletteInterpretation ) ), 2 );

    const GDALColorEntry *myColorEntry = nullptr;
    for ( int myIterator = 0; myIterator < myEntryCount; myIterator++ )
    {
      myColorEntry = GDALGetColorEntry( myGdalColorTable, myIterator );

      if ( !myColorEntry )
      {
        continue;
      }
      else
      {
        QString label = labels.value( myIterator );
        if ( label.isEmpty() )
        {
          label = QString::number( myIterator );
        }
        //Branch on the color interpretation type
        if ( myColorInterpretation == GCI_GrayIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.value = static_cast< double >( myIterator );
          myColorRampItem.label = label;
          myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          ct.append( myColorRampItem );
        }
        else if ( myColorInterpretation == GCI_PaletteIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.value = static_cast< double >( myIterator );
          myColorRampItem.label = label;
          //Branch on palette interpretation
          if ( myPaletteInterpretation  == GPI_RGB )
          {
            myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c2, myColorEntry->c3, myColorEntry->c4 );
          }
          else if ( myPaletteInterpretation  == GPI_CMYK )
          {
            myColorRampItem.color = QColor::fromCmyk( myColorEntry->c1, myColorEntry->c2, myColorEntry->c3, myColorEntry->c4 );
          }
          else if ( myPaletteInterpretation  == GPI_HLS )
          {
            myColorRampItem.color = QColor::fromHsv( myColorEntry->c1, myColorEntry->c3, myColorEntry->c2, myColorEntry->c4 );
          }
          else
          {
            myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          }
          ct.append( myColorRampItem );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Color interpretation type not supported yet" ), 2 );
          return ct;
        }
      }
    }
  }
  else
  {
    QgsDebugMsgLevel( "No color table found for band " + QString::number( bandNumber ), 2 );
    return ct;
  }

  QgsDebugMsgLevel( QStringLiteral( "Color table loaded successfully" ), 2 );
  return ct;
}

Qgis::DataType QgsGdalProviderBase::dataTypeFromGdal( const GDALDataType gdalDataType ) const
{
  switch ( gdalDataType )
  {
    case GDT_Byte:
      return Qgis::DataType::Byte;
    case GDT_UInt16:
      return Qgis::DataType::UInt16;
    case GDT_Int16:
      return Qgis::DataType::Int16;
    case GDT_UInt32:
      return Qgis::DataType::UInt32;
    case GDT_Int32:
      return Qgis::DataType::Int32;
    case GDT_Float32:
      return Qgis::DataType::Float32;
    case GDT_Float64:
      return Qgis::DataType::Float64;
    case GDT_CInt16:
      return Qgis::DataType::CInt16;
    case GDT_CInt32:
      return Qgis::DataType::CInt32;
    case GDT_CFloat32:
      return Qgis::DataType::CFloat32;
    case GDT_CFloat64:
      return Qgis::DataType::CFloat64;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
    case GDT_Int64:
    case GDT_UInt64:
      // Lossy conversion
      // NOTE: remove conversion from/to double in qgsgdalprovider.cpp if using
      // a native Qgis data type for Int64/UInt64 (look for GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0))
      return Qgis::DataType::Float64;
#endif
    case GDT_Unknown:
    case GDT_TypeCount:
      return Qgis::DataType::UnknownDataType;
  }
  return Qgis::DataType::UnknownDataType;
}

int QgsGdalProviderBase::colorInterpretationFromGdal( const GDALColorInterp gdalColorInterpretation ) const
{
  switch ( gdalColorInterpretation )
  {
    case GCI_GrayIndex:
      return QgsRaster::GrayIndex;
    case GCI_PaletteIndex:
      return QgsRaster::PaletteIndex;
    case GCI_RedBand:
      return QgsRaster::RedBand;
    case GCI_GreenBand:
      return QgsRaster::GreenBand;
    case GCI_BlueBand:
      return QgsRaster::BlueBand;
    case GCI_AlphaBand:
      return QgsRaster::AlphaBand;
    case GCI_HueBand:
      return QgsRaster::HueBand;
    case GCI_SaturationBand:
      return QgsRaster::SaturationBand;
    case GCI_LightnessBand:
      return QgsRaster::LightnessBand;
    case GCI_CyanBand:
      return QgsRaster::CyanBand;
    case GCI_MagentaBand:
      return QgsRaster::MagentaBand;
    case GCI_YellowBand:
      return QgsRaster::YellowBand;
    case GCI_BlackBand:
      return QgsRaster::BlackBand;
    case GCI_YCbCr_YBand:
      return QgsRaster::YCbCr_YBand;
    case GCI_YCbCr_CbBand:
      return QgsRaster::YCbCr_CbBand;
    case GCI_YCbCr_CrBand:
      return QgsRaster::YCbCr_CrBand;
    case GCI_Undefined:
      return QgsRaster::UndefinedColorInterpretation;
  }
  return QgsRaster::UndefinedColorInterpretation;
}

void QgsGdalProviderBase::registerGdalDrivers()
{
  static std::once_flag initialized;
  std::call_once( initialized, QgsApplication::registerGdalDriversFromSettings );
}

QgsRectangle QgsGdalProviderBase::extent( GDALDatasetH gdalDataset )const
{
  double myGeoTransform[6];

  const bool myHasGeoTransform = GDALGetGeoTransform( gdalDataset, myGeoTransform ) == CE_None;
  if ( !myHasGeoTransform )
  {
    // Initialize the affine transform matrix
    myGeoTransform[0] = 0;
    myGeoTransform[1] = 1;
    myGeoTransform[2] = 0;
    myGeoTransform[3] = 0;
    myGeoTransform[4] = 0;
    myGeoTransform[5] = -1;
  }

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  const double myXMax = myGeoTransform[0] +
                        GDALGetRasterXSize( gdalDataset ) * myGeoTransform[1] +
                        GDALGetRasterYSize( gdalDataset ) * myGeoTransform[2];
  const double myYMin = myGeoTransform[3] +
                        GDALGetRasterXSize( gdalDataset ) * myGeoTransform[4] +
                        GDALGetRasterYSize( gdalDataset ) * myGeoTransform[5];

  const QgsRectangle extent( myGeoTransform[0], myYMin, myXMax, myGeoTransform[3] );
  return extent;
}

GDALDatasetH QgsGdalProviderBase::gdalOpen( const QString &uri, unsigned int nOpenFlags )
{
  QVariantMap parts = decodeGdalUri( uri );
  const QStringList openOptions = parts.value( QStringLiteral( "openOptions" ) ).toStringList();
  parts.remove( QStringLiteral( "openOptions" ) );

  char **papszOpenOptions = nullptr;
  for ( const QString &option : openOptions )
  {
    papszOpenOptions = CSLAddString( papszOpenOptions,
                                     option.toUtf8().constData() );
  }

  const bool modify_OGR_GPKG_FOREIGN_KEY_CHECK = !CPLGetConfigOption( "OGR_GPKG_FOREIGN_KEY_CHECK", nullptr );
  if ( modify_OGR_GPKG_FOREIGN_KEY_CHECK )
  {
    CPLSetThreadLocalConfigOption( "OGR_GPKG_FOREIGN_KEY_CHECK", "NO" );
  }

  GDALDatasetH hDS = GDALOpenEx( encodeGdalUri( parts ).toUtf8().constData(), nOpenFlags, nullptr, papszOpenOptions, nullptr );
  CSLDestroy( papszOpenOptions );

  if ( modify_OGR_GPKG_FOREIGN_KEY_CHECK )
  {
    CPLSetThreadLocalConfigOption( "OGR_GPKG_FOREIGN_KEY_CHECK", nullptr );
  }
  return hDS;
}

int CPL_STDCALL _gdalProgressFnWithFeedback( double dfComplete, const char *pszMessage, void *pProgressArg )
{
  Q_UNUSED( dfComplete )
  Q_UNUSED( pszMessage )

  QgsRasterBlockFeedback *feedback = static_cast<QgsRasterBlockFeedback *>( pProgressArg );
  return !feedback->isCanceled();
}


CPLErr QgsGdalProviderBase::gdalRasterIO( GDALRasterBandH hBand, GDALRWFlag eRWFlag, int nXOff, int nYOff, int nXSize, int nYSize, void *pData, int nBufXSize, int nBufYSize, GDALDataType eBufType, int nPixelSpace, int nLineSpace, QgsRasterBlockFeedback *feedback )
{
  GDALRasterIOExtraArg extra;
  INIT_RASTERIO_EXTRA_ARG( extra );
  if ( false && feedback )  // disabled!
  {
    // Currently the cancellation is disabled... When RasterIO call is canceled,
    // GDAL returns CE_Failure with error code = 0 (CPLE_None), however one would
    // expect to get CPLE_UserInterrupt to clearly identify that the failure was
    // caused by the cancellation and not that something dodgy is going on.
    // Are both error codes acceptable?
    extra.pfnProgress = _gdalProgressFnWithFeedback;
    extra.pProgressData = ( void * ) feedback;
  }
  const CPLErr err = GDALRasterIOEx( hBand, eRWFlag, nXOff, nYOff, nXSize, nYSize, pData, nBufXSize, nBufYSize, eBufType, nPixelSpace, nLineSpace, &extra );

  return err;
}

int QgsGdalProviderBase::gdalGetOverviewCount( GDALRasterBandH hBand )
{
  const int count = GDALGetOverviewCount( hBand );
  return count;
}

QVariantMap QgsGdalProviderBase::decodeGdalUri( const QString &uri )
{
  QString path = uri;
  QString layerName;
  QString authcfg;
  QStringList openOptions;

  const QRegularExpression authcfgRegex( " authcfg='([^']+)'" );
  QRegularExpressionMatch match;
  if ( path.contains( authcfgRegex, &match ) )
  {
    path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
    authcfg = match.captured( 1 );
  }

  QString vsiPrefix = qgsVsiPrefix( path );
  QString vsiSuffix;
  if ( path.startsWith( vsiPrefix, Qt::CaseInsensitive ) )
  {
    path = path.mid( vsiPrefix.count() );

    const QRegularExpression vsiRegex( QStringLiteral( "(?:\\.zip|\\.tar|\\.gz|\\.tar\\.gz|\\.tgz)([^|]+)" ) );
    const QRegularExpressionMatch match = vsiRegex.match( path );
    if ( match.hasMatch() )
    {
      vsiSuffix = match.captured( 1 );
      path = path.remove( match.capturedStart( 1 ), match.capturedLength( 1 ) );
    }
  }
  else
  {
    vsiPrefix.clear();
  }

  if ( path.indexOf( ':' ) != -1 )
  {
    QStringList parts = path.split( ':' );
    if ( parts[0].toLower() == QLatin1String( "gpkg" ) )
    {
      parts.removeFirst();
      // Handle windows paths - which has an extra colon - and unix paths
      if ( ( parts[0].length() > 1 && parts.count() > 1 ) || parts.count() > 2 )
      {
        layerName = parts[parts.length() - 1];
        parts.removeLast();
      }
      path  = parts.join( ':' );
    }
  }

  if ( path.contains( '|' ) )
  {
    const QRegularExpression openOptionRegex( QStringLiteral( "\\|option:([^|]*)" ) );
    while ( true )
    {
      const QRegularExpressionMatch match = openOptionRegex.match( path );
      if ( match.hasMatch() )
      {
        openOptions << match.captured( 1 );
        path = path.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
      }
      else
      {
        break;
      }
    }
  }

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  uriComponents.insert( QStringLiteral( "layerName" ), layerName );
  if ( !openOptions.isEmpty() )
    uriComponents.insert( QStringLiteral( "openOptions" ), openOptions );
  if ( !vsiPrefix.isEmpty() )
    uriComponents.insert( QStringLiteral( "vsiPrefix" ), vsiPrefix );
  if ( !vsiSuffix.isEmpty() )
    uriComponents.insert( QStringLiteral( "vsiSuffix" ), vsiSuffix );
  if ( !authcfg.isEmpty() )
    uriComponents.insert( QStringLiteral( "authcfg" ), authcfg );
  return uriComponents;
}

QString QgsGdalProviderBase::encodeGdalUri( const QVariantMap &parts )
{
  const QString vsiPrefix = parts.value( QStringLiteral( "vsiPrefix" ) ).toString();
  const QString vsiSuffix = parts.value( QStringLiteral( "vsiSuffix" ) ).toString();
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  const QString layerName = parts.value( QStringLiteral( "layerName" ) ).toString();
  const QString authcfg = parts.value( QStringLiteral( "authcfg" ) ).toString();

  QString uri = vsiPrefix + path + vsiSuffix;
  if ( !layerName.isEmpty() && uri.endsWith( QLatin1String( "gpkg" ) ) )
    uri = QStringLiteral( "GPKG:%1:%2" ).arg( uri, layerName );
  else if ( !layerName.isEmpty() )
    uri = uri + QStringLiteral( "|%1" ).arg( layerName );

  const QStringList openOptions = parts.value( QStringLiteral( "openOptions" ) ).toStringList();

  for ( const QString &openOption : openOptions )
  {
    uri += QLatin1String( "|option:" );
    uri += openOption;
  }

  if ( !authcfg.isEmpty() )
    uri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );

  return uri;
}

///@endcond
