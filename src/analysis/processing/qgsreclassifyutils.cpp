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
#include "qgsrasterinterface.h"
#include "qgsrasteriterator.h"
#include "qgsrasterblock.h"
#include "qgsprocessingfeedback.h"
#include "qgsrasterdataprovider.h"

#include "qgis.h"

///@cond PRIVATE

void QgsReclassifyUtils::reportClasses( const QVector<QgsReclassifyUtils::RasterClass> &classes, QgsProcessingFeedback *feedback )
{
  int i = 1;
  feedback->pushInfo( QObject::tr( "Using classes:" ) );
  for ( const RasterClass &c : classes )
  {
    feedback->pushInfo( QStringLiteral( " %1) %2 %3 %4" ).arg( i )
                        .arg( c.asText() )
                        .arg( QChar( 0x2192 ) )
                        .arg( c.value ) );
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
        feedback->reportError( QObject::tr( "Warning: Class %1 (%2) overlaps with class %3 (%4)" ).arg( i + 1 )
                               .arg( class1.asText() )
                               .arg( j + 1 )
                               .arg( class2.asText() ) );
      }
    }
  }
}

void QgsReclassifyUtils::reclassify( const QVector<QgsReclassifyUtils::RasterClass> &classes, QgsRasterInterface *sourceRaster, int band,
                                     const QgsRectangle &extent, int sourceWidthPixels, int sourceHeightPixels,
                                     QgsRasterDataProvider *destinationRaster, double destNoDataValue, bool useNoDataForMissingValues,
                                     QgsProcessingFeedback *feedback )
{
  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;

  QgsRasterIterator iter( sourceRaster );
  iter.startRasterRead( band, sourceWidthPixels, sourceHeightPixels, extent );

  int nbBlocksWidth = static_cast< int >( std::ceil( 1.0 * sourceWidthPixels / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * sourceHeightPixels / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  destinationRaster->setEditable( true );
  std::unique_ptr< QgsRasterBlock > rasterBlock;
  bool reclassed = false;
  while ( iter.readNextRasterPart( band, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    if ( feedback && feedback->isCanceled() )
      break;
    std::unique_ptr< QgsRasterBlock > reclassifiedBlock = qgis::make_unique< QgsRasterBlock >( destinationRaster->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( rasterBlock->isNoData( row, column ) )
          reclassifiedBlock->setValue( row, column, destNoDataValue );
        else
        {
          double value = rasterBlock->value( row, column );
          double newValue = reclassifyValue( classes, value, reclassed );
          if ( reclassed )
            reclassifiedBlock->setValue( row, column, newValue );
          else
            reclassifiedBlock->setValue( row, column, useNoDataForMissingValues ? destNoDataValue : value );
        }
      }
    }
    destinationRaster->writeBlock( reclassifiedBlock.get(), 1, iterLeft, iterTop );
  }
  destinationRaster->setEditable( false );
}



///@endcond



