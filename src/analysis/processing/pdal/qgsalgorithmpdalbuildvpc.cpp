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

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalBuildVpcAlgorithm::name() const
{
  return u"virtualpointcloud"_s;
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
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalBuildVpcAlgorithm::tags() const
{
  return QObject::tr( "collect,merge,combine,pdal,lidar,virtual,vpc" ).split( ',' );
}

QString QgsPdalBuildVpcAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a virtual point cloud from input data." );
}

QString QgsPdalBuildVpcAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a virtual point cloud from input data." );
}

QgsPdalBuildVpcAlgorithm *QgsPdalBuildVpcAlgorithm::createInstance() const
{
  return new QgsPdalBuildVpcAlgorithm();
}

void QgsPdalBuildVpcAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  addParameter( new QgsProcessingParameterBoolean( u"BOUNDARY"_s, QObject::tr( "Calculate boundary polygons" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"STATISTICS"_s, QObject::tr( "Calculate statistics" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"OVERVIEW"_s, QObject::tr( "Build overview point cloud" ), false ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Virtual point cloud" ) ) );
}

QStringList QgsPdalBuildVpcAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFileName( outputName );

  QFileInfo fi( outputFileName );
  if ( fi.suffix() != "vpc"_L1 )
  {
    outputFileName = fi.path() + '/' + fi.completeBaseName() + u".vpc"_s;
    if ( context.willLoadLayerOnCompletion( outputName ) )
    {
      QMap<QString, QgsProcessingContext::LayerDetails> layersToLoad = context.layersToLoadOnCompletion();
      QgsProcessingContext::LayerDetails details = layersToLoad.take( outputName );
      layersToLoad[outputFileName] = details;
      context.setLayersToLoadOnCompletion( layersToLoad );
    }
  }

  setOutputValue( u"OUTPUT"_s, outputFileName );

  QStringList args;
  args.reserve( layers.count() + 5 );

  args << u"build_vpc"_s
       << u"--output=%1"_s.arg( outputFileName );

  if ( parameterAsBool( parameters, u"BOUNDARY"_s, context ) )
  {
    args << "--boundary";
  }

  if ( parameterAsBool( parameters, u"STATISTICS"_s, context ) )
  {
    args << "--stats";
  }

  if ( parameterAsBool( parameters, u"OVERVIEW"_s, context ) )
  {
    args << "--overview";
  }

  applyThreadsParameter( args, context );

  const QString fileName = QgsProcessingUtils::generateTempFilename( u"inputFiles.txt"_s, &context );
  QFile listFile( fileName );
  if ( !listFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not create input file list %1" ).arg( fileName ) );
  }

  QTextStream out( &listFile );
  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    out << layer->source() << "\n";
  }

  args << u"--input-file-list=%1"_s.arg( fileName );

  return args;
}

///@endcond
