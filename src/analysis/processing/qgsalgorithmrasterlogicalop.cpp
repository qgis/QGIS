/***************************************************************************
                         qgsalgorithmrasterlogicalop.cpp
                         ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmrasterlogicalop.h"

#include <algorithm>
#include <gdal.h>

#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterprojector.h"

///@cond PRIVATE


QStringList QgsRasterBooleanLogicAlgorithmBase::tags() const
{
  return QObject::tr( "logical,boolean" ).split( ',' );
}

QString QgsRasterBooleanLogicAlgorithmBase::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterBooleanLogicAlgorithmBase::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsRasterBooleanLogicAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"INPUT"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Raster ) );

  addParameter( new QgsProcessingParameterRasterLayer( u"REF_LAYER"_s, QObject::tr( "Reference layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"NODATA_AS_FALSE"_s, QObject::tr( "Treat NoData values as false" ), false ) );

  auto noDataValueParam = std::make_unique<QgsProcessingParameterNumber>( u"NO_DATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999 );
  noDataValueParam->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( noDataValueParam.release() );

  std::unique_ptr<QgsProcessingParameterDefinition> typeChoice = QgsRasterAnalysisUtils::createRasterTypeParameter( u"DATA_TYPE"_s, QObject::tr( "Output data type" ), Qgis::DataType::Float32 );
  typeChoice->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( typeChoice.release() );

  // backwards compatibility parameter
  // TODO QGIS 5: remove parameter and related logic
  auto createOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATE_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  createOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  createOptsParam->setFlags( createOptsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( createOptsParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Output layer" ) ) );

  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTAL_PIXEL_COUNT"_s, QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"NODATA_PIXEL_COUNT"_s, QObject::tr( "NoData pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TRUE_PIXEL_COUNT"_s, QObject::tr( "True pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"FALSE_PIXEL_COUNT"_s, QObject::tr( "False pixel count" ) ) );
}

bool QgsRasterBooleanLogicAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, u"REF_LAYER"_s, context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"REF_LAYER"_s ) );
  mCrs = referenceLayer->crs();
  mRasterUnitsPerPixelX = referenceLayer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = referenceLayer->rasterUnitsPerPixelY();
  mLayerWidth = referenceLayer->width();
  mLayerHeight = referenceLayer->height();
  mExtent = referenceLayer->extent();
  mNoDataValue = parameterAsDouble( parameters, u"NO_DATA"_s, context );
  mDataType = QgsRasterAnalysisUtils::rasterTypeChoiceToDataType( parameterAsEnum( parameters, u"DATA_TYPE"_s, context ) );
  if ( mDataType == Qgis::DataType::Int8 && atoi( GDALVersionInfo( "VERSION_NUM" ) ) < GDAL_COMPUTE_VERSION( 3, 7, 0 ) )
    throw QgsProcessingException( QObject::tr( "Int8 data type requires GDAL version 3.7 or later" ) );

  mTreatNodataAsFalse = parameterAsBoolean( parameters, u"NODATA_AS_FALSE"_s, context );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"INPUT"_s, context );
  QList<QgsRasterLayer *> rasterLayers;
  rasterLayers.reserve( layers.count() );
  for ( QgsMapLayer *l : layers )
  {
    if ( l->type() == Qgis::LayerType::Raster )
    {
      QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( l );
      QgsRasterAnalysisUtils::RasterLogicInput input;
      const int band = 1; // hardcoded for now - needs a way to supply this in the processing gui
      input.hasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
      input.sourceDataProvider.reset( layer->dataProvider()->clone() );
      input.interface = input.sourceDataProvider.get();
      // add projector if necessary
      if ( layer->crs() != mCrs )
      {
        input.projector = std::make_unique<QgsRasterProjector>();
        input.projector->setInput( input.sourceDataProvider.get() );
        input.projector->setCrs( layer->crs(), mCrs, context.transformContext() );
        input.interface = input.projector.get();
      }
      mInputs.emplace_back( std::move( input ) );
    }
  }

  return true;
}

QVariantMap QgsRasterBooleanLogicAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  // handle backwards compatibility parameter CREATE_OPTIONS
  const QString optionsString = parameterAsString( parameters, u"CREATE_OPTIONS"_s, context );
  if ( !optionsString.isEmpty() )
    creationOptions = optionsString;

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    writer->setCreationOptions( creationOptions.split( '|' ) );
  }
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  qgssize noDataCount = 0;
  qgssize trueCount = 0;
  qgssize falseCount = 0;
  const qgssize layerSize = static_cast<qgssize>( mLayerWidth ) * static_cast<qgssize>( mLayerHeight );

  QgsRasterAnalysisUtils::applyRasterLogicOperator( mInputs, std::move( provider ), mNoDataValue, mTreatNodataAsFalse, mLayerWidth, mLayerHeight, mExtent, feedback, mExtractValFunc, noDataCount, trueCount, falseCount );

  QVariantMap outputs;
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );
  outputs.insert( u"TOTAL_PIXEL_COUNT"_s, layerSize );
  outputs.insert( u"NODATA_PIXEL_COUNT"_s, noDataCount );
  outputs.insert( u"TRUE_PIXEL_COUNT"_s, trueCount );
  outputs.insert( u"FALSE_PIXEL_COUNT"_s, falseCount );
  outputs.insert( u"OUTPUT"_s, outputFile );

  return outputs;
}


//
// QgsRasterLogicalOrAlgorithm
//

QgsRasterLogicalOrAlgorithm::QgsRasterLogicalOrAlgorithm()
{
  mExtractValFunc = []( const std::vector<std::unique_ptr<QgsRasterBlock>> &inputs, bool &res, bool &resIsNoData, int row, int column, bool treatNoDataAsFalse ) {
    res = false;
    resIsNoData = false;
    bool isNoData = false;
    for ( auto &block : inputs )
    {
      double value = 0;
      if ( !block || !block->isValid() )
      {
        if ( treatNoDataAsFalse )
          continue;
        else
        {
          resIsNoData = true;
          break;
        }
      }
      else
      {
        value = block->valueAndNoData( row, column, isNoData );
        if ( isNoData && !treatNoDataAsFalse )
        {
          resIsNoData = true;
          break;
        }
        else if ( !qgsDoubleNear( value, 0.0 ) && !isNoData )
        {
          res = true;
          if ( treatNoDataAsFalse ) // otherwise we need to check all remaining rasters for nodata
            break;
        }
      }
    }
  };
}

QString QgsRasterLogicalOrAlgorithm::name() const
{
  return u"rasterlogicalor"_s;
}

QString QgsRasterLogicalOrAlgorithm::displayName() const
{
  return QObject::tr( "Raster boolean OR" );
}


QString QgsRasterLogicalOrAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the boolean OR for a set of input raster layers." );
}

QString QgsRasterLogicalOrAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the boolean OR for a set of input rasters. If any of the input rasters have a non-zero value for a pixel, "
                      "that pixel will be set to 1 in the output raster. If all the input rasters have 0 values for the pixel it will be set to 0 in the output raster.\n\n"
                      "The reference layer parameter specifies an existing raster layer to use as a reference when creating the output raster. The output raster "
                      "will have the same extent, CRS, and pixel dimensions as this layer.\n\n"
                      "By default, a NoData pixel in ANY of the input layers will result in a NoData pixel in the output raster. If the "
                      "'Treat NoData values as false' option is checked, then NoData inputs will be treated the same as a 0 input value." );
}

QgsRasterLogicalOrAlgorithm *QgsRasterLogicalOrAlgorithm::createInstance() const
{
  return new QgsRasterLogicalOrAlgorithm();
}

//
// QgsRasterLogicalAndAlgorithm
//

QgsRasterLogicalAndAlgorithm::QgsRasterLogicalAndAlgorithm()
{
  mExtractValFunc = []( const std::vector<std::unique_ptr<QgsRasterBlock>> &inputs, bool &res, bool &resIsNoData, int row, int column, bool treatNoDataAsFalse ) {
    res = true;
    resIsNoData = false;
    bool isNoData = false;
    for ( auto &block : inputs )
    {
      double value = 0;
      if ( !block || !block->isValid() )
      {
        if ( treatNoDataAsFalse )
        {
          res = false;
          break;
        }
        else
        {
          resIsNoData = true;
          break;
        }
      }
      else
      {
        value = block->valueAndNoData( row, column, isNoData );
        if ( isNoData && !treatNoDataAsFalse )
        {
          resIsNoData = true;
          break;
        }
        else if ( qgsDoubleNear( value, 0.0 ) || isNoData )
        {
          res = false;
          if ( treatNoDataAsFalse ) // otherwise we need to check remaining rasters for nodata
            break;
        }
      }
    }
  };
}

QString QgsRasterLogicalAndAlgorithm::name() const
{
  return u"rasterbooleanand"_s;
}

QString QgsRasterLogicalAndAlgorithm::displayName() const
{
  return QObject::tr( "Raster boolean AND" );
}


QString QgsRasterLogicalAndAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the boolean AND for a set of input raster layers." );
}

QString QgsRasterLogicalAndAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the boolean AND for a set of input rasters. If all of the input rasters have a non-zero value for a pixel, "
                      "that pixel will be set to 1 in the output raster. If any of the input rasters have 0 values for the pixel it will be set to 0 in the output raster.\n\n"
                      "The reference layer parameter specifies an existing raster layer to use as a reference when creating the output raster. The output raster "
                      "will have the same extent, CRS, and pixel dimensions as this layer.\n\n"
                      "By default, a NoData pixel in ANY of the input layers will result in a NoData pixel in the output raster. If the "
                      "'Treat NoData values as false' option is checked, then NoData inputs will be treated the same as a 0 input value." );
}

QgsRasterLogicalAndAlgorithm *QgsRasterLogicalAndAlgorithm::createInstance() const
{
  return new QgsRasterLogicalAndAlgorithm();
}

///@endcond
