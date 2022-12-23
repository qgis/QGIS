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

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <QUrlQuery>
#include <QDir>
#include <QTemporaryDir>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QHttpMultiPart>
#include <QMimeDatabase>
#include <QApplication>

#include "qgspythonrunner.h"
#include "qgsrunprocess.h"
#include "qgsexpressioncontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsexpressioncontextutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include "qgsvariantutils.h"

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
  const QString payload { url.query( QUrl::ComponentFormattingOption::FullyEncoded ).replace( QChar( '+' ), QStringLiteral( "%2B" ) ) };

  // Remove query string from URL
  const QUrlQuery queryString { url.query( ) };
  url.setQuery( QString( ) );

  QNetworkRequest req { url };

  // Specific code for testing, produces an invalid POST but we can still listen to
  // signals and examine the request
  if ( url.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    req.setUrl( QStringLiteral( "file://%1" ).arg( url.path() ) );
  }

  QNetworkReply *reply = nullptr;

  if ( mType != Qgis::AttributeActionType::SubmitUrlMultipart )
  {
    QString contentType { QStringLiteral( "application/x-www-form-urlencoded" ) };
    // check for json
    QJsonParseError jsonError;
    QJsonDocument::fromJson( payload.toUtf8(), &jsonError );
    if ( jsonError.error == QJsonParseError::ParseError::NoError )
    {
      contentType = QStringLiteral( "application/json" );
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
                      QStringLiteral( "form-data; name=\"%1\"" )
                      .arg( QString( queryItem.first ).replace( '"', QLatin1String( R"(\")" ) ) ) );
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
                ascii = cleaned;
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
            filename = QStringLiteral( "download.%1" ).arg( mimeType.preferredSuffix() );
          }
        }

        QTemporaryDir tempDir;
        tempDir.setAutoRemove( false );
        tempDir.path();
        const QString tempFilePath{ tempDir.path() + QDir::separator() + filename };
        QFile tempFile{ tempFilePath };
        tempFile.open( QIODevice::WriteOnly );
        tempFile.write( replyData );
        tempFile.close();
        QDesktopServices::openUrl( QUrl::fromLocalFile( tempFilePath ) );
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "Redirect is not supported!" ), QStringLiteral( "Form Submit Action" ), Qgis::MessageLevel::Critical );
      }
    }
    else
    {
      QgsMessageLog::logMessage( reply->errorString(), QStringLiteral( "Form Submit Action" ), Qgis::MessageLevel::Critical );
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
    QgsDebugMsg( QStringLiteral( "Invalid action cannot be run" ) );
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
  const QDomNodeList actionScopeNodes = actionElement.elementsByTagName( QStringLiteral( "actionScope" ) );

  if ( actionScopeNodes.isEmpty() )
  {
    mActionScopes
        << QStringLiteral( "Canvas" )
        << QStringLiteral( "Field" )
        << QStringLiteral( "Feature" );
  }
  else
  {
    for ( int j = 0; j < actionScopeNodes.length(); ++j )
    {
      const QDomElement actionScopeElem = actionScopeNodes.item( j ).toElement();
      mActionScopes << actionScopeElem.attribute( QStringLiteral( "id" ) );
    }
  }

  mType = static_cast< Qgis::AttributeActionType >( actionElement.attributeNode( QStringLiteral( "type" ) ).value().toInt() );
  mDescription = actionElement.attributeNode( QStringLiteral( "name" ) ).value();
  mCommand = actionElement.attributeNode( QStringLiteral( "action" ) ).value();
  mIcon = actionElement.attributeNode( QStringLiteral( "icon" ) ).value();
  mCaptureOutput = actionElement.attributeNode( QStringLiteral( "capture" ) ).value().toInt() != 0;
  mShortTitle = actionElement.attributeNode( QStringLiteral( "shortTitle" ) ).value();
  mNotificationMessage = actionElement.attributeNode( QStringLiteral( "notificationMessage" ) ).value();
  mIsEnabledOnlyWhenEditable = actionElement.attributeNode( QStringLiteral( "isEnabledOnlyWhenEditable" ) ).value().toInt() != 0;
  mId = QUuid( actionElement.attributeNode( QStringLiteral( "id" ) ).value() );
  if ( mId.isNull() )
    mId = QUuid::createUuid();
}

void QgsAction::writeXml( QDomNode &actionsNode ) const
{
  QDomElement actionSetting = actionsNode.ownerDocument().createElement( QStringLiteral( "actionsetting" ) );
  actionSetting.setAttribute( QStringLiteral( "type" ), static_cast< int >( mType ) );
  actionSetting.setAttribute( QStringLiteral( "name" ), mDescription );
  actionSetting.setAttribute( QStringLiteral( "shortTitle" ), mShortTitle );
  actionSetting.setAttribute( QStringLiteral( "icon" ), mIcon );
  actionSetting.setAttribute( QStringLiteral( "action" ), mCommand );
  actionSetting.setAttribute( QStringLiteral( "capture" ), mCaptureOutput );
  actionSetting.setAttribute( QStringLiteral( "notificationMessage" ), mNotificationMessage );
  actionSetting.setAttribute( QStringLiteral( "isEnabledOnlyWhenEditable" ), mIsEnabledOnlyWhenEditable );
  actionSetting.setAttribute( QStringLiteral( "id" ), mId.toString() );

  const auto constMActionScopes = mActionScopes;
  for ( const QString &scope : constMActionScopes )
  {
    QDomElement actionScopeElem = actionsNode.ownerDocument().createElement( QStringLiteral( "actionScope" ) );
    actionScopeElem.setAttribute( QStringLiteral( "id" ), scope );
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
      typeText = QObject::tr( "Mac" );
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
  )html" ).arg( mDescription, mShortTitle, typeText, actionScopes().values().join( QLatin1String( ", " ) ), mCommand )};
};
