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

QString QgsProcessingProvider::helpId() const
{
  return QString();
}

QString QgsProcessingProvider::longName() const
{
  return name();
}

QStringList QgsProcessingProvider::supportedOutputRasterLayerExtensions() const
{
  return QgsRasterFileWriter::supportedFormatExtensions();
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

QString QgsProcessingProvider::defaultVectorFileExtension( bool hasGeometry ) const
{
  QgsSettings settings;
  const QString defaultExtension = hasGeometry ? QStringLiteral( "shp" ) : QStringLiteral( "dbf" );
  const QString userDefault = settings.value( QStringLiteral( "Processing/DefaultOutputVectorLayerExt" ), defaultExtension, QgsSettings::Core ).toString();

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
    return defaultExtension;
  }
}

QString QgsProcessingProvider::defaultRasterFileExtension() const
{
  QgsSettings settings;
  const QString defaultExtension = QStringLiteral( "tif" );
  const QString userDefault = settings.value( QStringLiteral( "Processing/DefaultOutputRasterLayerExt" ), defaultExtension, QgsSettings::Core ).toString();

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
    return defaultExtension;
  }
}

bool QgsProcessingProvider::supportsNonFileBasedOutput() const
{
  return true;
}
