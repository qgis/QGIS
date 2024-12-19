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
#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudexpression.h"
#include "qgsrasterlayerelevationproperties.h"

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
  QString wrenchExecutable = QProcessEnvironment::systemEnvironment().value( QStringLiteral( "QGIS_WRENCH_EXECUTABLE" ) );
  if ( wrenchExecutable.isEmpty() )
  {
#if defined(Q_OS_WIN)
    wrenchExecutable = QgsApplication::libexecPath() + "pdal_wrench.exe";
#else
    wrenchExecutable = QgsApplication::libexecPath() + "pdal_wrench";
#endif
  }
  return QString( wrenchExecutable );
}

void QgsPdalAlgorithmBase::createCommonParameters()
{
  std::unique_ptr< QgsProcessingParameterExpression > filterParam = std::make_unique< QgsProcessingParameterExpression >( QStringLiteral( "FILTER_EXPRESSION" ), QObject::tr( "Filter expression" ), QVariant(), QStringLiteral( "INPUT" ), true, Qgis::ExpressionType::PointCloud );
  filterParam->setFlags( filterParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( filterParam.release() );

  std::unique_ptr< QgsProcessingParameterExtent > extentParam = std::make_unique< QgsProcessingParameterExtent >( QStringLiteral( "FILTER_EXTENT" ), QObject::tr( "Cropping extent" ), QVariant(), true );
  extentParam->setFlags( extentParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( extentParam.release() );
}

void QgsPdalAlgorithmBase::applyCommonParameters( QStringList &arguments, QgsCoordinateReferenceSystem crs, const QVariantMap &parameters, QgsProcessingContext &context )
{
  const QString filterExpression = parameterAsString( parameters, QStringLiteral( "FILTER_EXPRESSION" ), context ).trimmed();
  if ( !filterExpression.isEmpty() )
  {
    QgsPointCloudExpression exp( filterExpression );
    arguments << QStringLiteral( "--filter=%1" ).arg( exp.asPdalExpression() );
  }

  if ( parameters.value( QStringLiteral( "FILTER_EXTENT" ) ).isValid() )
  {
    if ( crs.isValid() )
    {
      const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "FILTER_EXTENT" ), context, crs );
      arguments << QStringLiteral( "--bounds=([%1, %2], [%3, %4])" )
                .arg( extent.xMinimum() )
                .arg( extent.xMaximum() )
                .arg( extent.yMinimum() )
                .arg( extent.yMaximum() );

    }
    else
    {
      const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "FILTER_EXTENT" ), context );
      arguments << QStringLiteral( "--bounds=([%1, %2], [%3, %4])" )
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
    arguments << QStringLiteral( "--threads=%1" ).arg( numThreads );
  }
}

QString QgsPdalAlgorithmBase::fixOutputFileName( const QString &inputFileName, const QString &outputFileName, QgsProcessingContext &context )
{
  bool inputIsVpc = inputFileName.endsWith( QStringLiteral( ".vpc" ), Qt::CaseInsensitive );
  bool isTempOutput = outputFileName.startsWith( QgsProcessingUtils::tempFolder(), Qt::CaseInsensitive );
  if ( inputIsVpc && isTempOutput )
  {
    QFileInfo fi( outputFileName );
    QString newFileName = fi.path() + '/' + fi.completeBaseName() + QStringLiteral( ".vpc" );

    if ( context.willLoadLayerOnCompletion( outputFileName ) )
    {
      QMap< QString, QgsProcessingContext::LayerDetails > layersToLoad = context.layersToLoadOnCompletion();
      QgsProcessingContext::LayerDetails details = layersToLoad.take( outputFileName );
      layersToLoad[ newFileName ] = details;
      context.setLayersToLoadOnCompletion( layersToLoad );
    }
    return newFileName;
  }
  return outputFileName;
}

void QgsPdalAlgorithmBase::checkOutputFormat( const QString &inputFileName, const QString &outputFileName )
{
  if ( outputFileName.endsWith( QStringLiteral( ".copc.laz" ), Qt::CaseInsensitive ) )
    throw QgsProcessingException(
      QObject::tr( "This algorithm does not support output to COPC. Please use LAS or LAZ as the output format. "
                   "LAS/LAZ files get automatically converted to COPC when loaded in QGIS, alternatively you can use "
                   "\"Create COPC\" algorithm." ) );

  bool inputIsVpc = inputFileName.endsWith( QStringLiteral( ".vpc" ), Qt::CaseInsensitive );
  bool outputIsVpc = outputFileName.endsWith( QStringLiteral( ".vpc" ), Qt::CaseInsensitive );
  if ( !inputIsVpc && outputIsVpc )
    throw QgsProcessingException(
      QObject::tr( "This algorithm does not support output to VPC if input is not a VPC. Please use LAS or LAZ as the output format. "
                   "To create a VPC please use \"Build virtual point cloud (VPC)\" algorithm." ) );
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
      if ( QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer ) )
      {
        QgsRasterLayerElevationProperties *props = qgis::down_cast< QgsRasterLayerElevationProperties * >( rl->elevationProperties() );
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

  QStringList logArgs;
  const thread_local QRegularExpression re( "[\\s\\\"\\'\\(\\)\\&;]" );
  for ( const QString &arg : processArgs )
  {
    if ( arg.contains( re ) )
    {
      logArgs << QStringLiteral( "\"%1\"" ).arg( arg );
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
  wrenchProcess.setStdErrHandler( [ = ]( const QByteArray & ba )
  {
    feedback->reportError( ba.trimmed() );
  } );
  wrenchProcess.setStdOutHandler( [ =, &progress, &buffer ]( const QByteArray & ba )
  {
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
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) )  ;
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
    outputs[ it.key() ] = it.value();

    const QString outputLayerName = it.value().toString();
    if ( context.willLoadLayerOnCompletion( outputLayerName ) && mEnableElevationProperties )
    {
      context.layerToLoadOnCompletionDetails( outputLayerName ).setPostProcessor( new EnableElevationPropertiesPostProcessor() );
    }
    ++it;
  }

  return outputs;
}

///@endcond
