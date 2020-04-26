/***************************************************************************
                         qgsalgorithmrandombinomialraster.cpp
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

#include "qgsalgorithmrandombinomialraster.h"
#include "qgsrasterfilewriter.h"
#include "qgsstringutils.h"
#include "random"

///@cond PRIVATE

//
// QgsRandomBinomialRasterAlgorithm
//
QString QgsRandomBinomialRasterAlgorithm::name() const
{
  return QStringLiteral( "createrandombinomialrasterlayer" );
}

QString QgsRandomBinomialRasterAlgorithm::group() const
{
  return QObject::tr( "Raster creation" );
}

QString QgsRandomBinomialRasterAlgorithm::groupId() const
{
  return QStringLiteral( "rastercreation" );
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
                      "By default, the values will be chosen given an N of 10 and a probability of 0.5."
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


void QgsRandomBinomialRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
                QgsProcessingParameterNumber::Double, 1, false, 0.01 ) );

  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > nParameter = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "N" ), QStringLiteral( "N" ), QgsProcessingParameterNumber::Integer, 10, true, 0 );
  nParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( nParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > probabilityParameter = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "PROBABILITY" ), QStringLiteral( "Probability" ), QgsProcessingParameterNumber::Double, 0.5, true, 0 );
  probabilityParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( probabilityParameter.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output raster" ) ) );
}

bool QgsRandomBinomialRasterAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );
  mPixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );

  mTypeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );
  mN = parameterAsInt( parameters, QStringLiteral( "N" ), context );
  mProbability = parameterAsDouble( parameters, QStringLiteral( "PROBABILITY" ), context );


  switch ( mTypeId )
  {
    case 0:
      mRasterDataType = Qgis::Int16;
      break;
    case 1:
      mRasterDataType = Qgis::UInt16;
      break;
    case 2:
      mRasterDataType = Qgis::Int32;
      break;
    case 3:
      mRasterDataType = Qgis::UInt32;
      break;
    case 4:
      mRasterDataType = Qgis::Float32;
      break;
    case 5:
      mRasterDataType = Qgis::Float64;
      break;
    default:
      mRasterDataType = Qgis::Int16; //standard output type
      break;
  }
  return true;
}


QVariantMap QgsRandomBinomialRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::random_device rd {};
  std::mt19937 mersenneTwister{rd()};

  std::binomial_distribution<long> randombinomialDistribution( mN, mProbability );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  int rows = std::max( std::ceil( mExtent.height() / mPixelSize ), 1.0 );
  int cols = std::max( std::ceil( mExtent.width() / mPixelSize ), 1.0 );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  QgsRectangle rasterExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMaximum() - ( rows * mPixelSize ), mExtent.xMinimum() + ( cols * mPixelSize ), mExtent.yMaximum() );

  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( mRasterDataType, cols, rows, rasterExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  double step = rows > 0 ? 100.0 / rows : 1;

  for ( int row = 0; row < rows ; row++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    //prepare raw data depending on raster data type
    QgsRasterBlock block( mRasterDataType, cols, 1 );
    switch ( mTypeId )
    {
      case 0:
      {
        std::vector<qint16> int16Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int16Row[col] = static_cast<qint16>( randombinomialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int16Row.data() ), QgsRasterBlock::typeSize( Qgis::Int16 ) * cols ) );
        break;
      }
      case 1:
      {
        std::vector<quint16> uInt16Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          uInt16Row[col] = static_cast<quint16>( randombinomialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( uInt16Row.data() ), QgsRasterBlock::typeSize( Qgis::UInt16 ) * cols ) );
        break;
      }
      case 2:
      {
        std::vector<qint32> int32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int32Row[col] = static_cast<qint32>( randombinomialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int32Row.data() ), QgsRasterBlock::typeSize( Qgis::Int32 ) * cols ) );
        break;
      }
      case 3:
      {
        std::vector<quint32> uInt32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          uInt32Row[col] = static_cast<quint32>( randombinomialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( uInt32Row.data() ), QgsRasterBlock::typeSize( Qgis::UInt32 ) * cols ) );
        break;
      }
      case 4:
      {
        std::vector<float> float32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float32Row[col] = static_cast<float>( randombinomialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float32Row.data() ), QgsRasterBlock::typeSize( Qgis::Float32 ) * cols ) );
        break;
      }
      case 5:
      {
        std::vector<double> float64Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float64Row[col] = static_cast<double>( randombinomialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float64Row.data() ), QgsRasterBlock::typeSize( Qgis::Float64 ) * cols ) );
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

///@endcond
