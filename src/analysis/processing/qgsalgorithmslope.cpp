/***************************************************************************
                         qgsalgorithmslope.cpp
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

#include "qgsalgorithmslope.h"
#include "qgsrasterfilewriter.h"
#include "qgsslopefilter.h"

///@cond PRIVATE

QString QgsSlopeAlgorithm::name() const
{
  return QStringLiteral( "slope" );
}

QString QgsSlopeAlgorithm::displayName() const
{
  return QObject::tr( "Slope" );
}

QStringList QgsSlopeAlgorithm::tags() const
{
  return QObject::tr( "dem,slope,terrain" ).split( ',' );
}

QString QgsSlopeAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsSlopeAlgorithm::groupId() const
{
  return QStringLiteral( "rasterterrainanalysis" );
}

QString QgsSlopeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the angle of inclination "
                      "of the terrain from an input raster layer. The slope "
                      "is expressed in degrees." );
}

QgsSlopeAlgorithm *QgsSlopeAlgorithm::createInstance() const
{
  return new QgsSlopeAlgorithm();
}

void QgsSlopeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "Z_FACTOR" ), QObject::tr( "Z factor" ),
                QgsProcessingParameterNumber::Double, 1, false, 0 ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Slope" ) ) );
}

QVariantMap QgsSlopeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const double zFactor = parameterAsDouble( parameters, QStringLiteral( "Z_FACTOR" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  QgsSlopeFilter slope( inputLayer->source(), outputFile, outputFormat );
  slope.setZFactor( zFactor );
  slope.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
