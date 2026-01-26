/***************************************************************************
                         qgsalgorithmrasterzonalstats.cpp
                         ---------------------
    begin                : December 2018
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

#include "qgsalgorithmrasterzonalstats.h"

#include "qgsrasterprojector.h"
#include "qgsstatisticalsummary.h"
#include "qgsstringutils.h"
#include "qgsunittypes.h"

///@cond PRIVATE

QString QgsRasterLayerZonalStatsAlgorithm::name() const
{
  return u"rasterlayerzonalstats"_s;
}

QString QgsRasterLayerZonalStatsAlgorithm::displayName() const
{
  return QObject::tr( "Raster layer zonal statistics" );
}

QStringList QgsRasterLayerZonalStatsAlgorithm::tags() const
{
  return QObject::tr( "count,area,statistics,stats,zones,categories,minimum,maximum,mean,sum,total" ).split( ',' );
}

QString QgsRasterLayerZonalStatsAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterLayerZonalStatsAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsRasterLayerZonalStatsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterRasterLayer( u"ZONES"_s, QObject::tr( "Zones layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"ZONES_BAND"_s, QObject::tr( "Zones band number" ), 1, u"ZONES"_s ) );

  auto refParam = std::make_unique<QgsProcessingParameterEnum>( u"REF_LAYER"_s, QObject::tr( "Reference layer" ), QStringList() << QObject::tr( "Input layer" ) << QObject::tr( "Zones layer" ), false, 0 );
  refParam->setFlags( refParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( refParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT_TABLE"_s, QObject::tr( "Statistics" ), Qgis::ProcessingSourceType::Vector ) );

  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTAL_PIXEL_COUNT"_s, QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"NODATA_PIXEL_COUNT"_s, QObject::tr( "NoData pixel count" ) ) );
}

QString QgsRasterLayerZonalStatsAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates statistics for a raster layer's values, categorized by zones defined in another raster layer." );
}

QString QgsRasterLayerZonalStatsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates statistics for a raster layer's values, categorized by zones defined in another raster layer.\n\n"
                      "If the reference layer parameter is set to \"Input layer\", then zones are determined by sampling the zone raster layer value at the centroid of each pixel from the source raster layer.\n\n"
                      "If the reference layer parameter is set to \"Zones layer\", then the input raster layer will be sampled at the centroid of each pixel from the zones raster layer.\n\n"
                      "If either the source raster layer or the zone raster layer value is NoData for a pixel, that pixel's value will be skipped and not included in the calculated statistics." );
}

QgsRasterLayerZonalStatsAlgorithm *QgsRasterLayerZonalStatsAlgorithm::createInstance() const
{
  return new QgsRasterLayerZonalStatsAlgorithm();
}

bool QgsRasterLayerZonalStatsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mRefLayer = static_cast<RefLayer>( parameterAsEnum( parameters, u"REF_LAYER"_s, context ) );

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  const int band = parameterAsInt( parameters, u"BAND"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );

  QgsRasterLayer *zonesLayer = parameterAsRasterLayer( parameters, u"ZONES"_s, context );

  if ( !zonesLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"ZONES"_s ) );

  mZonesBand = parameterAsInt( parameters, u"ZONES_BAND"_s, context );
  if ( mZonesBand < 1 || mZonesBand > zonesLayer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for ZONES_BAND (%1): Valid values for input raster are 1 to %2" ).arg( mZonesBand ).arg( zonesLayer->bandCount() ) );
  mZonesHasNoDataValue = zonesLayer->dataProvider()->sourceHasNoDataValue( band );

  mSourceDataProvider.reset( layer->dataProvider()->clone() );
  mSourceInterface = mSourceDataProvider.get();
  mZonesDataProvider.reset( zonesLayer->dataProvider()->clone() );
  mZonesInterface = mZonesDataProvider.get();

  switch ( mRefLayer )
  {
    case Source:
      mCrs = layer->crs();
      mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
      mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();
      mLayerWidth = layer->width();
      mLayerHeight = layer->height();
      mExtent = layer->extent();

      // add projector if necessary
      if ( layer->crs() != zonesLayer->crs() )
      {
        mProjector = std::make_unique<QgsRasterProjector>();
        mProjector->setInput( mZonesDataProvider.get() );
        mProjector->setCrs( zonesLayer->crs(), layer->crs(), context.transformContext() );
        mZonesInterface = mProjector.get();
      }
      break;

    case Zones:
      mCrs = zonesLayer->crs();
      mRasterUnitsPerPixelX = zonesLayer->rasterUnitsPerPixelX();
      mRasterUnitsPerPixelY = zonesLayer->rasterUnitsPerPixelY();
      mLayerWidth = zonesLayer->width();
      mLayerHeight = zonesLayer->height();
      mExtent = zonesLayer->extent();

      // add projector if necessary
      if ( layer->crs() != zonesLayer->crs() )
      {
        mProjector = std::make_unique<QgsRasterProjector>();
        mProjector->setInput( mSourceDataProvider.get() );
        mProjector->setCrs( layer->crs(), zonesLayer->crs(), context.transformContext() );
        mSourceInterface = mProjector.get();
      }
      break;
  }

  return true;
}

QVariantMap QgsRasterLayerZonalStatsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString areaUnit = QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( mCrs.mapUnits() ) );

  QString tableDest;
  std::unique_ptr<QgsFeatureSink> sink;
  if ( parameters.contains( u"OUTPUT_TABLE"_s ) && parameters.value( u"OUTPUT_TABLE"_s ).isValid() )
  {
    QgsFields outFields;
    outFields.append( QgsField( u"zone"_s, QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( areaUnit.isEmpty() ? "area" : areaUnit.replace( u"²"_s, "2"_L1 ), QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( u"sum"_s, QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( u"count"_s, QMetaType::Type::LongLong, QString(), 20 ) );
    outFields.append( QgsField( u"min"_s, QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( u"max"_s, QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( u"mean"_s, QMetaType::Type::Double, QString(), 20, 8 ) );

    sink.reset( parameterAsSink( parameters, u"OUTPUT_TABLE"_s, context, tableDest, outFields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT_TABLE"_s ) );
  }

  struct StatCalculator
  {
      // only calculate cheap stats-- we cannot calculate stats which require holding values in memory -- because otherwise we'll end
      // up trying to store EVERY pixel value from the input in memory
      QgsStatisticalSummary s { Qgis::Statistic::Count | Qgis::Statistic::Sum | Qgis::Statistic::Min | Qgis::Statistic::Max | Qgis::Statistic::Mean };
  };
  QHash<double, StatCalculator> zoneStats;
  qgssize noDataCount = 0;

  const qgssize layerSize = static_cast<qgssize>( mLayerWidth ) * static_cast<qgssize>( mLayerHeight );

  QgsRasterIterator iter = mRefLayer == Source ? QgsRasterIterator( mSourceInterface )
                                               : QgsRasterIterator( mZonesInterface );
  iter.startRasterRead( mRefLayer == Source ? mBand : mZonesBand, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  std::unique_ptr<QgsRasterBlock> zonesRasterBlock;
  bool isNoData = false;
  while ( true )
  {
    int band;
    if ( mRefLayer == Source )
    {
      band = mBand;
      if ( !iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop, &blockExtent ) )
        break;

      zonesRasterBlock.reset( mZonesInterface->block( mZonesBand, blockExtent, iterCols, iterRows ) );
    }
    else
    {
      band = mZonesBand;
      if ( !iter.readNextRasterPart( mZonesBand, iterCols, iterRows, zonesRasterBlock, iterLeft, iterTop, &blockExtent ) )
        break;

      rasterBlock.reset( mSourceInterface->block( mBand, blockExtent, iterCols, iterRows ) );
    }
    if ( !zonesRasterBlock || !rasterBlock )
      continue;

    feedback->setProgress( 100 * iter.progress( band ) );
    if ( !rasterBlock->isValid() || rasterBlock->isEmpty() || !zonesRasterBlock->isValid() || zonesRasterBlock->isEmpty() )
      continue;

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      for ( int column = 0; column < iterCols; column++ )
      {
        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        if ( mHasNoDataValue && isNoData )
        {
          noDataCount += 1;
          continue;
        }
        const double zone = zonesRasterBlock->valueAndNoData( row, column, isNoData );
        if ( mZonesHasNoDataValue && isNoData )
        {
          noDataCount += 1;
          continue;
        }
        zoneStats[zone].s.addValue( value );
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );
  outputs.insert( u"TOTAL_PIXEL_COUNT"_s, layerSize );
  outputs.insert( u"NODATA_PIXEL_COUNT"_s, noDataCount );

  const double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;

  for ( auto it = zoneStats.begin(); it != zoneStats.end(); ++it )
  {
    QgsFeature f;
    it->s.finalize();
    f.setAttributes( QgsAttributes() << it.key() << it->s.count() * pixelArea << it->s.sum() << it->s.count() << it->s.min() << it->s.max() << it->s.mean() );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT_TABLE"_s ) );
    sink->finalize();
  }
  outputs.insert( u"OUTPUT_TABLE"_s, tableDest );

  return outputs;
}


///@endcond
