/***************************************************************************
                         qgsalgorithmrescaleraster.cpp
                         ---------------------
    begin                : July 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#include "qgsalgorithmrescaleraster.h"

#include <limits>
#include <math.h>

#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsRescaleRasterAlgorithm::name() const
{
  return u"rescaleraster"_s;
}

QString QgsRescaleRasterAlgorithm::displayName() const
{
  return QObject::tr( "Rescale raster" );
}

QStringList QgsRescaleRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,rescale,minimum,maximum,range" ).split( ',' );
}

QString QgsRescaleRasterAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRescaleRasterAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsRescaleRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm rescales a raster layer to a new value range, while preserving the shape "
                      "(distribution) of the raster's histogram (pixel values). Input values "
                      "are mapped using a linear interpolation from the source raster's minimum "
                      "and maximum pixel values to the destination minimum and maximum pixel range.\n\n"
                      "By default the algorithm preserves the original NoData value, but there is "
                      "an option to override it." );
}

QString QgsRescaleRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Rescales a raster layer to a new value range, while preserving the shape "
                      "(distribution) of the raster's histogram (pixel values)." );
}

QgsRescaleRasterAlgorithm *QgsRescaleRasterAlgorithm::createInstance() const
{
  return new QgsRescaleRasterAlgorithm();
}

void QgsRescaleRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, u"Input raster"_s ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterNumber( u"MINIMUM"_s, QObject::tr( "New minimum value" ), Qgis::ProcessingNumberParameterType::Double, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"MAXIMUM"_s, QObject::tr( "New maximum value" ), Qgis::ProcessingNumberParameterType::Double, 255 ) );
  addParameter( new QgsProcessingParameterNumber( u"NODATA"_s, QObject::tr( "New NoData value" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true ) );

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

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Rescaled" ) ) );
}

bool QgsRescaleRasterAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" )
                                    .arg( mBand )
                                    .arg( layer->bandCount() ) );

  mMinimum = parameterAsDouble( parameters, u"MINIMUM"_s, context );
  mMaximum = parameterAsDouble( parameters, u"MAXIMUM"_s, context );

  mInterface.reset( layer->dataProvider()->clone() );

  mCrs = layer->crs();
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  if ( parameters.value( u"NODATA"_s ).isValid() )
  {
    mNoData = parameterAsDouble( parameters, u"NODATA"_s, context );
  }
  else
  {
    mNoData = layer->dataProvider()->sourceNoDataValue( mBand );
  }

  if ( std::isfinite( mNoData ) )
  {
    // Clamp nodata to float32 range, since that's the type of the raster
    if ( mNoData < std::numeric_limits<float>::lowest() )
      mNoData = std::numeric_limits<float>::lowest();
    else if ( mNoData > std::numeric_limits<float>::max() )
      mNoData = std::numeric_limits<float>::max();
  }

  mXSize = mInterface->xSize();
  mYSize = mInterface->ySize();

  return true;
}

QVariantMap QgsRescaleRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  feedback->pushInfo( QObject::tr( "Calculating raster minimum and maximum values…" ) );
  const QgsRasterBandStats stats = mInterface->bandStatistics( mBand, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max, QgsRectangle(), 0 );

  feedback->pushInfo( QObject::tr( "Rescaling values…" ) );

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
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( Qgis::DataType::Float32, mXSize, mYSize, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  QgsRasterDataProvider *destProvider = provider.get();
  destProvider->setEditable( true );
  destProvider->setNoDataValue( 1, mNoData );

  const bool hasReportsDuringClose = provider->hasReportsDuringClose();
  const double maxProgressDuringBlockWriting = hasReportsDuringClose ? 50.0 : 100.0;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr<QgsRasterBlock> inputBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, inputBlock, iterLeft, iterTop ) )
  {
    auto outputBlock = std::make_unique<QgsRasterBlock>( destProvider->dataType( 1 ), iterCols, iterRows );
    feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( mBand ) );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      for ( int col = 0; col < iterCols; col++ )
      {
        bool isNoData = false;
        const double val = inputBlock->valueAndNoData( row, col, isNoData );
        if ( isNoData )
        {
          outputBlock->setValue( row, col, mNoData );
        }
        else
        {
          const double newValue = ( ( val - stats.minimumValue ) * ( mMaximum - mMinimum ) / ( stats.maximumValue - stats.minimumValue ) ) + mMinimum;
          outputBlock->setValue( row, col, newValue );
        }
      }
    }
    if ( !destProvider->writeBlock( outputBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destProvider->error().summary() ) );
    }
  }
  destProvider->setEditable( false );

  if ( hasReportsDuringClose )
  {
    std::unique_ptr<QgsFeedback> scaledFeedback( QgsFeedback::createScaledFeedback( feedback, maxProgressDuringBlockWriting, 100.0 ) );
    if ( !provider->closeWithProgress( scaledFeedback.get() ) )
    {
      if ( feedback->isCanceled() )
        return {};
      throw QgsProcessingException( QObject::tr( "Could not write raster dataset" ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
