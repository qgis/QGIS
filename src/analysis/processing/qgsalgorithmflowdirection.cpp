/***************************************************************************
                         qgsalgorithmflowdirection.cpp
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsalgorithmflowdirection.h"

#include "qgsacademicreference.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsFlowDirectionD8Algorithm::name() const
{
  return u"flowdirection"_s;
}

QString QgsFlowDirectionD8Algorithm::displayName() const
{
  return QObject::tr( "Flow direction" );
}

QStringList QgsFlowDirectionD8Algorithm::tags() const
{
  return QObject::tr( "dem,flow,direction,d8,steepest,hydrology,catchment,drainage" ).split( ',' );
}

QString QgsFlowDirectionD8Algorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsFlowDirectionD8Algorithm::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QString QgsFlowDirectionD8Algorithm::shortDescription() const
{
  return QObject::tr( "Calculates single-direction D8 flow directions from a digital elevation model." );
}

QString QgsFlowDirectionD8Algorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates deterministic 8 (D8) flow directions for each cell in an input elevation raster (DEM).\n\n"
    "Flow direction values are output as 8-neighbor directional indices numbered clockwise starting from North:\n"
    "0 = North, 1 = North-East, 2 = East, 3 = South-East, 4 = South, 5 = South-West, 6 = West, 7 = North-West.\n\n"
    "Cells with no downslope neighbor or NoData elevation values are assigned nodata in the output.\n\n"
    "This algorithm is a port of the flow direction calculation from SAGA 'Channel Network and Drainage Basins' tool."
  );
}

QList<QgsAcademicReference> QgsFlowDirectionD8Algorithm::academicReferences() const
{
  const QgsAcademicReference ocallaghanReference = QgsAcademicReference::
    createJournalArticle( { u"O'Callaghan, J. F."_s, u"Mark, D. M."_s }, 1984, u"The extraction of drainage networks from digital elevation data"_s, u"Computer Vision, Graphics and Image Processing"_s, u"28"_s, QString(), u"323-344"_s );
  return { ocallaghanReference };
}

QList<QgsProcessingAlgorithm::ExternalLink> QgsFlowDirectionD8Algorithm::externalLinks() const
{
  return {
    QgsProcessingAlgorithm::ExternalLink { QObject::tr( "SAGA tool source code" ), u"https://sourceforge.net/p/saga-gis/code/ci/33d1062b7120c696c9dd258378c48d86dc33560c/tree/saga-gis/src/tools/terrain_analysis/ta_channels/D8_Flow_Analysis.cpp"_s }
  };
}

void QgsFlowDirectionD8Algorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Integer, -9999 );
  outputNodataParam->setHelp( QObject::tr( "The NODATA value to use in the output raster." ) );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setHelp( QObject::tr( "The raster creation options for the output raster. These options control things like colorimetry, compression, etc." ) );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Flow direction" ) );
  addParameter( outputParam.release() );
}

QgsProcessingAlgorithm *QgsFlowDirectionD8Algorithm::createInstance() const
{
  return new QgsFlowDirectionD8Algorithm();
}

bool QgsFlowDirectionD8Algorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer || !layer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mCellSizeX = layer->rasterUnitsPerPixelX();
  mCellSizeY = layer->rasterUnitsPerPixelY();

  return true;
}

QVariantMap QgsFlowDirectionD8Algorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const QString outputPath = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );
  const int outputNoData = parameterAsInt( parameters, u"NODATA"_s, context );

  std::unique_ptr<QgsRasterBlock> inputBlock( mInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !inputBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input raster block." ) );

  auto outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int16, mLayerWidth, mLayerHeight );
  outputBlock->setNoDataValue( outputNoData );

  int16_t *outData = reinterpret_cast<int16_t *>( outputBlock->bits() );

  // process D8 steepest gradient direction for every grid cell (matching SAGA CD8_Flow_Analysis::Get_Direction)
  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( feedback->isCanceled() )
      return {};

    feedback->setProgress( 100.0 * static_cast<double>( row ) / mLayerHeight );

    const std::size_t rowOffset = static_cast<std::size_t>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      const int dir = QgsRasterAnalysisUtils::steepestGradientDirection( inputBlock.get(), row, col, mCellSizeX, mCellSizeY, true, true );
      outData[rowOffset + col] = dir == -1 ? outputNoData : static_cast<int16_t>( dir );
    }
  }

  auto outputWriter = std::make_unique<QgsRasterFileWriter>( outputPath );
  outputWriter->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    outputWriter->setCreationOptions( creationOptions.split( '|' ) );
  }
  outputWriter->setOutputFormat( outputFormat );

  std::unique_ptr<QgsRasterDataProvider> destProvider( outputWriter->createOneBandRaster( Qgis::DataType::Int16, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !destProvider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputPath ) );
  if ( !destProvider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputPath, destProvider->error().message( QgsErrorMessage::Text ) ) );

  destProvider->setNoDataValue( 1, outputNoData );
  destProvider->setEditable( true );

  if ( !destProvider->writeBlock( outputBlock.get(), 1 ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destProvider->error().summary() ) );
  }

  destProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputPath );
  return outputs;
}

///@endcond
