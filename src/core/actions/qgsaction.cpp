/***************************************************************************
  qgsaction.cpp - QgsAction

 ---------------------
 begin                : 18.4.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaction.h"

#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgspythonrunner.h"
#include "qgsrunprocess.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QNetworkRequest>
#include <QTemporaryDir>
#include <QUrl>
#include <QUrlQuery>

bool QgsAction::runable() const
{
// clang analyzer is not happy because of the multiple duplicate return branches, but this is ok :)
// NOLINTBEGIN(bugprone-branch-clone)
  switch ( mType )
  {
    case Qgis::AttributeActionType::Generic:
    case Qgis::AttributeActionType::GenericPython:
    case Qgis::AttributeActionType::OpenUrl:
    case Qgis::AttributeActionType::SubmitUrlEncoded:
    case Qgis::AttributeActionType::SubmitUrlMultipart:
      return true;

#if defined(Q_OS_WIN)
    case Qgis::AttributeActionType::Windows:
      return true;
    case Qgis::AttributeActionType::Mac:
    case Qgis::AttributeActionType::Unix:
      return false;
#elif defined(Q_OS_MAC)
    case Qgis::AttributeActionType::Mac:
      return true;
    case Qgis::AttributeActionType::Windows:
    case Qgis::AttributeActionType::Unix:
      return false;
#else
    case Qgis::AttributeActionType::Unix:
      return true;
    case Qgis::AttributeActionType::Mac:
    case Qgis::AttributeActionType::Windows:
      return false;
#endif
  }
  return false;
  // NOLINTEND(bugprone-branch-clone)
}

void QgsAction::run( QgsVectorLayer *layer, const QgsFeature &feature, const QgsExpressionContext &expressionContext ) const
{
  QgsExpressionContext actionContext( expressionContext );

  actionContext << QgsExpressionContextUtils::layerScope( layer );
  actionContext.setFeature( feature );

  run( actionContext );
}

void QgsAction::handleFormSubmitAction( const QString &expandedAction ) const
{

  // Show busy in case the form subit is slow
  QApplication::setOverrideCursor( Qt::WaitCursor );

  QUrl url{ expandedAction };

  // Encode '+' (fully encoded doesn't encode it)
  const QString payload { url.query( QUrl::ComponentFormattingOption::FullyEncoded ).replace( QChar( '+' ), u"%2B"_s ) };

  // Remove query string from URL
  const QUrlQuery queryString { url.query( ) };
  url.setQuery( QString( ) );

  QNetworkRequest req { url };

  // Specific code for testing, produces an invalid POST but we can still listen to
  // signals and examine the request
  if ( url.toString().contains( "fake_qgis_http_endpoint"_L1 ) )
  {
    req.setUrl( u"file://%1"_s.arg( url.path() ) );
  }

  QNetworkReply *reply = nullptr;

  if ( mType != Qgis::AttributeActionType::SubmitUrlMultipart )
  {
    QString contentType { u"application/x-www-form-urlencoded"_s };
    // check for json
    QJsonParseError jsonError;
    QJsonDocument::fromJson( payload.toUtf8(), &jsonError );
    if ( jsonError.error == QJsonParseError::ParseError::NoError )
    {
      contentType = u"application/json"_s;
    }
    req.setHeader( QNetworkRequest::KnownHeaders::ContentTypeHeader, contentType );
    reply = QgsNetworkAccessManager::instance()->post( req, payload.toUtf8() );
  }
  // for multipart create parts and headers
  else
  {
    QHttpMultiPart *multiPart = new QHttpMultiPart( QHttpMultiPart::FormDataType );
    const QList<QPair<QString, QString>> queryItems { queryString.queryItems( QUrl::ComponentFormattingOption::FullyDecoded ) };
    for ( const QPair<QString, QString> &queryItem : std::as_const( queryItems ) )
    {
      QHttpPart part;
      part.setHeader( QNetworkRequest::ContentDispositionHeader,
                      u"form-data; name=\"%1\""_s
                      .arg( QString( queryItem.first ).replace( '"', R"(\")"_L1 ) ) );
      part.setBody( queryItem.second.toUtf8() );
      multiPart->append( part );
    }
    reply = QgsNetworkAccessManager::instance()->post( req, multiPart );
    multiPart->setParent( reply );
  }

  QObject::connect( reply, &QNetworkReply::finished, reply, [ reply ]
  {
    if ( reply->error() == QNetworkReply::NoError )
    {

      if ( QgsVariantUtils::isNull( reply->attribute( QNetworkRequest::RedirectionTargetAttribute ) ) )
      {

        const QByteArray replyData = reply->readAll();

        QString filename { "download.bin" };
        if ( const std::string header = reply->header( QNetworkRequest::KnownHeaders::ContentDispositionHeader ).toString().toStdString(); ! header.empty() )
        {

          // Extract filename dealing with ill formed headers with unquoted file names

          std::string ascii;

          const std::string q1 { R"(filename=)" };

          if ( size_t pos = header.find( q1 ); pos != std::string::npos )
          {

            // Deal with ill formed headers with unquoted file names
            if ( header.find( R"(filename=")" ) != std::string::npos )
            {
              pos++;
            }

            const size_t len = pos + q1.size();

            const std::string q2 { R"(")" };
            if ( size_t pos = header.find( q2, len ); pos != std::string::npos )
            {
              bool escaped = false;
              while ( pos != std::string::npos && header[pos - 1] == '\\' )
              {
                pos = header.find( q2, pos + 1 );
                escaped = true;
              }
              ascii = header.substr( len, pos - len );
              if ( escaped )
              {
                std::string cleaned;
                for ( size_t i = 0; i < ascii.size(); ++i )
                {
                  if ( ascii[i] == '\\' )
                  {
                    if ( i > 0 && ascii[i - 1] == '\\' )
                    {
                      cleaned.push_back( ascii[i] );
                    }
                  }
                  else
                  {
                    cleaned.push_back( ascii[i] );
                  }
                }
                ascii = std::move( cleaned );
              }
            }
          }

          std::string utf8;

          const std::string u { R"(UTF-8'')" };
          if ( const size_t pos = header.find( u ); pos != std::string::npos )
          {
            utf8 = header.substr( pos + u.size() );
          }

          // Prefer ascii over utf8
          if ( ascii.empty() )
          {
            if ( ! utf8.empty( ) )
            {
              filename = QString::fromStdString( utf8 );
            }
          }
          else
          {
            filename = QString::fromStdString( ascii );
          }
        }
        else if ( !QgsVariantUtils::isNull( reply->header( QNetworkRequest::KnownHeaders::ContentTypeHeader ) ) )
        {
          QString contentTypeHeader { reply->header( QNetworkRequest::KnownHeaders::ContentTypeHeader ).toString() };
          // Strip charset if any
          if ( contentTypeHeader.contains( ';' ) )
          {
            contentTypeHeader = contentTypeHeader.left( contentTypeHeader.indexOf( ';' ) );
          }

          QMimeType mimeType { QMimeDatabase().mimeTypeForName( contentTypeHeader ) };
          if ( mimeType.isValid() )
          {
            filename = u"download.%1"_s.arg( mimeType.preferredSuffix() );
          }
        }

        QTemporaryDir tempDir;
        tempDir.setAutoRemove( false );
        tempDir.path();
        const QString tempFilePath{ tempDir.path() + QDir::separator() + filename };
        QFile tempFile{ tempFilePath };
        if ( tempFile.open( QIODevice::WriteOnly ) )
        {
          tempFile.write( replyData );
          tempFile.close();
          QDesktopServices::openUrl( QUrl::fromLocalFile( tempFilePath ) );
        }
        else
        {
          QgsMessageLog::logMessage( QObject::tr( "Could not open temporary file for writing" ), u"Form Submit Action"_s, Qgis::MessageLevel::Critical );
        }
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "Redirect is not supported!" ), u"Form Submit Action"_s, Qgis::MessageLevel::Critical );
      }
    }
    else
    {
      QgsMessageLog::logMessage( reply->errorString(), u"Form Submit Action"_s, Qgis::MessageLevel::Critical );
    }
    reply->deleteLater();
    QApplication::restoreOverrideCursor( );
  } );

}

void QgsAction::setCommand( const QString &newCommand )
{
  mCommand = newCommand;
}

void QgsAction::run( const QgsExpressionContext &expressionContext ) const
{
  if ( !isValid() )
  {
    QgsDebugError( u"Invalid action cannot be run"_s );
    return;
  }

  QgsExpressionContextScope *scope = new QgsExpressionContextScope( mExpressionContextScope );
  QgsExpressionContext context( expressionContext );
  context << scope;

  // Show busy in case the expression evaluation is slow
  QApplication::setOverrideCursor( Qt::WaitCursor );
  const QString expandedAction = QgsExpression::replaceExpressionText( mCommand, &context );
  QApplication::restoreOverrideCursor();

  if ( mType == Qgis::AttributeActionType::OpenUrl )
  {
    const QFileInfo finfo( expandedAction );
    if ( finfo.exists() && finfo.isFile() )
      QDesktopServices::openUrl( QUrl::fromLocalFile( expandedAction ) );
    else
      QDesktopServices::openUrl( QUrl( expandedAction, QUrl::TolerantMode ) );
  }
  else if ( mType == Qgis::AttributeActionType::SubmitUrlEncoded || mType == Qgis::AttributeActionType::SubmitUrlMultipart )
  {
    handleFormSubmitAction( expandedAction );
  }
  else if ( mType == Qgis::AttributeActionType::GenericPython )
  {
    // TODO: capture output from QgsPythonRunner (like QgsRunProcess does)
    QgsPythonRunner::run( expandedAction );
  }
  else
  {
    // The QgsRunProcess instance created by this static function
    // deletes itself when no longer needed.
#ifndef __clang_analyzer__
    QgsRunProcess::create( expandedAction, mCaptureOutput );
#endif
  }
}

QSet<QString> QgsAction::actionScopes() const
{
  return mActionScopes;
}

void QgsAction::setActionScopes( const QSet<QString> &actionScopes )
{
  mActionScopes = actionScopes;
}

void QgsAction::readXml( const QDomNode &actionNode )
{
  QDomElement actionElement = actionNode.toElement();
  const QDomNodeList actionScopeNodes = actionElement.elementsByTagName( u"actionScope"_s );

  if ( actionScopeNodes.isEmpty() )
  {
    mActionScopes
        << u"Canvas"_s
        << u"Field"_s
        << u"Feature"_s;
  }
  else
  {
    for ( int j = 0; j < actionScopeNodes.length(); ++j )
    {
      const QDomElement actionScopeElem = actionScopeNodes.item( j ).toElement();
      mActionScopes << actionScopeElem.attribute( u"id"_s );
    }
  }

  mType = static_cast< Qgis::AttributeActionType >( actionElement.attributeNode( u"type"_s ).value().toInt() );
  mDescription = actionElement.attributeNode( u"name"_s ).value();
  mCommand = actionElement.attributeNode( u"action"_s ).value();
  mIcon = actionElement.attributeNode( u"icon"_s ).value();
  mCaptureOutput = actionElement.attributeNode( u"capture"_s ).value().toInt() != 0;
  mShortTitle = actionElement.attributeNode( u"shortTitle"_s ).value();
  mNotificationMessage = actionElement.attributeNode( u"notificationMessage"_s ).value();
  mIsEnabledOnlyWhenEditable = actionElement.attributeNode( u"isEnabledOnlyWhenEditable"_s ).value().toInt() != 0;
  mId = QUuid( actionElement.attributeNode( u"id"_s ).value() );
  if ( mId.isNull() )
    mId = QUuid::createUuid();
}

void QgsAction::writeXml( QDomNode &actionsNode ) const
{
  QDomElement actionSetting = actionsNode.ownerDocument().createElement( u"actionsetting"_s );
  actionSetting.setAttribute( u"type"_s, static_cast< int >( mType ) );
  actionSetting.setAttribute( u"name"_s, mDescription );
  actionSetting.setAttribute( u"shortTitle"_s, mShortTitle );
  actionSetting.setAttribute( u"icon"_s, mIcon );
  actionSetting.setAttribute( u"action"_s, mCommand );
  actionSetting.setAttribute( u"capture"_s, mCaptureOutput );
  actionSetting.setAttribute( u"notificationMessage"_s, mNotificationMessage );
  actionSetting.setAttribute( u"isEnabledOnlyWhenEditable"_s, mIsEnabledOnlyWhenEditable );
  actionSetting.setAttribute( u"id"_s, mId.toString() );

  const auto constMActionScopes = mActionScopes;
  for ( const QString &scope : constMActionScopes )
  {
    QDomElement actionScopeElem = actionsNode.ownerDocument().createElement( u"actionScope"_s );
    actionScopeElem.setAttribute( u"id"_s, scope );
    actionSetting.appendChild( actionScopeElem );
  }

  actionsNode.appendChild( actionSetting );
}

void QgsAction::setExpressionContextScope( const QgsExpressionContextScope &scope )
{
  mExpressionContextScope = scope;
}

QgsExpressionContextScope QgsAction::expressionContextScope() const
{
  return mExpressionContextScope;
}

QString QgsAction::html() const
{
  QString typeText;
  switch ( mType )
  {
    case Qgis::AttributeActionType::Generic:
    {
      typeText = QObject::tr( "Generic" );
      break;
    }
    case Qgis::AttributeActionType::GenericPython:
    {
      typeText = QObject::tr( "Generic Python" );
      break;
    }
    case Qgis::AttributeActionType::Mac:
    {
      typeText = QObject::tr( "macOS" );
      break;
    }
    case Qgis::AttributeActionType::Windows:
    {
      typeText = QObject::tr( "Windows" );
      break;
    }
    case Qgis::AttributeActionType::Unix:
    {
      typeText = QObject::tr( "Unix" );
      break;
    }
    case Qgis::AttributeActionType::OpenUrl:
    {
      typeText = QObject::tr( "Open URL" );
      break;
    }
    case Qgis::AttributeActionType::SubmitUrlEncoded:
    {
      typeText = QObject::tr( "Submit URL (urlencoded or JSON)" );
      break;
    }
    case Qgis::AttributeActionType::SubmitUrlMultipart:
    {
      typeText = QObject::tr( "Submit URL (multipart)" );
      break;
    }
  }
  return { QObject::tr( R"html(
<h2>Action Details</h2>
<p>
   <b>Description:</b> %1<br>
   <b>Short title:</b> %2<br>
   <b>Type:</b> %3<br>
   <b>Scope:</b> %4<br>
   <b>Action:</b><br>
   <pre>%6</pre>
</p>
  )html" ).arg( mDescription, mShortTitle, typeText, actionScopes().values().join( ", "_L1 ), mCommand )};
};
