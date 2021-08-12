/***************************************************************************
                         qgsalgorithmaspect.cpp
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

#include "qgsalgorithmaspect.h"
#include "qgsrasterfilewriter.h"
#include "qgsaspectfilter.h"

///@cond PRIVATE

QString QgsAspectAlgorithm::name() const
{
  return QStringLiteral( "aspect" );
}

QString QgsAspectAlgorithm::displayName() const
{
  return QObject::tr( "Aspect" );
}

QStringList QgsAspectAlgorithm::tags() const
{
  return QObject::tr( "dem,aspect,terrain" ).split( ',' );
}

QString QgsAspectAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsAspectAlgorithm::groupId() const
{
  return QStringLiteral( "rasterterrainanalysis" );
}

QString QgsAspectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the aspect of the Digital Terrain Model in input." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The final aspect raster layer contains values from 0 to 360 that express "
                        "the slope direction: starting from North (0°) and continuing clockwise." );
}

QgsAspectAlgorithm *QgsAspectAlgorithm::createInstance() const
{
  return new QgsAspectAlgorithm();
}

void QgsAspectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "Z_FACTOR" ), QObject::tr( "Z factor" ),
                QgsProcessingParameterNumber::Double, 1, false, 0 ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Aspect" ) ) );
}

QVariantMap QgsAspectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const double zFactor = parameterAsDouble( parameters, QStringLiteral( "Z_FACTOR" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  QgsAspectFilter aspect( inputLayer->source(), outputFile, outputFormat );
  aspect.setZFactor( zFactor );
  aspect.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
