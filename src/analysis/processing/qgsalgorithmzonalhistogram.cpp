/***************************************************************************
                         qgsalgorithmzonalhistogram.cpp
                         ---------------------
    begin                : May, 2018
    copyright            : (C) 2018 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmzonalhistogram.h"
#include "qgsgeos.h"
#include "qgslogger.h"

///@cond PRIVATE

QString QgsZonalHistogramAlgorithm::name() const
{
  return QStringLiteral( "zonalhistogram" );
}

QString QgsZonalHistogramAlgorithm::displayName() const
{
  return QObject::tr( "Zonal histogram" );
}

QStringList QgsZonalHistogramAlgorithm::tags() const
{
  return QObject::tr( "raster,unique,values,count,area,statistics" ).split( ',' );
}

QString QgsZonalHistogramAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsZonalHistogramAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsZonalHistogramAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_VECTOR" ),
                QObject::tr( "Vector layer containing zones" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "COLUMN_PREFIX" ), QObject::tr( "Output column prefix" ), QStringLiteral( "HISTO_" ), false, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output zones" ), QgsProcessing::TypeVectorPolygon ) );
}

QString QgsZonalHistogramAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm appends fields representing counts of each unique value from a raster layer contained within zones defined as polygons." );
}

QgsZonalHistogramAlgorithm *QgsZonalHistogramAlgorithm::createInstance() const
{
  return new QgsZonalHistogramAlgorithm();
}

bool QgsZonalHistogramAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );
  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( mBand );
  mNodataValue = layer->dataProvider()->sourceNoDataValue( mBand );
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = std::abs( layer->rasterUnitsPerPixelX() );
  mRasterUnitsPerPixelY = std::abs( layer->rasterUnitsPerPixelX() );
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();

  return true;
}

QVariantMap QgsZonalHistogramAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{

  std::unique_ptr< QgsFeatureSource > zones( parameterAsSource( parameters, QStringLiteral( "INPUT_VECTOR" ), context ) );
  if ( !zones )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_VECTOR" ) ) );

  long count = zones->featureCount();
  double step = count > 0 ? 100.0 / count : 1;
  long current = 0;

  QList< double > uniqueValues;
  QMap< QgsFeatureId, QHash< double, qgssize > > featuresUniqueValues;

  // First loop through the zones to build up a list of unique values across all zones to determine sink fields list
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  if ( zones->sourceCrs() != mCrs )
  {
    request.setDestinationCrs( mCrs, context.transformContext() );
  }
  QgsFeatureIterator it = zones->getFeatures( request );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }
    feedback->setProgress( current * step );

    if ( !f.hasGeometry() )
    {
      current++;
      continue;
    }

    QgsGeometry featureGeometry = f.geometry();
    QgsRectangle intersectRect = featureGeometry.boundingBox().intersect( &mExtent );
    if ( intersectRect.isEmpty() )
    {
      current++;
      continue;
    }

    int offsetX, offsetY, nCellsX, nCellsY;
    // Get offset in pixels in x- and y- direction
    offsetX = ( int )( ( intersectRect.xMinimum() - mExtent.xMinimum() ) / mRasterUnitsPerPixelX );
    offsetY = ( int )( ( mExtent.yMaximum() - intersectRect.yMaximum() ) / mRasterUnitsPerPixelY );

    int maxColumn = ( int )( ( intersectRect.xMaximum() - mExtent.xMinimum() ) / mRasterUnitsPerPixelX ) + 1;
    int maxRow = ( int )( ( mExtent.yMaximum() - intersectRect.yMinimum() ) / mRasterUnitsPerPixelY ) + 1;

    nCellsX = maxColumn - offsetX;
    nCellsY = maxRow - offsetY;

    // Avoid access to cells outside of the raster (may occur because of rounding)
    if ( ( offsetX + nCellsX ) > mNbCellsXProvider )
    {
      nCellsX = mNbCellsXProvider - offsetX;
    }
    if ( ( offsetY + nCellsY ) > mNbCellsYProvider )
    {
      nCellsY = mNbCellsYProvider - offsetY;
    }

    QHash< double, qgssize > fUniqueValues;
    middlePoints( featureGeometry, offsetX, offsetY, nCellsX, nCellsY, fUniqueValues );

    if ( fUniqueValues.count() < 1 )
    {
      // The cell resolution is probably larger than the polygon area. We switch to slower precise pixel - polygon intersection in this case
      preciseIntersection( featureGeometry, offsetX, offsetY, nCellsX, nCellsY, fUniqueValues );
    }

    for ( auto it = fUniqueValues.constBegin(); it != fUniqueValues.constEnd(); ++it )
    {
      if ( uniqueValues.indexOf( it.key() ) == -1 )
      {
        uniqueValues << it.key();
      }
      featuresUniqueValues[f.id()][it.key()] += it.value();
    }

    current++;
  }

  std::sort( uniqueValues.begin(), uniqueValues.end() );

  QString fieldPrefix = parameterAsString( parameters, QStringLiteral( "COLUMN_PREFIX" ), context );
  QgsFields newFields;
  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    newFields.append( QgsField( QStringLiteral( "%1%2" ).arg( fieldPrefix ).arg( mHasNoDataValue && *it == mNodataValue ? QStringLiteral( "NODATA" ) : QString::number( *it ) ), QVariant::LongLong, QString(), -1, 0 ) );
  }
  QgsFields fields = QgsProcessingUtils::combineFields( zones->fields(), newFields );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          zones->wkbType(), zones->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  it = zones->getFeatures( QgsFeatureRequest() );
  while ( it.nextFeature( f ) )
  {
    QgsAttributes attributes = f.attributes();
    QHash< double, qgssize > fUniqueValues = featuresUniqueValues.value( f.id() );
    for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
    {
      attributes += fUniqueValues.value( *it, 0 );
    }

    QgsFeature outputFeature;
    outputFeature.setGeometry( f.geometry() );
    outputFeature.setAttributes( attributes );

    sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

void QgsZonalHistogramAlgorithm::middlePoints( const QgsGeometry &poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY, QHash< double, qgssize > &uniqueValues )
{
  double cellCenterX, cellCenterY;

  cellCenterY = mExtent.yMaximum() - pixelOffsetY * mRasterUnitsPerPixelY - mRasterUnitsPerPixelY / 2;

  geos::unique_ptr polyGeos( QgsGeos::asGeos( poly ) );
  if ( !polyGeos )
  {
    return;
  }

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  geos::prepared_unique_ptr polyGeosPrepared( GEOSPrepare_r( geosctxt, polyGeos.get() ) );
  if ( !polyGeosPrepared )
  {
    return;
  }

  GEOSCoordSequence *cellCenterCoords = nullptr;
  geos::unique_ptr currentCellCenter;

  QgsRectangle blockRect( mExtent.xMinimum() + pixelOffsetX * mRasterUnitsPerPixelX,
                          mExtent.yMaximum() - pixelOffsetY * mRasterUnitsPerPixelY - nCellsY * mRasterUnitsPerPixelY,
                          mExtent.xMinimum() + pixelOffsetX * mRasterUnitsPerPixelX + nCellsX * mRasterUnitsPerPixelX,
                          mExtent.yMaximum() - pixelOffsetY * mRasterUnitsPerPixelY );
  std::unique_ptr< QgsRasterBlock > block( mInterface->block( mBand, blockRect, nCellsX, nCellsY ) );
  for ( int i = 0; i < nCellsY; ++i )
  {
    cellCenterX = mExtent.xMinimum() + pixelOffsetX * mRasterUnitsPerPixelX + mRasterUnitsPerPixelX / 2;
    for ( int j = 0; j < nCellsX; ++j )
    {
      if ( !std::isnan( block->value( i, j ) ) )
      {
        cellCenterCoords = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
        GEOSCoordSeq_setX_r( geosctxt, cellCenterCoords, 0, cellCenterX );
        GEOSCoordSeq_setY_r( geosctxt, cellCenterCoords, 0, cellCenterY );
        currentCellCenter.reset( GEOSGeom_createPoint_r( geosctxt, cellCenterCoords ) );
        if ( GEOSPreparedContains_r( geosctxt, polyGeosPrepared.get(), currentCellCenter.get() ) )
        {
          uniqueValues[block->value( i, j )]++;
        }
      }
      cellCenterX += mRasterUnitsPerPixelX;
    }
    cellCenterY -= mRasterUnitsPerPixelY;
  }
}

void QgsZonalHistogramAlgorithm::preciseIntersection( const QgsGeometry &poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY, QHash< double, qgssize > &uniqueValues )
{
  double currentY = mExtent.yMaximum() - pixelOffsetY * mRasterUnitsPerPixelY - mRasterUnitsPerPixelY / 2;
  QgsGeometry pixelRectGeometry;

  double hCellSizeX = mRasterUnitsPerPixelX / 2.0;
  double hCellSizeY = mRasterUnitsPerPixelY / 2.0;

  QgsRectangle blockRect( mExtent.xMinimum() + pixelOffsetX * mRasterUnitsPerPixelX,
                          mExtent.yMaximum() - pixelOffsetY * mRasterUnitsPerPixelY - nCellsY * mRasterUnitsPerPixelY,
                          mExtent.xMinimum() + pixelOffsetX * mRasterUnitsPerPixelX + nCellsX * mRasterUnitsPerPixelX,
                          mExtent.yMaximum() - pixelOffsetY * mRasterUnitsPerPixelY );
  std::unique_ptr< QgsRasterBlock > block( mInterface->block( mBand, blockRect, nCellsX, nCellsY ) );
  for ( int i = 0; i < nCellsY; ++i )
  {
    double currentX = mExtent.xMinimum() + mRasterUnitsPerPixelX / 2.0 + pixelOffsetX * mRasterUnitsPerPixelX;
    for ( int j = 0; j < nCellsX; ++j )
    {
      if ( !std::isnan( block->value( i, j ) ) )
      {
        pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - hCellSizeX, currentY - hCellSizeY, currentX + hCellSizeX, currentY + hCellSizeY ) );
        if ( !pixelRectGeometry.isNull() )
        {
          //intersection
          QgsGeometry intersectGeometry = pixelRectGeometry.intersection( poly );
          if ( !intersectGeometry.isNull() )
          {
            double intersectionArea = intersectGeometry.area();
            if ( intersectionArea >= 0.0 )
            {
              uniqueValues[block->value( i, j )]++;
            }
          }
          pixelRectGeometry = QgsGeometry();
        }
      }
      currentX += mRasterUnitsPerPixelX;
    }
    currentY -= mRasterUnitsPerPixelY;
  }
}

///@endcond



