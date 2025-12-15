/***************************************************************************
                         qgsalgorithmtotalcurvature.cpp
                         ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Alexander Bruy
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

#include "qgsalgorithmtotalcurvature.h"

#include "qgsrasterfilewriter.h"
#include "qgstotalcurvaturefilter.h"

///@cond PRIVATE

QString QgsTotalCurvatureAlgorithm::name() const
{
  return QStringLiteral( "totalcurvature" );
}

QString QgsTotalCurvatureAlgorithm::displayName() const
{
  return QObject::tr( "Total curvature" );
}

QStringList QgsTotalCurvatureAlgorithm::tags() const
{
  return QObject::tr( "dem,total curvature,terrain" ).split( ',' );
}

QString QgsTotalCurvatureAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsTotalCurvatureAlgorithm::groupId() const
{
  return QStringLiteral( "rasterterrainanalysis" );
}

QString QgsTotalCurvatureAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates total curvature of the input raster as described by Wilson, Gallant (2000): terrain analysis." );
}

QString QgsTotalCurvatureAlgorithm::shortDescription() const
{
  return QObject::tr( "Ccalculates total curvature as described by Wilson, Gallant (2000): terrain analysis." );
}

QgsTotalCurvatureAlgorithm *QgsTotalCurvatureAlgorithm::createInstance() const
{
  return new QgsTotalCurvatureAlgorithm();
}

void QgsTotalCurvatureAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "Z_FACTOR" ), QObject::tr( "Z factor" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 ) );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "NODATA" ), QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999.0 );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "CREATION_OPTIONS" ), QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { QStringLiteral( "widget_wrapper" ), QVariantMap( { { QStringLiteral( "widget_type" ), QStringLiteral( "rasteroptions" ) } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Total curvature" ) ) );
}

QVariantMap QgsTotalCurvatureAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const double zFactor = parameterAsDouble( parameters, QStringLiteral( "Z_FACTOR" ), context );
  const QString creationOptions = parameterAsString( parameters, QStringLiteral( "CREATION_OPTIONS" ), context ).trimmed();
  const double outputNodata = parameterAsDouble( parameters, QStringLiteral( "NODATA" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, QStringLiteral( "OUTPUT" ), context );

  QgsTotalCurvatureFilter curvature( inputLayer->source(), outputFile, outputFormat );
  curvature.setZFactor( zFactor );
  if ( !creationOptions.isEmpty() )
  {
    curvature.setCreationOptions( creationOptions.split( '|' ) );
  }
  curvature.setOutputNodataValue( outputNodata );
  curvature.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
