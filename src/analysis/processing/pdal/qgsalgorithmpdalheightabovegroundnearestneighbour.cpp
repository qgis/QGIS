/***************************************************************************
                         qgsalgorithmpdalheightabovegroundnearestneighbour.cpp
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

#include "qgsalgorithmpdalheightabovegroundnearestneighbour.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::name() const
{
  return QStringLiteral( "heightabovegroundbynearestneighbor" );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::displayName() const
{
  return QObject::tr( "Height Above Ground (by Nearest Neighbor)" );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,height above ground,nearest neighbour,elevation" ).split( ',' );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::shortHelpString() const
{
  return QObject::tr( "Calculates the height of points above the ground surface in a point cloud using a nearest neighbor algorithm." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "For each point, the algorithm finds the specified number of nearest ground-classified points (classification value 2) and interpolates the ground elevation from them using inverse distance weighting." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The output adds a HeightAboveGround dimension to the point cloud. If 'Replace Z values' is enabled, the Z coordinate will be replaced with the height above ground value." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Maximum Distance parameter can limit the search radius (0 = no limit)." );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the height of points above the ground surface in a point cloud using a nearest neighbor algorithm." );
}

QgsPdalHeightAboveGroundNearestNeighbourAlgorithm *QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::createInstance() const
{
  return new QgsPdalHeightAboveGroundNearestNeighbourAlgorithm();
}

void QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "REPLACE_Z" ), QObject::tr( "Replace Z values with height above ground" ), true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "COUNT" ), QObject::tr( "Number of neighbors for terrain interpolation" ), Qgis::ProcessingNumberParameterType::Integer, 1 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_DISTANCE" ), QObject::tr( "Maximum search distance" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );

  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Height above ground (nearest neighbour)" ) ) );
}

QStringList QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  const int count = parameterAsInt( parameters, QStringLiteral( "COUNT" ), context );
  const double maxDistance = parameterAsDouble( parameters, QStringLiteral( "MAX_DISTANCE" ), context );

  QString replaceZ = "false";
  if ( parameterAsBoolean( parameters, QStringLiteral( "REPLACE_Z" ), context ) )
  {
    replaceZ = "true";
  }

  QStringList args = { QStringLiteral( "height_above_ground" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--algorithm=nn" ), QStringLiteral( "--replace-z=%1" ).arg( replaceZ ), QStringLiteral( "--nn-count=%1" ).arg( count ), QStringLiteral( "--nn-max-distance=%1" ).arg( maxDistance ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
