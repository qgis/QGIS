/***************************************************************************
                         qgsalgorithmpdalinformation.cpp
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

#include "qgsalgorithmpdalinformation.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalInformationAlgorithm::name() const
{
  return QStringLiteral( "info" );
}

QString QgsPdalInformationAlgorithm::displayName() const
{
  return QObject::tr( "Information" );
}

QString QgsPdalInformationAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalInformationAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalInformationAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,las,laz,information" ).split( ',' );
}

QString QgsPdalInformationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm outputs basic metadata from the point cloud file." );
}

QgsPdalInformationAlgorithm *QgsPdalInformationAlgorithm::createInstance() const
{
  return new QgsPdalInformationAlgorithm();
}

void QgsPdalInformationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer information" ), QObject::tr( "HTML files (*.html)" ) ) );
}

QVariantMap QgsPdalInformationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QStringList processArgs = createArgumentLists( parameters, context, feedback );
  const QString wrenchPath = wrenchExecutableBinary();

  feedback->pushCommandInfo( QObject::tr( "wrench command: " ) + wrenchPath + ' ' + processArgs.join( ' ' ) );

  QStringList commandOutput;

  QgsBlockingProcess wrenchProcess( wrenchPath, processArgs );
  wrenchProcess.setStdErrHandler( [=]( const QByteArray &ba ) {
    feedback->reportError( ba.trimmed() );
  } );
  wrenchProcess.setStdOutHandler( [=, &commandOutput]( const QByteArray &ba ) {
    feedback->pushConsoleInfo( ba.trimmed() );
    commandOutput << ba;
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

  QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream out( &file );
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
      out.setCodec( "UTF-8" );
#endif
      out << QStringLiteral( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      out << QStringLiteral( "<pre>%1</pre>" ).arg( commandOutput.join( "" ) );
      out << QStringLiteral( "\n</body></html>" );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );

  return outputs;
}

QStringList QgsPdalInformationAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  return { QStringLiteral( "info" ), QStringLiteral( "--input=%1" ).arg( layer->source() ) };
}

///@endcond
