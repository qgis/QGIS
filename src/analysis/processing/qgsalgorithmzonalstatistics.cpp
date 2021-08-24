/***************************************************************************
                         qgsalgorithmzonalstatistics.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmzonalstatistics.h"

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

QString QgsZonalStatisticsAlgorithm::name() const
{
  return QStringLiteral( "zonalstatistics" );
}

QString QgsZonalStatisticsAlgorithm::displayName() const
{
  return QObject::tr( "Zonal statistics (in place)" );
}

QStringList QgsZonalStatisticsAlgorithm::tags() const
{
  return QObject::tr( "stats,statistics,zones,layer,sum,maximum,minimum,mean,count,standard,deviation,"
                      "median,range,majority,minority,variety,variance,summary,raster" ).split( ',' );
}

QString QgsZonalStatisticsAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsZonalStatisticsAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsZonalStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates statistics of a raster layer for each feature "
                      "of an overlapping polygon vector layer. The results will be written in place." );
}

QgsProcessingAlgorithm::Flags QgsZonalStatisticsAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagNoThreading | QgsProcessingAlgorithm::FlagDeprecated;
}

QgsZonalStatisticsAlgorithm *QgsZonalStatisticsAlgorithm::createInstance() const
{
  return new QgsZonalStatisticsAlgorithm();
}

void QgsZonalStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList statChoices;
  statChoices.reserve( STATS.size() );
  for ( const QgsZonalStatistics::Statistic stat : STATS )
  {
    statChoices << QgsZonalStatistics::displayName( stat );
  }

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ), QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Raster band" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT_VECTOR" ), QObject::tr( "Vector layer containing zones" ),
                QList< int >() << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "COLUMN_PREFIX" ), QObject::tr( "Output column prefix" ), QStringLiteral( "_" ) ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "STATISTICS" ), QObject::tr( "Statistics to calculate" ),
                statChoices, true, QVariantList() << 0 << 1 << 2 ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "INPUT_VECTOR" ), QObject::tr( "Zonal statistics" ), QgsProcessing::TypeVectorPolygon ) );
}

bool QgsZonalStatisticsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *rasterLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );
  if ( !rasterLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  if ( mBand < 1 || mBand > rasterLayer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( rasterLayer->bandCount() ) );

  mInterface.reset( rasterLayer->dataProvider()->clone() );
  mCrs = rasterLayer->crs();
  mPixelSizeX = rasterLayer->rasterUnitsPerPixelX();
  mPixelSizeY = rasterLayer->rasterUnitsPerPixelY();

  mPrefix = parameterAsString( parameters, QStringLiteral( "COLUMN_PREFIX" ), context );

  const QList< int > stats = parameterAsEnums( parameters, QStringLiteral( "STATISTICS" ), context );
  mStats = QgsZonalStatistics::Statistics();
  for ( const int s : stats )
  {
    mStats |= STATS.at( s );
  }

  return true;
}

QVariantMap QgsZonalStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT_VECTOR" ), context );
  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid zones layer" ) );

  QgsZonalStatistics zs( layer,
                         mInterface.get(),
                         mCrs,
                         mPixelSizeX,
                         mPixelSizeY,
                         mPrefix,
                         mBand,
                         QgsZonalStatistics::Statistics( mStats )
                       );

  zs.calculateStatistics( feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "INPUT_VECTOR" ), layer->id() );
  return outputs;
}

///@endcond
