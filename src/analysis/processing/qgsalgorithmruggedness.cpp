/***************************************************************************
                         qgsalgorithmruggedness.cpp
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

#include "qgsalgorithmruggedness.h"
#include "qgsrasterfilewriter.h"
#include "qgsruggednessfilter.h"

///@cond PRIVATE

QString QgsRuggednessAlgorithm::name() const
{
  return QStringLiteral( "ruggednessindex" );
}

QString QgsRuggednessAlgorithm::displayName() const
{
  return QObject::tr( "Ruggedness index" );
}

QStringList QgsRuggednessAlgorithm::tags() const
{
  return QObject::tr( "dem,ruggedness,index,terrain" ).split( ',' );
}

QString QgsRuggednessAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsRuggednessAlgorithm::groupId() const
{
  return QStringLiteral( "rasterterrainanalysis" );
}

QString QgsRuggednessAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the quantitative measurement of terrain "
                      "heterogeneity described by Riley et al. (1999)." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "It is calculated for every location, by summarizing the change "
                        "in elevation within the 3x3 pixel grid. Each pixel contains the "
                        "difference in elevation from a center cell and the 8 cells surrounding it." );
}

QgsRuggednessAlgorithm *QgsRuggednessAlgorithm::createInstance() const
{
  return new QgsRuggednessAlgorithm();
}

void QgsRuggednessAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "Z_FACTOR" ), QObject::tr( "Z factor" ),
                QgsProcessingParameterNumber::Double, 1, false, 0 ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Ruggedness" ) ) );
}

QVariantMap QgsRuggednessAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const double zFactor = parameterAsDouble( parameters, QStringLiteral( "Z_FACTOR" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  QgsRuggednessFilter ruggedness( inputLayer->source(), outputFile, outputFormat );
  ruggedness.setZFactor( zFactor );
  ruggedness.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
