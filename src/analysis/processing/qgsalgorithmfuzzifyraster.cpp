/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.cpp
                         ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Clemens Raffler
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

#include "qgsalgorithmfuzzifyraster.h"

#include "qgsrasterfilewriter.h"
#include "qgsstringutils.h"

///@cond PRIVATE

//
// QgsFuzzifyRasterAlgorithmBase
//

QString QgsFuzzifyRasterAlgorithmBase::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsFuzzifyRasterAlgorithmBase::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsFuzzifyRasterAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input Raster" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band Number" ), 1, u"INPUT"_s ) );

  //add specific fuzzification parameters from subclass alg
  addAlgorithmParams();

  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTAL_PIXEL_COUNT"_s, QObject::tr( "Total pixel count" ) ) );

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

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Fuzzified raster" ) ) );
}

bool QgsFuzzifyRasterAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mInputRaster = parameterAsRasterLayer( parameters, u"INPUT"_s, context );

  if ( !mInputRaster )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > mInputRaster->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( mInputRaster->bandCount() ) );

  mInterface.reset( mInputRaster->dataProvider()->clone() );
  mExtent = mInputRaster->extent();
  mLayerWidth = mInputRaster->width();
  mLayerHeight = mInputRaster->height();
  mCrs = mInputRaster->crs();
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();

  //prepare fuzzifcation parameters from subclass alg
  prepareAlgorithmFuzzificationParameters( parameters, context, feedback );

  return true;
}

QVariantMap QgsFuzzifyRasterAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( mDataType, mNbCellsXProvider, mNbCellsYProvider, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  provider->setEditable( true );
  const qgssize layerSize = static_cast<qgssize>( mLayerWidth ) * static_cast<qgssize>( mLayerHeight );

  const bool hasReportsDuringClose = provider->hasReportsDuringClose();
  mMaxProgressDuringBlockWriting = hasReportsDuringClose ? 50.0 : 100.0;

  fuzzify( provider.get(), feedback );

  provider->setEditable( false );

  if ( feedback && hasReportsDuringClose )
  {
    std::unique_ptr<QgsFeedback> scaledFeedback( QgsFeedback::createScaledFeedback( feedback, mMaxProgressDuringBlockWriting, 100.0 ) );
    if ( !provider->closeWithProgress( scaledFeedback.get() ) )
    {
      if ( feedback->isCanceled() )
        return {};
      throw QgsProcessingException( QObject::tr( "Could not write raster dataset" ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );
  outputs.insert( u"TOTAL_PIXEL_COUNT"_s, layerSize );
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}


//
// QgsFuzzfiyRasterLinearMembershipAlgorithm
//

QString QgsFuzzifyRasterLinearMembershipAlgorithm::name() const
{
  return u"fuzzifyrasterlinearmembership"_s;
}

QString QgsFuzzifyRasterLinearMembershipAlgorithm::displayName() const
{
  return QObject::tr( "Fuzzify raster (linear membership)" );
}

QStringList QgsFuzzifyRasterLinearMembershipAlgorithm::tags() const
{
  return QObject::tr( "fuzzy logic,fuzzify,fuzzy,logic,linear,membership" ).split( ',' );
}

QString QgsFuzzifyRasterLinearMembershipAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms an input raster to a fuzzified raster where values range from 0 to 1 following a "
                      "linear fuzzy membership function." );
}

QString QgsFuzzifyRasterLinearMembershipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transforms an input raster "
                      "to a fuzzified raster and thereby assigns values between 0 and 1 following a "
                      "linear fuzzy membership function. The value of 0 implies no membership with the "
                      "defined fuzzy set, a value of 1 depicts full membership. In between, the degree "
                      "of membership of raster values follows a linear membership function.\n\n"
                      "The linear function is constructed using two user-defined input raster values "
                      "which set the point of full membership (high bound, results to 1) and no membership "
                      "(low bound, results to 0) respectively. The fuzzy set in between those values is defined as a "
                      "linear function.\n\n"
                      "Both increasing and decreasing fuzzy sets can be modeled by "
                      "swapping the high and low bound parameters." );
}

QgsFuzzifyRasterLinearMembershipAlgorithm *QgsFuzzifyRasterLinearMembershipAlgorithm::createInstance() const
{
  return new QgsFuzzifyRasterLinearMembershipAlgorithm();
}

void QgsFuzzifyRasterLinearMembershipAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterNumber( u"FUZZYLOWBOUND"_s, u"Low fuzzy membership bound"_s, Qgis::ProcessingNumberParameterType::Double, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYHIGHBOUND"_s, u"High fuzzy membership bound"_s, Qgis::ProcessingNumberParameterType::Double, 1 ) );
}

bool QgsFuzzifyRasterLinearMembershipAlgorithm::prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mFuzzifyHighBound = parameterAsDouble( parameters, u"FUZZYHIGHBOUND"_s, context );
  mFuzzifyLowBound = parameterAsDouble( parameters, u"FUZZYLOWBOUND"_s, context );
  return true;
}

void QgsFuzzifyRasterLinearMembershipAlgorithm::fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback )
{
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
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    auto fuzzifiedBlock = std::make_unique<QgsRasterBlock>( destinationProvider->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        double fuzzifiedValue;

        if ( isNoData )
        {
          fuzzifiedValue = mNoDataValue;
        }
        else if ( mFuzzifyLowBound < mFuzzifyHighBound )
        {
          if ( value <= mFuzzifyLowBound )
            fuzzifiedValue = 0;
          else if ( value >= mFuzzifyHighBound )
            fuzzifiedValue = 1;
          else
            fuzzifiedValue = ( ( value - mFuzzifyLowBound ) / ( mFuzzifyHighBound - mFuzzifyLowBound ) );
        }
        else if ( mFuzzifyLowBound > mFuzzifyHighBound )
        {
          if ( value >= mFuzzifyLowBound )
            fuzzifiedValue = 0;
          else if ( value <= mFuzzifyHighBound )
            fuzzifiedValue = 1;
          else
            fuzzifiedValue = ( ( value - mFuzzifyLowBound ) / ( mFuzzifyHighBound - mFuzzifyLowBound ) );
        }
        else
        {
          throw QgsProcessingException( QObject::tr( "Please choose varying values for the high and low membership parameters" ) );
        }

        fuzzifiedBlock->setValue( row, column, fuzzifiedValue );
      }
    }
    if ( !destinationProvider->writeBlock( fuzzifiedBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationProvider->error().summary() ) );
    }
  }
}


//
// QgsFuzzfiyRasterPowerMembershipAlgorithm
//

QString QgsFuzzifyRasterPowerMembershipAlgorithm::name() const
{
  return u"fuzzifyrasterpowermembership"_s;
}

QString QgsFuzzifyRasterPowerMembershipAlgorithm::displayName() const
{
  return QObject::tr( "Fuzzify raster (power membership)" );
}

QStringList QgsFuzzifyRasterPowerMembershipAlgorithm::tags() const
{
  return QObject::tr( "fuzzy logic,fuzzify,fuzzy,logic,power,non-linear,membership,exponent" ).split( ',' );
}

QString QgsFuzzifyRasterPowerMembershipAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms an input raster to a fuzzified raster where values range from 0 to 1 following a "
                      "power function." );
}

QString QgsFuzzifyRasterPowerMembershipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transforms an input raster "
                      "to a fuzzified raster and thereby assigns values between 0 and 1 following a "
                      "power function. The value of 0 implies no membership with the "
                      "defined fuzzy set, a value of 1 depicts full membership. In between, the degree "
                      "of membership of raster values follows a power function.\n\n"
                      "The power function is constructed using three user-defined input raster values "
                      "which set the point of full membership (high bound, results to 1), no membership "
                      "(low bound, results to 0) and function exponent (only positive) respectively. "
                      "The fuzzy set in between those the upper and lower bounds values is then defined as "
                      "a power function.\n\n"
                      "Both increasing and decreasing fuzzy sets can be modeled by "
                      "swapping the high and low bound parameters." );
}

QgsFuzzifyRasterPowerMembershipAlgorithm *QgsFuzzifyRasterPowerMembershipAlgorithm::createInstance() const
{
  return new QgsFuzzifyRasterPowerMembershipAlgorithm();
}

void QgsFuzzifyRasterPowerMembershipAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterNumber( u"FUZZYLOWBOUND"_s, u"Low fuzzy membership bound"_s, Qgis::ProcessingNumberParameterType::Double, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYHIGHBOUND"_s, u"High fuzzy membership bound"_s, Qgis::ProcessingNumberParameterType::Double, 1 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYEXPONENT"_s, u"Membership function exponent"_s, Qgis::ProcessingNumberParameterType::Double, 2, false, 0 ) );
}

bool QgsFuzzifyRasterPowerMembershipAlgorithm::prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mFuzzifyHighBound = parameterAsDouble( parameters, u"FUZZYHIGHBOUND"_s, context );
  mFuzzifyLowBound = parameterAsDouble( parameters, u"FUZZYLOWBOUND"_s, context );
  mFuzzifyExponent = parameterAsDouble( parameters, u"FUZZYEXPONENT"_s, context );
  return true;
}

void QgsFuzzifyRasterPowerMembershipAlgorithm::fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback )
{
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
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    auto fuzzifiedBlock = std::make_unique<QgsRasterBlock>( destinationProvider->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        double fuzzifiedValue;

        if ( isNoData )
        {
          fuzzifiedValue = mNoDataValue;
        }
        else if ( mFuzzifyLowBound < mFuzzifyHighBound )
        {
          if ( value <= mFuzzifyLowBound )
            fuzzifiedValue = 0;
          else if ( value >= mFuzzifyHighBound )
            fuzzifiedValue = 1;
          else
            fuzzifiedValue = std::pow( ( value - mFuzzifyLowBound ) / ( mFuzzifyHighBound - mFuzzifyLowBound ), mFuzzifyExponent );
        }
        else if ( mFuzzifyLowBound > mFuzzifyHighBound )
        {
          if ( value >= mFuzzifyLowBound )
            fuzzifiedValue = 0;
          else if ( value <= mFuzzifyHighBound )
            fuzzifiedValue = 1;
          else
            fuzzifiedValue = std::pow( ( value - mFuzzifyLowBound ) / ( mFuzzifyHighBound - mFuzzifyLowBound ), mFuzzifyExponent );
        }
        else
        {
          throw QgsProcessingException( QObject::tr( "Please choose varying values for the high and low membership parameters" ) );
        }

        fuzzifiedBlock->setValue( row, column, fuzzifiedValue );
      }
    }
    if ( !destinationProvider->writeBlock( fuzzifiedBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationProvider->error().summary() ) );
    }
  }
}

//
// QgsFuzzfiyRasterLargeMembershipAlgorithm
//

QString QgsFuzzifyRasterLargeMembershipAlgorithm::name() const
{
  return u"fuzzifyrasterlargemembership"_s;
}

QString QgsFuzzifyRasterLargeMembershipAlgorithm::displayName() const
{
  return QObject::tr( "Fuzzify raster (large membership)" );
}

QStringList QgsFuzzifyRasterLargeMembershipAlgorithm::tags() const
{
  return QObject::tr( "fuzzy logic,fuzzify,fuzzy,logic,large,membership" ).split( ',' );
}

QString QgsFuzzifyRasterLargeMembershipAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms an input raster to a fuzzified raster where values range from 0 to 1 following the "
                      "'large' fuzzy membership function." );
}

QString QgsFuzzifyRasterLargeMembershipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transforms an input raster "
                      "to a fuzzified raster and thereby assigns values between 0 and 1 following the "
                      "'large' fuzzy membership function. The value of 0 implies no membership with the "
                      "defined fuzzy set, a value of 1 depicts full membership. In between, the degree "
                      "of membership of raster values follows the 'large' membership function.\n\n"
                      "The 'large' function is constructed using two user-defined input raster values "
                      "which set the point of half membership (midpoint, results to 0.5) and a predefined "
                      "function spread which controls the function uptake.\n\n"
                      "This function is typically used when larger input raster values should become members "
                      "of the fuzzy set more easily than smaller values." );
}

QgsFuzzifyRasterLargeMembershipAlgorithm *QgsFuzzifyRasterLargeMembershipAlgorithm::createInstance() const
{
  return new QgsFuzzifyRasterLargeMembershipAlgorithm();
}

void QgsFuzzifyRasterLargeMembershipAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterNumber( u"FUZZYMIDPOINT"_s, u"Function midpoint"_s, Qgis::ProcessingNumberParameterType::Double, 50 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYSPREAD"_s, u"Function spread"_s, Qgis::ProcessingNumberParameterType::Double, 5 ) );
}

bool QgsFuzzifyRasterLargeMembershipAlgorithm::prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mFuzzifyMidpoint = parameterAsDouble( parameters, u"FUZZYMIDPOINT"_s, context );
  mFuzzifySpread = parameterAsDouble( parameters, u"FUZZYSPREAD"_s, context );
  return true;
}

void QgsFuzzifyRasterLargeMembershipAlgorithm::fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback )
{
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
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    auto fuzzifiedBlock = std::make_unique<QgsRasterBlock>( destinationProvider->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        double fuzzifiedValue;

        if ( isNoData )
        {
          fuzzifiedValue = mNoDataValue;
        }
        else
        {
          fuzzifiedValue = 1 / ( 1 + std::pow( value / mFuzzifyMidpoint, -mFuzzifySpread ) );
        }

        fuzzifiedBlock->setValue( row, column, fuzzifiedValue );
      }
    }
    if ( !destinationProvider->writeBlock( fuzzifiedBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationProvider->error().summary() ) );
    }
  }
}


//
// QgsFuzzfiyRasterSmallMembershipAlgorithm
//

QString QgsFuzzifyRasterSmallMembershipAlgorithm::name() const
{
  return u"fuzzifyrastersmallmembership"_s;
}

QString QgsFuzzifyRasterSmallMembershipAlgorithm::displayName() const
{
  return QObject::tr( "Fuzzify raster (small membership)" );
}

QStringList QgsFuzzifyRasterSmallMembershipAlgorithm::tags() const
{
  return QObject::tr( "fuzzy logic,fuzzify,fuzzy,logic,small,membership" ).split( ',' );
}

QString QgsFuzzifyRasterSmallMembershipAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms an input raster to a fuzzified raster where values range from 0 to 1 following the "
                      "'small' fuzzy membership function." );
}

QString QgsFuzzifyRasterSmallMembershipAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Fuzzify raster (small membership) algorithm transforms an input raster "
                      "to a fuzzified raster and thereby assigns values between 0 and 1 following the "
                      "'small' fuzzy membership function. The value of 0 implies no membership with the "
                      "defined fuzzy set, a value of 1 depicts full membership. In between, the degree "
                      "of membership of raster values follows the 'small' membership function.\n\n"
                      "The 'small' function is constructed using two user-defined input raster values "
                      "which set the point of half membership (midpoint, results to 0.5) and a predefined "
                      "function spread which controls the function uptake.\n\n"
                      "This function is typically used when smaller input raster values should become members "
                      "of the fuzzy set more easily than higher values." );
}

QgsFuzzifyRasterSmallMembershipAlgorithm *QgsFuzzifyRasterSmallMembershipAlgorithm::createInstance() const
{
  return new QgsFuzzifyRasterSmallMembershipAlgorithm();
}

void QgsFuzzifyRasterSmallMembershipAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterNumber( u"FUZZYMIDPOINT"_s, u"Function midpoint"_s, Qgis::ProcessingNumberParameterType::Double, 50 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYSPREAD"_s, u"Function spread"_s, Qgis::ProcessingNumberParameterType::Double, 5 ) );
}

bool QgsFuzzifyRasterSmallMembershipAlgorithm::prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mFuzzifyMidpoint = parameterAsDouble( parameters, u"FUZZYMIDPOINT"_s, context );
  mFuzzifySpread = parameterAsDouble( parameters, u"FUZZYSPREAD"_s, context );
  return true;
}

void QgsFuzzifyRasterSmallMembershipAlgorithm::fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback )
{
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
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    auto fuzzifiedBlock = std::make_unique<QgsRasterBlock>( destinationProvider->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        double fuzzifiedValue;

        if ( isNoData )
        {
          fuzzifiedValue = mNoDataValue;
        }
        else
        {
          fuzzifiedValue = 1 / ( 1 + std::pow( value / mFuzzifyMidpoint, mFuzzifySpread ) );
        }

        fuzzifiedBlock->setValue( row, column, fuzzifiedValue );
      }
    }
    if ( !destinationProvider->writeBlock( fuzzifiedBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationProvider->error().summary() ) );
    }
  }
}


//
// QgsFuzzfiyRasterGaussianMembershipAlgorithm
//

QString QgsFuzzifyRasterGaussianMembershipAlgorithm::name() const
{
  return u"fuzzifyrastergaussianmembership"_s;
}

QString QgsFuzzifyRasterGaussianMembershipAlgorithm::displayName() const
{
  return QObject::tr( "Fuzzify raster (gaussian membership)" );
}

QStringList QgsFuzzifyRasterGaussianMembershipAlgorithm::tags() const
{
  return QObject::tr( "fuzzy logic,fuzzify,fuzzy,logic,gaussian,membership" ).split( ',' );
}

QString QgsFuzzifyRasterGaussianMembershipAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms an input raster to a fuzzified raster where values range from 0 to 1 following a "
                      "gaussian fuzzy membership function." );
}

QString QgsFuzzifyRasterGaussianMembershipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transforms an input raster "
                      "to a fuzzified raster and thereby assigns values between 0 and 1 following a "
                      "gaussian fuzzy membership function. The value of 0 implies no membership with the "
                      "defined fuzzy set, a value of 1 depicts full membership. In between, the degree "
                      "of membership of raster values follows a gaussian membership function.\n\n"
                      "The gaussian function is constructed using two user-defined input values "
                      "which set the midpoint of the gaussian function (midpoint, results to 1) and a "
                      "predefined function spread which controls the function spread.\n\n"
                      "This function is typically used when a certain range of raster values around a "
                      "predefined function midpoint should become members of the fuzzy set." );
}

QgsFuzzifyRasterGaussianMembershipAlgorithm *QgsFuzzifyRasterGaussianMembershipAlgorithm::createInstance() const
{
  return new QgsFuzzifyRasterGaussianMembershipAlgorithm();
}

void QgsFuzzifyRasterGaussianMembershipAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterNumber( u"FUZZYMIDPOINT"_s, u"Function midpoint"_s, Qgis::ProcessingNumberParameterType::Double, 10 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYSPREAD"_s, u"Function spread"_s, Qgis::ProcessingNumberParameterType::Double, 0.01 ) );
}

bool QgsFuzzifyRasterGaussianMembershipAlgorithm::prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mFuzzifyMidpoint = parameterAsDouble( parameters, u"FUZZYMIDPOINT"_s, context );
  mFuzzifySpread = parameterAsDouble( parameters, u"FUZZYSPREAD"_s, context );
  return true;
}

void QgsFuzzifyRasterGaussianMembershipAlgorithm::fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback )
{
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
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    auto fuzzifiedBlock = std::make_unique<QgsRasterBlock>( destinationProvider->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        double fuzzifiedValue;

        if ( isNoData )
        {
          fuzzifiedValue = mNoDataValue;
        }
        else
        {
          fuzzifiedValue = std::exp( -mFuzzifySpread * std::pow( value - mFuzzifyMidpoint, 2 ) );
        }

        fuzzifiedBlock->setValue( row, column, fuzzifiedValue );
      }
    }
    if ( !destinationProvider->writeBlock( fuzzifiedBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationProvider->error().summary() ) );
    }
  }
}


//
// QgsFuzzfiyRasterNearMembershipAlgorithm
//

QString QgsFuzzifyRasterNearMembershipAlgorithm::name() const
{
  return u"fuzzifyrasternearmembership"_s;
}

QString QgsFuzzifyRasterNearMembershipAlgorithm::displayName() const
{
  return QObject::tr( "Fuzzify raster (near membership)" );
}

QStringList QgsFuzzifyRasterNearMembershipAlgorithm::tags() const
{
  return QObject::tr( "fuzzy logic,fuzzify,fuzzy,logic,near,membership" ).split( ',' );
}

QString QgsFuzzifyRasterNearMembershipAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms an input raster to a fuzzified raster where values range from 0 to 1 following the "
                      "'near' fuzzy membership function." );
}

QString QgsFuzzifyRasterNearMembershipAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Fuzzify raster (near membership) algorithm transforms an input raster "
                      "to a fuzzified raster and thereby assigns values between 0 and 1 following the "
                      "'near' fuzzy membership function. The value of 0 implies no membership with the "
                      "defined fuzzy set, a value of 1 depicts full membership. In between, the degree "
                      "of membership of raster values follows the 'near' membership function.\n\n"
                      "The 'near' function is constructed using two user-defined input values "
                      "which set the midpoint of the 'near' function (midpoint, results to 1) and a "
                      "predefined function spread which controls the function spread.\n\n"
                      "This function is typically used when a certain range of raster values near a "
                      "predefined function midpoint should become members of the fuzzy set. The function"
                      " generally shows a higher rate of decay than the gaussian membership function." );
}

QgsFuzzifyRasterNearMembershipAlgorithm *QgsFuzzifyRasterNearMembershipAlgorithm::createInstance() const
{
  return new QgsFuzzifyRasterNearMembershipAlgorithm();
}

void QgsFuzzifyRasterNearMembershipAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterNumber( u"FUZZYMIDPOINT"_s, u"Function midpoint"_s, Qgis::ProcessingNumberParameterType::Double, 50 ) );
  addParameter( new QgsProcessingParameterNumber( u"FUZZYSPREAD"_s, u"Function spread"_s, Qgis::ProcessingNumberParameterType::Double, 0.01 ) );
}

bool QgsFuzzifyRasterNearMembershipAlgorithm::prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mFuzzifyMidpoint = parameterAsDouble( parameters, u"FUZZYMIDPOINT"_s, context );
  mFuzzifySpread = parameterAsDouble( parameters, u"FUZZYSPREAD"_s, context );
  return true;
}

void QgsFuzzifyRasterNearMembershipAlgorithm::fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback )
{
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
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    if ( feedback )
      feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    auto fuzzifiedBlock = std::make_unique<QgsRasterBlock>( destinationProvider->dataType( 1 ), iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        double fuzzifiedValue;

        if ( isNoData )
        {
          fuzzifiedValue = mNoDataValue;
        }
        else
        {
          fuzzifiedValue = 1 / ( 1 + mFuzzifySpread * std::pow( value - mFuzzifyMidpoint, 2 ) );
        }

        fuzzifiedBlock->setValue( row, column, fuzzifiedValue );
      }
    }
    if ( !destinationProvider->writeBlock( fuzzifiedBlock.get(), mBand, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destinationProvider->error().summary() ) );
    }
  }
}


///@endcond
