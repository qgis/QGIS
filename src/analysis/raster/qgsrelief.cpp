/***************************************************************************
                          qgsrelief.cpp  -  description
                          ---------------------------
    begin                : November 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelief.h"

#include <cfloat>
#include <cpl_string.h>

#include "qgis.h"
#include "qgsaspectfilter.h"
#include "qgsfeedback.h"
#include "qgsgdalutils.h"
#include "qgshillshadefilter.h"
#include "qgslogger.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterlayer.h"
#include "qgsslopefilter.h"

#include <QColor>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QVector>

using namespace Qt::StringLiterals;

QgsRelief::QgsRelief( const QString &inputFile, const QString &outputFile, const QString &outputFormat )
  : mInputFile( inputFile )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
  , mSlopeFilter( std::make_unique<QgsSlopeFilter>( inputFile, outputFile, outputFormat ) )
  , mAspectFilter( std::make_unique<QgsAspectFilter>( inputFile, outputFile, outputFormat ) )
  , mHillshadeFilter285( std::make_unique<QgsHillshadeFilter>( inputFile, outputFile, outputFormat, 285, 30 ) )
  , mHillshadeFilter300( std::make_unique<QgsHillshadeFilter>( inputFile, outputFile, outputFormat, 300, 30 ) )
  , mHillshadeFilter315( std::make_unique<QgsHillshadeFilter>( inputFile, outputFile, outputFormat, 315, 30 ) )
{
  /*mReliefColors = calculateOptimizedReliefClasses();
    setDefaultReliefColors();*/
}

QgsRelief::~QgsRelief() = default;

void QgsRelief::clearReliefColors()
{
  mReliefColors.clear();
}

void QgsRelief::addReliefColorClass( const QgsRasterReliefColor &color )
{
  mReliefColors.push_back( color );
}

void QgsRelief::setDefaultReliefColors()
{
  clearReliefColors();
  addReliefColorClass( QgsRasterReliefColor( QColor( 9, 176, 76 ), 0, 200 ) );
  addReliefColorClass( QgsRasterReliefColor( QColor( 20, 228, 128 ), 200, 500 ) );
  addReliefColorClass( QgsRasterReliefColor( QColor( 167, 239, 153 ), 500, 1000 ) );
  addReliefColorClass( QgsRasterReliefColor( QColor( 218, 188, 143 ), 1000, 2000 ) );
  addReliefColorClass( QgsRasterReliefColor( QColor( 233, 158, 91 ), 2000, 4000 ) );
  addReliefColorClass( QgsRasterReliefColor( QColor( 255, 255, 255 ), 4000, 9000 ) );
}

QgsRelief::Result QgsRelief::processRaster( QgsFeedback *feedback )
{
  auto inputLayer = std::make_unique< QgsRasterLayer >( mInputFile, u"relief"_s, u"gdal"_s );
  if ( !inputLayer->isValid() )
  {
    return Result::InvalidInput;
  }
  QgsRasterDataProvider *inputProvider = inputLayer->dataProvider();
  if ( !inputProvider )
  {
    return Result::InvalidInput;
  }

  const int xSize = inputProvider->xSize();
  const int ySize = inputProvider->ySize();
  mCellSizeX = std::fabs( inputLayer->rasterUnitsPerPixelX() );
  mCellSizeY = std::fabs( inputLayer->rasterUnitsPerPixelY() );

  //output driver
  QgsRasterFileWriter writer( mOutputFile );
  writer.setOutputFormat( mOutputFormat );
  std::unique_ptr<QgsRasterDataProvider> outputProvider( writer.createMultiBandRaster( Qgis::DataType::Byte, xSize, ySize, inputProvider->extent(), inputProvider->crs(), 3 ) );
  if ( !outputProvider )
  {
    return Result::OutputCreationFailed;
  }

  //initialize dependency filters with cell sizes
  mHillshadeFilter285->setCellSizeX( mCellSizeX );
  mHillshadeFilter285->setCellSizeY( mCellSizeY );
  mHillshadeFilter285->setZFactor( mZFactor );
  mHillshadeFilter300->setCellSizeX( mCellSizeX );
  mHillshadeFilter300->setCellSizeY( mCellSizeY );
  mHillshadeFilter300->setZFactor( mZFactor );
  mHillshadeFilter315->setCellSizeX( mCellSizeX );
  mHillshadeFilter315->setCellSizeY( mCellSizeY );
  mHillshadeFilter315->setZFactor( mZFactor );
  mSlopeFilter->setCellSizeX( mCellSizeX );
  mSlopeFilter->setCellSizeY( mCellSizeY );
  mSlopeFilter->setZFactor( mZFactor );
  mAspectFilter->setCellSizeX( mCellSizeX );
  mAspectFilter->setCellSizeY( mCellSizeY );
  mAspectFilter->setZFactor( mZFactor );

  if ( inputProvider->sourceHasNoDataValue( 1 ) )
  {
    mInputNodataValue = inputProvider->sourceNoDataValue( 1 );
  }
  else
  {
    mInputNodataValue = -9999;
  }

  mSlopeFilter->setInputNodataValue( mInputNodataValue );
  mAspectFilter->setInputNodataValue( mInputNodataValue );
  mHillshadeFilter285->setInputNodataValue( mInputNodataValue );
  mHillshadeFilter300->setInputNodataValue( mInputNodataValue );
  mHillshadeFilter315->setInputNodataValue( mInputNodataValue );

  mOutputNodataValue = -9999;
  outputProvider->setNoDataValue( 1, mOutputNodataValue );
  outputProvider->setNoDataValue( 2, mOutputNodataValue );
  outputProvider->setNoDataValue( 3, mOutputNodataValue );
  mSlopeFilter->setOutputNodataValue( mOutputNodataValue );
  mAspectFilter->setOutputNodataValue( mOutputNodataValue );
  mHillshadeFilter285->setOutputNodataValue( mOutputNodataValue );
  mHillshadeFilter300->setOutputNodataValue( mOutputNodataValue );
  mHillshadeFilter315->setOutputNodataValue( mOutputNodataValue );

  if ( ySize < 3 ) //we require at least three rows (should be true for most datasets)
  {
    return Result::InvalidInput;
  }

  // iterate row-by-row over the raster
  QgsRasterIterator iter( inputProvider );
  iter.setMaximumTileWidth( xSize );
  iter.setMaximumTileHeight( 1 );
  iter.startRasterRead( 1, xSize, ySize, inputProvider->extent() );

  //keep only three scanlines in memory at a time
  std::vector<float> scanLine1( xSize );
  std::vector<float> scanLine2( xSize );
  std::vector<float> scanLine3( xSize );
  std::vector<unsigned char> resultRedLine( xSize );
  std::vector<unsigned char> resultGreenLine( xSize );
  std::vector<unsigned char> resultBlueLine( xSize );

  bool hasReportsDuringClose = false;
  constexpr double maxProgressDuringBlockWriting = 100.0;

  auto readRow = [&iter, this]( std::vector<float> &scanLine ) {
    int iterCols = 0;
    int iterRows = 0;
    int iterLeft = 0;
    int iterTop = 0;
    std::unique_ptr<QgsRasterBlock> block;
    if ( iter.readNextRasterPart( 1, iterCols, iterRows, block, iterLeft, iterTop ) && block && block->isValid() )
    {
      bool isNoData = false;
      for ( int j = 0; j < iterCols; ++j )
      {
        const double val = block->valueAndNoData( 0, j, isNoData );
        scanLine[j] = isNoData ? mInputNodataValue : static_cast<float>( val );
      }
    }
    else
    {
      std::fill( scanLine.begin(), scanLine.end(), mInputNodataValue );
    }
  };

  //values outside the layer extent (if the 3x3 window is on the border) are sent to the processing method as (input) nodata values
  for ( int i = 0; i < ySize; ++i )
  {
    if ( feedback )
    {
      feedback->setProgress( maxProgressDuringBlockWriting * i / static_cast<double>( ySize ) );
    }

    if ( feedback && feedback->isCanceled() )
    {
      return Result::Canceled;
    }

    if ( i == 0 )
    {
      //fill scanline 1 with (input) nodata for the values above the first row and feed scanline2 with the first row
      std::fill( scanLine1.begin(), scanLine1.end(), mInputNodataValue );
      readRow( scanLine2 );
    }
    else
    {
      //normally fetch only scanLine3 and release scanline 1 if we move forward one row
      std::swap( scanLine1, scanLine2 );
      std::swap( scanLine2, scanLine3 );
    }

    if ( i == ySize - 1 ) //fill the row below the bottom with nodata values
    {
      std::fill( scanLine3.begin(), scanLine3.end(), mInputNodataValue );
    }
    else
    {
      readRow( scanLine3 );
    }

    for ( int j = 0; j < xSize; ++j )
    {
      bool resultOk = false;
      if ( j == 0 )
      {
        resultOk = processNineCellWindow(
          &mInputNodataValue,
          &scanLine1[j],
          &scanLine1[j + 1],
          &mInputNodataValue,
          &scanLine2[j],
          &scanLine2[j + 1],
          &mInputNodataValue,
          &scanLine3[j],
          &scanLine3[j + 1],
          &resultRedLine[j],
          &resultGreenLine[j],
          &resultBlueLine[j]
        );
      }
      else if ( j == xSize - 1 )
      {
        resultOk = processNineCellWindow(
          &scanLine1[j - 1],
          &scanLine1[j],
          &mInputNodataValue,
          &scanLine2[j - 1],
          &scanLine2[j],
          &mInputNodataValue,
          &scanLine3[j - 1],
          &scanLine3[j],
          &mInputNodataValue,
          &resultRedLine[j],
          &resultGreenLine[j],
          &resultBlueLine[j]
        );
      }
      else
      {
        resultOk = processNineCellWindow(
          &scanLine1[j - 1],
          &scanLine1[j],
          &scanLine1[j + 1],
          &scanLine2[j - 1],
          &scanLine2[j],
          &scanLine2[j + 1],
          &scanLine3[j - 1],
          &scanLine3[j],
          &scanLine3[j + 1],
          &resultRedLine[j],
          &resultGreenLine[j],
          &resultBlueLine[j]
        );
      }

      if ( !resultOk )
      {
        resultRedLine[j] = mOutputNodataValue;
        resultGreenLine[j] = mOutputNodataValue;
        resultBlueLine[j] = mOutputNodataValue;
      }
    }

    auto redBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Byte, xSize, 1 );
    auto greenBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Byte, xSize, 1 );
    auto blueBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Byte, xSize, 1 );
    for ( int j = 0; j < xSize; ++j )
    {
      redBlock->setValue( 0, j, resultRedLine[j] );
      greenBlock->setValue( 0, j, resultGreenLine[j] );
      blueBlock->setValue( 0, j, resultBlueLine[j] );
    }

    if ( !outputProvider->writeBlock( redBlock.get(), 1, 0, i ) )
    {
      QgsDebugError( u"Raster IO Error"_s );
    }
    if ( !outputProvider->writeBlock( greenBlock.get(), 2, 0, i ) )
    {
      QgsDebugError( u"Raster IO Error"_s );
    }
    if ( !outputProvider->writeBlock( blueBlock.get(), 3, 0, i ) )
    {
      QgsDebugError( u"Raster IO Error"_s );
    }
  }

  if ( feedback && hasReportsDuringClose )
  {
    std::unique_ptr<QgsFeedback> scaledFeedback( QgsFeedback::createScaledFeedback( feedback, maxProgressDuringBlockWriting, 100.0 ) );
    if ( !outputProvider->closeWithProgress( scaledFeedback.get() ) )
    {
      if ( feedback->isCanceled() )
        return Result::Canceled;
      return Result::OutputCreationFailed;
    }
  }

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  if ( feedback && feedback->isCanceled() )
  {
    return Result::Canceled;
  }

  return Result::Success;
}

bool QgsRelief::processNineCellWindow( float *x1, float *x2, float *x3, float *x4, float *x5, float *x6, float *x7, float *x8, float *x9, unsigned char *red, unsigned char *green, unsigned char *blue )
{
  //1. component: color and hillshade from 300 degrees
  int r = 0;
  int g = 0;
  int b = 0;

  const float hillShadeValue300 = mHillshadeFilter300->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  if ( hillShadeValue300 != mOutputNodataValue )
  {
    if ( !getElevationColor( *x5, &r, &g, &b ) )
    {
      r = hillShadeValue300;
      g = hillShadeValue300;
      b = hillShadeValue300;
    }
    else
    {
      r = r / 2.0 + hillShadeValue300 / 2.0;
      g = g / 2.0 + hillShadeValue300 / 2.0;
      b = b / 2.0 + hillShadeValue300 / 2.0;
    }
  }

  //2. component: hillshade and slope
  const float hillShadeValue315 = mHillshadeFilter315->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  const float slope = mSlopeFilter->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  if ( hillShadeValue315 != mOutputNodataValue && slope != mOutputNodataValue )
  {
    int r2, g2, b2;
    if ( slope > 15 )
    {
      r2 = 0 / 2.0 + hillShadeValue315 / 2.0;
      g2 = 0 / 2.0 + hillShadeValue315 / 2.0;
      b2 = 0 / 2.0 + hillShadeValue315 / 2.0;
    }
    else if ( slope >= 1 )
    {
      const int slopeValue = 255 - ( slope / 15.0 * 255.0 );
      r2 = slopeValue / 2.0 + hillShadeValue315 / 2.0;
      g2 = slopeValue / 2.0 + hillShadeValue315 / 2.0;
      b2 = slopeValue / 2.0 + hillShadeValue315 / 2.0;
    }
    else
    {
      r2 = hillShadeValue315;
      g2 = hillShadeValue315;
      b2 = hillShadeValue315;
    }

    //combine with r,g,b with 70 percentage coverage
    r = r * 0.7 + r2 * 0.3;
    g = g * 0.7 + g2 * 0.3;
    b = b * 0.7 + b2 * 0.3;
  }

  //3. combine yellow aspect with 10% transparency, illumination from 285 degrees
  const float hillShadeValue285 = mHillshadeFilter285->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  const float aspect = mAspectFilter->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  if ( hillShadeValue285 != mOutputNodataValue && aspect != mOutputNodataValue )
  {
    double angle_diff = std::fabs( 285 - aspect );
    if ( angle_diff > 180 )
    {
      angle_diff -= 180;
    }

    int r3, g3, b3;
    if ( angle_diff < 90 )
    {
      const int aspectVal = ( 1 - std::cos( angle_diff * M_PI / 180 ) ) * 255;
      r3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      g3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      b3 = 0.5 * aspectVal + hillShadeValue315 * 0.5;
    }
    else //white
    {
      r3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      g3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      b3 = 0.5 * 255 + hillShadeValue315 * 0.5;
    }

    r = r3 * 0.1 + r * 0.9;
    g = g3 * 0.1 + g * 0.9;
    b = b3 * 0.1 + b * 0.9;
  }

  *red = ( unsigned char ) r;
  *green = ( unsigned char ) g;
  *blue = ( unsigned char ) b;
  return true;
}

bool QgsRelief::getElevationColor( double elevation, int *red, int *green, int *blue ) const
{
  QList<QgsRasterReliefColor>::const_iterator reliefColorIt = mReliefColors.constBegin();
  for ( ; reliefColorIt != mReliefColors.constEnd(); ++reliefColorIt )
  {
    if ( elevation >= reliefColorIt->minElevation && elevation <= reliefColorIt->maxElevation )
    {
      const QColor &c = reliefColorIt->color;
      *red = c.red();
      *green = c.green();
      *blue = c.blue();

      return true;
    }
  }
  return false;
}

//this function is mainly there for debugging
bool QgsRelief::exportFrequencyDistributionToCsv( const QString &file )
{
  auto inputLayer = std::make_unique< QgsRasterLayer >( mInputFile, u"relief"_s, u"gdal"_s );
  if ( !inputLayer->isValid() )
  {
    return false;
  }

  QgsRasterDataProvider *inputProvider = inputLayer->dataProvider();
  if ( !inputProvider )
  {
    return false;
  }

  //open first raster band for reading (elevation raster is always single band)
  // get minimum and maximum of elevation raster -> 252 elevation classes
  const QgsRasterBandStats stats = inputProvider->bandStatistics( 1, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max );

  //go through raster cells and get frequency of classes

  //store elevation frequency in 252 elevation classes
  std::vector<double> frequency( 252, 0.0 );
  const double frequencyClassRange = ( stats.maximumValue - stats.minimumValue ) / 252.0;

  QgsRasterIterator iter( inputProvider );
  iter.startRasterRead( 1, inputProvider->xSize(), inputProvider->ySize(), inputProvider->extent() );

  int iterCols = 0;
  int iterRows = 0;
  int iterLeft = 0;
  int iterTop = 0;
  std::unique_ptr<QgsRasterBlock> block;

  bool isNoData = false;
  int elevationClass = -1;
  while ( iter.readNextRasterPart( 1, iterCols, iterRows, block, iterLeft, iterTop ) )
  {
    for ( int row = 0; row < iterRows; ++row )
    {
      for ( int col = 0; col < iterCols; ++col )
      {
        const double value = block->valueAndNoData( row, col, isNoData );
        if ( isNoData )
        {
          continue;
        }

        elevationClass = frequencyClassForElevation( value, stats.minimumValue, frequencyClassRange );
        if ( elevationClass >= 0 && elevationClass < 252 )
        {
          frequency[elevationClass] += 1.0;
        }
      }
    }
  }

  //log10 transformation for all frequency values
  for ( int i = 0; i < 252; ++i )
  {
    frequency[i] = std::log10( frequency[i] );
  }

  //write out frequency values to csv file for debugging
  QFile outFile( file );
  if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return false;
  }

  QTextStream outstream( &outFile );
  for ( int i = 0; i < 252; ++i )
  {
    outstream << QString::number( i ) + ',' + QString::number( frequency[i] ) << Qt::endl;
  }
  outFile.close();
  return true;
}

QList<QgsRasterReliefColor> QgsRelief::calculateOptimizedReliefClasses()
{
  auto inputLayer = std::make_unique< QgsRasterLayer >( mInputFile, u"relief"_s, u"gdal"_s );
  if ( !inputLayer->isValid() )
  {
    return {};
  }
  return QgsRasterLayerUtils::calculateOptimizedReliefClasses( inputLayer->dataProvider(), 1 );
}

int QgsRelief::frequencyClassForElevation( double elevation, double minElevation, double elevationClassRange )
{
  return ( elevation - minElevation ) / elevationClassRange;
}
