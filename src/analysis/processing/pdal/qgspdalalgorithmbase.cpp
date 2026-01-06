/***************************************************************************
                         qgspdalalgorithmbase.cpp
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

#include "qgspdalalgorithmbase.h"

#include "qgsapplication.h"
#include "qgscopcprovider.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsrunprocess.h"

#include <QRegularExpression>

///@cond PRIVATE

//
// QgsPdalAlgorithmBase
//

void QgsPdalAlgorithmBase::setOutputValue( const QString &name, const QVariant &value )
{
  mOutputValues.insert( name, value );
}

void QgsPdalAlgorithmBase::enableElevationPropertiesPostProcessor( bool enable )
{
  mEnableElevationProperties = enable;
}

QString QgsPdalAlgorithmBase::wrenchExecutableBinary() const
{
  QString wrenchExecutable = QProcessEnvironment::systemEnvironment().value( u"QGIS_WRENCH_EXECUTABLE"_s );
  if ( wrenchExecutable.isEmpty() )
  {
#if defined( Q_OS_WIN )
    wrenchExecutable = QgsApplication::libexecPath() + "pdal_wrench.exe";
#else
    wrenchExecutable = QgsApplication::libexecPath() + "pdal_wrench";
#endif
  }
  return QString( wrenchExecutable );
}

void QgsPdalAlgorithmBase::createCommonParameters()
{
  auto filterParam = std::make_unique<QgsProcessingParameterExpression>( u"FILTER_EXPRESSION"_s, QObject::tr( "Filter expression" ), QVariant(), u"INPUT"_s, true, Qgis::ExpressionType::PointCloud );
  filterParam->setFlags( filterParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( filterParam.release() );

  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"FILTER_EXTENT"_s, QObject::tr( "Cropping extent" ), QVariant(), true );
  extentParam->setFlags( extentParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( extentParam.release() );
}

void QgsPdalAlgorithmBase::applyCommonParameters( QStringList &arguments, QgsCoordinateReferenceSystem crs, const QVariantMap &parameters, QgsProcessingContext &context )
{
  const QString filterExpression = parameterAsString( parameters, u"FILTER_EXPRESSION"_s, context ).trimmed();
  if ( !filterExpression.isEmpty() )
  {
    QgsPointCloudExpression exp( filterExpression );
    arguments << u"--filter=%1"_s.arg( exp.asPdalExpression() );
  }

  if ( parameters.value( u"FILTER_EXTENT"_s ).isValid() )
  {
    if ( crs.isValid() )
    {
      const QgsRectangle extent = parameterAsExtent( parameters, u"FILTER_EXTENT"_s, context, crs );
      arguments << u"--bounds=([%1, %2], [%3, %4])"_s
                     .arg( extent.xMinimum() )
                     .arg( extent.xMaximum() )
                     .arg( extent.yMinimum() )
                     .arg( extent.yMaximum() );
    }
    else
    {
      const QgsRectangle extent = parameterAsExtent( parameters, u"FILTER_EXTENT"_s, context );
      arguments << u"--bounds=([%1, %2], [%3, %4])"_s
                     .arg( extent.xMinimum() )
                     .arg( extent.xMaximum() )
                     .arg( extent.yMinimum() )
                     .arg( extent.yMaximum() );
    }
  }
}

void QgsPdalAlgorithmBase::applyThreadsParameter( QStringList &arguments, QgsProcessingContext &context )
{
  const int numThreads = context.maximumThreads();

  if ( numThreads )
  {
    arguments << u"--threads=%1"_s.arg( numThreads );
  }
}

QString QgsPdalAlgorithmBase::fixOutputFileName( const QString &inputFileName, const QString &outputFileName, QgsProcessingContext &context )
{
  bool inputIsVpc = inputFileName.endsWith( u".vpc"_s, Qt::CaseInsensitive );
  bool isTempOutput = outputFileName.startsWith( QgsProcessingUtils::tempFolder(), Qt::CaseInsensitive );
  if ( inputIsVpc && isTempOutput )
  {
    QFileInfo fi( outputFileName );
    QString newFileName = fi.path() + '/' + fi.completeBaseName() + u".vpc"_s;

    if ( context.willLoadLayerOnCompletion( outputFileName ) )
    {
      QMap<QString, QgsProcessingContext::LayerDetails> layersToLoad = context.layersToLoadOnCompletion();
      QgsProcessingContext::LayerDetails details = layersToLoad.take( outputFileName );
      layersToLoad[newFileName] = details;
      context.setLayersToLoadOnCompletion( layersToLoad );
    }
    return newFileName;
  }
  return outputFileName;
}

void QgsPdalAlgorithmBase::checkOutputFormat( const QString &inputFileName, const QString &outputFileName )
{
  bool inputIsVpc = inputFileName.endsWith( u".vpc"_s, Qt::CaseInsensitive );
  bool outputIsVpc = outputFileName.endsWith( u".vpc"_s, Qt::CaseInsensitive );
  if ( !inputIsVpc && outputIsVpc )
    throw QgsProcessingException(
      QObject::tr( "This algorithm does not support output to VPC if input is not a VPC. Please use LAS or LAZ as the output format. "
                   "To create a VPC please use \"Build virtual point cloud (VPC)\" algorithm." )
    );
}

QStringList QgsPdalAlgorithmBase::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );
  Q_UNUSED( feedback );
  return QStringList();
}

class EnableElevationPropertiesPostProcessor : public QgsProcessingLayerPostProcessorInterface
{
  public:
    void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      if ( QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( layer ) )
      {
        QgsRasterLayerElevationProperties *props = qgis::down_cast<QgsRasterLayerElevationProperties *>( rl->elevationProperties() );
        props->setMode( Qgis::RasterElevationMode::RepresentsElevationSurface );
        props->setEnabled( true );
        rl->trigger3DUpdate();
      }
    }
};

QVariantMap QgsPdalAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QStringList processArgs = createArgumentLists( parameters, context, feedback );
  const QString wrenchPath = wrenchExecutableBinary();

  if ( !QFileInfo::exists( wrenchPath ) )
  {
    throw QgsProcessingException( QObject::tr( "wrench executable is not found. Either use QGIS build with PDAL support or provide correct path via QGIS_WRENCH_EXECUTABLE environment variable." ) );
  }

  QStringList logArgs;
  const thread_local QRegularExpression re( "[\\s\\\"\\'\\(\\)\\&;]" );
  for ( const QString &arg : processArgs )
  {
    if ( arg.contains( re ) )
    {
      logArgs << u"\"%1\""_s.arg( arg );
    }
    else
    {
      logArgs << arg;
    }
  }

  feedback->pushCommandInfo( QObject::tr( "wrench command: " ) + wrenchPath + ' ' + logArgs.join( ' ' ) );

  double progress = 0;
  QString buffer;

  QgsBlockingProcess wrenchProcess( wrenchPath, processArgs );
  wrenchProcess.setStdErrHandler( [feedback]( const QByteArray &ba ) {
    feedback->reportError( ba.trimmed() );
  } );
  wrenchProcess.setStdOutHandler( [feedback, &progress, &buffer]( const QByteArray &ba ) {
    QString data( ba );

    QRegularExpression re( "\\.*(\\d+)?\\.*$" );
    QRegularExpressionMatch match = re.match( data );
    if ( match.hasMatch() )
    {
      QString value = match.captured( 1 );
      if ( !value.isEmpty() )
      {
        progress = value.toInt();
        if ( progress != 100 )
        {
          int pos = match.capturedEnd();
          QString points = data.mid( pos );
          progress += 2.5 * points.size();
        }
      }
      else
      {
        progress += 2.5 * data.size();
      }

      feedback->setProgress( progress );
    }

    buffer.append( data );
    if ( buffer.contains( '\n' ) || buffer.contains( '\r' ) )
    {
      QStringList parts = buffer.split( '\n' );
      if ( parts.size() == 0 )
      {
        parts = buffer.split( '\r' );
      }

      for ( int i = 0; i < parts.size() - 1; i++ )
        feedback->pushConsoleInfo( parts.at( i ) );
      buffer = parts.at( parts.size() - 1 );
    }
  } );

  const int res = wrenchProcess.run( feedback );
  if ( feedback->isCanceled() && res != 0 )
  {
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) );
  }
  else if ( !feedback->isCanceled() && wrenchProcess.exitStatus() == QProcess::CrashExit )
  {
    throw QgsProcessingException( QObject::tr( "Process was unexpectedly terminated" ) );
  }
  else if ( res == 0 )
  {
    feedback->pushInfo( QObject::tr( "Process completed successfully" ) );
  }
  else if ( wrenchProcess.processError() == QProcess::FailedToStart )
  {
    throw QgsProcessingException( QObject::tr( "Process %1 failed to start. Either %1 is missing, or you may have insufficient permissions to run the program." ).arg( wrenchPath ) );
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Process returned error code %1" ).arg( res ) );
  }

  QVariantMap outputs;
  QgsProcessingOutputDefinitions outDefinitions = outputDefinitions();
  for ( const QgsProcessingOutputDefinition *output : outDefinitions )
  {
    QString outputName = output->name();
    if ( parameters.contains( outputName ) )
    {
      outputs.insert( outputName, parameters.value( outputName ) );
    }
  }

  QMap<QString, QVariant>::const_iterator it = mOutputValues.constBegin();
  while ( it != mOutputValues.constEnd() )
  {
    outputs[it.key()] = it.value();

    const QString outputLayerName = it.value().toString();
    if ( context.willLoadLayerOnCompletion( outputLayerName ) && mEnableElevationProperties )
    {
      context.layerToLoadOnCompletionDetails( outputLayerName ).setPostProcessor( new EnableElevationPropertiesPostProcessor() );
    }
    ++it;
  }

  return outputs;
}

QgsPointCloudLayer *QgsPdalAlgorithmBase::parameterAsPointCloudLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, QgsProcessing::LayerOptionsFlags flags ) const
{
  QgsPointCloudLayer *layer = QgsProcessingParameters::parameterAsPointCloudLayer( parameterDefinition( name ), parameters, context, flags );

  // if layer or its data provider are empty return nullptr
  if ( !layer || !layer->dataProvider() )
    return nullptr;

  // if COPC provider, return as it is
  if ( layer->dataProvider()->name() == "copc"_L1 )
  {
    return layer;
  }

  // if source is remote file, use it as it is
  const QUrl url = QUrl( layer->source() );
  if ( url.isValid() && ( url.scheme() == "http" || url.scheme() == "https" ) )
  {
    return layer;
  }

  // for local files try to find COPC index file
  const QString copcFileName = QgsPdalAlgorithmBase::copcIndexFile( layer->source() );

  if ( QFileInfo::exists( copcFileName ) )
  {
    QgsPointCloudLayer *copcLayer = new QgsPointCloudLayer( copcFileName, layer->name(), "copc" );
    if ( copcLayer && copcLayer->isValid() )
      return copcLayer;
  }

  return layer;
}

QString QgsPdalAlgorithmBase::copcIndexFile( const QString &filename )
{
  const QFileInfo fi( filename );
  const QDir directory = fi.absoluteDir();
  const QString outputFile = u"%1/%2.copc.laz"_s.arg( directory.absolutePath() ).arg( fi.completeBaseName() );
  return outputFile;
}

///@endcond
