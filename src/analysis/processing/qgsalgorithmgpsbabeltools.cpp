/***************************************************************************
                         qgsalgorithmgpsbabeltools.cpp
                         ------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include <QtGlobal>
#if QT_CONFIG(process)


#include "qgsalgorithmgpsbabeltools.h"
#include "qgsvectorlayer.h"
#include "qgsrunprocess.h"
#include "qgsproviderutils.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgsbabelformatregistry.h"
#include "qgsbabelformat.h"
#include "qgsgpsdetector.h"
#include "qgsbabelgpsdevice.h"

///@cond PRIVATE

QString QgsConvertGpxFeatureTypeAlgorithm::name() const
{
  return QStringLiteral( "convertgpxfeaturetype" );
}

QString QgsConvertGpxFeatureTypeAlgorithm::displayName() const
{
  return QObject::tr( "Convert GPX feature type" );
}

QStringList QgsConvertGpxFeatureTypeAlgorithm::tags() const
{
  return QObject::tr( "gps,tools,babel,tracks,waypoints,routes" ).split( ',' );
}

QString QgsConvertGpxFeatureTypeAlgorithm::group() const
{
  return QObject::tr( "GPS" );
}

QString QgsConvertGpxFeatureTypeAlgorithm::groupId() const
{
  return QStringLiteral( "gps" );
}

void QgsConvertGpxFeatureTypeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "INPUT" ), QObject::tr( "Input file" ), QgsProcessingParameterFile::File, QString(), QVariant(), false,
                QObject::tr( "GPX files" ) + QStringLiteral( " (*.gpx *.GPX)" ) ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "CONVERSION" ), QObject::tr( "Conversion" ),
  {
    QObject::tr( "Waypoints from a Route" ),
    QObject::tr( "Waypoints from a Track" ),
    QObject::tr( "Route from Waypoints" ),
    QObject::tr( "Track from Waypoints" )
  }, false, 0 ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ), QObject::tr( "GPX files" ) + QStringLiteral( " (*.gpx *.GPX)" ) ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Output layer" ) ) );
}

QIcon QgsConvertGpxFeatureTypeAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsConvertGpxFeatureTypeAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsConvertGpxFeatureTypeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm uses the GPSBabel tool to convert GPX features from one type to another (e.g. converting all waypoint features to a route feature)." );
}

QgsConvertGpxFeatureTypeAlgorithm *QgsConvertGpxFeatureTypeAlgorithm::createInstance() const
{
  return new QgsConvertGpxFeatureTypeAlgorithm();
}


QVariantMap QgsConvertGpxFeatureTypeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString inputPath = parameterAsString( parameters, QStringLiteral( "INPUT" ), context );
  const QString outputPath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );

  const ConversionType convertType = static_cast< ConversionType >( parameterAsEnum( parameters, QStringLiteral( "CONVERSION" ), context ) );

  QString babelPath = QgsSettingsRegistryCore::settingsGpsBabelPath.value();
  if ( babelPath.isEmpty() )
    babelPath = QStringLiteral( "gpsbabel" );

  QStringList processArgs;
  QStringList logArgs;
  createArgumentLists( inputPath, outputPath, convertType, processArgs, logArgs );
  feedback->pushCommandInfo( QObject::tr( "Conversion command: " ) + babelPath + ' ' + logArgs.join( ' ' ) );

  QgsBlockingProcess babelProcess( babelPath, processArgs );
  babelProcess.setStdErrHandler( [ = ]( const QByteArray & ba )
  {
    feedback->reportError( ba );
  } );
  babelProcess.setStdOutHandler( [ = ]( const QByteArray & ba )
  {
    feedback->pushDebugInfo( ba );
  } );

  const int res = babelProcess.run( feedback );
  if ( feedback->isCanceled() && res != 0 )
  {
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) )  ;
  }
  else if ( !feedback->isCanceled() && babelProcess.exitStatus() == QProcess::CrashExit )
  {
    throw QgsProcessingException( QObject::tr( "Process was unexpectedly terminated" ) );
  }
  else if ( res == 0 )
  {
    feedback->pushInfo( QObject::tr( "Process completed successfully" ) );
  }
  else if ( babelProcess.processError() == QProcess::FailedToStart )
  {
    throw QgsProcessingException( QObject::tr( "Process %1 failed to start. Either %1 is missing, or you may have insufficient permissions to run the program." ).arg( babelPath ) );
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Process returned error code %1" ).arg( res ) );
  }

  std::unique_ptr< QgsVectorLayer > layer;
  const QString layerName = QgsProviderUtils::suggestLayerNameFromFilePath( outputPath );
  // add the layer
  switch ( convertType )
  {
    case QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromRoute:
    case QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromTrack:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=waypoint", layerName, QStringLiteral( "gpx" ) );
      break;
    case QgsConvertGpxFeatureTypeAlgorithm::RouteFromWaypoints:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=route", layerName, QStringLiteral( "gpx" ) );
      break;
    case QgsConvertGpxFeatureTypeAlgorithm::TrackFromWaypoints:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=track", layerName, QStringLiteral( "gpx" ) );
      break;
  }

  QVariantMap outputs;
  if ( !layer->isValid() )
  {
    feedback->reportError( QObject::tr( "Resulting file is not a valid GPX layer" ) );
  }
  else
  {
    const QString layerId = layer->id();
    outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), layerId );
    const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), QStringLiteral( "OUTPUT_LAYER" ), QgsProcessingUtils::LayerHint::Vector );
    context.addLayerToLoadOnCompletion( layerId, details );
    context.temporaryLayerStore()->addMapLayer( layer.release() );
  }

  outputs.insert( QStringLiteral( "OUTPUT" ), outputPath );
  return outputs;
}

void QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( const QString &inputPath, const QString &outputPath, ConversionType conversion, QStringList &processArgs, QStringList &logArgs )
{
  logArgs.reserve( 10 );
  processArgs.reserve( 10 );
  for ( const QString &arg : { QStringLiteral( "-i" ), QStringLiteral( "gpx" ), QStringLiteral( "-f" ) } )
  {
    logArgs << arg;
    processArgs << arg;
  }

  // when showing the babel command, wrap filenames in "", which is what QProcess does internally.
  logArgs << QStringLiteral( "\"%1\"" ).arg( inputPath );
  processArgs << inputPath;

  QStringList convertStrings;
  switch ( conversion )
  {
    case QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromRoute:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,wpt=rte,del" );
      break;
    case QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromTrack:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,wpt=trk,del" );
      break;
    case QgsConvertGpxFeatureTypeAlgorithm::RouteFromWaypoints:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,rte=wpt,del" );
      break;
    case QgsConvertGpxFeatureTypeAlgorithm::TrackFromWaypoints:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,trk=wpt,del" );
      break;
  }
  logArgs << convertStrings;
  processArgs << convertStrings;

  for ( const QString &arg : { QStringLiteral( "-o" ), QStringLiteral( "gpx" ), QStringLiteral( "-F" ) } )
  {
    logArgs << arg;
    processArgs << arg;
  }

  logArgs << QStringLiteral( "\"%1\"" ).arg( outputPath );
  processArgs << outputPath;

}


//
// QgsConvertGpsDataAlgorithm
//

QString QgsConvertGpsDataAlgorithm::name() const
{
  return QStringLiteral( "convertgpsdata" );
}

QString QgsConvertGpsDataAlgorithm::displayName() const
{
  return QObject::tr( "Convert GPS data" );
}

QStringList QgsConvertGpsDataAlgorithm::tags() const
{
  return QObject::tr( "gps,tools,babel,tracks,waypoints,routes,gpx,import,export" ).split( ',' );
}

QString QgsConvertGpsDataAlgorithm::group() const
{
  return QObject::tr( "GPS" );
}

QString QgsConvertGpsDataAlgorithm::groupId() const
{
  return QStringLiteral( "gps" );
}

void QgsConvertGpsDataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "INPUT" ), QObject::tr( "Input file" ), QgsProcessingParameterFile::File, QString(), QVariant(), false,
                QgsApplication::gpsBabelFormatRegistry()->importFileFilter() + QStringLiteral( ";;%1" ).arg( QObject::tr( "All files (*.*)" ) ) ) );

  std::unique_ptr< QgsProcessingParameterString > formatParam = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "FORMAT" ), QObject::tr( "Format" ) );

  QStringList formats;
  const QStringList formatNames = QgsApplication::gpsBabelFormatRegistry()->importFormatNames();
  for ( const QString &format : formatNames )
    formats << QgsApplication::gpsBabelFormatRegistry()->importFormat( format )->description();

  std::sort( formats.begin(), formats.end(), []( const QString & a, const QString & b )
  {
    return a.compare( b, Qt::CaseInsensitive ) < 0;
  } );

  formatParam->setMetadata( {{
      QStringLiteral( "widget_wrapper" ), QVariantMap(
      {{QStringLiteral( "value_hints" ), formats }}
      )
    }
  } );
  addParameter( formatParam.release() );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "FEATURE_TYPE" ), QObject::tr( "Feature type" ),
  {
    QObject::tr( "Waypoints" ),
    QObject::tr( "Routes" ),
    QObject::tr( "Tracks" )
  }, false, 0 ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ), QObject::tr( "GPX files" ) + QStringLiteral( " (*.gpx *.GPX)" ) ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Output layer" ) ) );
}

QIcon QgsConvertGpsDataAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsConvertGpsDataAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsConvertGpsDataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm uses the GPSBabel tool to convert a GPS data file from a range of formats to the GPX standard format." );
}

QgsConvertGpsDataAlgorithm *QgsConvertGpsDataAlgorithm::createInstance() const
{
  return new QgsConvertGpsDataAlgorithm();
}

QVariantMap QgsConvertGpsDataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString inputPath = parameterAsString( parameters, QStringLiteral( "INPUT" ), context );
  const QString outputPath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );

  const Qgis::GpsFeatureType featureType = static_cast< Qgis::GpsFeatureType >( parameterAsEnum( parameters, QStringLiteral( "FEATURE_TYPE" ), context ) );

  QString babelPath = QgsSettingsRegistryCore::settingsGpsBabelPath.value();
  if ( babelPath.isEmpty() )
    babelPath = QStringLiteral( "gpsbabel" );

  const QString formatName = parameterAsString( parameters, QStringLiteral( "FORMAT" ), context );
  const QgsBabelSimpleImportFormat *format = QgsApplication::gpsBabelFormatRegistry()->importFormat( formatName );
  if ( !format ) // second try, match using descriptions instead of names
    format =  QgsApplication::gpsBabelFormatRegistry()->importFormatByDescription( formatName );

  if ( !format )
  {
    throw QgsProcessingException( QObject::tr( "Unknown GPSBabel format “%1”. Valid formats are: %2" )
                                  .arg( formatName,
                                        QgsApplication::gpsBabelFormatRegistry()->importFormatNames().join( QLatin1String( ", " ) ) ) );
  }

  switch ( featureType )
  {
    case Qgis::GpsFeatureType::Waypoint:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Waypoints ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support converting waypoints." )
                                      .arg( formatName ) );
      }
      break;

    case Qgis::GpsFeatureType::Route:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Routes ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support converting routes." )
                                      .arg( formatName ) );
      }
      break;

    case Qgis::GpsFeatureType::Track:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Tracks ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support converting tracks." )
                                      .arg( formatName ) );
      }
      break;
  }

  // note that for the log we should quote file paths, but for the actual command we don't. That's
  // because QProcess does this internally for us, and double quoting causes issues
  const QStringList logCommand = format->importCommand( babelPath, featureType, inputPath, outputPath, Qgis::BabelCommandFlag::QuoteFilePaths );
  const QStringList processCommand = format->importCommand( babelPath, featureType, inputPath, outputPath );
  feedback->pushCommandInfo( QObject::tr( "Conversion command: " ) + logCommand.join( ' ' ) );

  QgsBlockingProcess babelProcess( processCommand.value( 0 ), processCommand.mid( 1 ) );
  babelProcess.setStdErrHandler( [ = ]( const QByteArray & ba )
  {
    feedback->reportError( ba );
  } );
  babelProcess.setStdOutHandler( [ = ]( const QByteArray & ba )
  {
    feedback->pushDebugInfo( ba );
  } );

  const int res = babelProcess.run( feedback );
  if ( feedback->isCanceled() && res != 0 )
  {
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) )  ;
  }
  else if ( !feedback->isCanceled() && babelProcess.exitStatus() == QProcess::CrashExit )
  {
    throw QgsProcessingException( QObject::tr( "Process was unexpectedly terminated" ) );
  }
  else if ( res == 0 )
  {
    feedback->pushInfo( QObject::tr( "Process completed successfully" ) );
  }
  else if ( babelProcess.processError() == QProcess::FailedToStart )
  {
    throw QgsProcessingException( QObject::tr( "Process %1 failed to start. Either %1 is missing, or you may have insufficient permissions to run the program." ).arg( babelPath ) );
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Process returned error code %1" ).arg( res ) );
  }

  std::unique_ptr< QgsVectorLayer > layer;
  const QString layerName = QgsProviderUtils::suggestLayerNameFromFilePath( outputPath );
  // add the layer
  switch ( featureType )
  {
    case Qgis::GpsFeatureType::Waypoint:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=waypoint", layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Route:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=route", layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Track:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=track", layerName, QStringLiteral( "gpx" ) );
      break;
  }

  QVariantMap outputs;
  if ( !layer->isValid() )
  {
    feedback->reportError( QObject::tr( "Resulting file is not a valid GPX layer" ) );
  }
  else
  {
    const QString layerId = layer->id();
    outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), layerId );
    const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), QStringLiteral( "OUTPUT_LAYER" ), QgsProcessingUtils::LayerHint::Vector );
    context.addLayerToLoadOnCompletion( layerId, details );
    context.temporaryLayerStore()->addMapLayer( layer.release() );
  }

  outputs.insert( QStringLiteral( "OUTPUT" ), outputPath );
  return outputs;
}

//
// QgsDownloadGpsDataAlgorithm
//

QString QgsDownloadGpsDataAlgorithm::name() const
{
  return QStringLiteral( "downloadgpsdata" );
}

QString QgsDownloadGpsDataAlgorithm::displayName() const
{
  return QObject::tr( "Download GPS data from device" );
}

QStringList QgsDownloadGpsDataAlgorithm::tags() const
{
  return QObject::tr( "gps,tools,babel,tracks,waypoints,routes,gpx,import,export,export,device,serial" ).split( ',' );
}

QString QgsDownloadGpsDataAlgorithm::group() const
{
  return QObject::tr( "GPS" );
}

QString QgsDownloadGpsDataAlgorithm::groupId() const
{
  return QStringLiteral( "gps" );
}

void QgsDownloadGpsDataAlgorithm::initAlgorithm( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterString > deviceParam = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "DEVICE" ), QObject::tr( "Device" ) );

  QStringList deviceNames = QgsApplication::gpsBabelFormatRegistry()->deviceNames();
  std::sort( deviceNames.begin(), deviceNames.end(), []( const QString & a, const QString & b )
  {
    return a.compare( b, Qt::CaseInsensitive ) < 0;
  } );

  deviceParam->setMetadata( {{
      QStringLiteral( "widget_wrapper" ), QVariantMap(
      {{QStringLiteral( "value_hints" ), deviceNames }}
      )
    }
  } );
  addParameter( deviceParam.release() );


  const QList< QPair<QString, QString> > devices = QgsGpsDetector::availablePorts() << QPair<QString, QString>( QStringLiteral( "usb:" ), QStringLiteral( "usb:" ) );
  std::unique_ptr< QgsProcessingParameterString > portParam = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "PORT" ), QObject::tr( "Port" ) );

  QStringList ports;
  for ( auto it = devices.constBegin(); it != devices.constEnd(); ++ it )
    ports << it->second;
  std::sort( ports.begin(), ports.end(), []( const QString & a, const QString & b )
  {
    return a.compare( b, Qt::CaseInsensitive ) < 0;
  } );

  portParam->setMetadata( {{
      QStringLiteral( "widget_wrapper" ), QVariantMap(
      {{QStringLiteral( "value_hints" ), ports }}
      )
    }
  } );
  addParameter( portParam.release() );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "FEATURE_TYPE" ), QObject::tr( "Feature type" ),
  {
    QObject::tr( "Waypoints" ),
    QObject::tr( "Routes" ),
    QObject::tr( "Tracks" )
  }, false, 0 ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ), QObject::tr( "GPX files" ) + QStringLiteral( " (*.gpx *.GPX)" ) ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Output layer" ) ) );
}

QIcon QgsDownloadGpsDataAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsDownloadGpsDataAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsDownloadGpsDataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm uses the GPSBabel tool to download data from a GPS device into the GPX standard format." );
}

QgsDownloadGpsDataAlgorithm *QgsDownloadGpsDataAlgorithm::createInstance() const
{
  return new QgsDownloadGpsDataAlgorithm();
}

QVariantMap QgsDownloadGpsDataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputPath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  const Qgis::GpsFeatureType featureType = static_cast< Qgis::GpsFeatureType >( parameterAsEnum( parameters, QStringLiteral( "FEATURE_TYPE" ), context ) );

  QString babelPath = QgsSettingsRegistryCore::settingsGpsBabelPath.value();
  if ( babelPath.isEmpty() )
    babelPath = QStringLiteral( "gpsbabel" );

  const QString deviceName = parameterAsString( parameters, QStringLiteral( "DEVICE" ), context );
  const QgsBabelGpsDeviceFormat *format = QgsApplication::gpsBabelFormatRegistry()->deviceFormat( deviceName );
  if ( !format )
  {
    throw QgsProcessingException( QObject::tr( "Unknown GPSBabel device “%1”. Valid devices are: %2" )
                                  .arg( deviceName,
                                        QgsApplication::gpsBabelFormatRegistry()->deviceNames().join( QLatin1String( ", " ) ) ) );
  }

  const QString portName = parameterAsString( parameters, QStringLiteral( "PORT" ), context );
  QString inputPort;
  const QList< QPair<QString, QString> > devices = QgsGpsDetector::availablePorts() << QPair<QString, QString>( QStringLiteral( "usb:" ), QStringLiteral( "usb:" ) );
  QStringList validPorts;
  for ( auto it = devices.constBegin(); it != devices.constEnd(); ++it )
  {
    if ( it->first.compare( portName, Qt::CaseInsensitive ) == 0 || it->second.compare( portName, Qt::CaseInsensitive ) == 0 )
    {
      inputPort = it->first;
    }
    validPorts << it->first;
  }
  if ( inputPort.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Unknown port “%1”. Valid ports are: %2" )
                                  .arg( portName,
                                        validPorts.join( QLatin1String( ", " ) ) ) );
  }

  switch ( featureType )
  {
    case Qgis::GpsFeatureType::Waypoint:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Waypoints ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support converting waypoints." )
                                      .arg( deviceName ) );
      }
      break;

    case Qgis::GpsFeatureType::Route:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Routes ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support converting routes." )
                                      .arg( deviceName ) );
      }
      break;

    case Qgis::GpsFeatureType::Track:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Tracks ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support converting tracks." )
                                      .arg( deviceName ) );
      }
      break;
  }

  // note that for the log we should quote file paths, but for the actual command we don't. That's
  // because QProcess does this internally for us, and double quoting causes issues
  const QStringList logCommand = format->importCommand( babelPath, featureType, inputPort, outputPath, Qgis::BabelCommandFlag::QuoteFilePaths );
  const QStringList processCommand = format->importCommand( babelPath, featureType, inputPort, outputPath );
  feedback->pushCommandInfo( QObject::tr( "Download command: " ) + logCommand.join( ' ' ) );

  QgsBlockingProcess babelProcess( processCommand.value( 0 ), processCommand.mid( 1 ) );
  babelProcess.setStdErrHandler( [ = ]( const QByteArray & ba )
  {
    feedback->reportError( ba );
  } );
  babelProcess.setStdOutHandler( [ = ]( const QByteArray & ba )
  {
    feedback->pushDebugInfo( ba );
  } );

  const int res = babelProcess.run( feedback );
  if ( feedback->isCanceled() && res != 0 )
  {
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) )  ;
  }
  else if ( !feedback->isCanceled() && babelProcess.exitStatus() == QProcess::CrashExit )
  {
    throw QgsProcessingException( QObject::tr( "Process was unexpectedly terminated" ) );
  }
  else if ( res == 0 )
  {
    feedback->pushInfo( QObject::tr( "Process completed successfully" ) );
  }
  else if ( babelProcess.processError() == QProcess::FailedToStart )
  {
    throw QgsProcessingException( QObject::tr( "Process %1 failed to start. Either %1 is missing, or you may have insufficient permissions to run the program." ).arg( babelPath ) );
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Process returned error code %1" ).arg( res ) );
  }

  std::unique_ptr< QgsVectorLayer > layer;
  const QString layerName = QgsProviderUtils::suggestLayerNameFromFilePath( outputPath );
  // add the layer
  switch ( featureType )
  {
    case Qgis::GpsFeatureType::Waypoint:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=waypoint", layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Route:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=route", layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Track:
      layer = std::make_unique< QgsVectorLayer >( outputPath + "?type=track", layerName, QStringLiteral( "gpx" ) );
      break;
  }

  QVariantMap outputs;
  if ( !layer->isValid() )
  {
    feedback->reportError( QObject::tr( "Resulting file is not a valid GPX layer" ) );
  }
  else
  {
    const QString layerId = layer->id();
    outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), layerId );
    const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), QStringLiteral( "OUTPUT_LAYER" ), QgsProcessingUtils::LayerHint::Vector );
    context.addLayerToLoadOnCompletion( layerId, details );
    context.temporaryLayerStore()->addMapLayer( layer.release() );
  }

  outputs.insert( QStringLiteral( "OUTPUT" ), outputPath );
  return outputs;
}


//
// QgsUploadGpsDataAlgorithm
//

QString QgsUploadGpsDataAlgorithm::name() const
{
  return QStringLiteral( "uploadgpsdata" );
}

QString QgsUploadGpsDataAlgorithm::displayName() const
{
  return QObject::tr( "Upload GPS data to device" );
}

QStringList QgsUploadGpsDataAlgorithm::tags() const
{
  return QObject::tr( "gps,tools,babel,tracks,waypoints,routes,gpx,import,export,export,device,serial" ).split( ',' );
}

QString QgsUploadGpsDataAlgorithm::group() const
{
  return QObject::tr( "GPS" );
}

QString QgsUploadGpsDataAlgorithm::groupId() const
{
  return QStringLiteral( "gps" );
}

void QgsUploadGpsDataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "INPUT" ), QObject::tr( "Input file" ), QgsProcessingParameterFile::File, QString(), QVariant(), false,
                QObject::tr( "GPX files" ) + QStringLiteral( " (*.gpx *.GPX)" ) ) );

  std::unique_ptr< QgsProcessingParameterString > deviceParam = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "DEVICE" ), QObject::tr( "Device" ) );

  QStringList deviceNames = QgsApplication::gpsBabelFormatRegistry()->deviceNames();
  std::sort( deviceNames.begin(), deviceNames.end(), []( const QString & a, const QString & b )
  {
    return a.compare( b, Qt::CaseInsensitive ) < 0;
  } );

  deviceParam->setMetadata( {{
      QStringLiteral( "widget_wrapper" ), QVariantMap(
      {{QStringLiteral( "value_hints" ), deviceNames }}
      )
    }
  } );
  addParameter( deviceParam.release() );

  const QList< QPair<QString, QString> > devices = QgsGpsDetector::availablePorts() << QPair<QString, QString>( QStringLiteral( "usb:" ), QStringLiteral( "usb:" ) );
  std::unique_ptr< QgsProcessingParameterString > portParam = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "PORT" ), QObject::tr( "Port" ) );

  QStringList ports;
  for ( auto it = devices.constBegin(); it != devices.constEnd(); ++ it )
    ports << it->second;
  std::sort( ports.begin(), ports.end(), []( const QString & a, const QString & b )
  {
    return a.compare( b, Qt::CaseInsensitive ) < 0;
  } );

  portParam->setMetadata( {{
      QStringLiteral( "widget_wrapper" ), QVariantMap(
      {{QStringLiteral( "value_hints" ), ports }}
      )
    }
  } );
  addParameter( portParam.release() );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "FEATURE_TYPE" ), QObject::tr( "Feature type" ),
  {
    QObject::tr( "Waypoints" ),
    QObject::tr( "Routes" ),
    QObject::tr( "Tracks" )
  }, false, 0 ) );

}

QIcon QgsUploadGpsDataAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsUploadGpsDataAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/mIconGps.svg" ) );
}

QString QgsUploadGpsDataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm uses the GPSBabel tool to upload data to a GPS device from the GPX standard format." );
}

QgsUploadGpsDataAlgorithm *QgsUploadGpsDataAlgorithm::createInstance() const
{
  return new QgsUploadGpsDataAlgorithm();
}

QVariantMap QgsUploadGpsDataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString inputPath = parameterAsString( parameters, QStringLiteral( "INPUT" ), context );
  const Qgis::GpsFeatureType featureType = static_cast< Qgis::GpsFeatureType >( parameterAsEnum( parameters, QStringLiteral( "FEATURE_TYPE" ), context ) );

  QString babelPath = QgsSettingsRegistryCore::settingsGpsBabelPath.value();
  if ( babelPath.isEmpty() )
    babelPath = QStringLiteral( "gpsbabel" );

  const QString deviceName = parameterAsString( parameters, QStringLiteral( "DEVICE" ), context );
  const QgsBabelGpsDeviceFormat *format = QgsApplication::gpsBabelFormatRegistry()->deviceFormat( deviceName );
  if ( !format )
  {
    throw QgsProcessingException( QObject::tr( "Unknown GPSBabel device “%1”. Valid devices are: %2" )
                                  .arg( deviceName,
                                        QgsApplication::gpsBabelFormatRegistry()->deviceNames().join( QLatin1String( ", " ) ) ) );
  }

  const QString portName = parameterAsString( parameters, QStringLiteral( "PORT" ), context );
  QString outputPort;
  const QList< QPair<QString, QString> > devices = QgsGpsDetector::availablePorts() << QPair<QString, QString>( QStringLiteral( "usb:" ), QStringLiteral( "usb:" ) );
  QStringList validPorts;
  for ( auto it = devices.constBegin(); it != devices.constEnd(); ++it )
  {
    if ( it->first.compare( portName, Qt::CaseInsensitive ) == 0 || it->second.compare( portName, Qt::CaseInsensitive ) == 0 )
    {
      outputPort = it->first;
    }
    validPorts << it->first;
  }
  if ( outputPort.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Unknown port “%1”. Valid ports are: %2" )
                                  .arg( portName,
                                        validPorts.join( QLatin1String( ", " ) ) ) );
  }


  switch ( featureType )
  {
    case Qgis::GpsFeatureType::Waypoint:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Waypoints ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support waypoints." )
                                      .arg( deviceName ) );
      }
      break;

    case Qgis::GpsFeatureType::Route:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Routes ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support routes." )
                                      .arg( deviceName ) );
      }
      break;

    case Qgis::GpsFeatureType::Track:
      if ( !( format->capabilities() & Qgis::BabelFormatCapability::Tracks ) )
      {
        throw QgsProcessingException( QObject::tr( "The GPSBabel format “%1” does not support tracks." )
                                      .arg( deviceName ) );
      }
      break;
  }

  // note that for the log we should quote file paths, but for the actual command we don't. That's
  // because QProcess does this internally for us, and double quoting causes issues
  const QStringList logCommand = format->exportCommand( babelPath, featureType, inputPath, outputPort, Qgis::BabelCommandFlag::QuoteFilePaths );
  const QStringList processCommand = format->exportCommand( babelPath, featureType, inputPath, outputPort );
  feedback->pushCommandInfo( QObject::tr( "Upload command: " ) + logCommand.join( ' ' ) );

  QgsBlockingProcess babelProcess( processCommand.value( 0 ), processCommand.mid( 1 ) );
  babelProcess.setStdErrHandler( [ = ]( const QByteArray & ba )
  {
    feedback->reportError( ba );
  } );
  babelProcess.setStdOutHandler( [ = ]( const QByteArray & ba )
  {
    feedback->pushDebugInfo( ba );
  } );

  const int res = babelProcess.run( feedback );
  if ( feedback->isCanceled() && res != 0 )
  {
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) )  ;
  }
  else if ( !feedback->isCanceled() && babelProcess.exitStatus() == QProcess::CrashExit )
  {
    throw QgsProcessingException( QObject::tr( "Process was unexpectedly terminated" ) );
  }
  else if ( res == 0 )
  {
    feedback->pushInfo( QObject::tr( "Process completed successfully" ) );
  }
  else if ( babelProcess.processError() == QProcess::FailedToStart )
  {
    throw QgsProcessingException( QObject::tr( "Process %1 failed to start. Either %1 is missing, or you may have insufficient permissions to run the program." ).arg( babelPath ) );
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Process returned error code %1" ).arg( res ) );
  }

  return {};
}

///@endcond
#endif // process
