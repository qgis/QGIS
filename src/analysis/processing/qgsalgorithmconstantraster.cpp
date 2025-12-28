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

#include "qgsalgorithmconstantraster.h"

#include <limits>
#include <math.h>

#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsConstantRasterAlgorithm::name() const
{
  return u"createconstantrasterlayer"_s;
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
  return u"rastercreation"_s;
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
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( u"TARGET_CRS"_s, QObject::tr( "Target CRS" ), u"ProjectCrs"_s ) );
  addParameter( new QgsProcessingParameterNumber( u"PIXEL_SIZE"_s, QObject::tr( "Pixel size" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"NUMBER"_s, QObject::tr( "Constant value" ), Qgis::ProcessingNumberParameterType::Double, 1, false ) );

  QStringList rasterDataTypes; //currently supported raster data types that can be handled QgsRasterBlock::writeValue()
  rasterDataTypes << u"Byte"_s
                  << u"Integer16"_s
                  << u"Unsigned Integer16"_s
                  << u"Integer32"_s
                  << u"Unsigned Integer32"_s
                  << u"Float32"_s
                  << u"Float64"_s;

  //QGIS3: parameter set to Float32 by default so that existing models/scripts don't break
  std::unique_ptr<QgsProcessingParameterDefinition> rasterTypeParameter = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_TYPE"_s, QObject::tr( "Output raster data type" ), rasterDataTypes, false, 5, false );
  rasterTypeParameter->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( rasterTypeParameter.release() );

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

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Constant" ) ) );
}

QVariantMap QgsConstantRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"TARGET_CRS"_s, context );
  const QgsRectangle extent = parameterAsExtent( parameters, u"EXTENT"_s, context, crs );
  const double pixelSize = parameterAsDouble( parameters, u"PIXEL_SIZE"_s, context );
  const double value = parameterAsDouble( parameters, u"NUMBER"_s, context );
  const int typeId = parameterAsInt( parameters, u"OUTPUT_TYPE"_s, context );

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
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2" ).arg( std::numeric_limits<quint8>::min() ).arg( std::numeric_limits<quint8>::max() ).arg( "Byte"_L1 ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( "Byte"_L1 ) );
      break;
    case 1:
      rasterDataType = Qgis::DataType::Int16;
      if ( value < std::numeric_limits<qint16>::min() || value > std::numeric_limits<qint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2" ).arg( std::numeric_limits<qint16>::min() ).arg( std::numeric_limits<qint16>::max() ).arg( "Integer16"_L1 ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( "Integer16"_L1 ) );
      break;
    case 2:
      rasterDataType = Qgis::DataType::UInt16;
      if ( value < std::numeric_limits<quint16>::min() || value > std::numeric_limits<quint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2" ).arg( std::numeric_limits<quint16>::min() ).arg( std::numeric_limits<quint16>::max() ).arg( "Unsigned Integer16" ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( "Unsigned Integer16"_L1 ) );
      break;
    case 3:
      rasterDataType = Qgis::DataType::Int32;
      if ( value < std::numeric_limits<qint32>::min() || value > std::numeric_limits<qint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2" ).arg( std::numeric_limits<qint32>::min() ).arg( std::numeric_limits<qint32>::max() ).arg( "Integer32"_L1 ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( "Integer32"_L1 ) );
      break;
    case 4:
      rasterDataType = Qgis::DataType::UInt32;
      if ( value < std::numeric_limits<quint32>::min() || value > std::numeric_limits<quint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2" ).arg( std::numeric_limits<quint32>::min() ).arg( std::numeric_limits<quint32>::max() ).arg( "Unsigned Integer32"_L1 ) );
      if ( fractpart > 0 )
        feedback->reportError( QObject::tr( "The entered constant value has decimals but will be written to a raster dataset of type %1. The decimals of the constant value will be omitted." ).arg( "Unsigned Integer32"_L1 ) );
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

  QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  // handle backwards compatibility parameter CREATE_OPTIONS
  const QString optionsString = parameterAsString( parameters, u"CREATE_OPTIONS"_s, context );
  if ( !optionsString.isEmpty() )
    creationOptions = optionsString;

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  // round up width and height to the nearest integer as GDAL does (e.g. in gdal_rasterize)
  // see https://github.com/qgis/QGIS/issues/43547
  const int rows = static_cast<int>( 0.5 + extent.height() / pixelSize );
  const int cols = static_cast<int>( 0.5 + extent.width() / pixelSize );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  const QgsRectangle rasterExtent = QgsRectangle( extent.xMinimum(), extent.yMaximum() - ( rows * pixelSize ), extent.xMinimum() + ( cols * pixelSize ), extent.yMaximum() );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputProviderKey( u"gdal"_s );
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
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
