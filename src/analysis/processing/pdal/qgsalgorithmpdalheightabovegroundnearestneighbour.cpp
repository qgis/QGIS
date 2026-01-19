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
  return u"heightabovegroundbynearestneighbor"_s;
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::displayName() const
{
  return QObject::tr( "Height above ground" );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,height above ground,nearest neighbour,elevation" ).split( ',' );
}

QString QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the height of points above the ground surface in a point cloud using a nearest neighbor algorithm." )
         + u"\n\n"_s
         + QObject::tr( "For each point, the algorithm finds the specified number of nearest ground-classified points (classification value 2) and interpolates the ground elevation from them using inverse distance weighting." )
         + u"\n\n"_s
         + QObject::tr( "The output adds a HeightAboveGround dimension to the point cloud. If 'Replace Z values' is enabled, the Z coordinate will be replaced with the height above ground value." )
         + u"\n\n"_s
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
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"REPLACE_Z"_s, QObject::tr( "Replace Z values with height above ground" ), true ) );
  addParameter( new QgsProcessingParameterNumber( u"COUNT"_s, QObject::tr( "Number of neighbors for terrain interpolation" ), Qgis::ProcessingNumberParameterType::Integer, 1 ) );
  addParameter( new QgsProcessingParameterNumber( u"MAX_DISTANCE"_s, QObject::tr( "Maximum search distance" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );

  createVpcOutputFormatParameter();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Height above ground (nearest neighbour)" ) ) );
}

QStringList QgsPdalHeightAboveGroundNearestNeighbourAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  const int count = parameterAsInt( parameters, u"COUNT"_s, context );
  const double maxDistance = parameterAsDouble( parameters, u"MAX_DISTANCE"_s, context );

  QString replaceZ = "false";
  if ( parameterAsBoolean( parameters, u"REPLACE_Z"_s, context ) )
  {
    replaceZ = "true";
  }

  QStringList args = { u"height_above_ground"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--algorithm=nn"_s, u"--replace-z=%1"_s.arg( replaceZ ), u"--nn-count=%1"_s.arg( count ), u"--nn-max-distance=%1"_s.arg( maxDistance ) };

  applyVpcOutputFormatParameter( outputFile, args, parameters, context );
  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
