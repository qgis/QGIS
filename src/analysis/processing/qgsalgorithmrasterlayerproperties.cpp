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
  return u"rasterlayerproperties"_s;
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
  return u"rasteranalysis"_s;
}

void QgsRasterLayerPropertiesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), QVariant(), u"INPUT"_s, true ) );

  addOutput( new QgsProcessingOutputNumber( u"X_MIN"_s, QObject::tr( "Minimum x-coordinate" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"X_MAX"_s, QObject::tr( "Maximum x-coordinate" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"Y_MIN"_s, QObject::tr( "Minimum y-coordinate" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"Y_MAX"_s, QObject::tr( "Maximum y-coordinate" ) ) );
  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"PIXEL_WIDTH"_s, QObject::tr( "Pixel size (width) in map units" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"PIXEL_HEIGHT"_s, QObject::tr( "Pixel size (height) in map units" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputBoolean( u"HAS_NODATA_VALUE"_s, QObject::tr( "Band has a NoData value set" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"NODATA_VALUE"_s, QObject::tr( "Band NoData value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"BAND_COUNT"_s, QObject::tr( "Number of bands in raster" ) ) );
}

QString QgsRasterLayerPropertiesAlgorithm::shortDescription() const
{
  return QObject::tr( "Returns basic properties of a given raster layer, including the extent, size in pixels and dimensions of pixels (in map units)." );
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
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBandCount = layer->bandCount();

  if ( parameters.value( u"BAND"_s ).isValid() )
  {
    const int band = parameterAsInt( parameters, u"BAND"_s, context );
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
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"X_MIN"_s, mExtent.xMinimum() );
  outputs.insert( u"X_MAX"_s, mExtent.xMaximum() );
  outputs.insert( u"Y_MIN"_s, mExtent.yMinimum() );
  outputs.insert( u"Y_MAX"_s, mExtent.yMaximum() );

  outputs.insert( u"PIXEL_WIDTH"_s, mRasterUnitsPerPixelX );
  outputs.insert( u"PIXEL_HEIGHT"_s, mRasterUnitsPerPixelY );

  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );

  outputs.insert( u"HAS_NODATA_VALUE"_s, mHasNoDataValue );
  if ( mHasNoDataValue )
    outputs.insert( u"NODATA_VALUE"_s, mNoDataValue );
  outputs.insert( u"BAND_COUNT"_s, mBandCount );

  return outputs;
}


///@endcond
