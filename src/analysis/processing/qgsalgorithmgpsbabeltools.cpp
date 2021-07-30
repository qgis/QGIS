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

#include "qgsalgorithmgpsbabeltools.h"
#include "qgsvectorlayer.h"
#include "qgsrunprocess.h"
#include "qgsproviderutils.h"
#include "qgssettings.h"

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
  QStringList convertStrings;

  const QString inputPath = parameterAsString( parameters, QStringLiteral( "INPUT" ), context );
  const QString outputPath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );

  const ConversionType convertType = static_cast< ConversionType >( parameterAsEnum( parameters, QStringLiteral( "CONVERSION" ), context ) );

  // TODO -- fix the settings path when the rest of the GPS tools plugin is migrated
  QgsSettings settings;
  QString babelPath = settings.value( QStringLiteral( "Plugin-GPS/gpsbabelpath" ), QString() ).toString();
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
    QgsProcessingContext::LayerDetails details( layer->name(), context.project(), QStringLiteral( "OUTPUT_LAYER" ), QgsProcessingUtils::LayerHint::Vector );
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

///@endcond
