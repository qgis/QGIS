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
        //Branch on the color interpretation type
        if ( myColorInterpretation == GCI_GrayIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.label = "";
          myColorRampItem.value = ( double )myIterator;
          myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          ct.append( myColorRampItem );
        }
        else if ( myColorInterpretation == GCI_PaletteIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.value = ( double )myIterator;
          myColorRampItem.label = QString::number( myColorRampItem.value );
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

QgsRasterInterface::DataType QgsGdalProviderBase::dataTypeFromGdal( int theGdalDataType ) const
{
  switch ( theGdalDataType )
  {
    case GDT_Unknown:
      return QgsRasterDataProvider::UnknownDataType;
      break;
    case GDT_Byte:
      return QgsRasterDataProvider::Byte;
      break;
    case GDT_UInt16:
      return QgsRasterDataProvider::UInt16;
      break;
    case GDT_Int16:
      return QgsRasterDataProvider::Int16;
      break;
    case GDT_UInt32:
      return QgsRasterDataProvider::UInt32;
      break;
    case GDT_Int32:
      return QgsRasterDataProvider::Int32;
      break;
    case GDT_Float32:
      return QgsRasterDataProvider::Float32;
      break;
    case GDT_Float64:
      return QgsRasterDataProvider::Float64;
      break;
    case GDT_CInt16:
      return QgsRasterDataProvider::CInt16;
      break;
    case GDT_CInt32:
      return QgsRasterDataProvider::CInt32;
      break;
    case GDT_CFloat32:
      return QgsRasterDataProvider::CFloat32;
      break;
    case GDT_CFloat64:
      return QgsRasterDataProvider::CFloat64;
      break;
    case GDT_TypeCount:
      // make gcc happy
      break;
  }
  return QgsRasterDataProvider::UnknownDataType;
}

int QgsGdalProviderBase::colorInterpretationFromGdal( int gdalColorInterpretation ) const
{
  switch ( gdalColorInterpretation )
  {
    case GCI_Undefined:
      return QgsRasterDataProvider::UndefinedColorInterpretation;
      break;
    case GCI_GrayIndex:
      return QgsRasterDataProvider::GrayIndex;
      break;
    case GCI_PaletteIndex:
      return QgsRasterDataProvider::PaletteIndex;
      break;
    case GCI_RedBand:
      return QgsRasterDataProvider::RedBand;
      break;
    case GCI_GreenBand:
      return QgsRasterDataProvider::GreenBand;
      break;
    case GCI_BlueBand:
      return QgsRasterDataProvider::BlueBand;
      break;
    case GCI_AlphaBand:
      return QgsRasterDataProvider::AlphaBand;
      break;
    case GCI_HueBand:
      return QgsRasterDataProvider::HueBand;
      break;
    case GCI_SaturationBand:
      return QgsRasterDataProvider::SaturationBand;
      break;
    case GCI_LightnessBand:
      return QgsRasterDataProvider::LightnessBand;
      break;
    case GCI_CyanBand:
      return QgsRasterDataProvider::CyanBand;
      break;
    case GCI_MagentaBand:
      return QgsRasterDataProvider::MagentaBand;
      break;
    case GCI_YellowBand:
      return QgsRasterDataProvider::YellowBand;
      break;
    case GCI_BlackBand:
      return QgsRasterDataProvider::BlackBand;
      break;
    case GCI_YCbCr_YBand:
      return QgsRasterDataProvider::YCbCr_YBand;
      break;
    case GCI_YCbCr_CbBand:
      return QgsRasterDataProvider::YCbCr_CbBand;
      break;
    case GCI_YCbCr_CrBand:
      return QgsRasterDataProvider::YCbCr_CrBand;
      break;
    default:
      break;
  }
  return QgsRasterDataProvider::UndefinedColorInterpretation;
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
