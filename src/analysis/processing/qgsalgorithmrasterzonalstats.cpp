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
#include "qgsstringutils.h"
#include "qgsstatisticalsummary.h"
#include "qgsrasterprojector.h"

///@cond PRIVATE

QString QgsRasterLayerZonalStatsAlgorithm::name() const
{
  return QStringLiteral( "rasterlayerzonalstats" );
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
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterLayerZonalStatsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "ZONES" ),
                QObject::tr( "Zones layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "ZONES_BAND" ),
                QObject::tr( "Zones band number" ), 1, QStringLiteral( "ZONES" ) ) );

  std::unique_ptr< QgsProcessingParameterEnum > refParam = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "REF_LAYER" ), QObject::tr( "Reference layer" ),
      QStringList() << QObject::tr( "Input layer" ) << QObject::tr( "Zones layer" ), false, 0 );
  refParam->setFlags( refParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( refParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_TABLE" ),
                QObject::tr( "Statistics" ), QgsProcessing::TypeVector ) );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NODATA_PIXEL_COUNT" ), QObject::tr( "NODATA pixel count" ) ) );
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
                      "If either the source raster layer or the zone raster layer value is NODATA for a pixel, that pixel's value will be skipped and not including in the calculated statistics." );
}

QgsRasterLayerZonalStatsAlgorithm *QgsRasterLayerZonalStatsAlgorithm::createInstance() const
{
  return new QgsRasterLayerZonalStatsAlgorithm();
}

bool QgsRasterLayerZonalStatsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mRefLayer = static_cast< RefLayer >( parameterAsEnum( parameters, QStringLiteral( "REF_LAYER" ), context ) );

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( layer->bandCount() ) );

  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );

  QgsRasterLayer *zonesLayer = parameterAsRasterLayer( parameters, QStringLiteral( "ZONES" ), context );

  if ( !zonesLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "ZONES" ) ) );

  mZonesBand = parameterAsInt( parameters, QStringLiteral( "ZONES_BAND" ), context );
  if ( mZonesBand < 1 || mZonesBand > zonesLayer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for ZONES_BAND (%1): Valid values for input raster are 1 to %2" ).arg( mZonesBand )
                                  .arg( zonesLayer->bandCount() ) );
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
        mProjector = qgis::make_unique< QgsRasterProjector >();
        mProjector->setInput( mZonesDataProvider.get() );
        mProjector->setCrs( zonesLayer->crs(), layer->crs() );
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
        mProjector = qgis::make_unique< QgsRasterProjector >();
        mProjector->setInput( mSourceDataProvider.get() );
        mProjector->setCrs( layer->crs(), zonesLayer->crs() );
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
  std::unique_ptr< QgsFeatureSink > sink;
  if ( parameters.contains( QStringLiteral( "OUTPUT_TABLE" ) ) && parameters.value( QStringLiteral( "OUTPUT_TABLE" ) ).isValid() )
  {
    QgsFields outFields;
    outFields.append( QgsField( QStringLiteral( "zone" ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( areaUnit.replace( QStringLiteral( "²" ), QStringLiteral( "2" ) ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( QStringLiteral( "sum" ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( QStringLiteral( "count" ), QVariant::LongLong, QString(), 20 ) );
    outFields.append( QgsField( QStringLiteral( "min" ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( QStringLiteral( "max" ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( QStringLiteral( "mean" ), QVariant::Double, QString(), 20, 8 ) );

    sink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT_TABLE" ), context, tableDest, outFields, QgsWkbTypes::NoGeometry, QgsCoordinateReferenceSystem() ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_TABLE" ) ) );
  }

  struct StatCalculator
  {
    // only calculate cheap stats-- we cannot calculate stats which require holding values in memory -- because otherwise we'll end
    // up trying to store EVERY pixel value from the input in memory
    QgsStatisticalSummary s{ QgsStatisticalSummary::Count | QgsStatisticalSummary::Sum | QgsStatisticalSummary::Min | QgsStatisticalSummary::Max | QgsStatisticalSummary::Mean };
  };
  QHash<double, StatCalculator > zoneStats;
  qgssize noDataCount = 0;

  qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );
  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  QgsRasterIterator iter = mRefLayer == Source ? QgsRasterIterator( mSourceInterface )
                           : QgsRasterIterator( mZonesInterface );
  iter.startRasterRead( mRefLayer == Source ? mBand : mZonesBand, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  std::unique_ptr< QgsRasterBlock > rasterBlock;
  std::unique_ptr< QgsRasterBlock > zonesRasterBlock;
  bool isNoData = false;
  while ( true )
  {
    if ( mRefLayer == Source )
    {
      if ( !iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop, &blockExtent ) )
        break;

      zonesRasterBlock.reset( mZonesInterface->block( mZonesBand, blockExtent, iterCols, iterRows ) );
      if ( !zonesRasterBlock )
        continue;
    }
    else
    {
      if ( !iter.readNextRasterPart( mZonesBand, iterCols, iterRows, zonesRasterBlock, iterLeft, iterTop, &blockExtent ) )
        break;

      rasterBlock.reset( mSourceInterface->block( mBand, blockExtent, iterCols, iterRows ) );
      if ( !rasterBlock )
        continue;
    }

    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    if ( !rasterBlock->isValid() || rasterBlock->isEmpty() || !zonesRasterBlock->isValid() || zonesRasterBlock->isEmpty() )
      continue;

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      for ( int column = 0; column < iterCols; column++ )
      {
        double value = rasterBlock->valueAndNoData( row, column, isNoData );
        if ( mHasNoDataValue && isNoData )
        {
          noDataCount += 1;
          continue;
        }
        double zone = zonesRasterBlock->valueAndNoData( row, column, isNoData );
        if ( mZonesHasNoDataValue && isNoData )
        {
          noDataCount += 1;
          continue;
        }
        zoneStats[ zone ].s.addValue( value );
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );
  outputs.insert( QStringLiteral( "TOTAL_PIXEL_COUNT" ), layerSize );
  outputs.insert( QStringLiteral( "NODATA_PIXEL_COUNT" ), noDataCount );

  double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;

  for ( auto it = zoneStats.begin(); it != zoneStats.end(); ++it )
  {
    QgsFeature f;
    it->s.finalize();
    f.setAttributes( QgsAttributes() << it.key() << it->s.count() * pixelArea << it->s.sum() << it->s.count() <<
                     it->s.min() << it->s.max() << it->s.mean() );
    sink->addFeature( f, QgsFeatureSink::FastInsert );
  }
  outputs.insert( QStringLiteral( "OUTPUT_TABLE" ), tableDest );

  return outputs;
}


///@endcond



