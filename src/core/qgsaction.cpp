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
#include <QDialog>
#include <QLayout>
#include <QScreen>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QCryptographicHash>

#include "qgspythonrunner.h"
#include "qgsrunprocess.h"
#include "qgsexpressioncontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsexpressioncontextutils.h"
#include "qgswebview.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"


bool QgsAction::runable() const
{
  return mType == Generic ||
         mType == GenericPython ||
         mType == OpenUrl ||
         mType == SubmitUrl ||
         mType == SubmitUrlMultipart ||
#if defined(Q_OS_WIN)
         mType == Windows
#elif defined(Q_OS_MAC)
         mType == Mac
#else
         mType == Unix
#endif
         ;
}

void QgsAction::run( QgsVectorLayer *layer, const QgsFeature &feature, const QgsExpressionContext &expressionContext ) const
{
  QgsExpressionContext actionContext( expressionContext );

  actionContext << QgsExpressionContextUtils::layerScope( layer );
  actionContext.setFeature( feature );

  run( actionContext );
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

  const QString expandedAction = QgsExpression::replaceExpressionText( mCommand, &context );

  if ( mType == QgsAction::OpenUrl )
  {
    const QFileInfo finfo( expandedAction );
    if ( finfo.exists() && finfo.isFile() )
      QDesktopServices::openUrl( QUrl::fromLocalFile( expandedAction ) );
    else
      QDesktopServices::openUrl( QUrl( expandedAction, QUrl::TolerantMode ) );
  }
  else if ( mType == QgsAction::SubmitUrl || mType == QgsAction::SubmitUrlMultipart )
  {

    QUrl url{ command() };
    const QUrlQuery queryString { url.query( QUrl::ComponentFormattingOption::FullyDecoded ) };
    // Remove query
    QString payload { url.query() };
    url.setQuery( QString( ) );
    QDialog d;
    d.setWindowTitle( QObject::tr( "Form Submit Action" ) );
    d.setLayout( new QHBoxLayout( &d ) );
    QgsWebView *wv = new QgsWebView( &d );
    const double horizontalDpi { QgsApplication::primaryScreen()->logicalDotsPerInchX() };
    wv->setZoomFactor( horizontalDpi / 96.0 );
    wv->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
#ifdef QGISDEBUG
    wv->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif
#ifdef WITH_QTWEBKIT
    wv->page()->setForwardUnsupportedContent( true );
#endif
    wv->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
    wv->page()->settings()->setAttribute( QWebSettings::LocalStorageEnabled, true );
    wv->page()->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
    wv->page()->settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
    wv->page()->settings()->setAttribute( QWebSettings::PluginsEnabled, true );

    QObject::connect( wv->page(), &QWebPage::unsupportedContent, &d, [ &d ]( QNetworkReply * reply )
    {

      QString filename { "unknown.bin" };
      if ( const auto header = reply->header( QNetworkRequest::KnownHeaders::ContentDispositionHeader ).toString().toStdString(); ! header.empty() )
      {

        std::string ascii;
        const std::string q1 { R"(filename=")" };
        if ( const auto pos = header.find( q1 ); pos != std::string::npos )
        {
          const auto len = pos + q1.size();

          const std::string q2 { R"(")" };
          if ( auto pos = header.find( q2, len ); pos != std::string::npos )
          {
            bool escaped = false;
            while ( pos != std::string::npos && header[pos - 1] == '\\' )
            {
              std::cout << pos << std::endl;
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
        if ( const auto pos = header.find( u ); pos != std::string::npos )
        {
          utf8 = header.substr( pos + u.size() );
        }

        // Prefer ascii over utf8
        if ( ascii.empty() )
        {
          filename = QString::fromStdString( utf8 );
        }
        else
        {
          filename = QString::fromStdString( ascii );
        }
      }

      QTemporaryDir tempDir;
      tempDir.setAutoRemove( false );
      tempDir.path();
      const QString tempFilePath{ tempDir.path() + QDir::separator() + filename };
      QFile tempFile{tempFilePath};
      tempFile.open( QIODevice::WriteOnly );
      tempFile.write( reply->readAll() );
      tempFile.close();
      d.close();
      QDesktopServices::openUrl( QUrl::fromLocalFile( tempFilePath ) );
    } );

    d.layout()->addWidget( wv );
    QNetworkRequest req { url };

    // guess content type

    // check for json
    if ( mType != QgsAction::SubmitUrlMultipart )
    {
      QString contentType { QStringLiteral( "application/x-www-form-urlencoded" ) };
      QJsonParseError jsonError;
      QJsonDocument::fromJson( payload.toUtf8(), &jsonError );
      if ( jsonError.error == QJsonParseError::ParseError::NoError )
      {
        contentType = QStringLiteral( "application/json" );
      }
      req.setHeader( QNetworkRequest::KnownHeaders::ContentTypeHeader, contentType );
    }
    // for multipart create parts and headers
    else
    {
      QString newPayload;
      const auto queryItems { queryString.queryItems( QUrl::ComponentFormattingOption::FullyDecoded ) };
      QCryptographicHash hash{ QCryptographicHash::Algorithm::Md5 };
      hash.addData( QTime().toString( Qt::DateFormat::ISODateWithMs ).toUtf8() );
      const QString boundary{ hash.result().toHex() };
      const QString boundaryLine{ QStringLiteral( "-----------------------------%1\r\n" ).arg( boundary ) };
      req.setHeader( QNetworkRequest::KnownHeaders::ContentTypeHeader, QStringLiteral( "multipart/form-data; boundary=%1" ).arg( boundary ) );
      for ( const auto &queryItem : std::as_const( queryItems ) )
      {
        newPayload.push_back( boundaryLine );
        newPayload.push_back( QStringLiteral( "Content-Disposition: form-data; name=\"%1\"\r\n\r\n" )
                              .arg( QString( queryItem.first ).replace( '"', QStringLiteral( R"(\")" ) ) ) );
        newPayload.push_back( QStringLiteral( "%1\r\n" ).arg( queryItem.second ) );
      }
      newPayload.push_back( boundaryLine );
      payload = newPayload;
      req.setHeader( QNetworkRequest::KnownHeaders::ContentLengthHeader, newPayload.size() );
    }

    wv->load( req, QNetworkAccessManager::Operation::PostOperation, payload.toUtf8() );
    d.exec();
  }
  else if ( mType == QgsAction::GenericPython )
  {
    // TODO: capture output from QgsPythonRunner (like QgsRunProcess does)
    QgsPythonRunner::run( expandedAction );
  }
  else
  {
    // The QgsRunProcess instance created by this static function
    // deletes itself when no longer needed.
    QgsRunProcess::create( expandedAction, mCaptureOutput );
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

  mType = static_cast< QgsAction::ActionType >( actionElement.attributeNode( QStringLiteral( "type" ) ).value().toInt() );
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
  actionSetting.setAttribute( QStringLiteral( "type" ), mType );
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
    case Generic:
    {
      typeText = QObject::tr( "Generic" );
      break;
    }
    case GenericPython:
    {
      typeText = QObject::tr( "Generic Python" );
      break;
    }
    case Mac:
    {
      typeText = QObject::tr( "Mac" );
      break;
    }
    case Windows:
    {
      typeText = QObject::tr( "Windows" );
      break;
    }
    case Unix:
    {
      typeText = QObject::tr( "Unix" );
      break;
    }
    case OpenUrl:
    {
      typeText = QObject::tr( "Open URL" );
      break;
    }
    case SubmitUrl:
    {
      typeText = QObject::tr( "Submit URL (urlencoded or JSON)" );
      break;
    }
    case SubmitUrlMultipart:
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
