/***************************************************************************
                         qgsalgorithmzonalstatisticsfeaturebased.cpp
                         ---------------------
    begin                : September 2020
    copyright            : (C) 2020 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmzonalstatisticsfeaturebased.h"

///@cond PRIVATE

const std::vector< QgsZonalStatistics::Statistic > STATS
{
  QgsZonalStatistics::Count,
  QgsZonalStatistics::Sum,
  QgsZonalStatistics::Mean,
  QgsZonalStatistics::Median,
  QgsZonalStatistics::StDev,
  QgsZonalStatistics::Min,
  QgsZonalStatistics::Max,
  QgsZonalStatistics::Range,
  QgsZonalStatistics::Minority,
  QgsZonalStatistics::Majority,
  QgsZonalStatistics::Variety,
  QgsZonalStatistics::Variance,
};

QString QgsZonalStatisticsFeatureBasedAlgorithm::name() const
{
  return QStringLiteral( "zonalstatisticsfb" );
}

QString QgsZonalStatisticsFeatureBasedAlgorithm::displayName() const
{
  return QObject::tr( "Zonal statistics" );
}

QStringList QgsZonalStatisticsFeatureBasedAlgorithm::tags() const
{
  return QObject::tr( "stats,statistics,zones,layer,sum,maximum,minimum,mean,count,standard,deviation,"
                      "median,range,majority,minority,variety,variance,summary,raster" ).split( ',' );
}

QString QgsZonalStatisticsFeatureBasedAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsZonalStatisticsFeatureBasedAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsZonalStatisticsFeatureBasedAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates statistics of a raster layer for each feature "
                      "of an overlapping polygon vector layer." );
}

QList<int> QgsZonalStatisticsFeatureBasedAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsZonalStatisticsFeatureBasedAlgorithm *QgsZonalStatisticsFeatureBasedAlgorithm::createInstance() const
{
  return new QgsZonalStatisticsFeatureBasedAlgorithm();
}

void QgsZonalStatisticsFeatureBasedAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )
  QStringList statChoices;
  statChoices.reserve( STATS.size() );
  for ( const QgsZonalStatistics::Statistic stat : STATS )
  {
    statChoices << QgsZonalStatistics::displayName( stat );
  }

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ), QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Raster band" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "COLUMN_PREFIX" ), QObject::tr( "Output column prefix" ), QStringLiteral( "_" ) ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "STATISTICS" ), QObject::tr( "Statistics to calculate" ),
                statChoices, true, QVariantList() << 0 << 1 << 2 ) );
}

QString QgsZonalStatisticsFeatureBasedAlgorithm::outputName() const
{
  return QObject::tr( "Zonal Statistics" );
}

QgsFields QgsZonalStatisticsFeatureBasedAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  Q_UNUSED( inputFields )
  return mOutputFields;
}

bool QgsZonalStatisticsFeatureBasedAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mPrefix = parameterAsString( parameters, QStringLiteral( "COLUMN_PREFIX" ), context );

  const QList< int > stats = parameterAsEnums( parameters, QStringLiteral( "STATISTICS" ), context );
  mStats = QgsZonalStatistics::Statistics();
  for ( const int s : stats )
  {
    mStats |= STATS.at( s );
  }

  QgsRasterLayer *rasterLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );
  if ( !rasterLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  if ( mBand < 1 || mBand > rasterLayer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( rasterLayer->bandCount() ) );

  if ( !rasterLayer->dataProvider() )
    throw QgsProcessingException( QObject::tr( "Invalid raster layer. Layer %1 is invalid." ).arg( rasterLayer->id() ) );

  mRaster.reset( rasterLayer->dataProvider()->clone() );
  mCrs = rasterLayer->crs();
  mPixelSizeX = rasterLayer->rasterUnitsPerPixelX();
  mPixelSizeY = rasterLayer->rasterUnitsPerPixelY();
  std::unique_ptr<QgsFeatureSource> source( parameterAsSource( parameters, inputParameterName(), context ) );

  mOutputFields = source->fields();

  for ( const QgsZonalStatistics::Statistic stat : STATS )
  {
    if ( mStats & stat )
    {
      const QgsField field = QgsField( mPrefix + QgsZonalStatistics::shortName( stat ), QVariant::Double, QStringLiteral( "double precision" ) );
      if ( mOutputFields.names().contains( field.name() ) )
      {
        throw QgsProcessingException( QObject::tr( "Field %1 already exists" ).arg( field.name() ) );
      }
      mOutputFields.append( field );
      mStatFieldsMapping.insert( stat, mOutputFields.size() - 1 );
    }
  }

  return true;
}

QgsFeatureList QgsZonalStatisticsFeatureBasedAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( !mCreatedTransform )
  {
    mCreatedTransform = true;
    mFeatureToRasterTransform = QgsCoordinateTransform( sourceCrs(), mCrs, context.transformContext() );
  }

  Q_UNUSED( feedback )
  QgsAttributes attributes = feature.attributes();
  attributes.resize( mOutputFields.size() );

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

  const QMap<QgsZonalStatistics::Statistic, QVariant> results = QgsZonalStatistics::calculateStatistics( mRaster.get(), geometry, mPixelSizeX, mPixelSizeY, mBand, mStats );
  for ( auto result = results.constBegin(); result != results.constEnd(); ++result )
  {
    attributes.replace( mStatFieldsMapping.value( result.key() ), result.value() );
  }

  QgsFeature resultFeature = feature;
  resultFeature.setAttributes( attributes );

  return QgsFeatureList { resultFeature };
}

bool QgsZonalStatisticsFeatureBasedAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
