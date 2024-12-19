/***************************************************************************
                         qgsalgorithmzonalminmaxpoint.cpp
                         ------------------------------
    begin                : November 2024
    copyright            : (C) 2024 Nyall Dawson
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

#include "qgsalgorithmzonalminmaxpoint.h"
#include "qgszonalstatistics.h"

///@cond PRIVATE

QString QgsZonalMinimumMaximumPointAlgorithm::name() const
{
  return QStringLiteral( "zonalminmaxpoint" );
}

QString QgsZonalMinimumMaximumPointAlgorithm::displayName() const
{
  return QObject::tr( "Zonal minimum/maximum point" );
}

QStringList QgsZonalMinimumMaximumPointAlgorithm::tags() const
{
  return QObject::tr( "stats,statistics,zones,maximum,minimum,raster,extrema,extremum" ).split( ',' );
}

QString QgsZonalMinimumMaximumPointAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsZonalMinimumMaximumPointAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsZonalMinimumMaximumPointAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts point locations for minimum and maximum raster values within polygon zones." );
}

QString QgsZonalMinimumMaximumPointAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts point features corresponding to the minimum "
                      "and maximum pixel values contained within polygon zones.\n\n"
                      "The output will contain one point feature for the minimum and one "
                      "for the maximum raster value for every individual zonal feature "
                      "from a polygon layer.\n\n"
                      "The created point layer will be in the same spatial reference system as the selected raster layer." );
}

QList<int> QgsZonalMinimumMaximumPointAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QgsZonalMinimumMaximumPointAlgorithm *QgsZonalMinimumMaximumPointAlgorithm::createInstance() const
{
  return new QgsZonalMinimumMaximumPointAlgorithm();
}

void QgsZonalMinimumMaximumPointAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ), QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ), QObject::tr( "Raster band" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );
}

QString QgsZonalMinimumMaximumPointAlgorithm::outputName() const
{
  return QObject::tr( "Zonal Extrema" );
}

QgsFields QgsZonalMinimumMaximumPointAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  Q_UNUSED( inputFields )
  return mOutputFields;
}

Qgis::ProcessingSourceType QgsZonalMinimumMaximumPointAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPoint;
}

Qgis::WkbType QgsZonalMinimumMaximumPointAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::Point;
}

QgsFeatureSink::SinkFlags QgsZonalMinimumMaximumPointAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::SinkFlag::RegeneratePrimaryKey;
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsZonalMinimumMaximumPointAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsCoordinateReferenceSystem QgsZonalMinimumMaximumPointAlgorithm::outputCrs( const QgsCoordinateReferenceSystem & ) const
{
  return mCrs;
}

bool QgsZonalMinimumMaximumPointAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *rasterLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );
  if ( !rasterLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  if ( mBand < 1 || mBand > rasterLayer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( rasterLayer->bandCount() ) );

  if ( !rasterLayer->dataProvider() )
    throw QgsProcessingException( QObject::tr( "Invalid raster layer. Layer %1 is invalid." ).arg( rasterLayer->id() ) );

  mRaster.reset( rasterLayer->dataProvider()->clone() );
  mCrs = rasterLayer->crs();
  mPixelSizeX = rasterLayer->rasterUnitsPerPixelX();
  mPixelSizeY = rasterLayer->rasterUnitsPerPixelY();
  std::unique_ptr<QgsFeatureSource> source( parameterAsSource( parameters, inputParameterName(), context ) );

  QgsFields newFields;
  newFields.append( QgsField( QStringLiteral( "value" ), QMetaType::Type::Double, QString(), 20, 8 ) );
  newFields.append( QgsField( QStringLiteral( "extremum_type" ), QMetaType::Type::QString ) );
  mOutputFields = QgsProcessingUtils::combineFields( source->fields(), newFields );

  return true;
}

QgsFeatureList QgsZonalMinimumMaximumPointAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( !mCreatedTransform )
  {
    mCreatedTransform = true;
    mFeatureToRasterTransform = QgsCoordinateTransform( sourceCrs(), mCrs, context.transformContext() );
  }

  Q_UNUSED( feedback )
  QgsAttributes attributes = feature.attributes();

  QgsGeometry geometry = feature.geometry();
  try
  {
    geometry.transform( mFeatureToRasterTransform );
  }
  catch ( QgsCsException & )
  {
    if ( feedback )
      feedback->reportError( QObject::tr( "Encountered a transform error when reprojecting feature with id %1." ).arg( feature.id() ) );
  }

  const QMap<Qgis::ZonalStatistic, QVariant> results = QgsZonalStatistics::calculateStatistics( mRaster.get(), geometry, mPixelSizeX, mPixelSizeY, mBand, Qgis::ZonalStatistic::Min | Qgis::ZonalStatistic::MinimumPoint | Qgis::ZonalStatistic::Max | Qgis::ZonalStatistic::MaximumPoint );

  QgsFeature minPointFeature( mOutputFields );
  QgsAttributes minAttributes = attributes;
  minAttributes << results.value( Qgis::ZonalStatistic::Min ) << QStringLiteral( "minimum" );
  minPointFeature.setAttributes( minAttributes );
  minPointFeature.setGeometry( QgsGeometry::fromPointXY( results.value( Qgis::ZonalStatistic::MinimumPoint ).value<QgsPointXY>() ) );

  QgsFeature maxPointFeature( mOutputFields );
  QgsAttributes maxAttributes = attributes;
  maxAttributes << results.value( Qgis::ZonalStatistic::Max ) << QStringLiteral( "maximum" );
  maxPointFeature.setAttributes( maxAttributes );
  maxPointFeature.setGeometry( QgsGeometry::fromPointXY( results.value( Qgis::ZonalStatistic::MaximumPoint ).value<QgsPointXY>() ) );

  return QgsFeatureList { minPointFeature, maxPointFeature };
}

bool QgsZonalMinimumMaximumPointAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
