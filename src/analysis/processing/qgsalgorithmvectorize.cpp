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
  return u"vectorcreation"_s;
}

void QgsVectorizeAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_RASTER"_s, QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"RASTER_BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT_RASTER"_s ) );
  addParameter( new QgsProcessingParameterString( u"FIELD_NAME"_s, QObject::tr( "Field name" ), u"VALUE"_s ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, outputName(), outputType() ) );
}

bool QgsVectorizeAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT_RASTER"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_RASTER"_s ) );

  mBand = parameterAsInt( parameters, u"RASTER_BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for RASTER_BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

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
  const QString fieldName = parameterAsString( parameters, u"FIELD_NAME"_s, context );
  QgsFields fields;
  fields.append( QgsField( fieldName, QMetaType::Type::Double, QString(), 20, 8 ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, sinkType(), mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mNbCellsXProvider, mNbCellsYProvider, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  QgsRectangle blockExtent;
  bool isNoData = false;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop, &blockExtent ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * iter.progress( mBand ) );
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
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
        currentX += mRasterUnitsPerPixelX;
      }
      currentY -= mRasterUnitsPerPixelY;
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

//
// QgsRasterPixelsToPolygonsAlgorithm
//

QString QgsRasterPixelsToPolygonsAlgorithm::name() const
{
  return u"pixelstopolygons"_s;
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
                      "Any NoData pixels are skipped in the output." );
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

Qgis::ProcessingSourceType QgsRasterPixelsToPolygonsAlgorithm::outputType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsRasterPixelsToPolygonsAlgorithm::sinkType() const
{
  return Qgis::WkbType::Polygon;
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
  return u"pixelstopoints"_s;
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
                      "Any NoData pixels are skipped in the output." );
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

Qgis::ProcessingSourceType QgsRasterPixelsToPointsAlgorithm::outputType() const
{
  return Qgis::ProcessingSourceType::VectorPoint;
}

Qgis::WkbType QgsRasterPixelsToPointsAlgorithm::sinkType() const
{
  return Qgis::WkbType::Point;
}

QgsGeometry QgsRasterPixelsToPointsAlgorithm::createGeometryForPixel( double centerX, double centerY, double, double ) const
{
  return QgsGeometry( new QgsPoint( centerX, centerY ) );
}

///@endcond
