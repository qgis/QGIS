/***************************************************************************
                         qgsalgorithmhttprequest.cpp
                         ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Dave Signer
    email                : david at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmhttprequest.h"
#include "qgsprocessingparameters.h"
#include "qgis.h"

#include <QNetworkAccessManager>
#include "qgsnetworkaccessmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QMimeDatabase>
#include <QEventLoop>
#include <QTimer>

///@cond PRIVATE

QString QgsHttpRequestAlgorithm::name() const
{
  return QStringLiteral( "httprequest" );
}

QString QgsHttpRequestAlgorithm::displayName() const
{
  return tr( "HTTP(S) POST/GET request" );
}

QString QgsHttpRequestAlgorithm::shortDescription() const
{
  return tr( "Performs a HTTP(S) POST/GET request and returns the result code, error message and the data" );
}

QStringList QgsHttpRequestAlgorithm::tags() const
{
  return tr( "open,url,internet,url,fetch,get,post,request,https,http,download" ).split( ',' );
}

QString QgsHttpRequestAlgorithm::group() const
{
  return tr( "File tools" );
}

QString QgsHttpRequestAlgorithm::groupId() const
{
  return QStringLiteral( "filetools" );
}

QString QgsHttpRequestAlgorithm::shortHelpString() const
{
  return tr( "This algorithm performs a HTTP(S) POST/GET request and returns the HTTP status code and the reply data.\n"
             "If an error occurs then the error code and the message will be returned.\n\n"
             "Optionally, the result can be written to a file on disk.\n\n"
             "By default the algorithm will warn on errors. Optionally, the algorithm can be set to treat HTTP errors as failures." );
}

QgsHttpRequestAlgorithm *QgsHttpRequestAlgorithm::createInstance() const
{
  return new QgsHttpRequestAlgorithm();
}

void QgsHttpRequestAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "URL" ), tr( "URL" ), QVariant(), false, false ) );

  std::unique_ptr< QgsProcessingParameterEnum > methodParam = std::make_unique < QgsProcessingParameterEnum > (
        QStringLiteral( "METHOD" ),
        QObject::tr( "Method" ),
        QStringList()
        << QObject::tr( "GET" )
        << QObject::tr( "POST" ),
        false,
        0
      );
  methodParam->setHelp( QObject::tr( "The HTTP method to use for the request" ) );
  addParameter( methodParam.release() );

  std::unique_ptr< QgsProcessingParameterString > dataParam = std::make_unique < QgsProcessingParameterString >(
        QStringLiteral( "DATA" ), tr( "Data" ), QVariant(), false, true );
  dataParam->setHelp( QObject::tr( "The data to add in the body if the request is a POST" ) );
  addParameter( dataParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > failureParam = std::make_unique < QgsProcessingParameterBoolean >(
        QStringLiteral( "FAIL_ON_ERROR" ), tr( "Consider HTTP errors as failures" ), false );
  failureParam->setHelp( tr( "If set, the algorithm will fail on encountering a HTTP error" ) );
  addParameter( failureParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > writeToFileParam = std::make_unique < QgsProcessingParameterBoolean >(
        QStringLiteral( "WRITE_TO_FILE" ), tr( "Write reply data to temp file" ), false );
  writeToFileParam->setHelp( tr( "Write the reply data to a temporary binary file instead of passing back in a string" ) );
  addParameter( writeToFileParam.release() );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "STATUS_CODE" ), QObject::tr( "HTTP status code" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "ERROR_CODE" ), QObject::tr( "Network error code" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "ERROR_MESSAGE" ), QObject::tr( "Network error message" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "RESULT_DATA" ), QObject::tr( "Reply data or path to temporary file keeping the reply data" ) ) );
}

QVariantMap QgsHttpRequestAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString url = parameterAsString( parameters, QStringLiteral( "URL" ), context );
  if ( url.isEmpty() )
    throw QgsProcessingException( tr( "No URL specified" ) );
  const Qgis::HttpMethod httpMethod = static_cast< Qgis::HttpMethod>( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  const QString data = parameterAsString( parameters, QStringLiteral( "DATA" ), context );
  const bool failOnError  = parameterAsBool( parameters, QStringLiteral( "FAIL_ON_ERROR" ), context );
  const bool writeToFile = parameterAsBool( parameters, QStringLiteral( "WRITE_TO_FILE" ), context );
  const QUrl qurl = QUrl::fromUserInput( url );

  // Make Request
  const QNetworkRequest request( qurl );
  QNetworkReply *reply = nullptr;

  QEventLoop loop;
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  switch ( httpMethod )
  {
    case Qgis::HttpMethod::Get:
    {
      reply = nam->get( request );
      break;
    }
    case Qgis::HttpMethod::Post:
    {
      reply = nam->post( request, data.toUtf8() );
      break;
    }
  }

  connect( reply, &QNetworkReply::finished, this, [&loop]() { loop.exit(); }, Qt::UniqueConnection );
  connect( reply, &QNetworkReply::readyRead, this, [&loop]() { loop.exit(); }, Qt::UniqueConnection );
  connect( nam, qOverload< QNetworkReply *>( &QgsNetworkAccessManager::requestTimedOut ), this, [&loop]() { loop.exit(); }, Qt::UniqueConnection );

  loop.exec();

  // Handle reply

  const int statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const int errorCode = reply->error();
  QString errorMessage = QString();
  QString resultData = QString();

  if ( errorCode == QNetworkReply::NoError )
  {
    feedback->pushInfo( tr( "Request succeeded with code %1" ).arg( statusCode ) );

    if ( writeToFile )
    {
      QTemporaryDir tempDir;
      tempDir.setAutoRemove( false );
      tempDir.path();
      resultData = tempDir.path() + QDir::separator() + QStringLiteral( "data.bin" );
      QFile tempFile( resultData );
      tempFile.open( QIODevice::WriteOnly );
      tempFile.write( reply->readAll() );
      tempFile.close();

      feedback->pushInfo( tr( "Result data written to %1" ).arg( resultData ) );
    }
    else
    {
      resultData = reply->readAll();
    }
  }
  else
  {
    feedback->pushInfo( tr( "Request failed with code %1" ).arg( statusCode ) );
    errorMessage = reply->errorString();
    if ( failOnError )
    {
      throw QgsProcessingException( reply->errorString() );
    }
    feedback->pushWarning( reply->errorString() );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "STATUS_CODE" ), statusCode );
  outputs.insert( QStringLiteral( "ERROR_CODE" ), errorCode );
  outputs.insert( QStringLiteral( "ERROR_MESSAGE" ),  errorMessage );
  outputs.insert( QStringLiteral( "RESULT_DATA" ), resultData );
  return outputs;
}

///@endcond
