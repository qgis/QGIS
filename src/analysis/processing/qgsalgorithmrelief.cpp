/***************************************************************************
                         qgsalgorithmrelief.cpp
                         ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrelief.h"

#include "qgsprocessingparameterreliefcolors.h"
#include "qgsrelief.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE


QString QgsReliefAlgorithm::name() const
{
  return u"relief"_s;
}

QString QgsReliefAlgorithm::displayName() const
{
  return QObject::tr( "Relief" );
}

QStringList QgsReliefAlgorithm::tags() const
{
  return QObject::tr( "elevation" ).split( ',' );
}

QString QgsReliefAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsReliefAlgorithm::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QgsReliefAlgorithm::~QgsReliefAlgorithm() = default;

void QgsReliefAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto inputParam = std::make_unique<QgsProcessingParameterRasterLayer>( u"INPUT"_s, QObject::tr( "Elevation layer" ) );
  inputParam->setHelp( QObject::tr( "The digital elevation model (DEM) raster layer to use for calculating the shaded relief." ) );
  addParameter( inputParam.release() );

  auto zFactorParam = std::make_unique<QgsProcessingParameterNumber>( u"Z_FACTOR"_s, QObject::tr( "Z factor" ), Qgis::ProcessingNumberParameterType::Double, 1.0, false, 0.0 );
  zFactorParam->setHelp( QObject::tr( "The Z factor allows you to exaggerate or compress the elevation values. A value of 1.0 means no exaggeration. Values greater than 1.0 exaggerate the relief." ) );
  addParameter( zFactorParam.release() );

  auto autoColorsParam = std::make_unique<QgsProcessingParameterBoolean>( u"AUTO_COLORS"_s, QObject::tr( "Generate relief classes automatically" ), false );
  autoColorsParam->setHelp( QObject::tr( "If checked, relief classes (colors and elevation ranges) will be generated automatically based on the input layer's statistics." ) );
  addParameter( autoColorsParam.release() );

  auto colorsParam = std::make_unique<QgsProcessingParameterReliefColors>( u"COLORS"_s, QObject::tr( "Relief colors" ), u"INPUT"_s, true );
  colorsParam->setHelp(
    QObject::tr( "The color classes to use for the relief. Each class is defined by a minimum elevation, maximum elevation, and a color. Ignored if 'Generate relief classes automatically' is checked." )
  );
  addParameter( colorsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Relief" ) );
  outputParam->setHelp( QObject::tr( "The output shaded relief raster layer." ) );
  addParameter( outputParam.release() );

  auto freqDistParam
    = std::make_unique<QgsProcessingParameterFileDestination>( u"FREQUENCY_DISTRIBUTION"_s, QObject::tr( "Frequency distribution" ), QObject::tr( "CSV files (*.csv)" ), QVariant(), true, false );
  freqDistParam->setHelp( QObject::tr( "An optional CSV file to save the frequency distribution of the relief classes." ) );
  addParameter( freqDistParam.release() );
}

QString QgsReliefAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a shaded relief layer from digital elevation data." );
}

QString QgsReliefAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a shaded relief layer from digital elevation data." );
}

QgsReliefAlgorithm *QgsReliefAlgorithm::createInstance() const
{
  return new QgsReliefAlgorithm();
}

bool QgsReliefAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
  {
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );
  }

  mLayerSource = layer->source();
  return true;
}

QVariantMap QgsReliefAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const double zFactor = parameterAsDouble( parameters, u"Z_FACTOR"_s, context );
  const bool automaticColors = parameterAsBoolean( parameters, u"AUTO_COLORS"_s, context );
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );
  const QString frequencyDistribution = parameterAsFileOutput( parameters, u"FREQUENCY_DISTRIBUTION"_s, context );

  QgsRelief relief( mLayerSource, outputFile, outputFormat );

  QList<QgsRasterReliefColor> reliefColors;
  if ( automaticColors )
  {
    reliefColors = relief.calculateOptimizedReliefClasses();
  }
  else
  {
    reliefColors = qgis::down_cast< const QgsProcessingParameterReliefColors * >( parameterDefinition( u"COLORS"_s ) )->valueAsReliefColors( parameters.value( u"COLORS"_s ), context );
    if ( reliefColors.isEmpty() )
      throw QgsProcessingException( QObject::tr( "Specify relief colors or activate \"Generate relief classes automatically\" option." ) );
  }

  relief.setReliefColors( reliefColors );
  relief.setZFactor( zFactor );

  if ( !frequencyDistribution.isEmpty() )
    relief.exportFrequencyDistributionToCsv( frequencyDistribution );

  const QgsRelief::Result result = relief.processRaster( feedback );
  switch ( result )
  {
    case QgsRelief::Result::Success:
      break;

    case QgsRelief::Result::InvalidInput:
      throw QgsProcessingException( QObject::tr( "Could not open input file." ) );

    case QgsRelief::Result::OutputCreationFailed:
      throw QgsProcessingException( QObject::tr( "Could not create output file." ) );

    case QgsRelief::Result::InvalidInputSize:
      throw QgsProcessingException( QObject::tr( "Input raster size is too small (at least 3 rows needed)." ) );

    case QgsRelief::Result::Canceled:
      feedback->pushInfo( QObject::tr( "Canceled." ) );
      break;
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  if ( !frequencyDistribution.isEmpty() )
    outputs.insert( u"FREQUENCY_DISTRIBUTION"_s, frequencyDistribution );

  return outputs;
}

///@endcond
