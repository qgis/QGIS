/***************************************************************************
                         qgsalgorithmflowconnectivity.cpp
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

#include "qgsalgorithmflowconnectivity.h"

#include "qgsacademicreference.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsFlowConnectivityD8Algorithm::name() const
{
  return u"flowconnectivity"_s;
}

QString QgsFlowConnectivityD8Algorithm::displayName() const
{
  return QObject::tr( "Flow connectivity" );
}

QStringList QgsFlowConnectivityD8Algorithm::tags() const
{
  return QObject::tr( "dem,flow,connectivity,d8,confluence,topology,hydrology,drainage" ).split( ',' );
}

QString QgsFlowConnectivityD8Algorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsFlowConnectivityD8Algorithm::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QString QgsFlowConnectivityD8Algorithm::shortDescription() const
{
  return QObject::tr( "Calculates the number of adjacent cells flowing directly into each grid cell using D8 flow routing." );
}

QString QgsFlowConnectivityD8Algorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates deterministic 8 (D8) flow connectivity for each cell in an input elevation raster (DEM).\n\n"
    "Output cell values represent the number of immediate 8-neighbor adjacent cells (0 to 8) whose D8 steepest downslope flow direction points directly into the cell:\n"
    "• 0 = Ridge, crest, or spring cell receiving no incoming surface flow.\n"
    "• 1 = Channel segment cell receiving flow from a single upstream neighbor.\n"
    "• 2+ = Stream junction or confluence cell receiving flow from multiple converging upstream paths.\n\n"
    "This algorithm is a port of the flow connectivity calculation from SAGA 'Channel Network and Drainage Basins' tool."
  );
}

QList<QgsAcademicReference> QgsFlowConnectivityD8Algorithm::academicReferences() const
{
  const QgsAcademicReference ocallaghanReference = QgsAcademicReference::
    createJournalArticle( { u"O'Callaghan, J. F."_s, u"Mark, D. M."_s }, 1984, u"The extraction of drainage networks from digital elevation data"_s, u"Computer Vision, Graphics and Image Processing"_s, u"28"_s, QString(), u"323-344"_s );
  return { ocallaghanReference };
}

QList<QgsProcessingAlgorithm::ExternalLink> QgsFlowConnectivityD8Algorithm::externalLinks() const
{
  return {
    QgsProcessingAlgorithm::ExternalLink { QObject::tr( "SAGA tool source code" ), u"https://sourceforge.net/p/saga-gis/code/ci/33d1062b7120c696c9dd258378c48d86dc33560c/tree/saga-gis/src/tools/terrain_analysis/ta_channels/D8_Flow_Analysis.cpp"_s }
  };
}

void QgsFlowConnectivityD8Algorithm::initAlgorithm( const QVariantMap & )
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

QgsProcessingAlgorithm *QgsFlowConnectivityD8Algorithm::createInstance() const
{
  return new QgsFlowConnectivityD8Algorithm();
}

bool QgsFlowConnectivityD8Algorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
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

QVariantMap QgsFlowConnectivityD8Algorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const QString outputPath = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );
  const int outputNoData = parameterAsInt( parameters, u"NODATA"_s, context );

  std::unique_ptr<QgsRasterBlock> inputBlock( mInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !inputBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input raster block." ) );

  const qgssize totalCells = static_cast<qgssize>( mLayerWidth ) * mLayerHeight;

  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );
  multiStepFeedback.setCurrentStep( 0 );

  std::vector<int8_t> d8Directions( totalCells, -1 );

  // process D8 steepest gradient direction for every grid cell (matching SAGA CD8_Flow_Analysis::Get_Direction)
  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( feedback->isCanceled() )
      return {};

    multiStepFeedback.setProgress( 100.0 * static_cast<double>( row ) / mLayerHeight );

    const qgssize rowOffset = static_cast<qgssize>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      const int dir = QgsRasterAnalysisUtils::steepestGradientDirection( inputBlock.get(), row, col, mCellSizeX, mCellSizeY, true, true );
      d8Directions[rowOffset + col] = static_cast<int8_t>( dir );
    }
  }

  auto outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int16, mLayerWidth, mLayerHeight );
  outputBlock->setNoDataValue( outputNoData );

  // pass 2: calculate incoming flow connectivity (matching SAGA CD8_Flow_Analysis::Get_Direction connectivity pass)
  multiStepFeedback.setCurrentStep( 1 );
  int16_t *outData = reinterpret_cast<int16_t *>( outputBlock->bits() );

  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( feedback->isCanceled() )
      return {};

    multiStepFeedback.setProgress( static_cast<double>( row ) / mLayerHeight );

    const qgssize rowOffset = static_cast<qgssize>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      if ( inputBlock->isNoData( row, col ) )
      {
        outData[rowOffset + col] = outputNoData;
        continue;
      }

      int incomingCount = 0;
      for ( int dir = 0; dir < 8; ++dir )
      {
        const int oppositeDir = ( dir + 4 ) % 8;
        int neighborCol = 0;
        int neighborRow = 0;
        if ( QgsRasterAnalysisUtils::neighborCellCoordinates( oppositeDir, row, col, neighborRow, neighborCol, mLayerHeight, mLayerWidth ) )
        {
          const qgssize neighborIdx = static_cast<qgssize>( neighborRow ) * mLayerWidth + neighborCol;
          if ( d8Directions[neighborIdx] == dir )
          {
            incomingCount++;
          }
        }
      }

      outData[rowOffset + col] = static_cast<int16_t>( incomingCount );
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
