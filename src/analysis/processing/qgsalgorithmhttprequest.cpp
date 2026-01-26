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

#include "qgis.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsprocessingparameters.h"

#include <QDesktopServices>
#include <QMimeDatabase>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include "moc_qgsalgorithmhttprequest.cpp"

///@cond PRIVATE

QString QgsHttpRequestAlgorithm::name() const
{
  return u"httprequest"_s;
}

QString QgsHttpRequestAlgorithm::displayName() const
{
  return tr( "HTTP(S) POST/GET request" );
}

QString QgsHttpRequestAlgorithm::shortDescription() const
{
  return tr( "Performs a HTTP(S) POST/GET request and returns the result code, error message and the data." );
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
  return u"filetools"_s;
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
  addParameter( new QgsProcessingParameterString( u"URL"_s, tr( "URL" ), QVariant(), false, false ) );

  auto methodParam = std::make_unique<QgsProcessingParameterEnum>(
    u"METHOD"_s,
    QObject::tr( "Method" ),
    QStringList()
      << u"GET"_s
      << u"POST"_s,
    false,
    0
  );
  methodParam->setHelp( QObject::tr( "The HTTP method to use for the request" ) );
  addParameter( methodParam.release() );

  auto dataParam = std::make_unique<QgsProcessingParameterString>(
    u"DATA"_s, tr( "POST data" ), QVariant(), false, true
  );
  dataParam->setHelp( QObject::tr( "The data to add in the body if the request is a POST" ) );
  addParameter( dataParam.release() );

  auto outputFileParam = std::make_unique<QgsProcessingParameterFileDestination>(
    u"OUTPUT"_s, tr( "File destination" ), QObject::tr( "All files (*.*)" ), QVariant(), true, false
  );
  outputFileParam->setHelp( tr( "The result can be written to a file instead of being returned as a string" ) );
  addParameter( outputFileParam.release() );

  auto authConfigParam = std::make_unique<QgsProcessingParameterAuthConfig>(
    u"AUTH_CONFIG"_s, tr( "Authentication" ), QVariant(), true
  );
  authConfigParam->setHelp( tr( "An authentication configuration to pass" ) );
  addParameter( authConfigParam.release() );

  auto failureParam = std::make_unique<QgsProcessingParameterBoolean>(
    u"FAIL_ON_ERROR"_s, tr( "Consider HTTP errors as failures" ), false
  );
  failureParam->setHelp( tr( "If set, the algorithm will fail on encountering a HTTP error" ) );
  addParameter( failureParam.release() );

  addOutput( new QgsProcessingOutputNumber( u"ERROR_CODE"_s, QObject::tr( "Network error code" ) ) );
  addOutput( new QgsProcessingOutputString( u"ERROR_MESSAGE"_s, QObject::tr( "Network error message" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"STATUS_CODE"_s, QObject::tr( "HTTP status code" ) ) );
  addOutput( new QgsProcessingOutputString( u"RESULT_DATA"_s, QObject::tr( "Reply data" ) ) );
}

QVariantMap QgsHttpRequestAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString url = parameterAsString( parameters, u"URL"_s, context );
  if ( url.isEmpty() )
    throw QgsProcessingException( tr( "No URL specified" ) );
  const Qgis::HttpMethod httpMethod = static_cast<Qgis::HttpMethod>( parameterAsEnum( parameters, u"METHOD"_s, context ) );
  const QString data = parameterAsString( parameters, u"DATA"_s, context );
  const QString authCfg = parameterAsString( parameters, u"AUTH_CONFIG"_s, context );
  const QString outputFile = parameterAsFileOutput( parameters, u"OUTPUT"_s, context );
  const bool failOnError = parameterAsBool( parameters, u"FAIL_ON_ERROR"_s, context );
  const QUrl qurl = QUrl::fromUserInput( url );

  // Make Request
  QNetworkRequest request( qurl );
  QgsBlockingNetworkRequest blockingRequest;
  blockingRequest.setAuthCfg( authCfg );
  QgsBlockingNetworkRequest::ErrorCode errorCode = QgsBlockingNetworkRequest::NoError;

  switch ( httpMethod )
  {
    case Qgis::HttpMethod::Get:
    {
      errorCode = blockingRequest.get( request );
      break;
    }
    case Qgis::HttpMethod::Post:
    {
      errorCode = blockingRequest.post( request, data.toUtf8() );
      break;
    }

    case Qgis::HttpMethod::Head:
    case Qgis::HttpMethod::Put:
    case Qgis::HttpMethod::Delete:
      throw QgsProcessingException( QObject::tr( "Unsupported HTTP method: %1" ).arg( qgsEnumValueToKey( httpMethod ) ) );
  }

  // Handle reply
  const int statusCode = blockingRequest.reply().attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QString errorMessage = QString();
  QByteArray resultData = QByteArray();

  if ( errorCode == QgsBlockingNetworkRequest::NoError )
  {
    feedback->pushInfo( tr( "Request succeeded with code %1" ).arg( statusCode ) );
    resultData = blockingRequest.reply().content();

    if ( !outputFile.isEmpty() )
    {
      QFile tempFile( outputFile );
      if ( !tempFile.open( QIODevice::WriteOnly ) )
      {
        throw QgsProcessingException( QObject::tr( "Could not open %1 for writing" ).arg( outputFile ) );
      }
      tempFile.write( resultData );
      tempFile.close();

      feedback->pushInfo( tr( "Result data written to %1" ).arg( outputFile ) );
    }
  }
  else
  {
    feedback->pushInfo( tr( "Request failed with code %1" ).arg( statusCode ) );
    errorMessage = blockingRequest.reply().errorString();
    if ( failOnError )
    {
      throw QgsProcessingException( errorMessage );
    }
    feedback->pushWarning( errorMessage );
  }

  QVariantMap outputs;
  outputs.insert( u"STATUS_CODE"_s, statusCode );
  outputs.insert( u"ERROR_CODE"_s, errorCode );
  outputs.insert( u"ERROR_MESSAGE"_s, errorMessage );
  outputs.insert( u"RESULT_DATA"_s, QString( resultData ) );
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
