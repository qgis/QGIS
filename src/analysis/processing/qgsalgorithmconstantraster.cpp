/***************************************************************************
                         qgsalgorithmconstantraster.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#include <limits>
#include "math.h"
#include "qgsalgorithmconstantraster.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsConstantRasterAlgorithm::name() const
{
  return QStringLiteral( "createconstantrasterlayer" );
}

QString QgsConstantRasterAlgorithm::displayName() const
{
  return QObject::tr( "Create constant raster layer" );
}

QStringList QgsConstantRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,create,constant" ).split( ',' );
}

QString QgsConstantRasterAlgorithm::group() const
{
  return QObject::tr( "Raster creation" );
}

QString QgsConstantRasterAlgorithm::groupId() const
{
  return QStringLiteral( "rastercreation" );
}

QString QgsConstantRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "Generates raster layer for given extent and cell "
                      "size filled with the specified value.\n"
                      "Additionally an output data type can be specified. "
                      "The algorithm will abort if a value has been entered that "
                      "cannot be represented by the selected output raster data type." );
}

QgsConstantRasterAlgorithm *QgsConstantRasterAlgorithm::createInstance() const
{
  return new QgsConstantRasterAlgorithm();
}

void QgsConstantRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
                QgsProcessingParameterNumber::Double, 0.00001, false, 0.01 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NUMBER" ), QObject::tr( "Constant value" ),
                QgsProcessingParameterNumber::Double, 1, false ) );

  QStringList rasterDataTypes; //currently supported raster data types that can be handled QgsRasterBlock::writeValue()
  rasterDataTypes << QStringLiteral( "Byte" )
                  << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  //QGIS3: parameter set to Float32 by default so that existing models/scripts don't break
  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 5, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Constant" ) ) );
}

QVariantMap QgsConstantRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  const double pixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );
  const double value = parameterAsDouble( parameters, QStringLiteral( "NUMBER" ), context );
  const int typeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );

  //implement warning if input float has decimal places but is written to integer raster
  double fractpart;
  double intpart;
  fractpart = abs( std::modf( value, &intpart ) ); //@abs: negative values may be entered

  Qgis::DataType rasterDataType = Qgis::DataType::Float32; //standard output type
  switch ( typeId )
  {
    case 0:
      rasterDataType = Qgis::DataType::Byte;
      if ( value < std::numeric_limits<quint8>::min() || value > std::numeric_limits<quint8>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2" ).arg( std::numeric_limits<quint8>::min() ).arg( std::numeric_limits<quint8>::max() ).arg( QLatin1String( "Byte" ) ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( QLatin1String( "Byte" ) ) );
      break;
    case 1:
      rasterDataType = Qgis::DataType::Int16;
      if ( value < std::numeric_limits<qint16>::min() || value > std::numeric_limits<qint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2" ).arg( std::numeric_limits<qint16>::min() ).arg( std::numeric_limits<qint16>::max() ).arg( QLatin1String( "Integer16" ) ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( QLatin1String( "Integer16" ) ) );
      break;
    case 2:
      rasterDataType = Qgis::DataType::UInt16;
      if ( value < std::numeric_limits<quint16>::min() || value > std::numeric_limits<quint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2" ).arg( std::numeric_limits<quint16>::min() ).arg( std::numeric_limits<quint16>::max() ).arg( "Unsigned Integer16" ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( QLatin1String( "Unsigned Integer16" ) ) );
      break;
    case 3:
      rasterDataType = Qgis::DataType::Int32;
      if ( value < std::numeric_limits<qint32>::min() || value > std::numeric_limits<qint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2" ).arg( std::numeric_limits<qint32>::min() ).arg( std::numeric_limits<qint32>::max() ).arg( QLatin1String( "Integer32" ) ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( QLatin1String( "Integer32" ) ) );
      break;
    case 4:
      rasterDataType = Qgis::DataType::UInt32;
      if ( value < std::numeric_limits<quint32>::min() || value > std::numeric_limits<quint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2" ).arg( std::numeric_limits<quint32>::min() ).arg( std::numeric_limits<quint32>::max() ).arg( QLatin1String( "Unsigned Integer32" ) ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( QLatin1String( "Unsigned Integer32" ) ) );
      break;
    case 5:
      rasterDataType = Qgis::DataType::Float32;
      break;
    case 6:
      rasterDataType = Qgis::DataType::Float64;
      break;
    default:
      break;
  }
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  const int rows = std::max( std::ceil( extent.height() / pixelSize ), 1.0 );
  const int cols = std::max( std::ceil( extent.width() / pixelSize ), 1.0 );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  const QgsRectangle rasterExtent = QgsRectangle( extent.xMinimum(), extent.yMaximum() - ( rows * pixelSize ), extent.xMinimum() + ( cols * pixelSize ), extent.yMaximum() );

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( rasterDataType, cols, rows, rasterExtent, crs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  //Thoughts on noData:
  //Setting a noData value is disabled so that the user is protected from accidentally creating an empty raster (eg. when value is set to -9999)
  //We could also allow creating empty rasters by exposing a noData value parameter (usecases?).

  //prepare raw data depending on raster data type
  QgsRasterBlock block( rasterDataType, cols, 1 );
  switch ( typeId )
  {
    case 0:
    {
      std::vector<quint8> byteRow( cols );
      std::fill( byteRow.begin(), byteRow.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&byteRow[0], QgsRasterBlock::typeSize( Qgis::DataType::Byte ) * cols ) );
      break;
    }
    case 1:
    {
      std::vector<qint16> int16Row( cols );
      std::fill( int16Row.begin(), int16Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&int16Row[0], QgsRasterBlock::typeSize( Qgis::DataType::Int16 ) * cols ) );
      break;
    }
    case 2:
    {
      std::vector<quint16> uInt16Row( cols );
      std::fill( uInt16Row.begin(), uInt16Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&uInt16Row[0], QgsRasterBlock::typeSize( Qgis::DataType::UInt16 ) * cols ) );
      break;
    }
    case 3:
    {
      std::vector<qint32> int32Row( cols );
      std::fill( int32Row.begin(), int32Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&int32Row[0], QgsRasterBlock::typeSize( Qgis::DataType::Int32 ) * cols ) );
      break;
    }
    case 4:
    {
      std::vector<quint32> uInt32Row( cols );
      std::fill( uInt32Row.begin(), uInt32Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&uInt32Row[0], QgsRasterBlock::typeSize( Qgis::DataType::UInt32 ) * cols ) );
      break;
    }
    case 5:
    {
      std::vector<float> float32Row( cols );
      std::fill( float32Row.begin(), float32Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&float32Row[0], QgsRasterBlock::typeSize( Qgis::DataType::Float32 ) * cols ) );
      break;
    }
    case 6:
    {
      std::vector<double> float64Row( cols );
      std::fill( float64Row.begin(), float64Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&float64Row[0], QgsRasterBlock::typeSize( Qgis::DataType::Float64 ) * cols ) );
      break;
    }
    default:
    {
      std::vector<float> float32Row( cols );
      std::fill( float32Row.begin(), float32Row.end(), value );
      block.setData( QByteArray::fromRawData( ( char * )&float32Row[0], QgsRasterBlock::typeSize( Qgis::DataType::Float32 ) * cols ) );
      break;
    }
  }

  const double step = rows > 0 ? 100.0 / rows : 1;

  for ( int i = 0; i < rows ; i++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    provider->writeBlock( &block, 1, 0, i );
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
