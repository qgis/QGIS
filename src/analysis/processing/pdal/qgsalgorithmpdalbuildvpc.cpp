/***************************************************************************
                         qgsalgorithmpdalbuildvpc.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#include "qgsalgorithmpdalbuildvpc.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalBuildVpcAlgorithm::name() const
{
  return QStringLiteral( "virtualpointcloud" );
}

QString QgsPdalBuildVpcAlgorithm::displayName() const
{
  return QObject::tr( "Build virtual point cloud (VPC)" );
}

QString QgsPdalBuildVpcAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalBuildVpcAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalBuildVpcAlgorithm::tags() const
{
  return QObject::tr( "collect,merge,combine,point cloud,virtual,vpc" ).split( ',' );
}

QString QgsPdalBuildVpcAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a virtual point cloud from input data." );
}

QgsPdalBuildVpcAlgorithm *QgsPdalBuildVpcAlgorithm::createInstance() const
{
  return new QgsPdalBuildVpcAlgorithm();
}

void QgsPdalBuildVpcAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), QgsProcessing::TypePointCloud ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "BOUNDARY" ), QObject::tr( "Calculate boundary polygons" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "STATISTICS" ), QObject::tr( "Calculate statistics" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERVIEW" ), QObject::tr( "Build overview point cloud" ), false ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Virtual point cloud file" ), QObject::tr( "VPC files (*.vpc *.VPC)" ) ) );
}

QStringList QgsPdalBuildVpcAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args;
  args.reserve( layers.count() + 5 );

  args << QStringLiteral( "build_vpc" )
       << QStringLiteral( "--output=%1" ).arg( outputFile );

  if ( parameterAsBool( parameters, QStringLiteral( "BOUNDARY" ), context ) )
  {
    args << "--boundary";
  }

  if ( parameterAsBool( parameters, QStringLiteral( "STATISTICS" ), context ) )
  {
    args << "--stats";
  }

  if ( parameterAsBool( parameters, QStringLiteral( "OVERVIEW" ), context ) )
  {
    args << "--overview";
  }

  applyThreadsParameter( args );

  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    args << layer->source();
  }

  return args;
}

///@endcond
