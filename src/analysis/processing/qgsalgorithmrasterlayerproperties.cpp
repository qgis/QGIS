/***************************************************************************
                         qgsalgorithmrasterlayerproperties.cpp
                         ---------------------
    begin                : April 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsalgorithmrasterlayerproperties.h"
#include "qgsstringutils.h"

///@cond PRIVATE

QString QgsRasterLayerPropertiesAlgorithm::name() const
{
  return QStringLiteral( "rasterlayerproperties" );
}

QString QgsRasterLayerPropertiesAlgorithm::displayName() const
{
  return QObject::tr( "Raster layer properties" );
}

QStringList QgsRasterLayerPropertiesAlgorithm::tags() const
{
  return QObject::tr( "extent,pixel,size,width,height" ).split( ',' );
}

QString QgsRasterLayerPropertiesAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterLayerPropertiesAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterLayerPropertiesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ), QObject::tr( "Band number" ), QVariant(), QStringLiteral( "INPUT" ), true ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "X_MIN" ), QObject::tr( "Minimum x-coordinate" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "X_MAX" ), QObject::tr( "Maximum x-coordinate" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "Y_MIN" ), QObject::tr( "Minimum y-coordinate" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "Y_MAX" ), QObject::tr( "Maximum y-coordinate" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "PIXEL_WIDTH" ), QObject::tr( "Pixel size (width) in map units" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "PIXEL_HEIGHT" ), QObject::tr( "Pixel size (height) in map units" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputBoolean( QStringLiteral( "HAS_NODATA_VALUE" ), QObject::tr( "Band has a NoData value set" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NODATA_VALUE" ), QObject::tr( "Band NoData value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "BAND_COUNT" ), QObject::tr( "Number of bands in raster" ) ) );
}

QString QgsRasterLayerPropertiesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm returns basic properties of the given raster layer, including the extent, size in pixels and dimensions of pixels (in map units).\n\n"
                      "If an optional band number is specified then the NoData value for the selected band will also be returned." );
}

QgsRasterLayerPropertiesAlgorithm *QgsRasterLayerPropertiesAlgorithm::createInstance() const
{
  return new QgsRasterLayerPropertiesAlgorithm();
}

bool QgsRasterLayerPropertiesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  mBandCount = layer->bandCount();

  if ( parameters.value( QStringLiteral( "BAND" ) ).isValid() )
  {
    const int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
    if ( band < 1 || band > mBandCount )
      throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( band ).arg( layer->bandCount() ) );
    mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
    if ( mHasNoDataValue )
      mNoDataValue = layer->dataProvider()->sourceNoDataValue( band );
  }

  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();

  return true;
}

QVariantMap QgsRasterLayerPropertiesAlgorithm::processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "X_MIN" ), mExtent.xMinimum() );
  outputs.insert( QStringLiteral( "X_MAX" ), mExtent.xMaximum() );
  outputs.insert( QStringLiteral( "Y_MIN" ), mExtent.yMinimum() );
  outputs.insert( QStringLiteral( "Y_MAX" ), mExtent.yMaximum() );

  outputs.insert( QStringLiteral( "PIXEL_WIDTH" ), mRasterUnitsPerPixelX );
  outputs.insert( QStringLiteral( "PIXEL_HEIGHT" ), mRasterUnitsPerPixelY );

  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );

  outputs.insert( QStringLiteral( "HAS_NODATA_VALUE" ), mHasNoDataValue );
  if ( mHasNoDataValue )
    outputs.insert( QStringLiteral( "NODATA_VALUE" ), mNoDataValue );
  outputs.insert( QStringLiteral( "BAND_COUNT" ), mBandCount );

  return outputs;
}


///@endcond
