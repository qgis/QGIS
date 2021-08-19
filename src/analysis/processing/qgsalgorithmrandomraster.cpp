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
#include "qgsrasterfilewriter.h"
#include "qgsstringutils.h"
#include "random"
#include "limits"

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
  return QStringLiteral( "rastercreation" );
}

void QgsRandomRasterAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
                QgsProcessingParameterNumber::Double, 1, false, 0.01 ) );

  //add specific parameters
  addAlgorithmParams();

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output raster" ) ) );
}

bool QgsRandomRasterAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );
  mPixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );

  return true;
}

QVariantMap QgsRandomRasterAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int typeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );
  //prepare specific parameters
  mRasterDataType = getRasterDataType( typeId );
  prepareRandomParameters( parameters, context );

  std::random_device rd {};
  std::mt19937 mersenneTwister{rd()};

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  const int rows = std::max( std::ceil( mExtent.height() / mPixelSize ), 1.0 );
  const int cols = std::max( std::ceil( mExtent.width() / mPixelSize ), 1.0 );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  const QgsRectangle rasterExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMaximum() - ( rows * mPixelSize ), mExtent.xMinimum() + ( cols * mPixelSize ), mExtent.yMaximum() );

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( mRasterDataType, cols, rows, rasterExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  const double step = rows > 0 ? 100.0 / rows : 1;

  for ( int row = 0; row < rows ; row++ )
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
    provider->writeBlock( &block, 1, 0, row );
    feedback->setProgress( row * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

//
//QgsRandomUniformRasterAlgorithm
//
QString QgsRandomUniformRasterAlgorithm::name() const
{
  return QStringLiteral( "createrandomuniformrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
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

QgsRandomUniformRasterAlgorithm *QgsRandomUniformRasterAlgorithm::createInstance() const
{
  return new QgsRandomUniformRasterAlgorithm();
}

void QgsRandomUniformRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Byte" )
                  << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 5, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > lowerBoundParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "LOWER_BOUND" ), QStringLiteral( "Lower bound for random number range" ), QgsProcessingParameterNumber::Double, QVariant(), true );
  lowerBoundParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( lowerBoundParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > upperBoundParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "UPPER_BOUND" ), QStringLiteral( "Upper bound for random number range" ), QgsProcessingParameterNumber::Double, QVariant(), true );
  upperBoundParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  mRandomUpperBound = parameterAsDouble( parameters, QStringLiteral( "UPPER_BOUND" ), context );
  mRandomLowerBound = parameterAsDouble( parameters, QStringLiteral( "LOWER_BOUND" ), context );

  if ( mRandomLowerBound > mRandomUpperBound )
    throw QgsProcessingException( QObject::tr( "The chosen lower bound for random number range is greater than the upper bound. The lower bound value must be smaller than the upper bound value." ) );

  const int typeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );
  const Qgis::DataType rasterDataType = getRasterDataType( typeId );

  switch ( rasterDataType )
  {
    case Qgis::DataType::Byte:
      if ( mRandomLowerBound < std::numeric_limits<quint8>::min() || mRandomUpperBound > std::numeric_limits<quint8>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint8>::min() ).arg( std::numeric_limits<quint8>::max() ).arg( QLatin1String( "Byte" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        //if parameters unset (=both are 0 or equal) --> use the whole value range
        mRandomUpperBound = std::numeric_limits<quint8>::max();
        mRandomLowerBound = std::numeric_limits<quint8>::min();
      }
      break;
    case Qgis::DataType::Int16:
      if ( mRandomLowerBound < std::numeric_limits<qint16>::min() || mRandomUpperBound > std::numeric_limits<qint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint16>::min() ).arg( std::numeric_limits<qint16>::max() ).arg( QLatin1String( "Integer16" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<qint16>::max();
        mRandomLowerBound = std::numeric_limits<qint16>::min();
      }
      break;
    case Qgis::DataType::UInt16:
      if ( mRandomLowerBound < std::numeric_limits<quint16>::min() || mRandomUpperBound > std::numeric_limits<quint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint16>::min() ).arg( std::numeric_limits<quint16>::max() ).arg( QLatin1String( "Unsigned Integer16" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<quint16>::max();
        mRandomLowerBound = std::numeric_limits<quint16>::min();
      }
      break;
    case Qgis::DataType::Int32:
      if ( mRandomLowerBound < std::numeric_limits<qint32>::min() || mRandomUpperBound > std::numeric_limits<qint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint32>::min() ).arg( std::numeric_limits<qint32>::max() ).arg( QLatin1String( "Integer32" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<qint32>::max();
        mRandomLowerBound = std::numeric_limits<qint32>::min();
      }
      break;
    case Qgis::DataType::UInt32:
      if ( mRandomLowerBound < std::numeric_limits<quint32>::min() || mRandomUpperBound > std::numeric_limits<quint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint32>::min() ).arg( std::numeric_limits<quint32>::max() ).arg( QLatin1String( "Unsigned Integer32" ) ) );
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
    default:
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
  return QStringLiteral( "createrandombinomialrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with binomially distributed random values.\n"
                      "By default, the values will be chosen given an N of 10 and a probability of 0.5. "
                      "This can be overridden by using the advanced parameter for N and probability. "
                      "The raster data type is set to Integer types (Integer16 by default). "
                      "The binomial distribution random values are defined as positive integer numbers. "
                      "A floating point raster will represent a cast of integer values "
                      "to floating point." );
}

QgsRandomBinomialRasterAlgorithm *QgsRandomBinomialRasterAlgorithm::createInstance() const
{
  return new QgsRandomBinomialRasterAlgorithm();
}


void QgsRandomBinomialRasterAlgorithm::addAlgorithmParams( )
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > nParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "N" ), QStringLiteral( "N" ), QgsProcessingParameterNumber::Integer, 10, true, 0 );
  nParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( nParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > probabilityParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "PROBABILITY" ), QStringLiteral( "Probability" ), QgsProcessingParameterNumber::Double, 0.5, true, 0 );
  probabilityParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const int n = parameterAsInt( parameters, QStringLiteral( "N" ), context );
  const double probability = parameterAsDouble( parameters, QStringLiteral( "PROBABILITY" ), context );
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
  return QStringLiteral( "createrandomexponentialrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with exponentially distributed random values.\n"
                      "By default, the values will be chosen given a lambda of 1.0. "
                      "This can be overridden by using the advanced parameter for lambda. "
                      "The raster data type is set to Float32 by default as "
                      "the exponential distribution random values are floating point numbers." );
}

QgsRandomExponentialRasterAlgorithm *QgsRandomExponentialRasterAlgorithm::createInstance() const
{
  return new QgsRandomExponentialRasterAlgorithm();
}


void QgsRandomExponentialRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > lambdaParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "LAMBDA" ), QStringLiteral( "Lambda" ), QgsProcessingParameterNumber::Double, 1.0, true, 0.000001 );
  lambdaParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const double lambda = parameterAsDouble( parameters, QStringLiteral( "LAMBDA" ), context );
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
  return QStringLiteral( "createrandomgammarasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with gamma distributed random values.\n"
                      "By default, the values will be chosen given an alpha and beta value of 1.0. "
                      "This can be overridden by using the advanced parameter for alpha and beta. "
                      "The raster data type is set to Float32 by default as "
                      "the gamma distribution random values are floating point numbers." );
}

QgsRandomGammaRasterAlgorithm *QgsRandomGammaRasterAlgorithm::createInstance() const
{
  return new QgsRandomGammaRasterAlgorithm();
}


void QgsRandomGammaRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > alphaParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "ALPHA" ), QStringLiteral( "Alpha" ), QgsProcessingParameterNumber::Double, 1.0, true, 0.000001 );
  alphaParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( alphaParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > betaParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "BETA" ), QStringLiteral( "Beta" ), QgsProcessingParameterNumber::Double, 1.0, true, 0.000001 );
  betaParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const double alpha = parameterAsDouble( parameters, QStringLiteral( "ALPHA" ), context );
  const double beta = parameterAsDouble( parameters, QStringLiteral( "BETA" ), context );
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
  return QStringLiteral( "createrandomgeometricrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with geometrically distributed random values.\n"
                      "By default, the values will be chosen given a probability of 0.5. "
                      "This can be overridden by using the advanced parameter for mean "
                      "value. The raster data type is set to Integer types (Integer16 by default). "
                      "The geometric distribution random values are defined as positive integer numbers. "
                      "A floating point raster will represent a cast of "
                      "integer values to floating point." );
}

QgsRandomGeometricRasterAlgorithm *QgsRandomGeometricRasterAlgorithm::createInstance() const
{
  return new QgsRandomGeometricRasterAlgorithm();
}


void QgsRandomGeometricRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > probabilityParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "PROBABILITY" ), QStringLiteral( "Probability" ), QgsProcessingParameterNumber::Double, 0.5, true, 0.00001 );
  probabilityParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const double probability = parameterAsDouble( parameters, QStringLiteral( "PROBABILITY" ), context );
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
  return QStringLiteral( "createrandomnegativebinomialrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with negative binomially distributed random values.\n"
                      "By default, the values will be chosen given a distribution parameter k of 10.0 "
                      "and a probability of 0.5. "
                      "This can be overridden by using the advanced parameters for k and probability. "
                      "The raster data type is set to Integer types (Integer 16 by default). "
                      "The negative binomial distribution random values are defined as positive integer numbers. "
                      "A floating point raster will represent a cast of "
                      "integer values to floating point." );
}

QgsRandomNegativeBinomialRasterAlgorithm *QgsRandomNegativeBinomialRasterAlgorithm::createInstance() const
{
  return new QgsRandomNegativeBinomialRasterAlgorithm();
}


void QgsRandomNegativeBinomialRasterAlgorithm::addAlgorithmParams( )
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > kParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "K_PARAMETER" ), QStringLiteral( "Distribution parameter k" ), QgsProcessingParameterNumber::Integer, 10, true, 0.00001 );
  kParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( kParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > probabilityParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "PROBABILITY" ), QStringLiteral( "Probability" ), QgsProcessingParameterNumber::Double, 0.5, true, 0.00001 );
  probabilityParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const int k = parameterAsInt( parameters, QStringLiteral( "K_PARAMETER" ), context );
  const double probability = parameterAsDouble( parameters, QStringLiteral( "PROBABILITY" ), context );
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
  return QStringLiteral( "createrandomnormalrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with normally distributed random values.\n"
                      "By default, the values will be chosen given a mean of 0.0 and "
                      "a standard deviation of 1.0. This can be overridden by "
                      "using the advanced parameters for mean and standard deviation "
                      "value. The raster data type is set to Float32 by default "
                      "as the normal distribution random values are floating point numbers." );
}

QgsRandomNormalRasterAlgorithm *QgsRandomNormalRasterAlgorithm::createInstance() const
{
  return new QgsRandomNormalRasterAlgorithm();
}

void QgsRandomNormalRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > meanParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MEAN" ), QStringLiteral( "Mean of normal distribution" ), QgsProcessingParameterNumber::Double, 0, true );
  meanParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( meanParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > stdevParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "STDDEV" ), QStringLiteral( "Standard deviation of normal distribution" ), QgsProcessingParameterNumber::Double, 1, true, 0 );
  stdevParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const double mean = parameterAsDouble( parameters, QStringLiteral( "MEAN" ), context );
  const double stddev = parameterAsDouble( parameters, QStringLiteral( "STDDEV" ), context );
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
  return QStringLiteral( "createrandompoissonrasterlayer" );
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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with poisson distributed random values.\n"
                      "By default, the values will be chosen given a mean of 1.0. "
                      "This can be overridden by using the advanced parameter for mean "
                      "value. The raster data type is set to Integer types (Integer16 by default). "
                      "The poisson distribution random values are positive integer numbers. "
                      "A floating point raster will represent a cast of integer values to floating point." );
}

QgsRandomPoissonRasterAlgorithm *QgsRandomPoissonRasterAlgorithm::createInstance() const
{
  return new QgsRandomPoissonRasterAlgorithm();
}


void QgsRandomPoissonRasterAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > upperBoundParameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MEAN" ), QStringLiteral( "Mean" ), QgsProcessingParameterNumber::Double, 1.0, true, 0 );
  upperBoundParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
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
  const double mean = parameterAsDouble( parameters, QStringLiteral( "MEAN" ), context );
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
