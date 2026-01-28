/***************************************************************************
                         qgsalgorithmpdalfilternoiseradius.cpp
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

#include "qgsalgorithmpdalfilternoiseradius.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsPdalFilterNoiseRadiusAlgorithm::name() const
{
  return u"filternoiseradius"_s;
}

QString QgsPdalFilterNoiseRadiusAlgorithm::displayName() const
{
  return QObject::tr( "Filter noise (using radius)" );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalFilterNoiseRadiusAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,filter,noise,radius" ).split( ',' );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters noise in a point cloud using radius algorithm." )
         + u"\n\n"_s
         + QObject::tr( "Points are marked as noise if they have fewer than the minimum number of neighbors within the specified radius." );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters noise in a point cloud using radius algorithm." );
}

QgsPdalFilterNoiseRadiusAlgorithm *QgsPdalFilterNoiseRadiusAlgorithm::createInstance() const
{
  return new QgsPdalFilterNoiseRadiusAlgorithm();
}

void QgsPdalFilterNoiseRadiusAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"REMOVE_NOISE_POINTS"_s, QObject::tr( "Remove noise points" ), false ) );

  addParameter( new QgsProcessingParameterNumber( u"MIN_K"_s, QObject::tr( "Minimum number of neighbors in radius" ), Qgis::ProcessingNumberParameterType::Integer, 2 ) );
  addParameter( new QgsProcessingParameterNumber( u"RADIUS"_s, QObject::tr( "Radius" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );

  createVpcOutputFormatParameter();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Filtered (radius algorithm)" ) ) );
}

QStringList QgsPdalFilterNoiseRadiusAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  const double minK = parameterAsDouble( parameters, u"MIN_K"_s, context );
  const double radius = parameterAsDouble( parameters, u"RADIUS"_s, context );

  QString removeNoisePoints = "false";
  if ( parameterAsBoolean( parameters, u"REMOVE_NOISE_POINTS"_s, context ) )
  {
    removeNoisePoints = "true";
  }

  QStringList args = { u"filter_noise"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--algorithm=radius"_s, u"--remove-noise-points=%1"_s.arg( removeNoisePoints ), u"--radius-min-k=%1"_s.arg( minK ), u"--radius-radius=%1"_s.arg( radius ) };

  applyVpcOutputFormatParameter( outputFile, args, parameters, context, feedback );
  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
