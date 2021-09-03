/***************************************************************************
                         qgsalgorithmrastersampling.cpp
                         --------------------------
    begin                : August 2020
    copyright            : (C) 2020 by Mathieu Pellerin
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

#include "qgsalgorithmrastersampling.h"
#include "qgsmultipoint.h"

///@cond PRIVATE

QString QgsRasterSamplingAlgorithm::name() const
{
  return QStringLiteral( "rastersampling" );
}

QString QgsRasterSamplingAlgorithm::displayName() const
{
  return QObject::tr( "Sample raster values" );
}

QStringList QgsRasterSamplingAlgorithm::tags() const
{
  return QObject::tr( "extract,point,pixel,value" ).split( ',' );
}

QString QgsRasterSamplingAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterSamplingAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRasterSamplingAlgorithm::shortDescription() const
{
  return QObject::tr( "Samples raster values under a set of points." );
}

QString QgsRasterSamplingAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer with the same attributes of the input layer and the raster values corresponding on the point location.\n\n"
                      "If the raster layer has more than one band, all the band values are sampled." );
}

QgsRasterSamplingAlgorithm *QgsRasterSamplingAlgorithm::createInstance() const
{
  return new QgsRasterSamplingAlgorithm();
}

void QgsRasterSamplingAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "RASTERCOPY" ), QObject::tr( "Raster layer" ) ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "COLUMN_PREFIX" ), QObject::tr( "Output column prefix" ), QStringLiteral( "SAMPLE_" ), false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Sampled" ), QgsProcessing::TypeVectorPoint ) );
}

bool QgsRasterSamplingAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "RASTERCOPY" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "RASTERCOPY" ) ) );

  mBandCount = layer->bandCount();
  mCrs = layer->crs();
  mDataProvider.reset( static_cast< QgsRasterDataProvider *>( layer->dataProvider()->clone() ) );

  return true;
}

QVariantMap QgsRasterSamplingAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString fieldPrefix = parameterAsString( parameters, QStringLiteral( "COLUMN_PREFIX" ), context );
  QgsFields newFields;
  QgsAttributes emptySampleAttributes;
  for ( int band = 1; band <= mBandCount; band++ )
  {
    const Qgis::DataType dataType = mDataProvider->dataType( band );
    const bool intSafe = ( dataType == Qgis::DataType::Byte || dataType == Qgis::DataType::UInt16 || dataType == Qgis::DataType::Int16 || dataType == Qgis::DataType::UInt32 ||
                           dataType == Qgis::DataType::Int32 || dataType == Qgis::DataType::CInt16 || dataType == Qgis::DataType::CInt32 );

    newFields.append( QgsField( QStringLiteral( "%1%2" ).arg( fieldPrefix, QString::number( band ) ), intSafe ? QVariant::Int : QVariant::Double ) );
    emptySampleAttributes += QVariant();
  }
  const QgsFields fields = QgsProcessingUtils::combineFields( source->fields(), newFields );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const long count = source->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  long current = 0;

  const QgsCoordinateTransform ct( source->sourceCrs(), mCrs, context.transformContext() );
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest() );
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    feedback->setProgress( current * step );
    current++;

    QgsAttributes attributes = feature.attributes();
    QgsFeature outputFeature( feature );
    if ( !feature.hasGeometry() )
    {
      attributes += emptySampleAttributes;
      outputFeature.setAttributes( attributes );
      sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
      feedback->reportError( QObject::tr( "No geometry attached to feature %1." ).arg( feature.id() ) );
      continue;
    }

    QgsGeometry geometry = feature.geometry();
    if ( geometry.isMultipart() && geometry.get()->partCount() != 1 )
    {
      attributes += emptySampleAttributes;
      outputFeature.setAttributes( attributes );
      if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->reportError( QObject::tr( "Impossible to sample data of multipart feature %1." ).arg( feature.id() ) );
      continue;
    }
    QgsPointXY point( *( geometry.isMultipart() ? qgsgeometry_cast< const QgsPoint * >( qgsgeometry_cast< const QgsMultiPoint * >( geometry.constGet() )->geometryN( 0 ) ) :
                         qgsgeometry_cast< const QgsPoint * >( geometry.constGet() ) ) );
    try
    {
      point = ct.transform( point );
    }
    catch ( const QgsException & )
    {
      attributes += emptySampleAttributes;
      outputFeature.setAttributes( attributes );
      if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->reportError( QObject::tr( "Could not reproject feature %1 to raster CRS." ).arg( feature.id() ) );
      continue;
    }

    for ( int band = 1; band <= mBandCount; band ++ )
    {
      bool ok = false;
      const double value = mDataProvider->sample( point, band, &ok );
      attributes += ok ? value : QVariant();
    }
    outputFeature.setAttributes( attributes );
    if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
