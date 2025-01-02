/***************************************************************************
                         qgsalgorithmrasterdtmslopebasedfilter.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsalgorithmrasterdtmslopebasedfilter.h"
#include "qgsrasterfilewriter.h"
#include <algorithm>

///@cond PRIVATE

QString QgsRasterDtmSlopeBasedFilterAlgorithm::name() const
{
  return QStringLiteral( "dtmslopebasedfilter" );
}

QString QgsRasterDtmSlopeBasedFilterAlgorithm::displayName() const
{
  return QObject::tr( "DTM filter (slope-based)" );
}

QStringList QgsRasterDtmSlopeBasedFilterAlgorithm::tags() const
{
  return QObject::tr( "dem,filter,slope,dsm,dtm,terrain" ).split( ',' );
}

QString QgsRasterDtmSlopeBasedFilterAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsRasterDtmSlopeBasedFilterAlgorithm::groupId() const
{
  return QStringLiteral( "rasterterrainanalysis" );
}

QString QgsRasterDtmSlopeBasedFilterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm can be used to filter a digital elevation model in order to classify its cells into ground and object (non-ground) cells.\n\n"
                      "The tool uses concepts as described by Vosselman (2000) and is based on the assumption that a large height difference between two nearby "
                      "cells is unlikely to be caused by a steep slope in the terrain. The probability that the higher cell might be non-ground increases when "
                      "the distance between the two cells decreases. Therefore the filter defines a maximum height difference (<i>dz_max</i>) between two cells as a "
                      "function of the distance (<i>d</i>) between the cells (<i>dz_max( d ) = d</i>).\n\n"
                      "A cell is classified as terrain if there is no cell within the kernel radius to which the height difference is larger than the allowed "
                      "maximum height difference at the distance between these two cells.\n\n"
                      "The approximate terrain slope (<i>s</i>) parameter is used to modify the filter function to match the overall slope in the study "
                      "area (<i>dz_max( d ) = d * s</i>).\n\n"
                      "A 5 % confidence interval (<i>ci = 1.65 * sqrt( 2 * stddev )</i>) may be used to modify the filter function even further by either "
                      "relaxing (<i>dz_max( d ) = d * s + ci</i>) or amplifying (<i>dz_max( d ) = d * s - ci</i>) the filter criterium.\n\n"
                      "References: Vosselman, G. (2000): Slope based filtering of laser altimetry data. IAPRS, Vol. XXXIII, Part B3, Amsterdam, The Netherlands, 935-942\n\n"
                      "This algorithm is a port of the SAGA 'DTM Filter (slope-based)' tool." );
}

void QgsRasterDtmSlopeBasedFilterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ), QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr<QgsProcessingParameterNumber> radiusParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "RADIUS" ), QObject::tr( "Kernel radius (pixels)" ), Qgis::ProcessingNumberParameterType::Integer, 5, false, 1, 1000 );
  radiusParam->setHelp( QObject::tr( "The radius of the filter kernel (in pixels). Must be large enough to reach ground cells next to non-ground objects." ) );
  addParameter( radiusParam.release() );

  std::unique_ptr<QgsProcessingParameterNumber> terrainSlopeParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "TERRAIN_SLOPE" ), QObject::tr( "Terrain slope (%, pixel size/vertical units)" ), Qgis::ProcessingNumberParameterType::Double, 30, false, 0, 1000 );
  terrainSlopeParam->setHelp( QObject::tr( "The approximate terrain slope in %. The terrain slope must be adjusted to account for the ratio of height units vs raster pixel dimensions. Used to relax the filter criterium in steeper terrain." ) );
  addParameter( terrainSlopeParam.release() );

  std::unique_ptr<QgsProcessingParameterEnum> filterModificationParam = std::make_unique<QgsProcessingParameterEnum>( QStringLiteral( "FILTER_MODIFICATION" ), QObject::tr( "Filter modification" ), QStringList { QObject::tr( "None" ), QObject::tr( "Relax filter" ), QObject::tr( "Amplify" ) }, false, 0 );
  filterModificationParam->setHelp( QObject::tr( "Choose whether to apply the filter kernel without modification or to use a confidence interval to relax or amplify the height criterium." ) );
  addParameter( filterModificationParam.release() );

  std::unique_ptr<QgsProcessingParameterNumber> stDevParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "STANDARD_DEVIATION" ), QObject::tr( "Standard deviation" ), Qgis::ProcessingNumberParameterType::Double, 0.1, false, 0, 1000 );
  stDevParam->setHelp( QObject::tr( "The standard deviation used to calculate a 5% confidence interval applied to the height threshold." ) );
  addParameter( stDevParam.release() );

  std::unique_ptr<QgsProcessingParameterString> createOptsParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "CREATE_OPTIONS" ), QObject::tr( "Creation options" ), QVariant(), false, true );
  createOptsParam->setMetadata( QVariantMap( { { QStringLiteral( "widget_wrapper" ), QVariantMap( { { QStringLiteral( "widget_type" ), QStringLiteral( "rasteroptions" ) } } ) } } ) );
  createOptsParam->setFlags( createOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( createOptsParam.release() );

  std::unique_ptr<QgsProcessingParameterRasterDestination> outputLayerGroundParam = std::make_unique<QgsProcessingParameterRasterDestination>( QStringLiteral( "OUTPUT_GROUND" ), QObject::tr( "Output layer (ground)" ), QVariant(), true, true );
  outputLayerGroundParam->setHelp( QObject::tr( "The filtered DEM containing only cells classified as ground." ) );
  addParameter( outputLayerGroundParam.release() );

  std::unique_ptr<QgsProcessingParameterRasterDestination> outputLayerNonGroundParam = std::make_unique<QgsProcessingParameterRasterDestination>( QStringLiteral( "OUTPUT_NONGROUND" ), QObject::tr( "Output layer (non-ground objects)" ), QVariant(), true, false );
  outputLayerNonGroundParam->setHelp( QObject::tr( "The non-ground objects removed by the filter." ) );
  addParameter( outputLayerNonGroundParam.release() );
}

QgsRasterDtmSlopeBasedFilterAlgorithm *QgsRasterDtmSlopeBasedFilterAlgorithm::createInstance() const
{
  return new QgsRasterDtmSlopeBasedFilterAlgorithm();
}

bool QgsRasterDtmSlopeBasedFilterAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );

  mBand = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();
  mDataType = layer->dataProvider()->dataType( mBand );
  mNoData = layer->dataProvider()->sourceNoDataValue( mBand );
  return true;
}

QVariantMap QgsRasterDtmSlopeBasedFilterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString createOptions = parameterAsString( parameters, QStringLiteral( "CREATE_OPTIONS" ), context ).trimmed();
  const QString groundOutputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT_GROUND" ), context );
  std::unique_ptr<QgsRasterFileWriter> groundWriter;
  std::unique_ptr<QgsRasterDataProvider> groundDestProvider;

  if ( !groundOutputFile.isEmpty() )
  {
    const QFileInfo fi( groundOutputFile );
    const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

    groundWriter = std::make_unique<QgsRasterFileWriter>( groundOutputFile );
    groundWriter->setOutputProviderKey( QStringLiteral( "gdal" ) );
    if ( !createOptions.isEmpty() )
    {
      groundWriter->setCreateOptions( createOptions.split( '|' ) );
    }
    groundWriter->setOutputFormat( outputFormat );

    groundDestProvider.reset( groundWriter->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );

    if ( !groundDestProvider )
      throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( groundOutputFile ) );
    if ( !groundDestProvider->isValid() )
      throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( groundOutputFile, groundDestProvider->error().message( QgsErrorMessage::Text ) ) );

    groundDestProvider->setNoDataValue( 1, mNoData );
    groundDestProvider->setEditable( true );
  }

  const QString nonGroundOutputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT_NONGROUND" ), context );
  std::unique_ptr<QgsRasterFileWriter> nonGroundWriter;
  std::unique_ptr<QgsRasterDataProvider> nonGroundDestProvider;

  if ( !nonGroundOutputFile.isEmpty() )
  {
    const QFileInfo fi( groundOutputFile );
    const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

    nonGroundWriter = std::make_unique<QgsRasterFileWriter>( nonGroundOutputFile );
    nonGroundWriter->setOutputProviderKey( QStringLiteral( "gdal" ) );
    if ( !createOptions.isEmpty() )
    {
      nonGroundWriter->setCreateOptions( createOptions.split( '|' ) );
    }
    nonGroundWriter->setOutputFormat( outputFormat );

    nonGroundDestProvider.reset( nonGroundWriter->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );

    if ( !nonGroundDestProvider )
      throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( nonGroundOutputFile ) );
    if ( !nonGroundDestProvider->isValid() )
      throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( nonGroundOutputFile, nonGroundDestProvider->error().message( QgsErrorMessage::Text ) ) );

    nonGroundDestProvider->setNoDataValue( 1, mNoData );
    nonGroundDestProvider->setEditable( true );
  }

  const int blockWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int blockHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  const int numBlocksX = static_cast<int>( std::ceil( 1.0 * mLayerWidth / blockWidth ) );
  const int numBlocksY = static_cast<int>( std::ceil( 1.0 * mLayerHeight / blockHeight ) );
  const int numBlocks = numBlocksX * numBlocksY;

  const int radius = parameterAsInt( parameters, QStringLiteral( "RADIUS" ), context );

  const double terrainSlopePercent = parameterAsDouble( parameters, QStringLiteral( "TERRAIN_SLOPE" ), context ) / 100; //20.0 / 100 * 0.143;
  const int filterModification = parameterAsEnum( parameters, QStringLiteral( "FILTER_MODIFICATION" ), context );
  const double standardDeviation = parameterAsDouble( parameters, QStringLiteral( "STANDARD_DEVIATION" ), context );

  // create kernel
  QVector<double> kernel;
  kernel.reserve( ( radius * 2 ) * ( radius * 2 ) );
  int kernelSize = 0;
  for ( int y = -radius; y <= radius; y++ )
  {
    for ( int x = -radius; x <= radius; x++ )
    {
      const double distance = std::sqrt( x * x + y * y );
      if ( distance < radius )
      {
        kernelSize++;
        kernel.push_back( x );
        kernel.push_back( y );
        switch ( filterModification )
        {
          case 0:
            kernel.push_back( distance * terrainSlopePercent );
            break;

          case 1:
            kernel.push_back( distance * terrainSlopePercent + 1.65 * std::sqrt( 2 * standardDeviation ) );
            break;

          case 2:
          {
            const double dz = distance * terrainSlopePercent - 1.65 * std::sqrt( 2 * standardDeviation );
            kernel.push_back( dz > 0 ? dz : 0 );
            break;
          }
        }
      }
    }
  }

  QgsRasterIterator iter( mInterface.get(), radius );
  iter.startRasterRead( 1, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  int tileLeft = 0;
  int tileTop = 0;
  int tileCols = 0;
  int tileRows = 0;

  QgsRectangle blockExtent;

  std::unique_ptr<QgsRasterBlock> inputBlock;
  int blockIndex = 0;
  while ( iter.readNextRasterPart( 1, iterCols, iterRows, inputBlock, iterLeft, iterTop, &blockExtent, &tileCols, &tileRows, &tileLeft, &tileTop ) )
  {
    std::unique_ptr<QgsRasterBlock> outputGroundBlock;
    if ( groundDestProvider )
      outputGroundBlock = std::make_unique<QgsRasterBlock>( mDataType, tileCols, tileRows );

    std::unique_ptr<QgsRasterBlock> outputNonGroundBlock;
    if ( nonGroundDestProvider )
      outputNonGroundBlock = std::make_unique<QgsRasterBlock>( mDataType, tileCols, tileRows );

    double baseProgress = static_cast<double>( blockIndex ) / numBlocks;
    feedback->setProgress( 100.0 * baseProgress );
    blockIndex++;
    if ( feedback->isCanceled() )
      break;

    const int tileBoundaryLeft = tileLeft - iterLeft;
    const int tileBoundaryTop = tileTop - iterTop;

    const double rowProgressStep = 1.0 / numBlocks / tileRows;
    double rowProgress = 0;
    for ( int row = tileBoundaryTop; row < tileBoundaryTop + tileRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100.0 * ( baseProgress + rowProgress ) );
      rowProgress += rowProgressStep;

      for ( int col = tileBoundaryLeft; col < tileBoundaryLeft + tileCols; col++ )
      {
        if ( feedback->isCanceled() )
          break;

        bool isNoData = false;
        const double val = inputBlock->valueAndNoData( row, col, isNoData );
        if ( isNoData )
        {
          if ( outputGroundBlock )
            outputGroundBlock->setValue( row - tileBoundaryTop, col - tileBoundaryLeft, mNoData );
          if ( outputNonGroundBlock )
            outputNonGroundBlock->setValue( row - tileBoundaryTop, col - tileBoundaryLeft, mNoData );
        }
        else
        {
          bool nonGround = false;
          const double *kernelData = kernel.constData();
          for ( int i = 0; i < kernelSize; ++i )
          {
            const int dx = static_cast<int>( *kernelData++ );
            const int dy = static_cast<int>( *kernelData++ );
            const double distance = *kernelData++;
            const int rCol = col + dx;
            const int rRow = row + dy;
            if ( rCol >= 0 && ( rCol < ( iterLeft + iterCols ) ) && rRow >= 0 && ( rRow < ( iterTop + iterRows ) ) )
            {
              bool otherIsNoData = false;
              const double otherVal = inputBlock->valueAndNoData( rRow, rCol, otherIsNoData );
              if ( !otherIsNoData )
              {
                const double dz = val - otherVal;
                if ( dz > 0 && dz > distance )
                {
                  nonGround = true;
                  break;
                }
              }
            }
          }
          if ( nonGround )
          {
            if ( outputGroundBlock )
              outputGroundBlock->setValue( row - tileBoundaryTop, col - tileBoundaryLeft, mNoData );
            if ( outputNonGroundBlock )
              outputNonGroundBlock->setValue( row - tileBoundaryTop, col - tileBoundaryLeft, val );
          }
          else
          {
            if ( outputGroundBlock )
              outputGroundBlock->setValue( row - tileBoundaryTop, col - tileBoundaryLeft, val );
            if ( outputNonGroundBlock )
              outputNonGroundBlock->setValue( row - tileBoundaryTop, col - tileBoundaryLeft, mNoData );
          }
        }
      }
    }
    if ( groundDestProvider )
      groundDestProvider->writeBlock( outputGroundBlock.get(), mBand, tileLeft, tileTop );
    if ( nonGroundDestProvider )
      nonGroundDestProvider->writeBlock( outputNonGroundBlock.get(), mBand, tileLeft, tileTop );
  }
  if ( groundDestProvider )
    groundDestProvider->setEditable( false );
  if ( nonGroundDestProvider )
    nonGroundDestProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_GROUND" ), groundOutputFile );
  outputs.insert( QStringLiteral( "OUTPUT_NONGROUND" ), nonGroundOutputFile );
  return outputs;
}


///@endcond
