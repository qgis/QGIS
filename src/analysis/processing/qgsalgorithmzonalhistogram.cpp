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
#include "qgsrasteranalysisutils.h"
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
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mRasterBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( mRasterBand );
  mNodataValue = layer->dataProvider()->sourceNoDataValue( mRasterBand );
  mRasterInterface.reset( layer->dataProvider()->clone() );
  mRasterExtent = layer->extent();
  mCrs = layer->crs();
  mCellSizeX = std::abs( layer->rasterUnitsPerPixelX() );
  mCellSizeY = std::abs( layer->rasterUnitsPerPixelX() );
  mNbCellsXProvider = mRasterInterface->xSize();
  mNbCellsYProvider = mRasterInterface->ySize();

  return true;
}

QVariantMap QgsZonalHistogramAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{

  std::unique_ptr< QgsFeatureSource > zones( parameterAsSource( parameters, QStringLiteral( "INPUT_VECTOR" ), context ) );
  if ( !zones )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_VECTOR" ) ) );

  const long count = zones->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  long current = 0;

  QList< double > uniqueValues;
  QMap< QgsFeatureId, QHash< double, qgssize > > featuresUniqueValues;

  // First loop through the zones to build up a list of unique values across all zones to determine sink fields list
  QgsFeatureRequest request;
  request.setNoAttributes();
  if ( zones->sourceCrs() != mCrs )
  {
    request.setDestinationCrs( mCrs, context.transformContext() );
  }
  QgsFeatureIterator it = zones->getFeatures( request );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    feedback->setProgress( current * step );

    if ( !f.hasGeometry() )
    {
      current++;
      continue;
    }

    const QgsGeometry featureGeometry = f.geometry();
    const QgsRectangle featureRect = featureGeometry.boundingBox().intersect( mRasterExtent );
    if ( featureRect.isEmpty() )
    {
      current++;
      continue;
    }

    int nCellsX, nCellsY;
    QgsRectangle rasterBlockExtent;
    QgsRasterAnalysisUtils::cellInfoForBBox( mRasterExtent, featureRect, mCellSizeX, mCellSizeY, nCellsX, nCellsY, mNbCellsXProvider, mNbCellsYProvider, rasterBlockExtent );

    QHash< double, qgssize > fUniqueValues;
    QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( mRasterInterface.get(), mRasterBand, featureGeometry, nCellsX, nCellsY, mCellSizeX, mCellSizeY,
    rasterBlockExtent, [ &fUniqueValues]( double value ) { fUniqueValues[value]++; }, false );

    if ( fUniqueValues.count() < 1 )
    {
      // The cell resolution is probably larger than the polygon area. We switch to slower precise pixel - polygon intersection in this case
      // TODO: eventually deal with weight if needed
      QgsRasterAnalysisUtils::statisticsFromPreciseIntersection( mRasterInterface.get(), mRasterBand, featureGeometry, nCellsX, nCellsY, mCellSizeX, mCellSizeY,
      rasterBlockExtent, [ &fUniqueValues]( double value, double ) { fUniqueValues[value]++; }, false );
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

  const QString fieldPrefix = parameterAsString( parameters, QStringLiteral( "COLUMN_PREFIX" ), context );
  QgsFields newFields;
  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    newFields.append( QgsField( QStringLiteral( "%1%2" ).arg( fieldPrefix, mHasNoDataValue && *it == mNodataValue ? QStringLiteral( "NODATA" ) : QString::number( *it ) ), QVariant::LongLong, QString(), -1, 0 ) );
  }
  const QgsFields fields = QgsProcessingUtils::combineFields( zones->fields(), newFields );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          zones->wkbType(), zones->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  it = zones->getFeatures( QgsFeatureRequest() );
  while ( it.nextFeature( f ) )
  {
    QgsAttributes attributes = f.attributes();
    const QHash< double, qgssize > fUniqueValues = featuresUniqueValues.value( f.id() );
    for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
    {
      attributes += fUniqueValues.value( *it, 0 );
    }

    QgsFeature outputFeature;
    outputFeature.setGeometry( f.geometry() );
    outputFeature.setAttributes( attributes );

    if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond



