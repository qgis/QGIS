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
  return QObject::tr( "export,tile" ).split( ',' );
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
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), QgsProcessing::TypePointCloud ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "LENGTH" ), QObject::tr( "Tile length" ), QgsProcessingParameterNumber::Double, 1000.0, false, 1 ) );

  std::unique_ptr< QgsProcessingParameterCrs > paramCrs = std::make_unique< QgsProcessingParameterCrs >( QStringLiteral( "CRS" ), QObject::tr( "Assign CRS" ), QVariant(), true );
  paramCrs->setFlags( paramCrs->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( paramCrs.release() );

  std::unique_ptr< QgsProcessingParameterFile > paramTempDir = std::make_unique< QgsProcessingParameterFile >( QStringLiteral( "TEMP_DIR" ), QObject::tr( "Temp directory" ), QgsProcessingParameterFile::Folder, QString(), QVariant(), true );
  paramTempDir->setFlags( paramTempDir->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( paramTempDir.release() );

  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output directory" ) ) );
}

QStringList QgsPdalTileAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  const QString outputDir = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputDir );

  int length = parameterAsInt( parameters, QStringLiteral( "LENGTH" ), context );

  QStringList args;
  args.reserve( layers.count() + 4 );

  args << QStringLiteral( "tile" )
       << QStringLiteral( "--length=%1" ).arg( length )
       << QStringLiteral( "--output=%1" ).arg( outputDir );

  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
    args << QStringLiteral( "--a_srs=%1" ).arg( crs.authid() );
  }

  if ( parameters.value( QStringLiteral( "TEMP_DIR" ) ).isValid() )
  {
    QString tempPath = parameterAsString( parameters, QStringLiteral( "TEMP_DIR" ), context );
    if ( !tempPath.isEmpty() )
    {
      args << QStringLiteral( "--temp_dir=%1" ).arg( tempPath );
    }
  }

  addThreadsParameter( args );

  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    args << layer->source();
  }

  return args;
}

///@endcond
