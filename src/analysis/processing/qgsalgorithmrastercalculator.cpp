/***************************************************************************
                         qgsalgorithmrastercalculator.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#include "qgsalgorithmrastercalculator.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsRasterCalculatorAlgorithm::name() const
{
  return QStringLiteral( "rastercalc" );
}

QString QgsRasterCalculatorAlgorithm::displayName() const
{
  return QObject::tr( "Raster calculator" );
}

QStringList QgsRasterCalculatorAlgorithm::tags() const
{
  return QObject::tr( "raster,calculator" ).split( ',' );
}

QString QgsRasterCalculatorAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterCalculatorAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRasterCalculatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "Performing algebraic operations using raster layers." );
}

QgsRasterCalculatorAlgorithm *QgsRasterCalculatorAlgorithm::createInstance() const
{
  return new QgsRasterCalculatorAlgorithm();
}

void QgsRasterCalculatorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT" ), QObject::tr( "Input layers" ), QgsProcessing::SourceType::TypeRaster ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(),  QStringLiteral( "INPUT" ), false, Qgis::ExpressionType::RasterCalculator ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Output extent" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE" ), QObject::tr( "Output cell size (leave empty to set automatically)" ), QgsProcessingParameterNumber::Double, QVariant(), true, 0.0 ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Output CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Calculated" ) ) );
}

QVariantMap QgsRasterCalculatorAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond

