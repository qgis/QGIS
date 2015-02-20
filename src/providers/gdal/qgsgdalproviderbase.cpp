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

#define CPL_SUPRESS_CPLUSPLUS
#include "cpl_conv.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsgdalproviderbase.h"

#include <QSettings>

QgsGdalProviderBase::QgsGdalProviderBase()
{
  QgsDebugMsg( "Entered" );

  // first get the GDAL driver manager
  QgsGdalProviderBase::registerGdalDrivers();
}

QgsGdalProviderBase::~QgsGdalProviderBase()
{
}

/**
 * @param theBandNumber the number of the band for which you want a color table
 * @param theList a pointer the object that will hold the color table
 * @return true of a color table was able to be read, false otherwise
 */
QList<QgsColorRampShader::ColorRampItem> QgsGdalProviderBase::colorTable( GDALDatasetH gdalDataset, int theBandNumber )const
{
  QgsDebugMsg( "entered." );
  QList<QgsColorRampShader::ColorRampItem> ct;

  //Invalid band number, segfault prevention
  if ( 0 >= theBandNumber )
  {
    QgsDebugMsg( "Invalid parameter" );
    return ct;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( gdalDataset, theBandNumber );
  GDALColorTableH myGdalColorTable = GDALGetRasterColorTable( myGdalBand );

  if ( myGdalColorTable )
  {
    QgsDebugMsg( "Color table found" );

    // load category labels
    char ** categoryNames = GDALGetRasterCategoryNames( myGdalBand );
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

    int myEntryCount = GDALGetColorEntryCount( myGdalColorTable );
    GDALColorInterp myColorInterpretation =  GDALGetRasterColorInterpretation( myGdalBand );
    QgsDebugMsg( "Color Interpretation: " + QString::number(( int )myColorInterpretation ) );
    GDALPaletteInterp myPaletteInterpretation  = GDALGetPaletteInterpretation( myGdalColorTable );
    QgsDebugMsg( "Palette Interpretation: " + QString::number(( int )myPaletteInterpretation ) );

    const GDALColorEntry* myColorEntry = 0;
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
          myColorRampItem.value = ( double )myIterator;
          myColorRampItem.label = label;
          myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          ct.append( myColorRampItem );
        }
        else if ( myColorInterpretation == GCI_PaletteIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.value = ( double )myIterator;
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
          QgsDebugMsg( "Color interpretation type not supported yet" );
          return ct;
        }
      }
    }
  }
  else
  {
    QgsDebugMsg( "No color table found for band " + QString::number( theBandNumber ) );
    return ct;
  }

  QgsDebugMsg( "Color table loaded successfully" );
  return ct;
}

QGis::DataType QgsGdalProviderBase::dataTypeFromGdal( const GDALDataType theGdalDataType ) const
{
  switch ( theGdalDataType )
  {
    case GDT_Byte: return QGis::Byte;
    case GDT_UInt16: return QGis::UInt16;
    case GDT_Int16: return QGis::Int16;
    case GDT_UInt32: return QGis::UInt32;
    case GDT_Int32: return QGis::Int32;
    case GDT_Float32: return QGis::Float32;
    case GDT_Float64: return QGis::Float64;
    case GDT_CInt16: return QGis::CInt16;
    case GDT_CInt32: return QGis::CInt32;
    case GDT_CFloat32: return QGis::CFloat32;
    case GDT_CFloat64: return QGis::CFloat64;
    case GDT_Unknown: return QGis::UnknownDataType;
  }
  return QGis::UnknownDataType;
}

int QgsGdalProviderBase::colorInterpretationFromGdal( const GDALColorInterp gdalColorInterpretation ) const
{
  switch ( gdalColorInterpretation )
  {
    case GCI_GrayIndex: return QgsRaster::GrayIndex;
    case GCI_PaletteIndex: return QgsRaster::PaletteIndex;
    case GCI_RedBand: return QgsRaster::RedBand;
    case GCI_GreenBand: return QgsRaster::GreenBand;
    case GCI_BlueBand: return QgsRaster::BlueBand;
    case GCI_AlphaBand: return QgsRaster::AlphaBand;
    case GCI_HueBand: return QgsRaster::HueBand;
    case GCI_SaturationBand: return QgsRaster::SaturationBand;
    case GCI_LightnessBand: return QgsRaster::LightnessBand;
    case GCI_CyanBand: return QgsRaster::CyanBand;
    case GCI_MagentaBand: return QgsRaster::MagentaBand;
    case GCI_YellowBand: return QgsRaster::YellowBand;
    case GCI_BlackBand: return QgsRaster::BlackBand;
    case GCI_YCbCr_YBand: return QgsRaster::YCbCr_YBand;
    case GCI_YCbCr_CbBand: return QgsRaster::YCbCr_CbBand;
    case GCI_YCbCr_CrBand: return QgsRaster::YCbCr_CrBand;
    case GCI_Undefined: return QgsRaster::UndefinedColorInterpretation;
  }
  return QgsRaster::UndefinedColorInterpretation;
}

void QgsGdalProviderBase::registerGdalDrivers()
{
  GDALAllRegister();
  QSettings mySettings;
  QString myJoinedList = mySettings.value( "gdal/skipList", "" ).toString();
  if ( !myJoinedList.isEmpty() )
  {
    QStringList myList = myJoinedList.split( " " );
    for ( int i = 0; i < myList.size(); ++i )
    {
      QgsApplication::skipGdalDriver( myList.at( i ) );
    }
    QgsApplication::applyGdalSkippedDrivers();
  }
}

QgsRectangle QgsGdalProviderBase::extent( GDALDatasetH gdalDataset )const
{
  double myGeoTransform[6];

  bool myHasGeoTransform = GDALGetGeoTransform( gdalDataset, myGeoTransform ) == CE_None;
  if ( !myHasGeoTransform )
  {
    // Initialise the affine transform matrix
    myGeoTransform[0] =  0;
    myGeoTransform[1] =  1;
    myGeoTransform[2] =  0;
    myGeoTransform[3] =  0;
    myGeoTransform[4] =  0;
    myGeoTransform[5] = -1;
  }

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  double myXMax = myGeoTransform[0] +
                  GDALGetRasterXSize( gdalDataset ) * myGeoTransform[1] +
                  GDALGetRasterYSize( gdalDataset ) * myGeoTransform[2];
  double myYMin = myGeoTransform[3] +
                  GDALGetRasterXSize( gdalDataset ) * myGeoTransform[4] +
                  GDALGetRasterYSize( gdalDataset ) * myGeoTransform[5];

  QgsRectangle extent( myGeoTransform[0], myYMin, myXMax, myGeoTransform[3] );
  return extent;
}

GDALDatasetH QgsGdalProviderBase::gdalOpen( const char *pszFilename, GDALAccess eAccess )
{
  // See http://hub.qgis.org/issues/8356 and http://trac.osgeo.org/gdal/ticket/5170
#if GDAL_VERSION_MAJOR == 1 && ( (GDAL_VERSION_MINOR == 9 && GDAL_VERSION_REV <= 2) || (GDAL_VERSION_MINOR == 10 && GDAL_VERSION_REV <= 0) )
  char* pszOldVal = CPLStrdup( CPLGetConfigOption( "VSI_CACHE", "FALSE" ) );
  CPLSetThreadLocalConfigOption( "VSI_CACHE", "FALSE" );
  QgsDebugMsg( "Disabled VSI_CACHE" );
#endif

  GDALDatasetH hDS = GDALOpen( pszFilename, eAccess );

#if GDAL_VERSION_MAJOR == 1 && ( (GDAL_VERSION_MINOR == 9 && GDAL_VERSION_REV <= 2) || (GDAL_VERSION_MINOR == 10 && GDAL_VERSION_REV <= 0) )
  CPLSetThreadLocalConfigOption( "VSI_CACHE", pszOldVal );
  CPLFree( pszOldVal );
  QgsDebugMsg( "Reset VSI_CACHE" );
#endif

  return hDS;
}

CPLErr QgsGdalProviderBase::gdalRasterIO( GDALRasterBandH hBand, GDALRWFlag eRWFlag, int nXOff, int nYOff, int nXSize, int nYSize, void * pData, int nBufXSize, int nBufYSize, GDALDataType eBufType, int nPixelSpace, int nLineSpace )
{
  // See http://hub.qgis.org/issues/8356 and http://trac.osgeo.org/gdal/ticket/5170
#if GDAL_VERSION_MAJOR == 1 && ( (GDAL_VERSION_MINOR == 9 && GDAL_VERSION_REV <= 2) || (GDAL_VERSION_MINOR == 10 && GDAL_VERSION_REV <= 0) )
  char* pszOldVal = CPLStrdup( CPLGetConfigOption( "VSI_CACHE", "FALSE" ) );
  CPLSetThreadLocalConfigOption( "VSI_CACHE", "FALSE" );
  QgsDebugMsg( "Disabled VSI_CACHE" );
#endif

  CPLErr err = GDALRasterIO( hBand, eRWFlag, nXOff, nYOff, nXSize, nYSize, pData, nBufXSize, nBufYSize, eBufType, nPixelSpace, nLineSpace );

#if GDAL_VERSION_MAJOR == 1 && ( (GDAL_VERSION_MINOR == 9 && GDAL_VERSION_REV <= 2) || (GDAL_VERSION_MINOR == 10 && GDAL_VERSION_REV <= 0) )
  CPLSetThreadLocalConfigOption( "VSI_CACHE", pszOldVal );
  CPLFree( pszOldVal );
  QgsDebugMsg( "Reset VSI_CACHE" );
#endif

  return err;
}

int QgsGdalProviderBase::gdalGetOverviewCount( GDALRasterBandH hBand )
{
  // See http://hub.qgis.org/issues/8356 and http://trac.osgeo.org/gdal/ticket/5170
#if GDAL_VERSION_MAJOR == 1 && ( (GDAL_VERSION_MINOR == 9 && GDAL_VERSION_REV <= 2) || (GDAL_VERSION_MINOR == 10 && GDAL_VERSION_REV <= 0) )
  char* pszOldVal = CPLStrdup( CPLGetConfigOption( "VSI_CACHE", "FALSE" ) );
  CPLSetThreadLocalConfigOption( "VSI_CACHE", "FALSE" );
  QgsDebugMsg( "Disabled VSI_CACHE" );
#endif

  int count = GDALGetOverviewCount( hBand );

#if GDAL_VERSION_MAJOR == 1 && ( (GDAL_VERSION_MINOR == 9 && GDAL_VERSION_REV <= 2) || (GDAL_VERSION_MINOR == 10 && GDAL_VERSION_REV <= 0) )
  CPLSetThreadLocalConfigOption( "VSI_CACHE", pszOldVal );
  CPLFree( pszOldVal );
  QgsDebugMsg( "Reset VSI_CACHE" );
#endif

  return count;
}
