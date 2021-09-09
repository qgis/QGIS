/***************************************************************************
                         qgsalgorithmhillshade.cpp
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

#include "qgsalgorithmhillshade.h"
#include "qgsrasterfilewriter.h"
#include "qgshillshadefilter.h"

///@cond PRIVATE

QString QgsHillshadeAlgorithm::name() const
{
  return QStringLiteral( "hillshade" );
}

QString QgsHillshadeAlgorithm::displayName() const
{
  return QObject::tr( "Hillshade" );
}

QStringList QgsHillshadeAlgorithm::tags() const
{
  return QObject::tr( "dem,hillshade,terrain" ).split( ',' );
}

QString QgsHillshadeAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsHillshadeAlgorithm::groupId() const
{
  return QStringLiteral( "rasterterrainanalysis" );
}

QString QgsHillshadeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the hillshade of the Digital Terrain Model in input." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The shading of the layer is calculated according to the sun position (azimuth and elevation)." );
}

QgsHillshadeAlgorithm *QgsHillshadeAlgorithm::createInstance() const
{
  return new QgsHillshadeAlgorithm();
}

void QgsHillshadeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "Z_FACTOR" ), QObject::tr( "Z factor" ),
                QgsProcessingParameterNumber::Double, 1, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "AZIMUTH" ), QObject::tr( "Azimuth (horizontal angle)" ),
                QgsProcessingParameterNumber::Double, 300, false, 0, 360 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "V_ANGLE" ), QObject::tr( "Vertical angle" ),
                QgsProcessingParameterNumber::Double, 40, false, 0, 90 ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Hillshade" ) ) );
}

QVariantMap QgsHillshadeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const double zFactor = parameterAsDouble( parameters, QStringLiteral( "Z_FACTOR" ), context );
  const double azimuth = parameterAsDouble( parameters, QStringLiteral( "AZIMUTH" ), context );
  const double vAngle = parameterAsDouble( parameters, QStringLiteral( "V_ANGLE" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  QgsHillshadeFilter hillshade( inputLayer->source(), outputFile, outputFormat, azimuth, vAngle );
  hillshade.setZFactor( zFactor );
  hillshade.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
