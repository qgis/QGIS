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

#include "qgsaspectfilter.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsAspectAlgorithm::name() const
{
  return u"aspect"_s;
}

QString QgsAspectAlgorithm::displayName() const
{
  return QObject::tr( "Aspect" );
}

QStringList QgsAspectAlgorithm::tags() const
{
  return QObject::tr( "dem,aspect,terrain,slope" ).split( ',' );
}

QString QgsAspectAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsAspectAlgorithm::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QString QgsAspectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the aspect of the Digital Terrain Model in input." )
         + u"\n\n"_s
         + QObject::tr( "The final aspect raster layer contains values from 0 to 360 that express "
                        "the slope direction: starting from North (0°) and continuing clockwise." );
}

QString QgsAspectAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a raster layer representing the slope direction from a Digital Terrain Model." );
}

QgsAspectAlgorithm *QgsAspectAlgorithm::createInstance() const
{
  return new QgsAspectAlgorithm();
}

void QgsAspectAlgorithm::initAlgorithm( const QVariantMap & )
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

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Aspect" ) ) );
}

bool QgsAspectAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
  {
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );
  }

  mLayerSource = layer->source();
  return true;
}

QVariantMap QgsAspectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const double zFactor = parameterAsDouble( parameters, u"Z_FACTOR"_s, context );
  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const double outputNodata = parameterAsDouble( parameters, u"NODATA"_s, context );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  QgsAspectFilter aspect( mLayerSource, outputFile, outputFormat );
  aspect.setZFactor( zFactor );
  if ( !creationOptions.isEmpty() )
  {
    aspect.setCreationOptions( creationOptions.split( '|' ) );
  }
  aspect.setOutputNodataValue( outputNodata );
  aspect.processRaster( feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
