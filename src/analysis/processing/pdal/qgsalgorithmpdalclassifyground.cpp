/***************************************************************************
                         qgsalgorithmpdalclassifyground.cpp
                         ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalclassifyground.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalClassifyGroundAlgorithm::name() const
{
  return u"classifyground"_s;
}

QString QgsPdalClassifyGroundAlgorithm::displayName() const
{
  return QObject::tr( "Classify ground points" );
}

QString QgsPdalClassifyGroundAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalClassifyGroundAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalClassifyGroundAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,classify,ground,elevation" ).split( ',' );
}

QString QgsPdalClassifyGroundAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm classifies ground points using the Simple Morphological Filter (SMRF) algorithm." )
         + u"\n\n"_s
         + QObject::tr( "Cell Size is the cell size for processing grid in map units, where smaller values give finer detail but may increase noise. Scalar is the threshold for steeper slopes; a higher value is needed if the terrain is rough, otherwise real ground might be misclassified. Slope is the slope threshold measured as rise over run, indicating how much slope is tolerated as ground and should be higher for steep terrain. Threshold is the elevation threshold for separating ground from objects; higher values allow larger deviations from the ground. Window Size is the maximum filter window size, where higher values better identify large buildings or objects while smaller values protect smaller features." );
}

QString QgsPdalClassifyGroundAlgorithm::shortDescription() const
{
  return QObject::tr( "Classifies ground points using the Simple Morphological Filter (SMRF) algorithm." );
}

QgsPdalClassifyGroundAlgorithm *QgsPdalClassifyGroundAlgorithm::createInstance() const
{
  return new QgsPdalClassifyGroundAlgorithm();
}

void QgsPdalClassifyGroundAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  QgsProcessingParameterNumber *cellSizeParam = new QgsProcessingParameterNumber( u"CELL_SIZE"_s, QObject::tr( "Grid Cell Size" ), Qgis::ProcessingNumberParameterType::Double, 1.0 );
  cellSizeParam->setHelp( QObject::tr( "Cell size for processing grid (in map units). Smaller values give finer detail but may increase noise." ) );
  addParameter( cellSizeParam );

  QgsProcessingParameterNumber *scalarParam = new QgsProcessingParameterNumber( u"SCALAR"_s, QObject::tr( "Scalar" ), Qgis::ProcessingNumberParameterType::Double, 1.25 );
  scalarParam->setHelp( QObject::tr( "Threshold for steeper slopes. Higher value if the terrain is rough, otherwise real ground might be misclassified." ) );
  addParameter( scalarParam );

  QgsProcessingParameterNumber *slopeParam = new QgsProcessingParameterNumber( u"SLOPE"_s, QObject::tr( "Slope" ), Qgis::ProcessingNumberParameterType::Double, 0.15 );
  slopeParam->setHelp( QObject::tr( "Slope threshold (rise over run). How much slope is tolerated as ground. Should be higher for steep terrain." ) );
  addParameter( slopeParam );

  QgsProcessingParameterNumber *thresholdParam = new QgsProcessingParameterNumber( u"THRESHOLD"_s, QObject::tr( "Threshold" ), Qgis::ProcessingNumberParameterType::Double, 0.5 );
  thresholdParam->setHelp( QObject::tr( "Elevation threshold for separating ground from objects. Higher values allow larger deviations from the ground." ) );
  addParameter( thresholdParam );

  QgsProcessingParameterNumber *windowSizeParam = new QgsProcessingParameterNumber( u"WINDOW_SIZE"_s, QObject::tr( "Window Size" ), Qgis::ProcessingNumberParameterType::Double, 18.0 );
  windowSizeParam->setHelp( QObject::tr( "Maximum filter window size. Higher values better identify large buildings or objects, smaller values protect smaller features." ) );
  addParameter( windowSizeParam );

  createVpcOutputFormatParameter();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Classified Ground" ) ) );
}

QStringList QgsPdalClassifyGroundAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  const double cellSize = parameterAsDouble( parameters, u"CELL_SIZE"_s, context );
  const double scalar = parameterAsDouble( parameters, u"SCALAR"_s, context );
  const double slope = parameterAsDouble( parameters, u"SLOPE"_s, context );
  const double threshold = parameterAsDouble( parameters, u"THRESHOLD"_s, context );
  const double windowSize = parameterAsDouble( parameters, u"WINDOW_SIZE"_s, context );

  QStringList args = { u"classify_ground"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--cell-size=%1"_s.arg( cellSize ), u"--scalar=%1"_s.arg( scalar ), u"--slope=%1"_s.arg( slope ), u"--threshold=%1"_s.arg( threshold ), u"--window-size=%1"_s.arg( windowSize ) };

  applyVpcOutputFormatParameter( outputFile, args, parameters, context, feedback );
  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
