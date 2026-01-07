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

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalMergeAlgorithm::name() const
{
  return u"merge"_s;
}

QString QgsPdalMergeAlgorithm::displayName() const
{
  return QObject::tr( "Merge point cloud" );
}

QString QgsPdalMergeAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalMergeAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalMergeAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,collect,merge,combine" ).split( ',' );
}

QString QgsPdalMergeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm merges multiple point cloud files to a single one." );
}

QString QgsPdalMergeAlgorithm::shortDescription() const
{
  return QObject::tr( "Merges multiple point clouds to a single one." );
}

QgsPdalMergeAlgorithm *QgsPdalMergeAlgorithm::createInstance() const
{
  return new QgsPdalMergeAlgorithm();
}

void QgsPdalMergeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Merged" ) ) );
}

QStringList QgsPdalMergeAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );

  if ( outputFile.endsWith( u".vpc"_s, Qt::CaseInsensitive ) )
    throw QgsProcessingException(
      QObject::tr( "This algorithm does not support output to VPC. Please use LAS or LAZ as the output format. "
                   "To create a VPC please use \"Build virtual point cloud (VPC)\" algorithm." )
    );

  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args;
  args.reserve( layers.count() + 3 );

  args << u"merge"_s
       << u"--output=%1"_s.arg( outputFile );

  applyCommonParameters( args, layers.at( 0 )->crs(), parameters, context );
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
