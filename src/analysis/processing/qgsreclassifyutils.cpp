/***************************************************************************
                         qgsreclassifyutils.cpp
                         ---------------------
    begin                : June, 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsreclassifyutils.h"

#include "qgis.h"
#include "qgsprocessingfeedback.h"
#include "qgsrasterblock.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasteriterator.h"

///@cond PRIVATE

void QgsReclassifyUtils::reportClasses( const QVector<QgsReclassifyUtils::RasterClass> &classes, QgsProcessingFeedback *feedback )
{
  int i = 1;
  feedback->pushInfo( QObject::tr( "Using classes:" ) );
  for ( const RasterClass &c : classes )
  {
    feedback->pushInfo( u" %1) %2 %3 %4"_s.arg( i ).arg( c.asText() ).arg( QChar( 0x2192 ) ).arg( c.value ) );
    i++;
  }
}

void QgsReclassifyUtils::checkForOverlaps( const QVector<QgsReclassifyUtils::RasterClass> &classes, QgsProcessingFeedback *feedback )
{
  // test each class against each other class
  for ( int i = 0; i < classes.count() - 1; i++ )
  {
    for ( int j = i + 1; j < classes.count(); j++ )
    {
      const QgsReclassifyUtils::RasterClass &class1 = classes.at( i );
      const QgsReclassifyUtils::RasterClass &class2 = classes.at( j );
      if ( class1.overlaps( class2 ) )
      {
        feedback->reportError( QObject::tr( "Warning: Class %1 (%2) overlaps with class %3 (%4)" ).arg( i + 1 ).arg( class1.asText() ).arg( j + 1 ).arg( class2.asText() ) );
      }
    }
  }
}

void QgsReclassifyUtils::reclassify( const QVector<QgsReclassifyUtils::RasterClass> &classes, QgsRasterInterface *sourceRaster, int band, const QgsRectangle &extent, int sourceWidthPixels, int sourceHeightPixels, std::unique_ptr<QgsRasterDataProvider> destinationRaster, double destNoDataValue, bool useNoDataForMissingValues, QgsProcessingFeedback *feedback )
{
  QgsRasterIterator iter( sourceRaster );
  iter.startRasterRead( band, sourceWidthPixels, sourceHeightPixels, extent );

  const bool hasReportsDuringClose = destinationRaster->hasReportsDuringClose();
  const double maxProgressDuringBlockWriting = hasReportsDuringClose ? 50.0 : 100.0;

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  destinationRaster->setEditable( true );
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  bool reclassed = false;
  bool isNoData = false;
  while ( iter.readNextRasterPart( band, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( band ) );
    if ( feedback && feedback->isCanceled() )
      break;
    auto reclassifiedBlock = std::make_unique<QgsRasterBlock>( destinationRaster->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        if ( isNoData )
          reclassifiedBlock->setValue( row, column, destNoDataValue );
        else
        {
          const double newValue = reclassifyValue( classes, value, reclassed );
          if ( reclassed )
            reclassifiedBlock->setValue( row, column, newValue );
          else
            reclassifiedBlock->setValue( row, column, useNoDataForMissingValues ? destNoDataValue : value );
        }
      }
    }
    if ( !destinationRaster->writeBlock( reclassifiedBlock.get(), 1, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationRaster->error().summary() ) );
    }
  }
  destinationRaster->setEditable( false );

  if ( feedback && hasReportsDuringClose )
  {
    std::unique_ptr<QgsFeedback> scaledFeedback( QgsFeedback::createScaledFeedback( feedback, maxProgressDuringBlockWriting, 100.0 ) );
    if ( !destinationRaster->closeWithProgress( scaledFeedback.get() ) )
    {
      if ( feedback->isCanceled() )
        return;
      throw QgsProcessingException( QObject::tr( "Could not write raster dataset" ) );
    }
  }
}


///@endcond
