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

QString QgsVectorizeAlgorithmBase::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsVectorizeAlgorithmBase::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsVectorizeAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ),
                QObject::tr( "Field name" ), QStringLiteral( "VALUE" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), outputName(), outputType() ) );
}

bool QgsVectorizeAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
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

QVariantMap QgsVectorizeAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  QgsFields fields;
  fields.append( QgsField( fieldName, QVariant::Double, QString(), 20, 8 ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, sinkType(), mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );


  const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mNbCellsXProvider, mNbCellsYProvider, mExtent );

  const int nbBlocksWidth = static_cast< int >( std::ceil( 1.0 * mNbCellsXProvider / maxWidth ) );
  const int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mNbCellsYProvider / maxHeight ) );
  const int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr< QgsRasterBlock > rasterBlock;
  QgsRectangle blockExtent;
  bool isNoData = false;
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
        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        if ( !isNoData )
        {
          const QgsGeometry pixelRectGeometry = createGeometryForPixel( currentX, currentY, mRasterUnitsPerPixelX, mRasterUnitsPerPixelY );

          QgsFeature f;
          f.setGeometry( pixelRectGeometry );
          f.setAttributes( QgsAttributes() << value );
          if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
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

//
// QgsRasterPixelsToPolygonsAlgorithm
//

QString QgsRasterPixelsToPolygonsAlgorithm::name() const
{
  return QStringLiteral( "pixelstopolygons" );
}

QString QgsRasterPixelsToPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Raster pixels to polygons" );
}

QStringList QgsRasterPixelsToPolygonsAlgorithm::tags() const
{
  return QObject::tr( "vectorize,polygonize,raster,convert,pixels" ).split( ',' );
}

QString QgsRasterPixelsToPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm converts a raster layer to a vector layer, by creating polygon features "
                      "for each individual pixel's extent in the raster layer.\n\n"
                      "Any nodata pixels are skipped in the output." );
}

QString QgsRasterPixelsToPolygonsAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer of polygons corresponding to each pixel in a raster layer." );
}

QgsRasterPixelsToPolygonsAlgorithm *QgsRasterPixelsToPolygonsAlgorithm::createInstance() const
{
  return new QgsRasterPixelsToPolygonsAlgorithm();
}

QString QgsRasterPixelsToPolygonsAlgorithm::outputName() const
{
  return QObject::tr( "Vector polygons" );
}

QgsProcessing::SourceType QgsRasterPixelsToPolygonsAlgorithm::outputType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsRasterPixelsToPolygonsAlgorithm::sinkType() const
{
  return QgsWkbTypes::Polygon;
}

QgsGeometry QgsRasterPixelsToPolygonsAlgorithm::createGeometryForPixel( double centerX, double centerY, double pixelWidthX, double pixelWidthY ) const
{
  const double hCellSizeX = pixelWidthX / 2.0;
  const double hCellSizeY = pixelWidthY / 2.0;
  return QgsGeometry::fromRect( QgsRectangle( centerX - hCellSizeX, centerY - hCellSizeY, centerX + hCellSizeX, centerY + hCellSizeY ) );
}


//
// QgsRasterPixelsToPointsAlgorithm
//

QString QgsRasterPixelsToPointsAlgorithm::name() const
{
  return QStringLiteral( "pixelstopoints" );
}

QString QgsRasterPixelsToPointsAlgorithm::displayName() const
{
  return QObject::tr( "Raster pixels to points" );
}

QStringList QgsRasterPixelsToPointsAlgorithm::tags() const
{
  return QObject::tr( "vectorize,polygonize,raster,convert,pixels,centers" ).split( ',' );
}

QString QgsRasterPixelsToPointsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm converts a raster layer to a vector layer, by creating point features "
                      "for each individual pixel's center in the raster layer.\n\n"
                      "Any nodata pixels are skipped in the output." );
}

QString QgsRasterPixelsToPointsAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer of points corresponding to each pixel in a raster layer." );
}

QgsRasterPixelsToPointsAlgorithm *QgsRasterPixelsToPointsAlgorithm::createInstance() const
{
  return new QgsRasterPixelsToPointsAlgorithm();
}

QString QgsRasterPixelsToPointsAlgorithm::outputName() const
{
  return QObject::tr( "Vector points" );
}

QgsProcessing::SourceType QgsRasterPixelsToPointsAlgorithm::outputType() const
{
  return QgsProcessing::TypeVectorPoint;
}

QgsWkbTypes::Type QgsRasterPixelsToPointsAlgorithm::sinkType() const
{
  return QgsWkbTypes::Point;
}

QgsGeometry QgsRasterPixelsToPointsAlgorithm::createGeometryForPixel( double centerX, double centerY, double, double ) const
{
  return QgsGeometry( new QgsPoint( centerX, centerY ) );
}

///@endcond


