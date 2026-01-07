/***************************************************************************
                         qgsalgorithmpdalheightabovegrounddelaunay.cpp
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

#include "qgsalgorithmpdalheightabovegrounddelaunay.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalHeightAboveGroundDelaunayAlgorithm::name() const
{
  return QStringLiteral( "heightabovegroundbydelaunay" );
}

QString QgsPdalHeightAboveGroundDelaunayAlgorithm::displayName() const
{
  return QObject::tr( "Height Above Ground (by Delaunay)" );
}

QString QgsPdalHeightAboveGroundDelaunayAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalHeightAboveGroundDelaunayAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalHeightAboveGroundDelaunayAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,height above ground,delaunay,elevation" ).split( ',' );
}

QString QgsPdalHeightAboveGroundDelaunayAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the height of points above the ground surface in a point cloud using a Delaunay algorithm." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The algorithm uses ground-classified points (classification value 2) to create a triangulated irregular network (TIN) from specified number of neighbors, then computes the height above this surface for all points." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The output adds a HeightAboveGround dimension to the point cloud. If 'Replace Z values' is enabled, the Z coordinate will be replaced with the height above ground value." );
}

QString QgsPdalHeightAboveGroundDelaunayAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the height of points above the ground surface using a Delaunay algorithm." );
}

QgsPdalHeightAboveGroundDelaunayAlgorithm *QgsPdalHeightAboveGroundDelaunayAlgorithm::createInstance() const
{
  return new QgsPdalHeightAboveGroundDelaunayAlgorithm();
}

void QgsPdalHeightAboveGroundDelaunayAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "REPLACE_Z" ), QObject::tr( "Replace Z values with height above ground" ), true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "COUNT" ), QObject::tr( "Number of ground neighbors for terrain construction" ), Qgis::ProcessingNumberParameterType::Integer, 10 ) );

  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Height above ground (delaunay algorithm)" ) ) );
}

QStringList QgsPdalHeightAboveGroundDelaunayAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  QString replaceZ = "false";
  if ( parameterAsBoolean( parameters, QStringLiteral( "REPLACE_Z" ), context ) )
  {
    replaceZ = "true";
  }

  QStringList args = { QStringLiteral( "height_above_ground" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--algorithm=delaunay" ), QStringLiteral( "--replace-z=%1" ).arg( replaceZ ), QStringLiteral( "--delaunay-count=%1" ).arg( count ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
