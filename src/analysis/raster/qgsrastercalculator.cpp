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

#include "qgsgdalutils.h"
#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasterlayer.h"
#include "qgsrastermatrix.h"
#include "qgsrasterprojector.h"
#include "qgsfeedback.h"
#include "qgsogrutils.h"

#include <QFile>

#include <cpl_string.h>
#include <gdalwarper.h>

QgsRasterCalculator::QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
    const QgsRectangle &outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries )
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

QgsRasterCalculator::QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
    const QgsRectangle &outputExtent, const QgsCoordinateReferenceSystem &outputCrs, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries )
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

QgsRasterCalculator::Result QgsRasterCalculator::processCalculation( QgsFeedback *feedback )
{
  mLastError.clear();

  //prepare search string / tree
  std::unique_ptr< QgsRasterCalcNode > calcNode( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );
  if ( !calcNode )
  {
    //error
    return ParserError;
  }

  // Check input layers and bands
  for ( const auto &entry : qgis::as_const( mRasterEntries ) )
  {
    if ( !entry.raster ) // no raster layer in entry
    {
      mLastError = QObject::tr( "No raster layer for entry %1" ).arg( entry.ref );
      return InputLayerError;
    }
    if ( entry.bandNumber <= 0 || entry.bandNumber > entry.raster->bandCount() )
    {
      mLastError = QObject::tr( "Band number %1 is not valid for entry %2" ).arg( entry.bandNumber ).arg( entry.ref );
      return BandError;
    }
  }

  //open output dataset for writing
  GDALDriverH outputDriver = openOutputDriver();
  if ( !outputDriver )
  {
    mLastError = QObject::tr( "Could not obtain driver for %1" ).arg( mOutputFormat );
    return CreateOutputError;
  }

  gdal::dataset_unique_ptr outputDataset( openOutputFile( outputDriver ) );
  if ( !outputDataset )
  {
    mLastError = QObject::tr( "Could not create output %1" ).arg( mOutputFile );
    return CreateOutputError;
  }

  GDALSetProjection( outputDataset.get(), mOutputCrs.toWkt().toLocal8Bit().data() );
  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset.get(), 1 );

  float outputNodataValue = -FLT_MAX;
  GDALSetRasterNoDataValue( outputRasterBand, outputNodataValue );


  // If we need to read the raster as a whole
  bool requiresMatrix = ! calcNode->findNodes( QgsRasterCalcNode::Type::tMatrix ).isEmpty();


  if ( ! requiresMatrix )
  {
    // Map of raster names -> blocks
    std::map<QString, std::unique_ptr<QgsRasterBlock>> inputBlocks;
    std::map<QString, QgsRasterCalculatorEntry> uniqueRasterEntries;
    for ( const auto &r : calcNode->findNodes( QgsRasterCalcNode::Type::tRasterRef ) )
    {
      QString layerRef( r->toString().remove( 0, 1 ) );
      layerRef.chop( 1 );
      if ( ! inputBlocks.count( layerRef ) )
      {
        for ( const auto &ref : mRasterEntries )
        {
          if ( ref.ref == layerRef )
          {
            uniqueRasterEntries[layerRef] = ref;
            inputBlocks[layerRef ] = qgis::make_unique<QgsRasterBlock>();
          }
        }
      }
    }

    //read / write line by line
    QMap<QString, QgsRasterBlock * > _rasterData;
    // Cast to float
    std::vector<float> castedResult;
    castedResult.reserve( static_cast<size_t>( mNumOutputColumns ) );
    auto rowHeight = mOutputRectangle.height() / mNumOutputRows;
    for ( size_t row = 0; row < mNumOutputRows; ++row )
    {
      if ( feedback )
      {
        feedback->setProgress( 100.0 * static_cast< double >( row ) / mNumOutputRows );
      }

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      // Read one row
      QgsRectangle rect( mOutputRectangle );
      rect.setYMaximum( rect.yMaximum() - rowHeight * row );
      rect.setYMinimum( rect.yMaximum() - rowHeight );

      // Read blocks
      for ( auto &layerRef : inputBlocks )
      {
        QgsRasterCalculatorEntry ref = uniqueRasterEntries[layerRef.first];
        if ( uniqueRasterEntries[layerRef.first].raster->crs() != mOutputCrs )
        {
          QgsRasterProjector proj;
          proj.setCrs( ref.raster->crs(), mOutputCrs );
          proj.setInput( ref.raster->dataProvider() );
          proj.setPrecision( QgsRasterProjector::Exact );
          layerRef.second.reset( proj.block( ref.bandNumber, rect, mNumOutputColumns, 1 ) );
        }
        else
        {
          inputBlocks[layerRef.first].reset( ref.raster->dataProvider()->block( ref.bandNumber, rect, mNumOutputColumns, 1 ) );
        }
      }

      QgsRasterMatrix resultMatrix;
      resultMatrix.setNodataValue( outputNodataValue );

      _rasterData.clear();
      for ( const auto &layerRef : inputBlocks )
      {
        _rasterData.insert( layerRef.first, inputBlocks[layerRef.first].get() );
        //for ( int i = 0; i < mNumOutputColumns; i++ )
        //  qDebug() << "Input: " << row << i << " = " << inputBlocks[layerRef.first]->value(0, i);
      }

      if ( calcNode->calculate( _rasterData, resultMatrix, 0 ) )
      {
        //write scanline to the dataset
        for ( size_t i = 0; i < static_cast<size_t>( mNumOutputColumns ); i++ )
        {
          castedResult[i] = static_cast<float>( resultMatrix.data()[i] );
          // qDebug() << "Calculated: " << row << i << " = " << castedResult[i];
        }
        if ( GDALRasterIO( outputRasterBand, GF_Write, 0, row, mNumOutputColumns, 1, castedResult.data(), mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
        {
          QgsDebugMsg( QStringLiteral( "RasterIO error!" ) );
        }
      }
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 );
    }


  }
  else
  {
    QMap< QString, QgsRasterBlock * > inputBlocks;
    QVector<QgsRasterCalculatorEntry>::const_iterator it = mRasterEntries.constBegin();
    for ( ; it != mRasterEntries.constEnd(); ++it )
    {

      std::unique_ptr< QgsRasterBlock > block;
      // if crs transform needed
      if ( it->raster->crs() != mOutputCrs )
      {
        QgsRasterProjector proj;
        proj.setCrs( it->raster->crs(), mOutputCrs );
        proj.setInput( it->raster->dataProvider() );
        proj.setPrecision( QgsRasterProjector::Exact );

        QgsRasterBlockFeedback *rasterBlockFeedback = new QgsRasterBlockFeedback();
        QObject::connect( feedback, &QgsFeedback::canceled, rasterBlockFeedback, &QgsRasterBlockFeedback::cancel );
        block.reset( proj.block( it->bandNumber, mOutputRectangle, mNumOutputColumns, mNumOutputRows, rasterBlockFeedback ) );
        if ( rasterBlockFeedback->isCanceled() )
        {
          qDeleteAll( inputBlocks );
          return Canceled;
        }
      }
      else
      {
        block.reset( it->raster->dataProvider()->block( it->bandNumber, mOutputRectangle, mNumOutputColumns, mNumOutputRows ) );
      }
      if ( block->isEmpty() )
      {
        mLastError = QObject::tr( "Could not allocate required memory for %1" ).arg( it->ref );
        qDeleteAll( inputBlocks );
        return MemoryError;
      }
      inputBlocks.insert( it->ref, block.release() );
    }

    QgsRasterMatrix resultMatrix;
    resultMatrix.setNodataValue( outputNodataValue );

    //read / write line by line
    for ( int i = 0; i < mNumOutputRows; ++i )
    {
      if ( feedback )
      {
        feedback->setProgress( 100.0 * static_cast< double >( i ) / mNumOutputRows );
      }

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( calcNode->calculate( inputBlocks, resultMatrix, i ) )
      {
        bool resultIsNumber = resultMatrix.isNumber();
        float *calcData = new float[mNumOutputColumns];

        for ( int j = 0; j < mNumOutputColumns; ++j )
        {
          calcData[j] = ( float )( resultIsNumber ? resultMatrix.number() : resultMatrix.data()[j] );
        }

        //write scanline to the dataset
        if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, mNumOutputColumns, 1, calcData, mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
        {
          QgsDebugMsg( QStringLiteral( "RasterIO error!" ) );
        }

        delete[] calcData;
      }

    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 );
    }

    //close datasets and release memory
    calcNode.reset();
    qDeleteAll( inputBlocks );
    inputBlocks.clear();

  }


  if ( feedback && feedback->isCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
    return Canceled;
  }
  return Success;
}

GDALDriverH QgsRasterCalculator::openOutputDriver()
{
  //open driver
  GDALDriverH outputDriver = GDALGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if ( !outputDriver )
  {
    return outputDriver; //return nullptr, driver does not exist
  }

  if ( !QgsGdalUtils::supportsRasterCreate( outputDriver ) )
  {
    return nullptr; //driver exist, but it does not support the create operation
  }

  return outputDriver;
}

gdal::dataset_unique_ptr QgsRasterCalculator::openOutputFile( GDALDriverH outputDriver )
{
  //open output file
  char **papszOptions = nullptr;
  gdal::dataset_unique_ptr outputDataset( GDALCreate( outputDriver, mOutputFile.toUtf8().constData(), mNumOutputColumns, mNumOutputRows, 1, GDT_Float32, papszOptions ) );
  if ( !outputDataset )
  {
    return nullptr;
  }

  //assign georef information
  double geotransform[6];
  outputGeoTransform( geotransform );
  GDALSetGeoTransform( outputDataset.get(), geotransform );

  return outputDataset;
}

void QgsRasterCalculator::outputGeoTransform( double *transform ) const
{
  transform[0] = mOutputRectangle.xMinimum();
  transform[1] = mOutputRectangle.width() / mNumOutputColumns;
  transform[2] = 0;
  transform[3] = mOutputRectangle.yMaximum();
  transform[4] = 0;
  transform[5] = -mOutputRectangle.height() / mNumOutputRows;
}

QString QgsRasterCalculator::lastError() const
{
  return mLastError;
}
