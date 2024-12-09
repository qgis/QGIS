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

#include "qgsprocessingutils.h"
#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalTileAlgorithm::name() const
{
  return QStringLiteral( "tile" );
}

QString QgsPdalTileAlgorithm::displayName() const
{
  return QObject::tr( "Tile" );
}

QString QgsPdalTileAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalTileAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalTileAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,export,tile" ).split( ',' );
}

QString QgsPdalTileAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates tiles from input data." );
}

QgsPdalTileAlgorithm *QgsPdalTileAlgorithm::createInstance() const
{
  return new QgsPdalTileAlgorithm();
}

void QgsPdalTileAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "LENGTH" ), QObject::tr( "Tile length" ), Qgis::ProcessingNumberParameterType::Double, 1000.0, false, 1 ) );

  std::unique_ptr<QgsProcessingParameterCrs> paramCrs = std::make_unique<QgsProcessingParameterCrs>( QStringLiteral( "CRS" ), QObject::tr( "Assign CRS" ), QVariant(), true );
  paramCrs->setFlags( paramCrs->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramCrs.release() );

  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output directory" ) ) );
}

QStringList QgsPdalTileAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputDir = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputDir );

  if ( !QDir().mkpath( outputDir ) )
    throw QgsProcessingException( QObject::tr( "Failed to create output directory." ) );

  int length = parameterAsInt( parameters, QStringLiteral( "LENGTH" ), context );

  QStringList args;
  args.reserve( layers.count() + 4 );

  const QString tempDir = context.temporaryFolder().isEmpty() ? QgsProcessingUtils::tempFolder( &context ) : context.temporaryFolder();

  args << QStringLiteral( "tile" )
       << QStringLiteral( "--length=%1" ).arg( length )
       << QStringLiteral( "--output=%1" ).arg( outputDir )
       << QStringLiteral( "--temp_dir=%1" ).arg( tempDir );

  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
    args << QStringLiteral( "--a_srs=%1" ).arg( crs.authid() );
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
