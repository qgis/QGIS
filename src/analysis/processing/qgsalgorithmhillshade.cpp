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

#include "qgshillshadefilter.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsHillshadeAlgorithm::name() const
{
  return u"hillshade"_s;
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
  return u"rasterterrainanalysis"_s;
}

QString QgsHillshadeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the hillshade of the Digital Terrain Model in input." )
         + u"\n\n"_s
         + QObject::tr( "The shading of the layer is calculated according to the sun position (azimuth and elevation)." );
}

QString QgsHillshadeAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the hillshade of a Digital Terrain Model." );
}

QgsHillshadeAlgorithm *QgsHillshadeAlgorithm::createInstance() const
{
  return new QgsHillshadeAlgorithm();
}

void QgsHillshadeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Elevation layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"Z_FACTOR"_s, QObject::tr( "Z factor" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"AZIMUTH"_s, QObject::tr( "Azimuth (horizontal angle)" ), Qgis::ProcessingNumberParameterType::Double, 300, false, 0, 360 ) );
  addParameter( new QgsProcessingParameterNumber( u"V_ANGLE"_s, QObject::tr( "Vertical angle" ), Qgis::ProcessingNumberParameterType::Double, 40, false, 0, 90 ) );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999.0 );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Hillshade" ) ) );
}

bool QgsHillshadeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
  {
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );
  }

  mLayerSource = layer->source();
  return true;
}

QVariantMap QgsHillshadeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const double zFactor = parameterAsDouble( parameters, u"Z_FACTOR"_s, context );
  const double azimuth = parameterAsDouble( parameters, u"AZIMUTH"_s, context );
  const double vAngle = parameterAsDouble( parameters, u"V_ANGLE"_s, context );
  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const double outputNodata = parameterAsDouble( parameters, u"NODATA"_s, context );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  QgsHillshadeFilter hillshade( mLayerSource, outputFile, outputFormat, azimuth, vAngle );
  hillshade.setZFactor( zFactor );
  if ( !creationOptions.isEmpty() )
  {
    hillshade.setCreationOptions( creationOptions.split( '|' ) );
  }
  hillshade.setOutputNodataValue( outputNodata );
  hillshade.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
