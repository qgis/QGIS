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
  return QObject::tr( "collect,merge,combine,pdal,lidar,virtual,vpc" ).split( ',' );
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
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "BOUNDARY" ), QObject::tr( "Calculate boundary polygons" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "STATISTICS" ), QObject::tr( "Calculate statistics" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERVIEW" ), QObject::tr( "Build overview point cloud" ), false ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Virtual point cloud" ) ) );
}

QStringList QgsPdalBuildVpcAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFileName( outputName );

  QFileInfo fi( outputFileName );
  if ( fi.suffix() != QLatin1String( "vpc" ) )
  {
    outputFileName = fi.path() + '/' + fi.completeBaseName() + QStringLiteral( ".vpc" );
    if ( context.willLoadLayerOnCompletion( outputName ) )
    {
      QMap<QString, QgsProcessingContext::LayerDetails> layersToLoad = context.layersToLoadOnCompletion();
      QgsProcessingContext::LayerDetails details = layersToLoad.take( outputName );
      layersToLoad[outputFileName] = details;
      context.setLayersToLoadOnCompletion( layersToLoad );
    }
  }

  setOutputValue( QStringLiteral( "OUTPUT" ), outputFileName );

  QStringList args;
  args.reserve( layers.count() + 5 );

  args << QStringLiteral( "build_vpc" )
       << QStringLiteral( "--output=%1" ).arg( outputFileName );

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

  applyThreadsParameter( args, context );

  const QString fileName = QgsProcessingUtils::generateTempFilename( QStringLiteral( "inputFiles.txt" ), &context );
  QFile listFile( fileName );
  if ( !listFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not create input file list %1" ).arg( fileName ) );
  }

  QTextStream out( &listFile );
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
  out.setCodec( "UTF-8" );
#endif
  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    out << layer->source() << "\n";
  }

  args << QStringLiteral( "--input-file-list=%1" ).arg( fileName );

  return args;
}

///@endcond
