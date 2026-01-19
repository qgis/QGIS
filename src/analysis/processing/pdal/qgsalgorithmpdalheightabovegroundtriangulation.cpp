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

#include "qgsalgorithmpdalheightabovegroundtriangulation.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalHeightAboveGroundTriangulationAlgorithm::name() const
{
  return u"heightabovegroundtriangulation"_s;
}

QString QgsPdalHeightAboveGroundTriangulationAlgorithm::displayName() const
{
  return QObject::tr( "Height above ground (using triangulation)" );
}

QString QgsPdalHeightAboveGroundTriangulationAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalHeightAboveGroundTriangulationAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalHeightAboveGroundTriangulationAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,height above ground,delaunay,triangulation,elevation" ).split( ',' );
}

QString QgsPdalHeightAboveGroundTriangulationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the height of points above the ground surface in a point cloud using a Delaunay triangulation algorithm." )
         + u"\n\n"_s
         + QObject::tr( "The algorithm uses ground-classified points (classification value 2) to create a triangulated irregular network (TIN) from specified number of neighbors, then computes the height above this surface for all points." )
         + u"\n\n"_s
         + QObject::tr( "The output adds a HeightAboveGround dimension to the point cloud. If 'Replace Z values' is enabled, the Z coordinate will be replaced with the height above ground value." );
}

QString QgsPdalHeightAboveGroundTriangulationAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the height of points above the ground surface using a Delaunay algorithm." );
}

QgsPdalHeightAboveGroundTriangulationAlgorithm *QgsPdalHeightAboveGroundTriangulationAlgorithm::createInstance() const
{
  return new QgsPdalHeightAboveGroundTriangulationAlgorithm();
}

void QgsPdalHeightAboveGroundTriangulationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"REPLACE_Z"_s, QObject::tr( "Replace Z values with height above ground" ), true ) );
  addParameter( new QgsProcessingParameterNumber( u"COUNT"_s, QObject::tr( "Number of ground neighbors for terrain construction" ), Qgis::ProcessingNumberParameterType::Integer, 10 ) );

  createVpcOutputFormatParameter();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Height above ground (delaunay algorithm)" ) ) );
}

QStringList QgsPdalHeightAboveGroundTriangulationAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  QString replaceZ = "false";
  if ( parameterAsBoolean( parameters, u"REPLACE_Z"_s, context ) )
  {
    replaceZ = "true";
  }

  QStringList args = { u"height_above_ground"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--algorithm=delaunay"_s, u"--replace-z=%1"_s.arg( replaceZ ), u"--delaunay-count=%1"_s.arg( count ) };

  applyVpcOutputFormatParameter( outputFile, args, parameters, context );
  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
