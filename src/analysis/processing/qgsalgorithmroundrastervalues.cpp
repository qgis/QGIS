/***************************************************************************
                         qgsalgorithmroundrastervalues.cpp
                         ---------------------
    begin                : April 2020
    copyright            : (C) 2020 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmroundrastervalues.h"

#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsRoundRasterValuesAlgorithm::name() const
{
  return u"roundrastervalues"_s;
}

QString QgsRoundRasterValuesAlgorithm::displayName() const
{
  return QObject::tr( "Round raster" );
}

QStringList QgsRoundRasterValuesAlgorithm::tags() const
{
  return QObject::tr( "data,cells,round,truncate" ).split( ',' );
}

QString QgsRoundRasterValuesAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRoundRasterValuesAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsRoundRasterValuesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, u"Input raster"_s ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum( u"ROUNDING_DIRECTION"_s, QObject::tr( "Rounding direction" ), QStringList() << QObject::tr( "Round up" ) << QObject::tr( "Round to nearest" ) << QObject::tr( "Round down" ), false, 1 ) );
  addParameter( new QgsProcessingParameterNumber( u"DECIMAL_PLACES"_s, QObject::tr( "Number of decimals places" ), Qgis::ProcessingNumberParameterType::Integer, 2 ) );
  std::unique_ptr<QgsProcessingParameterDefinition> baseParameter = std::make_unique<QgsProcessingParameterNumber>( u"BASE_N"_s, QObject::tr( "Base n for rounding to multiples of n" ), Qgis::ProcessingNumberParameterType::Integer, 10, true, 1 );
  baseParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( baseParameter.release() );

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

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Output raster" ) ) );
}

QString QgsRoundRasterValuesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm rounds the cell values of a raster dataset to the specified number of decimals.\n "
                      "Alternatively, a negative number of decimal places may be used to round values to powers of a base n "
                      "(specified in the advanced parameter Base n). For example, with a Base value n of 10 and Decimal places of -1 "
                      "the algorithm rounds cell values to multiples of 10, -2 rounds to multiples of 100, and so on. Arbitrary base values "
                      "may be chosen, the algorithm applies the same multiplicative principle. Rounding cell values to multiples of "
                      "a base n may be used to generalize raster layers.\n"
                      "The algorithm preserves the data type of the input raster. Therefore byte/integer rasters can only be rounded "
                      "to multiples of a base n, otherwise a warning is raised and the raster gets copied as byte/integer raster." );
}

QString QgsRoundRasterValuesAlgorithm::shortDescription() const
{
  return QObject::tr( "Rounds the cell values of a raster dataset to a specified number of decimals." );
}

QgsRoundRasterValuesAlgorithm *QgsRoundRasterValuesAlgorithm::createInstance() const
{
  return new QgsRoundRasterValuesAlgorithm();
}

bool QgsRoundRasterValuesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  QgsRasterLayer *inputRaster = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  mDecimalPrecision = parameterAsInt( parameters, u"DECIMAL_PLACES"_s, context );
  mBaseN = parameterAsInt( parameters, u"BASE_N"_s, context );
  mMultipleOfBaseN = pow( mBaseN, abs( mDecimalPrecision ) );
  mScaleFactor = std::pow( 10.0, mDecimalPrecision );

  if ( !inputRaster )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > inputRaster->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( inputRaster->bandCount() ) );

  mRoundingDirection = parameterAsEnum( parameters, u"ROUNDING_DIRECTION"_s, context );

  mInterface.reset( inputRaster->dataProvider()->clone() );
  mDataType = mInterface->dataType( mBand );

  switch ( mDataType )
  {
    case Qgis::DataType::Byte:
    case Qgis::DataType::Int16:
    case Qgis::DataType::UInt16:
    case Qgis::DataType::Int32:
    case Qgis::DataType::UInt32:
      mIsInteger = true;
      if ( mDecimalPrecision > -1 )
        feedback->reportError( QObject::tr( "Input raster is of byte or integer type. The cell values cannot be rounded and will be output using the same data type." ), false );
      break;
    default:
      mIsInteger = false;
      break;
  }

  mInputNoDataValue = inputRaster->dataProvider()->sourceNoDataValue( mBand );
  mExtent = inputRaster->extent();
  mLayerWidth = inputRaster->width();
  mLayerHeight = inputRaster->height();
  mCrs = inputRaster->crs();
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();
  return true;
}

QVariantMap QgsRoundRasterValuesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  //prepare output dataset
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
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( mInterface->dataType( mBand ), mNbCellsXProvider, mNbCellsYProvider, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  //prepare output provider
  QgsRasterDataProvider *destinationRasterProvider;
  destinationRasterProvider = provider.get();
  destinationRasterProvider->setEditable( true );
  destinationRasterProvider->setNoDataValue( 1, mInputNoDataValue );

  const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  const int nbBlocksWidth = static_cast<int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  const int nbBlocksHeight = static_cast<int>( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  const int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr<QgsRasterBlock> analysisRasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, analysisRasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    if ( mIsInteger && mDecimalPrecision > -1 )
    {
      //nothing to round, just write raster block
      analysisRasterBlock->setNoDataValue( mInputNoDataValue );
      if ( !destinationRasterProvider->writeBlock( analysisRasterBlock.get(), mBand, iterLeft, iterTop ) )
      {
        throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationRasterProvider->error().summary() ) );
      }
    }
    else
    {
      for ( int row = 0; row < iterRows; row++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;
        for ( int column = 0; column < iterCols; column++ )
        {
          bool isNoData = false;
          const double val = analysisRasterBlock->valueAndNoData( row, column, isNoData );
          if ( isNoData )
          {
            analysisRasterBlock->setValue( row, column, mInputNoDataValue );
          }
          else
          {
            double roundedVal = mInputNoDataValue;
            if ( mRoundingDirection == 0 && mDecimalPrecision < 0 )
            {
              roundedVal = roundUpBaseN( val );
            }
            else if ( mRoundingDirection == 0 && mDecimalPrecision > -1 )
            {
              const double m = ( val < 0.0 ) ? -1.0 : 1.0;
              roundedVal = roundUp( val, m );
            }
            else if ( mRoundingDirection == 1 && mDecimalPrecision < 0 )
            {
              roundedVal = roundNearestBaseN( val );
            }
            else if ( mRoundingDirection == 1 && mDecimalPrecision > -1 )
            {
              const double m = ( val < 0.0 ) ? -1.0 : 1.0;
              roundedVal = roundNearest( val, m );
            }
            else if ( mRoundingDirection == 2 && mDecimalPrecision < 0 )
            {
              roundedVal = roundDownBaseN( val );
            }
            else
            {
              const double m = ( val < 0.0 ) ? -1.0 : 1.0;
              roundedVal = roundDown( val, m );
            }
            //integer values get automatically cast to double when reading and back to int when writing
            analysisRasterBlock->setValue( row, column, roundedVal );
          }
        }
      }
      if ( !destinationRasterProvider->writeBlock( analysisRasterBlock.get(), mBand, iterLeft, iterTop ) )
      {
        throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationRasterProvider->error().summary() ) );
      }
    }
  }
  destinationRasterProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

double QgsRoundRasterValuesAlgorithm::roundNearest( double value, double m )
{
  return ( std::round( value * m * mScaleFactor ) / mScaleFactor ) * m;
}

double QgsRoundRasterValuesAlgorithm::roundUp( double value, double m )
{
  return ( std::ceil( value * m * mScaleFactor ) / mScaleFactor ) * m;
}

double QgsRoundRasterValuesAlgorithm::roundDown( double value, double m )
{
  return ( std::floor( value * m * mScaleFactor ) / mScaleFactor ) * m;
}

double QgsRoundRasterValuesAlgorithm::roundNearestBaseN( double value )
{
  return static_cast<double>( mMultipleOfBaseN * round( value / mMultipleOfBaseN ) );
}

double QgsRoundRasterValuesAlgorithm::roundUpBaseN( double value )
{
  return static_cast<double>( mMultipleOfBaseN * ceil( value / mMultipleOfBaseN ) );
}

double QgsRoundRasterValuesAlgorithm::roundDownBaseN( double value )
{
  return static_cast<double>( mMultipleOfBaseN * floor( value / mMultipleOfBaseN ) );
}

///@endcond
