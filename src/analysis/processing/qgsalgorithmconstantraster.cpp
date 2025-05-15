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
  return QObject::tr( "This algorithm generates a raster layer for a given extent and cell "
                      "size filled with a single constant value.\n"
                      "Additionally an output data type can be specified. "
                      "The algorithm will abort if a value has been entered that "
                      "cannot be represented by the selected output raster data type." );
}

QString QgsConstantRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer for a given extent and cell size filled with a single constant value." );
}

QgsConstantRasterAlgorithm *QgsConstantRasterAlgorithm::createInstance() const
{
  return new QgsConstantRasterAlgorithm();
}

void QgsConstantRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NUMBER" ), QObject::tr( "Constant value" ), Qgis::ProcessingNumberParameterType::Double, 1, false ) );

  QStringList rasterDataTypes; //currently supported raster data types that can be handled QgsRasterBlock::writeValue()
  rasterDataTypes << QStringLiteral( "Byte" )
                  << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  //QGIS3: parameter set to Float32 by default so that existing models/scripts don't break
  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ), rasterDataTypes, false, 5, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

  // backwards compatibility parameter
  // TODO QGIS 4: remove parameter and related logic
  auto createOptsParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "CREATE_OPTIONS" ), QObject::tr( "Creation options" ), QVariant(), false, true );
  createOptsParam->setMetadata( QVariantMap( { { QStringLiteral( "widget_wrapper" ), QVariantMap( { { QStringLiteral( "widget_type" ), QStringLiteral( "rasteroptions" ) } } ) } } ) );
  createOptsParam->setFlags( createOptsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( createOptsParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "CREATION_OPTIONS" ), QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { QStringLiteral( "widget_wrapper" ), QVariantMap( { { QStringLiteral( "widget_type" ), QStringLiteral( "rasteroptions" ) } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Constant" ) ) );
}

QVariantMap QgsConstantRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  const double pixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );
  const double value = parameterAsDouble( parameters, QStringLiteral( "NUMBER" ), context );
  const int typeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );

  if ( pixelSize <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Pixel size must be greater than 0." ) );
  }

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

  QString creationOptions = parameterAsString( parameters, QStringLiteral( "CREATION_OPTIONS" ), context ).trimmed();
  // handle backwards compatibility parameter CREATE_OPTIONS
  const QString optionsString = parameterAsString( parameters, QStringLiteral( "CREATE_OPTIONS" ), context );
  if ( !optionsString.isEmpty() )
    creationOptions = optionsString;

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  // round up width and height to the nearest integer as GDAL does (e.g. in gdal_rasterize)
  // see https://github.com/qgis/QGIS/issues/43547
  const int rows = static_cast<int>( 0.5 + extent.height() / pixelSize );
  const int cols = static_cast<int>( 0.5 + extent.width() / pixelSize );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  const QgsRectangle rasterExtent = QgsRectangle( extent.xMinimum(), extent.yMaximum() - ( rows * pixelSize ), extent.xMinimum() + ( cols * pixelSize ), extent.yMaximum() );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  if ( !creationOptions.isEmpty() )
  {
    writer->setCreationOptions( creationOptions.split( '|' ) );
  }
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( rasterDataType, cols, rows, rasterExtent, crs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  //Thoughts on noData:
  //Setting a noData value is disabled so that the user is protected from accidentally creating an empty raster (eg. when value is set to -9999)
  //We could also allow creating empty rasters by exposing a noData value parameter (usecases?).

  //prepare raw data depending on raster data type
  QgsRasterBlock block( rasterDataType, cols, 1 );
  block.fill( value );

  const double step = rows > 0 ? 100.0 / rows : 1;

  for ( int i = 0; i < rows; i++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !provider->writeBlock( &block, 1, 0, i ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( provider->error().summary() ) );
    }
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
