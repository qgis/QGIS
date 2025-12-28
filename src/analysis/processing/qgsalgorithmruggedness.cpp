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
  return u"ruggednessindex"_s;
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
  return u"rasterterrainanalysis"_s;
}

QString QgsRuggednessAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the quantitative measurement of terrain "
                      "heterogeneity described by Riley et al. (1999)." )
         + u"\n\n"_s
         + QObject::tr( "It is calculated for every location, by summarizing the change "
                        "in elevation within the 3x3 pixel grid. Each pixel contains the "
                        "difference in elevation from a center cell and the 8 cells surrounding it." );
}

QString QgsRuggednessAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the quantitative measurement of terrain heterogeneity described by Riley et al. (1999)." );
}

QgsRuggednessAlgorithm *QgsRuggednessAlgorithm::createInstance() const
{
  return new QgsRuggednessAlgorithm();
}

void QgsRuggednessAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"Z_FACTOR"_s, QObject::tr( "Z factor" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 ) );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999.0 );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Ruggedness" ) ) );
}

bool QgsRuggednessAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
  {
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );
  }

  mLayerSource = layer->source();
  return true;
}

QVariantMap QgsRuggednessAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const double zFactor = parameterAsDouble( parameters, u"Z_FACTOR"_s, context );
  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const double outputNodata = parameterAsDouble( parameters, u"NODATA"_s, context );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  QgsRuggednessFilter ruggedness( mLayerSource, outputFile, outputFormat );
  ruggedness.setZFactor( zFactor );
  if ( !creationOptions.isEmpty() )
  {
    ruggedness.setCreationOptions( creationOptions.split( '|' ) );
  }
  ruggedness.setOutputNodataValue( outputNodata );
  ruggedness.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
