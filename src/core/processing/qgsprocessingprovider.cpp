/***************************************************************************
                         qgsprocessingprovider.cpp
                         --------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#include "qgsprocessingprovider.h"
#include "qgsapplication.h"
#include "qgsvectorfilewriter.h"
#include "qgsrasterfilewriter.h"
#include "qgssettings.h"

QgsProcessingProvider::QgsProcessingProvider( QObject *parent SIP_TRANSFERTHIS )
  : QObject( parent )
{}


QgsProcessingProvider::~QgsProcessingProvider()
{
  qDeleteAll( mAlgorithms );
}

QIcon QgsProcessingProvider::icon() const
{
  return QgsApplication::getThemeIcon( "/processingAlgorithm.svg" );
}

QString QgsProcessingProvider::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "processingAlgorithm.svg" ) );
}

QgsProcessingProvider::Flags QgsProcessingProvider::flags() const
{
  return QgsProcessingProvider::Flags();
}

QString QgsProcessingProvider::helpId() const
{
  return QString();
}

QString QgsProcessingProvider::longName() const
{
  return name();
}

QString QgsProcessingProvider::versionInfo() const
{
  return QString();
}

QStringList QgsProcessingProvider::supportedOutputRasterLayerExtensions() const
{
  return QgsRasterFileWriter::supportedFormatExtensions();
}

QStringList QgsProcessingProvider::supportedOutputPointCloudLayerExtensions() const
{
  return QStringList();
}

void QgsProcessingProvider::refreshAlgorithms()
{
  qDeleteAll( mAlgorithms );
  mAlgorithms.clear();
  if ( isActive() )
  {
    loadAlgorithms();
    emit algorithmsLoaded();
  }
}

QList<const QgsProcessingAlgorithm *> QgsProcessingProvider::algorithms() const
{
  return mAlgorithms.values();
}

const QgsProcessingAlgorithm *QgsProcessingProvider::algorithm( const QString &name ) const
{
  return mAlgorithms.value( name );
}

bool QgsProcessingProvider::addAlgorithm( QgsProcessingAlgorithm *algorithm )
{
  if ( !algorithm )
    return false;

  if ( mAlgorithms.contains( algorithm->name() ) )
  {
    QgsMessageLog::logMessage( tr( "Duplicate algorithm name %1 for provider %2" ).arg( algorithm->name(), id() ), QObject::tr( "Processing" ) );
    return false;
  }

  // init the algorithm - this allows direct querying of the algorithm's parameters
  // and outputs from the provider's copy
  algorithm->initAlgorithm( QVariantMap() );

  algorithm->setProvider( this );
  mAlgorithms.insert( algorithm->name(), algorithm );
  return true;
}

QStringList QgsProcessingProvider::supportedOutputVectorLayerExtensions() const
{
  return QgsVectorFileWriter::supportedFormatExtensions();
}

QStringList QgsProcessingProvider::supportedOutputTableExtensions() const
{
  return supportedOutputVectorLayerExtensions();
}

bool QgsProcessingProvider::isSupportedOutputValue( const QVariant &outputValue, const QgsProcessingDestinationParameter *parameter, QgsProcessingContext &context, QString &error ) const
{
  error.clear();
  QString outputPath = QgsProcessingParameters::parameterAsOutputLayer( parameter, outputValue, context ).trimmed();

  if ( outputPath.isEmpty() )
  {
    if ( parameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
    {
      return true;
    }
    else
    {
      error = tr( "Missing parameter value %1" ).arg( parameter->description() );
      return false;
    }
  }

  if ( parameter->type() == QgsProcessingParameterVectorDestination::typeName()
       ||  parameter->type() == QgsProcessingParameterFeatureSink::typeName() )
  {
    if ( outputPath.startsWith( QLatin1String( "memory:" ) ) )
    {
      if ( !supportsNonFileBasedOutput() )
      {
        error = tr( "This algorithm only supports disk-based outputs" );
        return false;
      }
      return true;
    }

    QString providerKey;
    QString uri;
    QString layerName;
    QMap<QString, QVariant> options;
    bool useWriter = false;
    QString format;
    QString extension;
    QgsProcessingUtils::parseDestinationString( outputPath, providerKey, uri, layerName, format, options, useWriter, extension );

    if ( providerKey != QLatin1String( "ogr" ) )
    {
      if ( !supportsNonFileBasedOutput() )
      {
        error = tr( "This algorithm only supports disk-based outputs" );
        return false;
      }
      return true;
    }

    if ( !supportedOutputVectorLayerExtensions().contains( extension, Qt::CaseInsensitive ) )
    {
      error = tr( "“.%1” files are not supported as outputs for this algorithm" ).arg( extension );
      return false;
    }
    return true;
  }
  else if ( parameter->type() == QgsProcessingParameterRasterDestination::typeName() )
  {
    const QFileInfo fi( outputPath );
    const QString extension = fi.completeSuffix();
    if ( !supportedOutputRasterLayerExtensions().contains( extension, Qt::CaseInsensitive ) )
    {
      error = tr( "“.%1” files are not supported as outputs for this algorithm" ).arg( extension );
      return false;
    }
    return true;
  }
  else if ( parameter->type() == QgsProcessingParameterPointCloudDestination::typeName() )
  {
    const QFileInfo fi( outputPath );
    const QString extension = fi.completeSuffix();
    if ( !supportedOutputPointCloudLayerExtensions().contains( extension, Qt::CaseInsensitive ) )
    {
      error = tr( "“.%1” files are not supported as outputs for this algorithm" ).arg( extension );
      return false;
    }
    return true;
  }
  else
  {
    return true;
  }
}

QString QgsProcessingProvider::defaultVectorFileExtension( bool hasGeometry ) const
{
  const QString userDefault = QgsProcessingUtils::defaultVectorExtension();

  const QStringList supportedExtensions = supportedOutputVectorLayerExtensions();
  if ( supportedExtensions.contains( userDefault, Qt::CaseInsensitive ) )
  {
    // user set default is supported by provider, use that
    return userDefault;
  }
  else if ( !supportedExtensions.empty() )
  {
    return supportedExtensions.at( 0 );
  }
  else
  {
    // who knows? provider says it has no file support at all...
    // let's say shp. even MapInfo supports shapefiles.
    return hasGeometry ? QStringLiteral( "shp" ) : QStringLiteral( "dbf" );
  }
}

QString QgsProcessingProvider::defaultRasterFileExtension() const
{
  const QString userDefault = QgsProcessingUtils::defaultRasterExtension();

  const QStringList supportedExtensions = supportedOutputRasterLayerExtensions();
  if ( supportedExtensions.contains( userDefault, Qt::CaseInsensitive ) )
  {
    // user set default is supported by provider, use that
    return userDefault;
  }
  else if ( !supportedExtensions.empty() )
  {
    return supportedExtensions.at( 0 );
  }
  else
  {
    // who knows? provider says it has no file support at all...
    return QStringLiteral( "tif" );
  }
}

QString QgsProcessingProvider::defaultPointCloudFileExtension() const
{
  const QString userDefault = QgsProcessingUtils::defaultPointCloudExtension();

  const QStringList supportedExtensions = supportedOutputPointCloudLayerExtensions();
  if ( supportedExtensions.contains( userDefault, Qt::CaseInsensitive ) )
  {
    // user set default is supported by provider, use that
    return userDefault;
  }
  else if ( !supportedExtensions.empty() )
  {
    return supportedExtensions.at( 0 );
  }
  else
  {
    // who knows? provider says it has no file support at all...
    return QStringLiteral( "las" );
  }
}

bool QgsProcessingProvider::supportsNonFileBasedOutput() const
{
  return true;
}
