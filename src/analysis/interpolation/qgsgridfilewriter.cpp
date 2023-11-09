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
#include <QFile>
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
  QFile outputFile( mOutputFilePath );

  if ( !outputFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    return 1;
  }

  if ( !mInterpolator )
  {
    outputFile.remove();
    return 2;
  }

  QTextStream outStream( &outputFile );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  outStream.setCodec( "UTF-8" );
#endif
  outStream.setRealNumberPrecision( 8 );
  writeHeader( outStream );

  double currentYValue = mInterpolationExtent.yMaximum() - mCellSizeY / 2.0; //calculate value in the center of the cell
  double currentXValue;
  double interpolatedValue;

  for ( int i = 0; i < mNumRows; ++i )
  {
    currentXValue = mInterpolationExtent.xMinimum() + mCellSizeX / 2.0; //calculate value in the center of the cell
    for ( int j = 0; j < mNumColumns; ++j )
    {
      if ( mInterpolator->interpolatePoint( currentXValue, currentYValue, interpolatedValue, feedback ) == 0 )
      {
        outStream << interpolatedValue << ' ';
      }
      else
      {
        outStream << "-9999 ";
      }
      currentXValue += mCellSizeX;
    }

    outStream << Qt::endl;
    currentYValue -= mCellSizeY;

    if ( feedback )
    {
      if ( feedback->isCanceled() )
      {
        outputFile.remove();
        return 3;
      }
      feedback->setProgress( 100.0 * i / static_cast< double >( mNumRows ) );
    }
  }

  // create prj file
  QgsInterpolator::LayerData ld;
  ld = mInterpolator->layerData().at( 0 );
  QgsFeatureSource *source = ld.source;
  const QString crs = source->sourceCrs().toWkt();
  const QFileInfo fi( mOutputFilePath );
  const QString fileName = fi.absolutePath() + '/' + fi.completeBaseName() + ".prj";
  QFile prjFile( fileName );
  if ( !prjFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    return 1;
  }
  QTextStream prjStream( &prjFile );
  prjStream << crs;
  prjStream << Qt::endl;
  prjFile.close();

  return 0;
}

int QgsGridFileWriter::writeHeader( QTextStream &outStream )
{
  outStream << "NCOLS " << mNumColumns << Qt::endl;
  outStream << "NROWS " << mNumRows << Qt::endl;
  outStream << "XLLCORNER " << mInterpolationExtent.xMinimum() << Qt::endl;
  outStream << "YLLCORNER " <<  mInterpolationExtent.yMinimum() << Qt::endl;
  if ( mCellSizeX == mCellSizeY ) //standard way
  {
    outStream << "CELLSIZE " << mCellSizeX << Qt::endl;
  }
  else //this is supported by GDAL but probably not by other products
  {
    outStream << "DX " << mCellSizeX << Qt::endl;
    outStream << "DY " << mCellSizeY << Qt::endl;
  }
  outStream << "NODATA_VALUE -9999" << Qt::endl;
  return 0;
}
