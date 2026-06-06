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
#include "qgsmessagelog.h"
#include "qgsrasterfilewriter.h"
#include "qgsvectorfilewriter.h"

#include <QRegularExpressionMatch>
#include <QString>

#include "moc_qgsprocessingprovider.cpp"

using namespace Qt::StringLiterals;

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
  return QgsApplication::iconPath( u"processingAlgorithm.svg"_s );
}

Qgis::ProcessingProviderFlags QgsProcessingProvider::flags() const
{
  return Qgis::ProcessingProviderFlags();
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
  const QList<QgsProcessingFormatExtensionPair> formatAndExtensions = supportedOutputRasterLayerFormatAndExtensions();
  QSet< QString > extensions;
  QStringList res;
  for ( const QgsProcessingFormatExtensionPair &formatAndExt : std::as_const( formatAndExtensions ) )
  {
    if ( !extensions.contains( formatAndExt.extension ) )
    {
      extensions.insert( formatAndExt.extension );
      res << formatAndExt.extension;
    }
  }
  return res;
}

QList<QgsProcessingFormatExtensionPair> QgsProcessingProvider::supportedOutputRasterLayerFormatAndExtensions() const
{
  return supportedOutputRasterLayerFormatAndExtensionsDefault();
}

QList<QgsProcessingFormatExtensionPair> QgsProcessingProvider::supportedOutputRasterLayerFormatAndExtensionsDefault()
{
  const auto formats = QgsRasterFileWriter::supportedFiltersAndFormats();
  QList<QgsProcessingFormatExtensionPair> res;

  const thread_local QRegularExpression rx( u"\\*\\.([a-zA-Z0-9]*)"_s );

  for ( const QgsRasterFileWriter::FilterFormatDetails &format : formats )
  {
    const QString ext = format.filterString;
    const QRegularExpressionMatch match = rx.match( ext );
    if ( !match.hasMatch() )
      continue;

    const QString matched = match.captured( 1 );
    res << QgsProcessingFormatExtensionPair( format.driverName, matched );
  }

  std::sort( res.begin(), res.end(), []( const QgsProcessingFormatExtensionPair &a, const QgsProcessingFormatExtensionPair &b ) -> bool {
    for ( const QString &tifExt : { u"tif"_s, u"tiff"_s } )
    {
      if ( a.extension == tifExt )
      {
        if ( b.extension == a.extension )
        {
          if ( a.format == "GTiff"_L1 )
            return true;
          else if ( b.format == "GTiff"_L1 )
            return false;
          return a.format.toLower().localeAwareCompare( b.format.toLower() ) < 0;
        }
        return true;
      }
      else if ( b.extension == tifExt )
        return false;
    }

    if ( a.extension == "gpkg"_L1 )
    {
      if ( b.extension == a.extension )
        return a.format.toLower().localeAwareCompare( b.format.toLower() ) < 0;
      return true;
    }
    else if ( b.extension == "gpkg"_L1 )
      return false;

    return a.extension.toLower().localeAwareCompare( b.extension.toLower() ) < 0;
  } );

  return res;
}

QStringList QgsProcessingProvider::supportedOutputPointCloudLayerExtensions() const
{
  return QStringList();
}

QStringList QgsProcessingProvider::supportedOutputVectorTileLayerExtensions() const
{
  return QStringList() << QgsProcessingUtils::defaultVectorTileExtension();
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
  QString outputPath = QgsProcessingParameters::parameterAsOutputLayer( parameter, outputValue, context, true ).trimmed();

  if ( outputPath.isEmpty() )
  {
    if ( parameter->flags() & Qgis::ProcessingParameterFlag::Optional )
    {
      return true;
    }
    else
    {
      error = tr( "Missing parameter value %1" ).arg( parameter->description() );
      return false;
    }
  }

  if ( parameter->type() == QgsProcessingParameterVectorDestination::typeName() || parameter->type() == QgsProcessingParameterFeatureSink::typeName() )
  {
    if ( outputPath.startsWith( "memory:"_L1 ) )
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

    if ( providerKey != "ogr"_L1 )
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
    const QString extension = fi.suffix();
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
  else if ( parameter->type() == QgsProcessingParameterVectorTileDestination::typeName() )
  {
    const QFileInfo fi( outputPath );
    const QString extension = fi.completeSuffix();
    if ( !supportedOutputVectorTileLayerExtensions().contains( extension, Qt::CaseInsensitive ) )
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
    return hasGeometry ? u"shp"_s : u"dbf"_s;
  }
}

QString QgsProcessingProvider::defaultRasterFileFormat() const
{
  const QString userDefault = QgsProcessingUtils::defaultRasterFormat();

  const QList<QgsProcessingFormatExtensionPair> formatAndExtensions = supportedOutputRasterLayerFormatAndExtensions();
  for ( const QgsProcessingFormatExtensionPair &formatAndExt : std::as_const( formatAndExtensions ) )
  {
    if ( formatAndExt.format.compare( userDefault, Qt::CaseInsensitive ) == 0 )
    {
      // user set default is supported by provider, use that
      return userDefault;
    }
  }

  if ( !formatAndExtensions.empty() )
  {
    return formatAndExtensions.at( 0 ).format;
  }
  else
  {
    // who knows? provider says it has no file support at all...
    return u"GTiff"_s;
  }
}

QString QgsProcessingProvider::defaultRasterFileExtension() const
{
  QString format = defaultRasterFileFormat();
  QStringList extensions = QgsRasterFileWriter::extensionsForFormat( format );
  if ( !extensions.isEmpty() )
    return extensions[0];

  return u"tif"_s;
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
    return u"las"_s;
  }
}

QString QgsProcessingProvider::defaultVectorTileFileExtension() const
{
  const QString userDefault = QgsProcessingUtils::defaultVectorTileExtension();

  const QStringList supportedExtensions = supportedOutputVectorTileLayerExtensions();
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
    return u"mbtiles"_s;
  }
}

bool QgsProcessingProvider::supportsNonFileBasedOutput() const
{
  return true;
}
