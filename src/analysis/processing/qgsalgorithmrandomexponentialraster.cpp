/***************************************************************************
                         qgsalgorithmrandomexponentialraster.cpp
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

#include "qgsalgorithmrandomexponentialraster.h"
#include "qgsrasterfilewriter.h"
#include "qgsstringutils.h"
#include "random"

///@cond PRIVATE

//
// QgsRandomExponentialRasterAlgorithm
//
QString QgsRandomExponentialRasterAlgorithm::name() const
{
  return QStringLiteral( "createrandomexponentialrasterlayer" );
}

QString QgsRandomExponentialRasterAlgorithm::group() const
{
  return QObject::tr( "Raster creation" );
}

QString QgsRandomExponentialRasterAlgorithm::groupId() const
{
  return QStringLiteral( "rastercreation" );
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


void QgsRandomExponentialRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
                QgsProcessingParameterNumber::Double, 1, false, 0.01 ) );

  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 0, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > lambdaParameter = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "LAMBDA" ), QStringLiteral( "Lambda" ), QgsProcessingParameterNumber::Double, 1.0, true, 0.000001 );
  lambdaParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( lambdaParameter.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output raster" ) ) );
}

bool QgsRandomExponentialRasterAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );
  mPixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );

  mTypeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );
  mLambda = parameterAsDouble( parameters, QStringLiteral( "LAMBDA" ), context );

  if ( mTypeId == 0 )
  {
    mRasterDataType = Qgis::Float32;
  }
  else
  {
    mRasterDataType = Qgis::Float64;
  }

  return true;
}


QVariantMap QgsRandomExponentialRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::random_device rd {};
  std::mt19937 mersenneTwister{rd()};

  std::exponential_distribution<double> exponentialDistribution( mLambda );

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
      case ( 0 ):
      {
        std::vector<float> float32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float32Row[col] = static_cast<float>( exponentialDistribution( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float32Row.data() ), QgsRasterBlock::typeSize( Qgis::Float32 ) * cols ) );
        break;
      }
      case ( 1 ):
      {
        std::vector<double> float64Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float64Row[col] = exponentialDistribution( mersenneTwister );
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
