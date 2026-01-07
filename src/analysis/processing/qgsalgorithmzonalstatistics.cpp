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

#include "qgsalgorithmzonalstatistics_private.h"
#include "qgsvectorlayer.h"
#include "qgszonalstatistics.h"

///@cond PRIVATE

QString QgsZonalStatisticsAlgorithm::name() const
{
  return u"zonalstatistics"_s;
}

QString QgsZonalStatisticsAlgorithm::displayName() const
{
  return QObject::tr( "Zonal statistics (in place)" );
}

QStringList QgsZonalStatisticsAlgorithm::tags() const
{
  return QObject::tr( "stats,statistics,zones,layer,sum,maximum,minimum,mean,count,standard,deviation,"
                      "median,range,majority,minority,variety,variance,summary,raster" )
    .split( ',' );
}

QString QgsZonalStatisticsAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsZonalStatisticsAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsZonalStatisticsAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates statistics for a raster layer's values for each feature of an overlapping polygon vector layer." );
}

QString QgsZonalStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates statistics of a raster layer for each feature "
                      "of an overlapping polygon vector layer. The results will be written in place." );
}

Qgis::ProcessingAlgorithmFlags QgsZonalStatisticsAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::Deprecated;
}

QgsZonalStatisticsAlgorithm *QgsZonalStatisticsAlgorithm::createInstance() const
{
  return new QgsZonalStatisticsAlgorithm();
}

void QgsZonalStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList statChoices;
  statChoices.reserve( STATS.size() );
  for ( const Qgis::ZonalStatistic stat : STATS )
  {
    statChoices << QgsZonalStatistics::displayName( stat );
  }

  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_RASTER"_s, QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"RASTER_BAND"_s, QObject::tr( "Raster band" ), 1, u"INPUT_RASTER"_s ) );
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT_VECTOR"_s, QObject::tr( "Vector layer containing zones" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  addParameter( new QgsProcessingParameterString( u"COLUMN_PREFIX"_s, QObject::tr( "Output column prefix" ), u"_"_s ) );

  addParameter( new QgsProcessingParameterEnum( u"STATISTICS"_s, QObject::tr( "Statistics to calculate" ), statChoices, true, QVariantList() << 0 << 1 << 2 ) );

  addOutput( new QgsProcessingOutputVectorLayer( u"INPUT_VECTOR"_s, QObject::tr( "Zonal statistics" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

bool QgsZonalStatisticsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *rasterLayer = parameterAsRasterLayer( parameters, u"INPUT_RASTER"_s, context );
  if ( !rasterLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_RASTER"_s ) );

  mBand = parameterAsInt( parameters, u"RASTER_BAND"_s, context );
  if ( mBand < 1 || mBand > rasterLayer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( rasterLayer->bandCount() ) );

  mInterface.reset( rasterLayer->dataProvider()->clone() );
  mCrs = rasterLayer->crs();
  mPixelSizeX = rasterLayer->rasterUnitsPerPixelX();
  mPixelSizeY = rasterLayer->rasterUnitsPerPixelY();

  mPrefix = parameterAsString( parameters, u"COLUMN_PREFIX"_s, context );

  const QList<int> stats = parameterAsEnums( parameters, u"STATISTICS"_s, context );
  mStats = Qgis::ZonalStatistics();
  for ( const int s : stats )
  {
    mStats |= STATS.at( s );
  }

  return true;
}

QVariantMap QgsZonalStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT_VECTOR"_s, context );
  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid zones layer" ) );

  QgsZonalStatistics zs( layer, mInterface.get(), mCrs, mPixelSizeX, mPixelSizeY, mPrefix, mBand, Qgis::ZonalStatistics( mStats ) );

  zs.calculateStatistics( feedback );

  QVariantMap outputs;
  outputs.insert( u"INPUT_VECTOR"_s, layer->id() );
  return outputs;
}

///@endcond
