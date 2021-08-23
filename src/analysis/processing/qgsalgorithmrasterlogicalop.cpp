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
#include "qgsrasterprojector.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasteranalysisutils.h"
#include <algorithm>

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
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterBooleanLogicAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layers" ), QgsProcessing::TypeRaster ) );

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "REF_LAYER" ), QObject::tr( "Reference layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "NODATA_AS_FALSE" ), QObject::tr( "Treat nodata values as false" ), false ) );

  std::unique_ptr< QgsProcessingParameterNumber > noDataValueParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "NO_DATA" ),
      QObject::tr( "Output no data value" ), QgsProcessingParameterNumber::Double, -9999 );
  noDataValueParam->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( noDataValueParam.release() );

  std::unique_ptr< QgsProcessingParameterDefinition > typeChoice = QgsRasterAnalysisUtils::createRasterTypeParameter( QStringLiteral( "DATA_TYPE" ), QObject::tr( "Output data type" ), Qgis::DataType::Float32 );
  typeChoice->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( typeChoice.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Output layer" ) ) );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NODATA_PIXEL_COUNT" ), QObject::tr( "NODATA pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TRUE_PIXEL_COUNT" ), QObject::tr( "True pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "FALSE_PIXEL_COUNT" ), QObject::tr( "False pixel count" ) ) );
}

bool QgsRasterBooleanLogicAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, QStringLiteral( "REF_LAYER" ), context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "REF_LAYER" ) ) );
  mCrs = referenceLayer->crs();
  mRasterUnitsPerPixelX = referenceLayer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = referenceLayer->rasterUnitsPerPixelY();
  mLayerWidth = referenceLayer->width();
  mLayerHeight = referenceLayer->height();
  mExtent = referenceLayer->extent();
  mNoDataValue = parameterAsDouble( parameters, QStringLiteral( "NO_DATA" ), context );
  mDataType = QgsRasterAnalysisUtils::rasterTypeChoiceToDataType( parameterAsEnum( parameters, QStringLiteral( "DATA_TYPE" ), context ) );

  mTreatNodataAsFalse = parameterAsBoolean( parameters, QStringLiteral( "NODATA_AS_FALSE" ), context );

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "INPUT" ), context );
  QList< QgsRasterLayer * > rasterLayers;
  rasterLayers.reserve( layers.count() );
  for ( QgsMapLayer *l : layers )
  {
    if ( l->type() == QgsMapLayerType::RasterLayer )
    {
      QgsRasterLayer *layer = qobject_cast< QgsRasterLayer * >( l );
      QgsRasterAnalysisUtils::RasterLogicInput input;
      const int band = 1; // hardcoded for now - needs a way to supply this in the processing gui
      input.hasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
      input.sourceDataProvider.reset( layer->dataProvider()->clone() );
      input.interface = input.sourceDataProvider.get();
      // add projector if necessary
      if ( layer->crs() != mCrs )
      {
        input.projector = std::make_unique< QgsRasterProjector >();
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
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  qgssize noDataCount = 0;
  qgssize trueCount = 0;
  qgssize falseCount = 0;
  const qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );

  QgsRasterAnalysisUtils::applyRasterLogicOperator( mInputs, provider.get(), mNoDataValue, mTreatNodataAsFalse, mLayerWidth, mLayerHeight,
      mExtent, feedback, mExtractValFunc, noDataCount, trueCount, falseCount );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );
  outputs.insert( QStringLiteral( "TOTAL_PIXEL_COUNT" ), layerSize );
  outputs.insert( QStringLiteral( "NODATA_PIXEL_COUNT" ), noDataCount );
  outputs.insert( QStringLiteral( "TRUE_PIXEL_COUNT" ), trueCount );
  outputs.insert( QStringLiteral( "FALSE_PIXEL_COUNT" ), falseCount );
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );

  return outputs;
}


//
// QgsRasterLogicalOrAlgorithm
//

QgsRasterLogicalOrAlgorithm::QgsRasterLogicalOrAlgorithm()
{
  mExtractValFunc = [ = ]( const std::vector< std::unique_ptr< QgsRasterBlock > > &inputs, bool & res, bool & resIsNoData, int row, int column, bool treatNoDataAsFalse )
  {
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
  return QStringLiteral( "rasterlogicalor" );
}

QString QgsRasterLogicalOrAlgorithm::displayName() const
{
  return QObject::tr( "Raster boolean OR" );
}


QString QgsRasterLogicalOrAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the boolean OR for a set of input raster layers" );
}

QString QgsRasterLogicalOrAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the boolean OR for a set of input rasters. If any of the input rasters have a non-zero value for a pixel, "
                      "that pixel will be set to 1 in the output raster. If all the input rasters have 0 values for the pixel it will be set to 0 in the output raster.\n\n"
                      "The reference layer parameter specifies an existing raster layer to use as a reference when creating the output raster. The output raster "
                      "will have the same extent, CRS, and pixel dimensions as this layer.\n\n"
                      "By default, a nodata pixel in ANY of the input layers will result in a nodata pixel in the output raster. If the "
                      "'Treat nodata values as false' option is checked, then nodata inputs will be treated the same as a 0 input value." );
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
  mExtractValFunc = [ = ]( const std::vector< std::unique_ptr< QgsRasterBlock > > &inputs, bool & res, bool & resIsNoData, int row, int column, bool treatNoDataAsFalse )
  {
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
  return QStringLiteral( "rasterbooleanand" );
}

QString QgsRasterLogicalAndAlgorithm::displayName() const
{
  return QObject::tr( "Raster boolean AND" );
}


QString QgsRasterLogicalAndAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the boolean AND for a set of input raster layers" );
}

QString QgsRasterLogicalAndAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the boolean AND for a set of input rasters. If all of the input rasters have a non-zero value for a pixel, "
                      "that pixel will be set to 1 in the output raster. If any of the input rasters have 0 values for the pixel it will be set to 0 in the output raster.\n\n"
                      "The reference layer parameter specifies an existing raster layer to use as a reference when creating the output raster. The output raster "
                      "will have the same extent, CRS, and pixel dimensions as this layer.\n\n"
                      "By default, a nodata pixel in ANY of the input layers will result in a nodata pixel in the output raster. If the "
                      "'Treat nodata values as false' option is checked, then nodata inputs will be treated the same as a 0 input value." );
}

QgsRasterLogicalAndAlgorithm *QgsRasterLogicalAndAlgorithm::createInstance() const
{
  return new QgsRasterLogicalAndAlgorithm();
}

///@endcond



