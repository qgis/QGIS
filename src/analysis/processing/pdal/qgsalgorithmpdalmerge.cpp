/***************************************************************************
                         qgsalgorithmpdalmerge.cpp
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

#include "qgsalgorithmpdalmerge.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalMergeAlgorithm::name() const
{
  return QStringLiteral( "merge" );
}

QString QgsPdalMergeAlgorithm::displayName() const
{
  return QObject::tr( "Merge" );
}

QString QgsPdalMergeAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalMergeAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalMergeAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,collect,merge,combine" ).split( ',' );
}

QString QgsPdalMergeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm merges multiple point cloud files to a single one." );
}

QgsPdalMergeAlgorithm *QgsPdalMergeAlgorithm::createInstance() const
{
  return new QgsPdalMergeAlgorithm();
}

void QgsPdalMergeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Merged" ) ) );
}

QStringList QgsPdalMergeAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );

  if ( outputFile.endsWith( QStringLiteral( ".copc.laz" ), Qt::CaseInsensitive ) )
    throw QgsProcessingException(
      QObject::tr( "This algorithm does not support output to COPC. Please use LAS or LAZ as the output format. "
                   "LAS/LAZ files get automatically converted to COPC when loaded in QGIS, alternatively you can use "
                   "\"Create COPC\" algorithm." )
    );

  if ( outputFile.endsWith( QStringLiteral( ".vpc" ), Qt::CaseInsensitive ) )
    throw QgsProcessingException(
      QObject::tr( "This algorithm does not support output to VPC. Please use LAS or LAZ as the output format. "
                   "To create a VPC please use \"Build virtual point cloud (VPC)\" algorithm." )
    );

  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args;
  args.reserve( layers.count() + 3 );

  args << QStringLiteral( "merge" )
       << QStringLiteral( "--output=%1" ).arg( outputFile );

  applyCommonParameters( args, layers.at( 0 )->crs(), parameters, context );
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
