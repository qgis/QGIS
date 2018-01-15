/***************************************************************************
                         qgsalgorithmpackage.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpackage.h"
#include "qgsgeometryengine.h"
#include "qgsogrutils.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsPackageAlgorithm::name() const
{
  return QStringLiteral( "package" );
}

QString QgsPackageAlgorithm::displayName() const
{
  return QObject::tr( "Package layers" );
}

QStringList QgsPackageAlgorithm::tags() const
{
  return QObject::tr( "geopackage,collect,merge,combine" ).split( ',' );
}

QString QgsPackageAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsPackageAlgorithm::groupId() const
{
  return QStringLiteral( "database" );
}

QgsProcessingAlgorithm::Flags QgsPackageAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsPackageAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Destination GeoPackage" ), QObject::tr( "GeoPackage files (*.gpkg)" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERWRITE" ), QObject::tr( "Overwrite existing GeoPackage" ), false ) );
  addOutput( new QgsProcessingOutputFile( QStringLiteral( "OUTPUT" ), QObject::tr( "GeoPackage" ) ) );
}

QString QgsPackageAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm collects a number of existing layers and packages them together into a single GeoPackage database." );
}

QgsPackageAlgorithm *QgsPackageAlgorithm::createInstance() const
{
  return new QgsPackageAlgorithm();
}

QVariantMap QgsPackageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  bool overwrite = parameterAsBool( parameters, QStringLiteral( "OVERWRITE" ), context );
  QString packagePath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( packagePath.isEmpty() )
    throw QgsProcessingException( QObject::tr( "No output file specified." ) );

  // delete existing geopackage if it exists
  if ( overwrite && QFile::exists( packagePath ) )
  {
    feedback->pushInfo( QObject::tr( "Removing existing file '%1'" ).arg( packagePath ) );
    if ( !QFile( packagePath ).remove() )
    {
      throw QgsProcessingException( QObject::tr( "Could not remove existing file '%1'" ) );
    }
  }

  OGRSFDriverH hGpkgDriver = OGRGetDriverByName( "GPKG" );
  if ( !hGpkgDriver )
  {
    throw QgsProcessingException( QObject::tr( "GeoPackage driver not found." ) );
  }

  gdal::ogr_datasource_unique_ptr hDS( OGR_Dr_CreateDataSource( hGpkgDriver, packagePath.toUtf8().constData(), nullptr ) );
  if ( !hDS )
    throw QgsProcessingException( QObject::tr( "Creation of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );

  bool errored = false;
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );

  QgsProcessingMultiStepFeedback multiStepFeedback( layers.count(), feedback );

  int i = 0;
  for ( QgsMapLayer *layer : layers )
  {
    if ( feedback->isCanceled() )
      break;

    multiStepFeedback.setCurrentStep( i );
    i++;

    feedback->pushInfo( QObject::tr( "Packaging layer %1/%2: %3" ).arg( i ).arg( layers.count() ).arg( layer ? layer->name() : QString() ) );

    if ( !layer )
    {
      // don't throw immediately - instead do what we can and error out later
      feedback->pushDebugInfo( QObject::tr( "Error retrieving map layer." ) );
      errored = true;
      continue;
    }

    switch ( layer->type() )
    {
      case QgsMapLayer::VectorLayer:
      {
        if ( !packageVectorLayer( qobject_cast< QgsVectorLayer * >( layer ), packagePath,
                                  context, &multiStepFeedback ) )
          errored = true;
        break;
      }

      case QgsMapLayer::RasterLayer:
      {
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Raster layers are not currently supported." ) );
        errored = true;
        break;
      }

      case QgsMapLayer::PluginLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging plugin layers is not supported." ) );
        errored = true;
        break;
    }
  }

  if ( errored )
    throw QgsProcessingException( QObject::tr( "Error obtained while packaging one or more layers." ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), packagePath );
  return outputs;
}

bool QgsPackageAlgorithm::packageVectorLayer( QgsVectorLayer *layer, const QString &path, QgsProcessingContext &context,
    QgsProcessingFeedback *feedback )
{
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = QStringLiteral( "GPKG" );
  options.layerName = layer->name();
  options.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteLayer;
  options.fileEncoding = context.defaultEncoding();
  options.feedback = feedback;

  QString error;
  if ( QgsVectorFileWriter::writeAsVectorFormat( layer, path, options, &error ) != QgsVectorFileWriter::NoError )
  {
    feedback->pushDebugInfo( QObject::tr( "Packaging layer failed: %1" ).arg( error ) );
    return false;
  }
  else
  {
    return true;
  }
}

///@endcond
