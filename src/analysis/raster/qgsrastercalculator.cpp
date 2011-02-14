/***************************************************************************
                          qgsrastercalculator.cpp  -  description
                          -----------------------
    begin                : September 28th, 2010
    copyright            : (C) 2010 by Marco Hugentobler
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

#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterlayer.h"
#include "qgsrastermatrix.h"
#include "cpl_string.h"
#include <QProgressDialog>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x) (x).toUtf8().constData()
#else
#define TO8(x) (x).toLocal8Bit().constData()
#endif

QgsRasterCalculator::QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
    const QgsRectangle& outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries ): mFormulaString( formulaString ), mOutputFile( outputFile ), mOutputFormat( outputFormat ),
  mOutputRectangle( outputExtent ), mNumOutputColumns( nOutputColumns ), mNumOutputRows( nOutputRows ), mRasterEntries( rasterEntries )
{
}

QgsRasterCalculator::~QgsRasterCalculator()
{
}

int QgsRasterCalculator::processCalculation( QProgressDialog* p )
{
  //prepare search string / tree
  QString errorString;
  QgsRasterCalcNode* calcNode = QgsRasterCalcNode::parseRasterCalcString( mFormulaString, errorString );
  if( !calcNode )
  {
    //error
  }

  double targetGeoTransform[6];
  outputGeoTransform( targetGeoTransform );

  //open all input rasters for reading
  QMap< QString, GDALRasterBandH > mInputRasterBands; //raster references and corresponding scanline data
  QMap< QString, QgsRasterMatrix* > inputScanLineData; //stores raster references and corresponding scanline data
  QVector< GDALDatasetH > mInputDatasets; //raster references and corresponding dataset

  QVector<QgsRasterCalculatorEntry>::const_iterator it = mRasterEntries.constBegin();
  for( ; it != mRasterEntries.constEnd(); ++it )
  {
    if( !it->raster )  // no raster layer in entry
    {
      return 2;
    }
    GDALDatasetH inputDataset = GDALOpen( TO8( it->raster->source() ), GA_ReadOnly );
    if( inputDataset == NULL )
    {
      return 2;
    }
    GDALRasterBandH inputRasterBand = GDALGetRasterBand( inputDataset, it->bandNumber );
    if( inputRasterBand == NULL )
    {
      return 2;
    }

    int nodataSuccess;
    double nodataValue = GDALGetRasterNoDataValue( inputRasterBand, &nodataSuccess );

    mInputDatasets.push_back( inputDataset );
    mInputRasterBands.insert( it->ref, inputRasterBand );
    inputScanLineData.insert( it->ref, new QgsRasterMatrix( mNumOutputColumns, 1, new float[mNumOutputColumns], nodataValue ) );
  }

  //open output dataset for writing
  GDALDriverH outputDriver = openOutputDriver();
  if( outputDriver == NULL )
  {
    return 1;
  }
  GDALDatasetH outputDataset = openOutputFile( outputDriver );
  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset, 1 );

  float outputNodataValue = -FLT_MAX;
  GDALSetRasterNoDataValue( outputRasterBand, outputNodataValue );

  float* resultScanLine = ( float * ) CPLMalloc( sizeof( float ) * mNumOutputColumns );

  if( p )
  {
    p->setMaximum( mNumOutputRows );
  }

  QgsRasterMatrix resultMatrix;

  //read / write line by line
  for( int i = 0; i < mNumOutputRows; ++i )
  {
    if( p )
    {
      p->setValue( i );
    }

    if( p && p->wasCanceled() )
    {
      break;
    }

    //fill buffers
    QMap< QString, QgsRasterMatrix* >::iterator bufferIt = inputScanLineData.begin();
    for( ; bufferIt != inputScanLineData.end(); ++bufferIt )
    {
      double sourceTransformation[6];
      GDALRasterBandH sourceRasterBand = mInputRasterBands[bufferIt.key()];
      GDALGetGeoTransform( GDALGetBandDataset( sourceRasterBand ), sourceTransformation );
      //the function readRasterPart calls GDALRasterIO (and ev. does some conversion if raster transformations are not the same)
      readRasterPart( targetGeoTransform, 0, i, mNumOutputColumns, 1, sourceTransformation, sourceRasterBand, bufferIt.value()->data() );
    }

    if( calcNode->calculate( inputScanLineData, resultMatrix ) )
    {
      bool resultIsNumber = resultMatrix.isNumber();
      float* calcData;

      if( resultIsNumber )  //scalar result. Insert number for every pixel
      {
        calcData = new float[mNumOutputColumns];
        for( int j = 0; j < mNumOutputColumns; ++j )
        {
          calcData[j] = resultMatrix.number();
        }
      }
      else //result is real matrix
      {
        calcData = resultMatrix.data();
      }

      //replace all matrix nodata values with output nodatas
      for( int j = 0; j < mNumOutputColumns; ++j )
      {
        if( calcData[j] == resultMatrix.nodataValue() )
        {
          calcData[j] = outputNodataValue;
        }
      }

      //write scanline to the dataset
      if( GDALRasterIO( outputRasterBand, GF_Write, 0, i, mNumOutputColumns, 1, calcData, mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        qWarning( "RasterIO error!" );
      }

      if( resultIsNumber )
      {
        delete[] calcData;
      }
    }

  }

  if( p )
  {
    p->setValue( mNumOutputRows );
  }

  //close datasets and release memory
  delete calcNode;
  QMap< QString, QgsRasterMatrix* >::iterator bufferIt = inputScanLineData.begin();
  for( ; bufferIt != inputScanLineData.end(); ++bufferIt )
  {
    delete bufferIt.value();
  }
  inputScanLineData.clear();

  QVector< GDALDatasetH >::iterator datasetIt = mInputDatasets.begin();
  for( ; datasetIt != mInputDatasets.end(); ++ datasetIt )
  {
    GDALClose( *datasetIt );
  }

  if( p && p->wasCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    GDALDeleteDataset( outputDriver, mOutputFile.toLocal8Bit().data() );
    return 3;
  }
  GDALClose( outputDataset );
  CPLFree( resultScanLine );
  return 0;
}

QgsRasterCalculator::QgsRasterCalculator()
{
}

GDALDriverH QgsRasterCalculator::openOutputDriver()
{
  char **driverMetadata;

  //open driver
  GDALDriverH outputDriver = GDALGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if( outputDriver == NULL )
  {
    return outputDriver; //return NULL, driver does not exist
  }

  driverMetadata = GDALGetMetadata( outputDriver, NULL );
  if( !CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
  {
    return NULL; //driver exist, but it does not support the create operation
  }

  return outputDriver;
}

GDALDatasetH QgsRasterCalculator::openOutputFile( GDALDriverH outputDriver )
{
  //open output file
  char **papszOptions = NULL;
  GDALDatasetH outputDataset = GDALCreate( outputDriver, mOutputFile.toLocal8Bit().data(), mNumOutputColumns, mNumOutputRows, 1, GDT_Float32, papszOptions );
  if( outputDataset == NULL )
  {
    return outputDataset;
  }

  //assign georef information
  double geotransform[6];
  outputGeoTransform( geotransform );
  GDALSetGeoTransform( outputDataset, geotransform );

  return outputDataset;
}

void QgsRasterCalculator::readRasterPart( double* targetGeotransform, int xOffset, int yOffset, int nCols, int nRows, double* sourceTransform, GDALRasterBandH sourceBand, float* rasterBuffer )
{
  //If dataset transform is the same as the requested transform, do a normal GDAL raster io
  if( transformationsEqual( targetGeotransform, sourceTransform ) )
  {
    GDALRasterIO( sourceBand, GF_Read, xOffset, yOffset, nCols, nRows, rasterBuffer, nCols, nRows, GDT_Float32, 0, 0 );
    return;
  }

  //pixel calculation needed because of different raster position / resolution
  int nodataSuccess;
  double nodataValue = GDALGetRasterNoDataValue( sourceBand, &nodataSuccess );
  QgsRectangle targetRect( targetGeotransform[0] + targetGeotransform[1] * xOffset, targetGeotransform[3] + yOffset * targetGeotransform[5] + nRows * targetGeotransform[5]
                           , targetGeotransform[0] + targetGeotransform[1] * xOffset + targetGeotransform[1] * nCols, targetGeotransform[3] + yOffset * targetGeotransform[5] );
  QgsRectangle sourceRect( sourceTransform[0], sourceTransform[3] + GDALGetRasterBandYSize( sourceBand ) * sourceTransform[5],
                           sourceTransform[0] +  GDALGetRasterBandXSize( sourceBand )* sourceTransform[1], sourceTransform[3] );
  QgsRectangle intersection = targetRect.intersect( &sourceRect );

  //no intersection, fill all the pixels with nodata values
  if( intersection.isEmpty() )
  {
    int nPixels = nCols * nRows;
    for( int i = 0; i < nPixels; ++i )
    {
      rasterBuffer[i] = nodataValue;
    }
    return;
  }

  //do raster io in source resolution
  int sourcePixelOffsetXMin = floor(( intersection.xMinimum() - sourceTransform[0] ) / sourceTransform[1] );
  int sourcePixelOffsetXMax = ceil(( intersection.xMaximum() - sourceTransform[0] ) / sourceTransform[1] );
  int nSourcePixelsX = sourcePixelOffsetXMax - sourcePixelOffsetXMin;
  int sourcePixelOffsetYMax = floor(( intersection.yMaximum() - sourceTransform[3] ) / sourceTransform[5] );
  int sourcePixelOffsetYMin = ceil(( intersection.yMinimum() - sourceTransform[3] ) / sourceTransform[5] );
  int nSourcePixelsY = sourcePixelOffsetYMin - sourcePixelOffsetYMax;
  float* sourceRaster = ( float * ) CPLMalloc( sizeof( float ) * nSourcePixelsX * nSourcePixelsY );
  double sourceRasterXMin = sourceRect.xMinimum() + sourcePixelOffsetXMin * sourceTransform[1];
  double sourceRasterYMax = sourceRect.yMaximum() + sourcePixelOffsetYMax * sourceTransform[5];
  GDALRasterIO( sourceBand, GF_Read, sourcePixelOffsetXMin, sourcePixelOffsetYMax, nSourcePixelsX, nSourcePixelsY,
                sourceRaster, nSourcePixelsX, nSourcePixelsY, GDT_Float32, 0, 0 );


  double targetPixelX;
  double targetPixelXMin = targetGeotransform[0] + targetGeotransform[1] * xOffset + targetGeotransform[1] / 2.0;
  double targetPixelY = targetGeotransform[3] + targetGeotransform[5] * yOffset + targetGeotransform[5] / 2.0; //coordinates of current target pixel
  int sourceIndexX, sourceIndexY; //current raster index in  source pixels
  double sx, sy;
  for( int i = 0; i < nRows; ++i )
  {
    targetPixelX = targetPixelXMin;
    for( int j = 0; j < nCols; ++j )
    {
      sx = ( targetPixelX - sourceRasterXMin ) / sourceTransform[1];
      sourceIndexX = sx > 0 ? sx : floor( sx );
      sy = ( targetPixelY - sourceRasterYMax ) / sourceTransform[5];
      sourceIndexY = sy > 0 ? sy : floor( sy );
      if( sourceIndexX >= 0 && sourceIndexX < nSourcePixelsX
          && sourceIndexY >= 0 && sourceIndexY < nSourcePixelsY )
      {
        rasterBuffer[j + i*nRows] = sourceRaster[ sourceIndexX  + nSourcePixelsX * sourceIndexY ];
      }
      else
      {
        rasterBuffer[j + i*j] = nodataValue;
      }
      targetPixelX += targetGeotransform[1];
    }
    targetPixelY += targetGeotransform[5];
  }

  CPLFree( sourceRaster );
  return;
}

bool QgsRasterCalculator::transformationsEqual( double* t1, double* t2 ) const
{
  for( int i = 0; i < 6; ++i )
  {
    if( !doubleNear( t1[i], t2[i], 0.00001 ) )
    {
      return false;
    }
  }
  return true;
}

void QgsRasterCalculator::outputGeoTransform( double* transform ) const
{
  transform[0] = mOutputRectangle.xMinimum();
  transform[1] = mOutputRectangle.width() / mNumOutputColumns;
  transform[2] = 0;
  transform[3] = mOutputRectangle.yMaximum();
  transform[4] = 0;
  transform[5] = -mOutputRectangle.height() / mNumOutputRows;
}


