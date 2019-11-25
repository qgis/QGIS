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
  return QObject::tr( "Raster tools" );
}

QString QgsConstantRasterAlgorithm::groupId() const
{
  return QStringLiteral( "rastertools" );
}

QString QgsConstantRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "Generates raster layer for given extent and cell "
                      "size filled with the specified value." );
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

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Constant" ) ) );
}

QVariantMap QgsConstantRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  double pixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );
  double value = parameterAsDouble( parameters, QStringLiteral( "NUMBER" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  int rows = std::max( std::ceil( extent.height() / pixelSize ), 1.0 );
  int cols = std::max( std::ceil( extent.width() / pixelSize ), 1.0 );

  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( Qgis::Float32, cols, rows, extent, crs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, -9999 );

  std::vector<float> line( cols );
  std::fill( line.begin(), line.end(), value );
  QgsRasterBlock block( Qgis::Float32, cols, 1 );
  block.setData( QByteArray::fromRawData( ( char * )&line[0], QgsRasterBlock::typeSize( Qgis::Float32 ) * cols ) );

  double step = rows > 0 ? 100.0 / rows : 1;

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
