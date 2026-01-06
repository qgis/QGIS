/***************************************************************************
                         qgsalgorithmpdaltile.cpp
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

#include "qgsalgorithmpdaltile.h"

#include "qgspointcloudlayer.h"
#include "qgsprocessingutils.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalTileAlgorithm::name() const
{
  return u"tile"_s;
}

QString QgsPdalTileAlgorithm::displayName() const
{
  return QObject::tr( "Create tiles from point cloud" );
}

QString QgsPdalTileAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalTileAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalTileAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,export,tile" ).split( ',' );
}

QString QgsPdalTileAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates tiles from an input point cloud." );
}

QString QgsPdalTileAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates tiles from a point cloud." );
}

QgsPdalTileAlgorithm *QgsPdalTileAlgorithm::createInstance() const
{
  return new QgsPdalTileAlgorithm();
}

void QgsPdalTileAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  addParameter( new QgsProcessingParameterNumber( u"LENGTH"_s, QObject::tr( "Tile length" ), Qgis::ProcessingNumberParameterType::Double, 1000.0, false, 1 ) );

  auto paramCrs = std::make_unique<QgsProcessingParameterCrs>( u"CRS"_s, QObject::tr( "Assign CRS" ), QVariant(), true );
  paramCrs->setFlags( paramCrs->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramCrs.release() );

  addParameter( new QgsProcessingParameterFolderDestination( u"OUTPUT"_s, QObject::tr( "Output directory" ) ) );
}

QStringList QgsPdalTileAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputDir = parameterAsString( parameters, u"OUTPUT"_s, context );
  setOutputValue( u"OUTPUT"_s, outputDir );

  if ( !QDir().mkpath( outputDir ) )
    throw QgsProcessingException( QObject::tr( "Failed to create output directory." ) );

  int length = parameterAsInt( parameters, u"LENGTH"_s, context );

  QStringList args;
  args.reserve( layers.count() + 4 );

  const QString tempDir = context.temporaryFolder().isEmpty() ? QgsProcessingUtils::tempFolder( &context ) : context.temporaryFolder();

  args << u"tile"_s
       << u"--length=%1"_s.arg( length )
       << u"--output=%1"_s.arg( outputDir )
       << u"--temp_dir=%1"_s.arg( tempDir );

  if ( parameters.value( u"CRS"_s ).isValid() )
  {
    QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );
    args << u"--a_srs=%1"_s.arg( crs.authid() );
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
