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

#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QThread>

#ifdef HAVE_PDAL_QGIS
#include "QgisUntwine.hpp"
#endif
#include "qgsapplication.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalCreateCopcAlgorithm::name() const
{
  return u"createcopc"_s;
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
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalCreateCopcAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,copc,convert,format,translate" ).split( ',' );
}

QString QgsPdalCreateCopcAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a COPC file for each input point cloud file." );
}

QString QgsPdalCreateCopcAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a COPC file for each input point cloud." );
}

QgsPdalCreateCopcAlgorithm *QgsPdalCreateCopcAlgorithm::createInstance() const
{
  return new QgsPdalCreateCopcAlgorithm();
}

void QgsPdalCreateCopcAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::PointCloud ) );
  addParameter( new QgsProcessingParameterFolderDestination( u"OUTPUT"_s, QObject::tr( "Output directory" ), QVariant(), true, false ) );
  addOutput( new QgsProcessingOutputMultipleLayers( u"OUTPUT_LAYERS"_s, QObject::tr( "Output layers" ) ) );
}

bool QgsPdalCreateCopcAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters )
  Q_UNUSED( context )
  Q_UNUSED( feedback )

#ifdef HAVE_PDAL_QGIS
  return true;
#else
  throw QgsProcessingException( QObject::tr( "This algorithm requires a QGIS installation with PDAL support enabled." ) );
#endif
}

QVariantMap QgsPdalCreateCopcAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
#ifdef HAVE_PDAL_QGIS
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( layers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), true );
  }

  QString untwineExecutable = QProcessEnvironment::systemEnvironment().value( u"QGIS_UNTWINE_EXECUTABLE"_s );
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

  const QString outputDir = parameterAsString( parameters, u"OUTPUT"_s, context );
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

    feedback->pushInfo( QObject::tr( "Processing layer %1/%2: %3" ).arg( i ).arg( layers.count() ).arg( layer->name() ) );

    if ( pcl->source().endsWith( u".copc.laz"_s, Qt::CaseInsensitive ) )
    {
      feedback->pushInfo( QObject::tr( "File %1 is a COPC file. Skipping…" ).arg( pcl->source() ) );
      continue;
    }

    const QFileInfo fi( pcl->source() );
    const QDir directory = fi.absoluteDir();
    const QString outputFile = u"%1/%2.copc.laz"_s.arg( outputDir.isEmpty() ? directory.absolutePath() : outputDir ).arg( fi.completeBaseName() );

    const QFileInfo outputFileInfo( outputFile );
    if ( outputFileInfo.exists() )
    {
      feedback->pushInfo( QObject::tr( "File %1 is already indexed. Skipping…" ).arg( pcl->source() ) );
      continue;
    }
    QString tmpDir = outputFile + u"_tmp"_s;
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
  outputs.insert( u"OUTPUT_LAYERS"_s, outputLayers );
  return outputs;
#else
  Q_UNUSED( parameters )
  Q_UNUSED( context )
  Q_UNUSED( feedback )
  throw QgsProcessingException( QObject::tr( "This algorithm requires a QGIS installation with PDAL support enabled." ) );
#endif
}

///@endcond
