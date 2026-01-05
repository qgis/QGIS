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
  return QStringLiteral( "classifyground" );
}

QString QgsPdalClassifyGroundAlgorithm::displayName() const
{
  return QObject::tr( "Classify Ground Points" );
}

QString QgsPdalClassifyGroundAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalClassifyGroundAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalClassifyGroundAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,classify,ground,elevation" ).split( ',' );
}

QString QgsPdalClassifyGroundAlgorithm::shortHelpString() const
{
  return QObject::tr( "Classifies ground points using the Simple Morphological Filter (SMRF) algorithm." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Parameters:" )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Cell Size - cell size for processing grid (in map units). Smaller values give finer detail but may increase noise." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Scalar - Threshold for steeper slopes. Higher value if the terrain is rough, otherwise real ground might be misclassified." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Slope - slope threshold (rise over run). How much slope is tolerated as ground. Should be higher for steep terrain." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Threshold - elevation threshold for separating ground from objects. Higher values allow larger deviations from the ground." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Window Size - maximum filter window size. Higher values better identify large buildings or objects, smaller values protect smaller features." );
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
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE" ), QObject::tr( "Grid Cell Size" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SCALAR" ), QObject::tr( "Scalar" ), Qgis::ProcessingNumberParameterType::Double, 1.25 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SLOPE" ), QObject::tr( "Slope" ), Qgis::ProcessingNumberParameterType::Double, 0.15 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "THRESHOLD" ), QObject::tr( "Threshold" ), Qgis::ProcessingNumberParameterType::Double, 0.5 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "WINDOW_SIZE" ), QObject::tr( "Window size" ), Qgis::ProcessingNumberParameterType::Double, 18.0 ) );

  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Classified Ground" ) ) );
}

QStringList QgsPdalClassifyGroundAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  const double cellSize = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE" ), context );
  const double scalar = parameterAsDouble( parameters, QStringLiteral( "SCALAR" ), context );
  const double slope = parameterAsDouble( parameters, QStringLiteral( "SLOPE" ), context );
  const double threshold = parameterAsDouble( parameters, QStringLiteral( "THRESHOLD" ), context );
  const double windowSize = parameterAsDouble( parameters, QStringLiteral( "WINDOW_SIZE" ), context );

  QStringList args = { QStringLiteral( "classify_ground" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--cell-size=%1" ).arg( cellSize ), QStringLiteral( "--scalar=%1" ).arg( scalar ), QStringLiteral( "--slope=%1" ).arg( slope ), QStringLiteral( "--threshold=%1" ).arg( threshold ), QStringLiteral( "--window-size=%1" ).arg( windowSize ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
