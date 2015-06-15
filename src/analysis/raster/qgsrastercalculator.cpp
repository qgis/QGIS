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

#include <QProgressDialog>
#include <QFile>

#include <cpl_string.h>
#include <gdalwarper.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x)   (x).toUtf8().constData()
#define TO8F(x)  (x).toUtf8().constData()
#else
#define TO8(x)   (x).toLocal8Bit().constData()
#define TO8F(x)  QFile::encodeName( x ).constData()
#endif

QgsRasterCalculator::QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
    const QgsRectangle& outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries )
    : mFormulaString( formulaString )
    , mOutputFile( outputFile )
    , mOutputFormat( outputFormat )
    , mOutputRectangle( outputExtent )
    , mNumOutputColumns( nOutputColumns )
    , mNumOutputRows( nOutputRows )
    , mRasterEntries( rasterEntries )
{
  //default to first layer's crs
  mOutputCrs = mRasterEntries.at( 0 ).raster->crs();
}

QgsRasterCalculator::QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
    const QgsRectangle& outputExtent, const QgsCoordinateReferenceSystem& outputCrs, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries )
    : mFormulaString( formulaString )
    , mOutputFile( outputFile )
    , mOutputFormat( outputFormat )
    , mOutputRectangle( outputExtent )
    , mOutputCrs( outputCrs )
    , mNumOutputColumns( nOutputColumns )
    , mNumOutputRows( nOutputRows )
    , mRasterEntries( rasterEntries )
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
  if ( !calcNode )
  {
    //error
    return 4;
  }

  QMap< QString, QgsRasterBlock* > inputBlocks;
  QVector<QgsRasterCalculatorEntry>::const_iterator it = mRasterEntries.constBegin();
  for ( ; it != mRasterEntries.constEnd(); ++it )
  {
    if ( !it->raster ) // no raster layer in entry
    {
      return 2;
    }

    QgsRasterBlock* block = 0;
    // if crs transform needed
    if ( it->raster->crs() != mOutputCrs )
    {
      QgsRasterProjector* proj = new QgsRasterProjector();
      proj->setCRS( it->raster->crs(), mOutputCrs );
      proj->setInput( it->raster->dataProvider()->clone() );
      proj->setPrecision( QgsRasterProjector::Exact );

      block = proj->block( it->bandNumber, mOutputRectangle, mNumOutputColumns, mNumOutputRows );
    }
    else
    {
      block = it->raster->dataProvider()->block( it->bandNumber, mOutputRectangle, mNumOutputColumns, mNumOutputRows );
    }
    inputBlocks.insert( it->ref, block );
  }

  //open output dataset for writing
  GDALDriverH outputDriver = openOutputDriver();
  if ( outputDriver == NULL )
  {
    return 1;
  }

  GDALDatasetH outputDataset = openOutputFile( outputDriver );
  GDALSetProjection( outputDataset, mOutputCrs.toWkt().toLocal8Bit().data() );
  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset, 1 );

  float outputNodataValue = -FLT_MAX;
  GDALSetRasterNoDataValue( outputRasterBand, outputNodataValue );

  if ( p )
  {
    p->setMaximum( mNumOutputRows );
  }

  QgsRasterMatrix resultMatrix;
  resultMatrix.setNodataValue( outputNodataValue );

  //read / write line by line
  for ( int i = 0; i < mNumOutputRows; ++i )
  {
    if ( p )
    {
      p->setValue( i );
    }

    if ( p && p->wasCanceled() )
    {
      break;
    }

    if ( calcNode->calculate( inputBlocks, resultMatrix, i ) )
    {
      bool resultIsNumber = resultMatrix.isNumber();
      float* calcData = new float[mNumOutputColumns];

      for ( int j = 0; j < mNumOutputColumns; ++j )
      {
        calcData[j] = ( float )( resultIsNumber ? resultMatrix.number() : resultMatrix.data()[j] );
      }

      //write scanline to the dataset
      if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, mNumOutputColumns, 1, calcData, mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        qWarning( "RasterIO error!" );
      }

      if ( resultIsNumber )
      {
        delete[] calcData;
      }
    }

  }

  if ( p )
  {
    p->setValue( mNumOutputRows );
  }

  //close datasets and release memory
  delete calcNode;
  qDeleteAll( inputBlocks );
  inputBlocks.clear();

  if ( p && p->wasCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    GDALDeleteDataset( outputDriver, TO8F( mOutputFile ) );
    return 3;
  }
  GDALClose( outputDataset );

  return 0;
}

QgsRasterCalculator::QgsRasterCalculator()
    : mNumOutputColumns( 0 )
    , mNumOutputRows( 0 )
{
}

GDALDriverH QgsRasterCalculator::openOutputDriver()
{
  char **driverMetadata;

  //open driver
  GDALDriverH outputDriver = GDALGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if ( outputDriver == NULL )
  {
    return outputDriver; //return NULL, driver does not exist
  }

  driverMetadata = GDALGetMetadata( outputDriver, NULL );
  if ( !CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
  {
    return NULL; //driver exist, but it does not support the create operation
  }

  return outputDriver;
}

GDALDatasetH QgsRasterCalculator::openOutputFile( GDALDriverH outputDriver )
{
  //open output file
  char **papszOptions = NULL;
  GDALDatasetH outputDataset = GDALCreate( outputDriver, TO8F( mOutputFile ), mNumOutputColumns, mNumOutputRows, 1, GDT_Float32, papszOptions );
  if ( outputDataset == NULL )
  {
    return outputDataset;
  }

  //assign georef information
  double geotransform[6];
  outputGeoTransform( geotransform );
  GDALSetGeoTransform( outputDataset, geotransform );

  return outputDataset;
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
