/***************************************************************************
                              qgsgridfilewriter.cpp
                              ---------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgridfilewriter.h"
#include "qgsinterpolator.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterblock.h"
#include <QFileInfo>

QgsGridFileWriter::QgsGridFileWriter( QgsInterpolator *i, const QString &outputPath, const QgsRectangle &extent, int nCols, int nRows )
  : mInterpolator( i )
  , mOutputFilePath( outputPath )
  , mInterpolationExtent( extent )
  , mNumColumns( nCols )
  , mNumRows( nRows )
  , mCellSizeX( extent.width() / nCols )
  , mCellSizeY( extent.height() / nRows )
{}

int QgsGridFileWriter::writeFile( QgsFeedback *feedback )
{
  const QFileInfo fi( mOutputFilePath );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  QgsInterpolator::LayerData ld = mInterpolator->layerData().at( 0 );
  const QgsCoordinateReferenceSystem crs = ld.source->sourceCrs();

  std::unique_ptr<QgsRasterFileWriter> writer = std::make_unique<QgsRasterFileWriter>( mOutputFilePath );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );

  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( Qgis::DataType::Float32, mNumColumns, mNumRows, mInterpolationExtent, crs ) );
  if ( !provider )
  {
    QgsDebugMsgLevel( QStringLiteral( "Could not create raster output: %1" ).arg( mOutputFilePath ), 2 );
    return 1;
  }
  if ( !provider->isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Could not create raster output: %1: %2" ).arg( mOutputFilePath, provider->error().message( QgsErrorMessage::Text ) ), 2 );
    return 2;
  }

  provider->setNoDataValue( 1, -9999 );

  double currentYValue = mInterpolationExtent.yMaximum() - mCellSizeY / 2.0; //calculate value in the center of the cell
  double currentXValue;
  double interpolatedValue;

  const double step = mNumRows > 0 ? 100.0 / mNumRows : 1;
  for ( int row = 0; row < mNumRows; row++ )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    currentXValue = mInterpolationExtent.xMinimum() + mCellSizeX / 2.0; //calculate value in the center of the cell

    QgsRasterBlock block( Qgis::DataType::Float32, mNumColumns, 1 );

    std::vector<float> float32Row( mNumColumns );
    for ( int col = 0; col < mNumColumns; col++ )
    {
      if ( mInterpolator->interpolatePoint( currentXValue, currentYValue, interpolatedValue, feedback ) == 0 )
      {
        float32Row[col] = interpolatedValue;
      }
      else
      {
        float32Row[col] = -9999;
      }
      currentXValue += mCellSizeX;
    }
    block.setData( QByteArray( reinterpret_cast<const char *>( float32Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Float32 ) * mNumColumns ) );
    provider->writeBlock( &block, 1, 0, row );
    currentYValue -= mCellSizeY;
    if ( feedback )
    {
      feedback->setProgress( row * step );
    }
  }

  return 0;
}
