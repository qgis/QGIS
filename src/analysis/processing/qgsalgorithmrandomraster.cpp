/***************************************************************************
                         qgsalgorithmrandomraster.cpp
                         ---------------------
    begin                : May 2020
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

#include "qgsalgorithmrandomraster.h"

#include <limits>
#include <random>

#include "qgsrasterfilewriter.h"
#include "qgsstringutils.h"

///@cond PRIVATE

//
// QgsRandomRasterAlgorithmBase
//
QString QgsRandomRasterAlgorithmBase::group() const
{
  return QObject::tr( "Raster creation" );
}

QString QgsRandomRasterAlgorithmBase::groupId() const
{
  return u"rastercreation"_s;
}

void QgsRandomRasterAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( u"TARGET_CRS"_s, QObject::tr( "Target CRS" ), u"ProjectCrs"_s ) );
  addParameter( new QgsProcessingParameterNumber( u"PIXEL_SIZE"_s, QObject::tr( "Pixel size" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 ) );

  //add specific parameters
  addAlgorithmParams();

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

bool QgsRandomRasterAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mCrs = parameterAsCrs( parameters, u"TARGET_CRS"_s, context );
  mExtent = parameterAsExtent( parameters, u"EXTENT"_s, context, mCrs );
  mPixelSize = parameterAsDouble( parameters, u"PIXEL_SIZE"_s, context );

  if ( mPixelSize <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Pixel size must be greater than 0." ) );
  }

  return true;
}

QVariantMap QgsRandomRasterAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int typeId = parameterAsInt( parameters, u"OUTPUT_TYPE"_s, context );
  //prepare specific parameters
  mRasterDataType = getRasterDataType( typeId );
  prepareRandomParameters( parameters, context );

  std::random_device rd {};
  std::mt19937 mersenneTwister { rd() };

  QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  // handle backwards compatibility parameter CREATE_OPTIONS
  const QString optionsString = parameterAsString( parameters, u"CREATE_OPTIONS"_s, context );
  if ( !optionsString.isEmpty() )
    creationOptions = optionsString;

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  // round up width and height to the nearest integer as GDAL does (e.g. in gdal_rasterize)
  // see https://github.com/qgis/QGIS/issues/43547
  const int rows = static_cast<int>( 0.5 + mExtent.height() / mPixelSize );
  const int cols = static_cast<int>( 0.5 + mExtent.width() / mPixelSize );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  const QgsRectangle rasterExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMaximum() - ( rows * mPixelSize ), mExtent.xMinimum() + ( cols * mPixelSize ), mExtent.yMaximum() );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    writer->setCreationOptions( creationOptions.split( '|' ) );
  }

  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( mRasterDataType, cols, rows, rasterExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  const double step = rows > 0 ? 100.0 / rows : 1;

  for ( int row = 0; row < rows; row++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    //prepare raw data depending on raster data type
    QgsRasterBlock block( mRasterDataType, cols, 1 );
    switch ( mRasterDataType )
    {
      case Qgis::DataType::Byte:
      {
        std::vector<quint8> byteRow( cols );
        for ( int col = 0; col < cols; col++ )
        {
          byteRow[col] = static_cast<quint8>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( byteRow.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Byte ) * cols ) );
        break;
      }
      case Qgis::DataType::Int8:
      {
        std::vector<qint8> int8Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int8Row[col] = static_cast<qint8>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int8Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Int8 ) * cols ) );
        break;
      }
      case Qgis::DataType::Int16:
      {
        std::vector<qint16> int16Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int16Row[col] = static_cast<qint16>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int16Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Int16 ) * cols ) );
        break;
      }
      case Qgis::DataType::UInt16:
      {
        std::vector<quint16> uInt16Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          uInt16Row[col] = static_cast<quint16>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( uInt16Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::UInt16 ) * cols ) );
        break;
      }
      case Qgis::DataType::Int32:
      {
        std::vector<qint32> int32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int32Row[col] = generateRandomLongValue( mersenneTwister );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int32Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Int32 ) * cols ) );
        break;
      }
      case Qgis::DataType::UInt32:
      {
        std::vector<quint32> uInt32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          uInt32Row[col] = static_cast<quint32>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( uInt32Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::UInt32 ) * cols ) );
        break;
      }
      case Qgis::DataType::Float32:
      {
        std::vector<float> float32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float32Row[col] = static_cast<float>( generateRandomDoubleValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float32Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Float32 ) * cols ) );
        break;
      }
      case Qgis::DataType::Float64:
      {
        std::vector<double> float64Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float64Row[col] = generateRandomDoubleValue( mersenneTwister );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float64Row.data() ), QgsRasterBlock::typeSize( Qgis::DataType::Float64 ) * cols ) );
        break;
      }
      default:
        break;
    }
    if ( !provider->writeBlock( &block, 1, 0, row ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( provider->error().summary() ) );
    }
    feedback->setProgress( row * step );
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

//
//QgsRandomUniformRasterAlgorithm
//
QString QgsRandomUniformRasterAlgorithm::name() const
{
  return u"createrandomuniformrasterlayer"_s;
}

QString QgsRandomUniformRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (uniform distribution)" );
}

QStringList QgsRandomUniformRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random" ).split( ',' );
}

QString QgsRandomUniformRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with random values.\n"
                      "By default, the values will range between the minimum and "
                      "maximum value of the specified output raster type. This can "
                      "be overridden by using the advanced parameters for lower and "
                      "upper bound value. If the bounds have the same value or both "
                      "are zero (default) the algorithm will create random values in "
                      "the full value range of the chosen raster data type. "
                      "Choosing bounds outside the acceptable range of the output "
                      "raster type will abort the algorithm." );
}

QString QgsRandomUniformRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with random values." );
}

QgsRandomUniformRasterAlgorithm *QgsRandomUniformRasterAlgorithm::createInstance() const
{
  return new QgsRandomUniformRasterAlgorithm();
}

void QgsRandomUniformRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Byte"_s
                  << u"Integer16"_s
                  << u"Unsigned Integer16"_s
                  << u"Integer32"_s
                  << u"Unsigned Integer32"_s
                  << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 5, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto lowerBoundParameter = std::make_unique<QgsProcessingParameterNumber>( u"LOWER_BOUND"_s, u"Lower bound for random number range"_s, Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  lowerBoundParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( lowerBoundParameter.release() );

  auto upperBoundParameter = std::make_unique<QgsProcessingParameterNumber>( u"UPPER_BOUND"_s, u"Upper bound for random number range"_s, Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  upperBoundParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( upperBoundParameter.release() );
}

Qgis::DataType QgsRandomUniformRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Byte;
    case 1:
      return Qgis::DataType::Int16;
    case 2:
      return Qgis::DataType::UInt16;
    case 3:
      return Qgis::DataType::Int32;
    case 4:
      return Qgis::DataType::UInt32;
    case 5:
      return Qgis::DataType::Float32;
    case 6:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomUniformRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  mRandomUpperBound = parameterAsDouble( parameters, u"UPPER_BOUND"_s, context );
  mRandomLowerBound = parameterAsDouble( parameters, u"LOWER_BOUND"_s, context );

  if ( mRandomLowerBound > mRandomUpperBound )
    throw QgsProcessingException( QObject::tr( "The chosen lower bound for random number range is greater than the upper bound. The lower bound value must be smaller than the upper bound value." ) );

  const int typeId = parameterAsInt( parameters, u"OUTPUT_TYPE"_s, context );
  const Qgis::DataType rasterDataType = getRasterDataType( typeId );

  switch ( rasterDataType )
  {
    case Qgis::DataType::Byte:
      if ( mRandomLowerBound < std::numeric_limits<quint8>::min() || mRandomUpperBound > std::numeric_limits<quint8>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint8>::min() ).arg( std::numeric_limits<quint8>::max() ).arg( "Byte"_L1 ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        //if parameters unset (=both are 0 or equal) --> use the whole value range
        mRandomUpperBound = std::numeric_limits<quint8>::max();
        mRandomLowerBound = std::numeric_limits<quint8>::min();
      }
      break;
    case Qgis::DataType::Int8:
      if ( mRandomLowerBound < std::numeric_limits<qint8>::min() || mRandomUpperBound > std::numeric_limits<qint8>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint8>::min() ).arg( std::numeric_limits<qint8>::max() ).arg( "Int8"_L1 ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        //if parameters unset (=both are 0 or equal) --> use the whole value range
        mRandomUpperBound = std::numeric_limits<qint8>::max();
        mRandomLowerBound = std::numeric_limits<qint8>::min();
      }
      break;
    case Qgis::DataType::Int16:
      if ( mRandomLowerBound < std::numeric_limits<qint16>::min() || mRandomUpperBound > std::numeric_limits<qint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint16>::min() ).arg( std::numeric_limits<qint16>::max() ).arg( "Integer16"_L1 ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<qint16>::max();
        mRandomLowerBound = std::numeric_limits<qint16>::min();
      }
      break;
    case Qgis::DataType::UInt16:
      if ( mRandomLowerBound < std::numeric_limits<quint16>::min() || mRandomUpperBound > std::numeric_limits<quint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint16>::min() ).arg( std::numeric_limits<quint16>::max() ).arg( "Unsigned Integer16"_L1 ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<quint16>::max();
        mRandomLowerBound = std::numeric_limits<quint16>::min();
      }
      break;
    case Qgis::DataType::Int32:
      if ( mRandomLowerBound < std::numeric_limits<qint32>::min() || mRandomUpperBound > std::numeric_limits<qint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint32>::min() ).arg( std::numeric_limits<qint32>::max() ).arg( "Integer32"_L1 ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<qint32>::max();
        mRandomLowerBound = std::numeric_limits<qint32>::min();
      }
      break;
    case Qgis::DataType::UInt32:
      if ( mRandomLowerBound < std::numeric_limits<quint32>::min() || mRandomUpperBound > std::numeric_limits<quint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint32>::min() ).arg( std::numeric_limits<quint32>::max() ).arg( "Unsigned Integer32"_L1 ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<quint32>::max();
        mRandomLowerBound = std::numeric_limits<quint32>::min();
      }
      break;
    case Qgis::DataType::Float32:
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<float>::max();
        mRandomLowerBound = std::numeric_limits<float>::min();
      }
      break;
    case Qgis::DataType::Float64:
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<double>::max();
        mRandomLowerBound = std::numeric_limits<double>::min();
      }
      break;
    case Qgis::DataType::CInt16:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
    case Qgis::DataType::UnknownDataType:
      break;
  }

  mRandomUniformIntDistribution = std::uniform_int_distribution<long>( mRandomLowerBound, mRandomUpperBound );
  mRandomUniformDoubleDistribution = std::uniform_real_distribution<double>( mRandomLowerBound, mRandomUpperBound );

  return true;
}

long QgsRandomUniformRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return mRandomUniformIntDistribution( mersenneTwister );
}

double QgsRandomUniformRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return mRandomUniformDoubleDistribution( mersenneTwister );
}

//
// QgsRandomBinomialRasterAlgorithm
//
QString QgsRandomBinomialRasterAlgorithm::name() const
{
  return u"createrandombinomialrasterlayer"_s;
}

QString QgsRandomBinomialRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (binomial distribution)" );
}

QStringList QgsRandomBinomialRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,binomial,random" ).split( ',' );
}

QString QgsRandomBinomialRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with binomially distributed random values.\n"
                      "By default, the values will be chosen given an N of 10 and a probability of 0.5. "
                      "This can be overridden by using the advanced parameter for N and probability. "
                      "The raster data type is set to Integer types (Integer16 by default). "
                      "The binomial distribution random values are defined as positive integer numbers. "
                      "A floating point raster will represent a cast of integer values "
                      "to floating point." );
}

QString QgsRandomBinomialRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with binomially distributed random values." );
}

QgsRandomBinomialRasterAlgorithm *QgsRandomBinomialRasterAlgorithm::createInstance() const
{
  return new QgsRandomBinomialRasterAlgorithm();
}


void QgsRandomBinomialRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Integer16"_s
                  << u"Unsigned Integer16"_s
                  << u"Integer32"_s
                  << u"Unsigned Integer32"_s
                  << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto nParameter = std::make_unique<QgsProcessingParameterNumber>( u"N"_s, u"N"_s, Qgis::ProcessingNumberParameterType::Integer, 10, true, 0 );
  nParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( nParameter.release() );

  auto probabilityParameter = std::make_unique<QgsProcessingParameterNumber>( u"PROBABILITY"_s, u"Probability"_s, Qgis::ProcessingNumberParameterType::Double, 0.5, true, 0 );
  probabilityParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( probabilityParameter.release() );
}

Qgis::DataType QgsRandomBinomialRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Int16;
    case 1:
      return Qgis::DataType::UInt16;
    case 2:
      return Qgis::DataType::Int32;
    case 3:
      return Qgis::DataType::UInt32;
    case 4:
      return Qgis::DataType::Float32;
    case 5:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomBinomialRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const int n = parameterAsInt( parameters, u"N"_s, context );
  const double probability = parameterAsDouble( parameters, u"PROBABILITY"_s, context );
  mRandombinomialDistribution = std::binomial_distribution<long>( n, probability );
  return true;
}

long QgsRandomBinomialRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return mRandombinomialDistribution( mersenneTwister );
}

double QgsRandomBinomialRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return static_cast<double>( mRandombinomialDistribution( mersenneTwister ) );
}

//
// QgsRandomExponentialRasterAlgorithm
//
QString QgsRandomExponentialRasterAlgorithm::name() const
{
  return u"createrandomexponentialrasterlayer"_s;
}

QString QgsRandomExponentialRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (exponential distribution)" );
}

QStringList QgsRandomExponentialRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random,exponential" ).split( ',' );
}

QString QgsRandomExponentialRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with exponentially distributed random values.\n"
                      "By default, the values will be chosen given a lambda of 1.0. "
                      "This can be overridden by using the advanced parameter for lambda. "
                      "The raster data type is set to Float32 by default as "
                      "the exponential distribution random values are floating point numbers." );
}

QString QgsRandomExponentialRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with exponentially distributed random values." );
}

QgsRandomExponentialRasterAlgorithm *QgsRandomExponentialRasterAlgorithm::createInstance() const
{
  return new QgsRandomExponentialRasterAlgorithm();
}


void QgsRandomExponentialRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto lambdaParameter = std::make_unique<QgsProcessingParameterNumber>( u"LAMBDA"_s, u"Lambda"_s, Qgis::ProcessingNumberParameterType::Double, 1.0, true, 0.000001 );
  lambdaParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( lambdaParameter.release() );
}

Qgis::DataType QgsRandomExponentialRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Float32;
    case 1:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomExponentialRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const double lambda = parameterAsDouble( parameters, u"LAMBDA"_s, context );
  mRandomExponentialDistribution = std::exponential_distribution<double>( lambda );
  return true;
}

long QgsRandomExponentialRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return static_cast<long>( mRandomExponentialDistribution( mersenneTwister ) );
}

double QgsRandomExponentialRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return mRandomExponentialDistribution( mersenneTwister );
}

//
// QgsRandomGammaRasterAlgorithm
//
QString QgsRandomGammaRasterAlgorithm::name() const
{
  return u"createrandomgammarasterlayer"_s;
}

QString QgsRandomGammaRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (gamma distribution)" );
}

QStringList QgsRandomGammaRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random,gamma" ).split( ',' );
}

QString QgsRandomGammaRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with gamma distributed random values.\n"
                      "By default, the values will be chosen given an alpha and beta value of 1.0. "
                      "This can be overridden by using the advanced parameter for alpha and beta. "
                      "The raster data type is set to Float32 by default as "
                      "the gamma distribution random values are floating point numbers." );
}

QString QgsRandomGammaRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with gamma distributed random values." );
}

QgsRandomGammaRasterAlgorithm *QgsRandomGammaRasterAlgorithm::createInstance() const
{
  return new QgsRandomGammaRasterAlgorithm();
}


void QgsRandomGammaRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto alphaParameter = std::make_unique<QgsProcessingParameterNumber>( u"ALPHA"_s, u"Alpha"_s, Qgis::ProcessingNumberParameterType::Double, 1.0, true, 0.000001 );
  alphaParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( alphaParameter.release() );

  auto betaParameter = std::make_unique<QgsProcessingParameterNumber>( u"BETA"_s, u"Beta"_s, Qgis::ProcessingNumberParameterType::Double, 1.0, true, 0.000001 );
  betaParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( betaParameter.release() );
}

Qgis::DataType QgsRandomGammaRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Float32;
    case 1:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomGammaRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const double alpha = parameterAsDouble( parameters, u"ALPHA"_s, context );
  const double beta = parameterAsDouble( parameters, u"BETA"_s, context );
  mRandomGammaDistribution = std::gamma_distribution<double>( alpha, beta );
  return true;
}

long QgsRandomGammaRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return static_cast<long>( mRandomGammaDistribution( mersenneTwister ) );
}

double QgsRandomGammaRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return mRandomGammaDistribution( mersenneTwister );
}

//
// QgsRandomGeometricRasterAlgorithm
//
QString QgsRandomGeometricRasterAlgorithm::name() const
{
  return u"createrandomgeometricrasterlayer"_s;
}

QString QgsRandomGeometricRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (geometric distribution)" );
}

QStringList QgsRandomGeometricRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random,geometric" ).split( ',' );
}

QString QgsRandomGeometricRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with geometrically distributed random values.\n"
                      "By default, the values will be chosen given a probability of 0.5. "
                      "This can be overridden by using the advanced parameter for mean "
                      "value. The raster data type is set to Integer types (Integer16 by default). "
                      "The geometric distribution random values are defined as positive integer numbers. "
                      "A floating point raster will represent a cast of "
                      "integer values to floating point." );
}

QString QgsRandomGeometricRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with geometrically distributed random values." );
}

QgsRandomGeometricRasterAlgorithm *QgsRandomGeometricRasterAlgorithm::createInstance() const
{
  return new QgsRandomGeometricRasterAlgorithm();
}


void QgsRandomGeometricRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Integer16"_s
                  << u"Unsigned Integer16"_s
                  << u"Integer32"_s
                  << u"Unsigned Integer32"_s
                  << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto probabilityParameter = std::make_unique<QgsProcessingParameterNumber>( u"PROBABILITY"_s, u"Probability"_s, Qgis::ProcessingNumberParameterType::Double, 0.5, true, 0.00001 );
  probabilityParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( probabilityParameter.release() );
}

Qgis::DataType QgsRandomGeometricRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Int16;
    case 1:
      return Qgis::DataType::UInt16;
    case 2:
      return Qgis::DataType::Int32;
    case 3:
      return Qgis::DataType::UInt32;
    case 4:
      return Qgis::DataType::Float32;
    case 5:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomGeometricRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const double probability = parameterAsDouble( parameters, u"PROBABILITY"_s, context );
  mRandomGeometricDistribution = std::geometric_distribution<long>( probability );
  return true;
}

long QgsRandomGeometricRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return mRandomGeometricDistribution( mersenneTwister );
}

double QgsRandomGeometricRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return static_cast<double>( mRandomGeometricDistribution( mersenneTwister ) );
}

//
// QgsRandomNegativeBinomialRasterAlgorithm
//
QString QgsRandomNegativeBinomialRasterAlgorithm::name() const
{
  return u"createrandomnegativebinomialrasterlayer"_s;
}

QString QgsRandomNegativeBinomialRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (negative binomial distribution)" );
}

QStringList QgsRandomNegativeBinomialRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random,negative,binomial,negative-binomial" ).split( ',' );
}

QString QgsRandomNegativeBinomialRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with negative binomially distributed random values.\n"
                      "By default, the values will be chosen given a distribution parameter k of 10.0 "
                      "and a probability of 0.5. "
                      "This can be overridden by using the advanced parameters for k and probability. "
                      "The raster data type is set to Integer types (Integer 16 by default). "
                      "The negative binomial distribution random values are defined as positive integer numbers. "
                      "A floating point raster will represent a cast of "
                      "integer values to floating point." );
}

QString QgsRandomNegativeBinomialRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with negative binomially distributed random values." );
}

QgsRandomNegativeBinomialRasterAlgorithm *QgsRandomNegativeBinomialRasterAlgorithm::createInstance() const
{
  return new QgsRandomNegativeBinomialRasterAlgorithm();
}


void QgsRandomNegativeBinomialRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Integer16"_s
                  << u"Unsigned Integer16"_s
                  << u"Integer32"_s
                  << u"Unsigned Integer32"_s
                  << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto kParameter = std::make_unique<QgsProcessingParameterNumber>( u"K_PARAMETER"_s, u"Distribution parameter k"_s, Qgis::ProcessingNumberParameterType::Integer, 10, true, 0.00001 );
  kParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( kParameter.release() );

  auto probabilityParameter = std::make_unique<QgsProcessingParameterNumber>( u"PROBABILITY"_s, u"Probability"_s, Qgis::ProcessingNumberParameterType::Double, 0.5, true, 0.00001 );
  probabilityParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( probabilityParameter.release() );
}

Qgis::DataType QgsRandomNegativeBinomialRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Int16;
    case 1:
      return Qgis::DataType::UInt16;
    case 2:
      return Qgis::DataType::Int32;
    case 3:
      return Qgis::DataType::UInt32;
    case 4:
      return Qgis::DataType::Float32;
    case 5:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomNegativeBinomialRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const int k = parameterAsInt( parameters, u"K_PARAMETER"_s, context );
  const double probability = parameterAsDouble( parameters, u"PROBABILITY"_s, context );
  mRandomNegativeBinomialDistribution = std::negative_binomial_distribution<long>( k, probability );
  return true;
}

long QgsRandomNegativeBinomialRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return mRandomNegativeBinomialDistribution( mersenneTwister );
}

double QgsRandomNegativeBinomialRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return static_cast<double>( mRandomNegativeBinomialDistribution( mersenneTwister ) );
}

//
// QgsRandomNormalRasterAlgorithm
//
QString QgsRandomNormalRasterAlgorithm::name() const
{
  return u"createrandomnormalrasterlayer"_s;
}

QString QgsRandomNormalRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (normal distribution)" );
}

QStringList QgsRandomNormalRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,normal,distribution,random" ).split( ',' );
}

QString QgsRandomNormalRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with normally distributed random values.\n"
                      "By default, the values will be chosen given a mean of 0.0 and "
                      "a standard deviation of 1.0. This can be overridden by "
                      "using the advanced parameters for mean and standard deviation "
                      "value. The raster data type is set to Float32 by default "
                      "as the normal distribution random values are floating point numbers." );
}

QString QgsRandomNormalRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with normally distributed random values." );
}

QgsRandomNormalRasterAlgorithm *QgsRandomNormalRasterAlgorithm::createInstance() const
{
  return new QgsRandomNormalRasterAlgorithm();
}

void QgsRandomNormalRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto meanParameter = std::make_unique<QgsProcessingParameterNumber>( u"MEAN"_s, u"Mean of normal distribution"_s, Qgis::ProcessingNumberParameterType::Double, 0, true );
  meanParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( meanParameter.release() );

  auto stdevParameter = std::make_unique<QgsProcessingParameterNumber>( u"STDDEV"_s, u"Standard deviation of normal distribution"_s, Qgis::ProcessingNumberParameterType::Double, 1, true, 0 );
  stdevParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( stdevParameter.release() );
}

Qgis::DataType QgsRandomNormalRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Float32;
    case 1:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomNormalRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const double mean = parameterAsDouble( parameters, u"MEAN"_s, context );
  const double stddev = parameterAsDouble( parameters, u"STDDEV"_s, context );
  mRandomNormalDistribution = std::normal_distribution<double>( mean, stddev );
  return true;
}

long QgsRandomNormalRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return static_cast<long>( mRandomNormalDistribution( mersenneTwister ) );
}

double QgsRandomNormalRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return mRandomNormalDistribution( mersenneTwister );
}

//
// QgsRandomPoissonRasterAlgorithm
//
QString QgsRandomPoissonRasterAlgorithm::name() const
{
  return u"createrandompoissonrasterlayer"_s;
}

QString QgsRandomPoissonRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (poisson distribution)" );
}

QStringList QgsRandomPoissonRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random,poisson" ).split( ',' );
}

QString QgsRandomPoissonRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell size "
                      "filled with poisson distributed random values.\n"
                      "By default, the values will be chosen given a mean of 1.0. "
                      "This can be overridden by using the advanced parameter for mean "
                      "value. The raster data type is set to Integer types (Integer16 by default). "
                      "The poisson distribution random values are positive integer numbers. "
                      "A floating point raster will represent a cast of integer values to floating point." );
}

QString QgsRandomPoissonRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size "
                      "filled with poisson distributed random values." );
}

QgsRandomPoissonRasterAlgorithm *QgsRandomPoissonRasterAlgorithm::createInstance() const
{
  return new QgsRandomPoissonRasterAlgorithm();
}


void QgsRandomPoissonRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << u"Integer16"_s
                  << u"Unsigned Integer16"_s
                  << u"Integer32"_s
                  << u"Unsigned Integer32"_s
                  << u"Float32"_s
                  << u"Float64"_s;

  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  auto upperBoundParameter = std::make_unique<QgsProcessingParameterNumber>( u"MEAN"_s, u"Mean"_s, Qgis::ProcessingNumberParameterType::Double, 1.0, true, 0 );
  upperBoundParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( upperBoundParameter.release() );
}

Qgis::DataType QgsRandomPoissonRasterAlgorithm::getRasterDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::DataType::Int16;
    case 1:
      return Qgis::DataType::UInt16;
    case 2:
      return Qgis::DataType::Int32;
    case 3:
      return Qgis::DataType::UInt32;
    case 4:
      return Qgis::DataType::Float32;
    case 5:
      return Qgis::DataType::Float64;
    default:
      return Qgis::DataType::Float32;
  }
}

bool QgsRandomPoissonRasterAlgorithm::prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context )
{
  const double mean = parameterAsDouble( parameters, u"MEAN"_s, context );
  mRandomPoissonDistribution = std::poisson_distribution<long>( mean );
  return true;
}

long QgsRandomPoissonRasterAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return mRandomPoissonDistribution( mersenneTwister );
}

double QgsRandomPoissonRasterAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return static_cast<double>( mRandomPoissonDistribution( mersenneTwister ) );
}

///@endcond
