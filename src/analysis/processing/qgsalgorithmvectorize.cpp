/***************************************************************************
                         qgsalgorithmvectorize.cpp
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

#include "qgsalgorithmvectorize.h"
#include "qgis.h"
#include "qgsprocessing.h"

///@cond PRIVATE

QString QgsVectorizeAlgorithm::name() const
{
  return QStringLiteral( "pixelstopolygons" );
}

QString QgsVectorizeAlgorithm::displayName() const
{
  return QObject::tr( "Raster pixels to polygons" );
}

QStringList QgsVectorizeAlgorithm::tags() const
{
  return QObject::tr( "vectorize,polygonize,raster,convert,pixels" ).split( ',' );
}

QString QgsVectorizeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm converts a raster layer to a vector layer, by creating polygon features for each individual pixel in the raster layer." );
}

QgsVectorizeAlgorithm *QgsVectorizeAlgorithm::createInstance() const
{
  return new QgsVectorizeAlgorithm();
}

QString QgsVectorizeAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsVectorizeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsVectorizeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ),
                QObject::tr( "Field name" ), QStringLiteral( "VALUE" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Vectorized layer" ), QgsProcessing::TypeVectorPolygon ) );
}

bool QgsVectorizeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for RASTER_BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = std::abs( layer->rasterUnitsPerPixelX() );
  mRasterUnitsPerPixelY = std::abs( layer->rasterUnitsPerPixelY() );
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();
  return true;
}

QVariantMap QgsVectorizeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  QgsFields fields;
  fields.append( QgsField( fieldName, QVariant::Double, QString(), 20, 8 ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );


  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mNbCellsXProvider, mNbCellsYProvider, mExtent );

  int nbBlocksWidth = static_cast< int >( std::ceil( 1.0 * mNbCellsXProvider / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mNbCellsYProvider / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  double hCellSizeX = mRasterUnitsPerPixelX / 2.0;
  double hCellSizeY = mRasterUnitsPerPixelY / 2.0;

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr< QgsRasterBlock > rasterBlock;
  QgsRectangle blockExtent;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop, &blockExtent ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    if ( feedback && feedback->isCanceled() )
      break;

    double currentY = blockExtent.yMaximum() - 0.5 * mRasterUnitsPerPixelY;

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      double currentX = blockExtent.xMinimum() + 0.5 * mRasterUnitsPerPixelX;

      for ( int column = 0; column < iterCols; column++ )
      {
        if ( !rasterBlock->isNoData( row, column ) )
        {

          QgsGeometry pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - hCellSizeX, currentY - hCellSizeY, currentX + hCellSizeX, currentY + hCellSizeY ) );

          double value = rasterBlock->value( row, column );

          QgsFeature f;
          f.setGeometry( pixelRectGeometry );
          f.setAttributes( QgsAttributes() << value );
          sink->addFeature( f, QgsFeatureSink::FastInsert );
        }
        currentX += mRasterUnitsPerPixelX;
      }
      currentY -= mRasterUnitsPerPixelY;
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond


