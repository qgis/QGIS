/***************************************************************************
                         qgsalgorithmpdalcreatecopc.cpp
                         ---------------------
    begin                : May 2023
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

#include "qgsalgorithmpdalcreatecopc.h"

#include <QProcessEnvironment>
#include <QThread>
#include <QFileInfo>
#include <QDir>

#include "QgisUntwine.hpp"
#include "qgsapplication.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalCreateCopcAlgorithm::name() const
{
  return QStringLiteral( "createcopc" );
}

QString QgsPdalCreateCopcAlgorithm::displayName() const
{
  return QObject::tr( "Create COPC" );
}

QString QgsPdalCreateCopcAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalCreateCopcAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalCreateCopcAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,copc,convert,format,translate" ).split( ',' );
}

QString QgsPdalCreateCopcAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a COPC file for each input point cloud file." );
}

QgsPdalCreateCopcAlgorithm *QgsPdalCreateCopcAlgorithm::createInstance() const
{
  return new QgsPdalCreateCopcAlgorithm();
}

void QgsPdalCreateCopcAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output directory" ), QVariant(), true, false ) );
  addOutput( new QgsProcessingOutputMultipleLayers( QStringLiteral( "OUTPUT_LAYERS" ), QObject::tr( "Output layers" ) ) );
}

QVariantMap QgsPdalCreateCopcAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  QString untwineExecutable = QProcessEnvironment::systemEnvironment().value( QStringLiteral( "QGIS_UNTWINE_EXECUTABLE" ) );
  if ( untwineExecutable.isEmpty() )
  {
#if defined( Q_OS_WIN )
    untwineExecutable = QgsApplication::libexecPath() + "untwine.exe";
#else
    untwineExecutable = QgsApplication::libexecPath() + "untwine";
#endif
  }
  const QFileInfo executable( untwineExecutable );
  if ( !executable.isExecutable() )
  {
    throw QgsProcessingException( QObject::tr( "Untwine executable not found %1" ).arg( untwineExecutable ) );
  }

  const QString outputDir = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( !outputDir.isEmpty() && !QDir().mkpath( outputDir ) )
    throw QgsProcessingException( QObject::tr( "Failed to create output directory." ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( layers.size(), feedback );
  QStringList outputLayers;

  int i = 0;
  for ( QgsMapLayer *layer : layers )
  {
    if ( feedback->isCanceled() )
      break;

    multiStepFeedback.setCurrentStep( i );
    i++;

    if ( !layer )
      continue;

    QgsPointCloudLayer *pcl = qobject_cast<QgsPointCloudLayer *>( layer );
    if ( !pcl )
      continue;

    feedback->pushInfo( QObject::tr( "Processing layer %1/%2: %3" ).arg( i ).arg( layers.count() ).arg( layer ? layer->name() : QString() ) );

    if ( pcl->source().endsWith( QStringLiteral( ".copc.laz" ), Qt::CaseInsensitive ) )
    {
      feedback->pushInfo( QObject::tr( "File %1 is a COPC file. Skipping…" ).arg( pcl->source() ) );
      continue;
    }

    const QFileInfo fi( pcl->source() );
    const QDir directory = fi.absoluteDir();
    const QString outputFile = QStringLiteral( "%1/%2.copc.laz" ).arg( outputDir.isEmpty() ? directory.absolutePath() : outputDir ).arg( fi.completeBaseName() );

    const QFileInfo outputFileInfo( outputFile );
    if ( outputFileInfo.exists() )
    {
      feedback->pushInfo( QObject::tr( "File %1 is already indexed. Skipping…" ).arg( pcl->source() ) );
      continue;
    }
    QString tmpDir = outputFile + QStringLiteral( "_tmp" );
    if ( QDir( tmpDir ).exists() )
    {
      feedback->pushInfo( QObject::tr( "Another indexing process is running (or finished with crash) in directory %1." ).arg( tmpDir ) );
      continue;
    }

    untwine::QgisUntwine untwineProcess( untwineExecutable.toStdString() );
    untwine::QgisUntwine::Options options;
    // calculate stats for attributes
    options.push_back( { "stats", std::string() } );
    // generate COPC files
    options.push_back( { "single_file", std::string() } );

    const std::vector<std::string> files = { pcl->source().toStdString() };
    untwineProcess.start( files, outputFile.toStdString(), options );
    const int lastPercent = 0;
    while ( true )
    {
      if ( feedback->isCanceled() )
        break;

      QThread::msleep( 100 );
      const int percent = untwineProcess.progressPercent();
      if ( lastPercent != percent )
      {
        multiStepFeedback.setProgress( percent );
      }

      if ( !untwineProcess.running() )
      {
        if ( !untwineProcess.errorMessage().empty() )
        {
          feedback->pushWarning( QObject::tr( "Failed to index file %1: %2" ).arg( outputFile, QString::fromStdString( untwineProcess.errorMessage() ) ) );
        }
        break;
      }
    }

    QDir( tmpDir ).removeRecursively();

    outputLayers << outputFile;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_LAYERS" ), outputLayers );
  return outputs;
}

///@endcond
