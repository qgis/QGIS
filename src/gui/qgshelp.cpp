/***************************************************************************
  qgshelp.cpp
  --------------------------------------
  Date                 : December 2016
  Copyright            : (C) 2016 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshelp.h"

#include "qgis.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsexpressioncontext.h"
#include "qgsmessagelog.h"
#include "qgsexpressioncontextutils.h"

#include <QUrl>
#include <QFileInfo>
#include <QTcpSocket>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>

#include <memory>


void QgsHelp::openHelp( const QString &key )
{
  QDesktopServices::openUrl( QgsHelp::helpUrl( key ) );
}

QUrl QgsHelp::helpUrl( const QString &key )
{
  QUrl helpNotFound = QUrl::fromLocalFile( QgsApplication::pkgDataPath() + "/doc/nohelp.html" );

  const QgsSettings settings;
  const QStringList paths = settings.value( QStringLiteral( "help/helpSearchPath" ) ).toStringList();
  if ( paths.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Help location is not configured!" ), QObject::tr( "QGIS Help" ) );
    return helpNotFound;
  }

  std::unique_ptr<QgsExpressionContextScope> scope( QgsExpressionContextUtils::globalScope() );

  QUrl helpUrl;
  QString helpPath, fullPath;
  bool helpFound = false;

  const auto constPaths = paths;
  for ( const QString &path : constPaths )
  {
    if ( path.endsWith( QLatin1String( "\\" ) ) || path.endsWith( QLatin1Char( '/' ) ) )
    {
      fullPath = path.left( path.size() - 1 );
    }
    else
    {
      fullPath = path;
    }

    const auto constVariableNames = scope->variableNames();
    for ( const QString &var : constVariableNames )
    {
      const QRegularExpression rx( QStringLiteral( "(<!\\$\\$)*(\\$%1)" ).arg( var ) );
      fullPath.replace( rx, scope->variable( var ).toString() );
    }
    fullPath.replace( QRegularExpression( QStringLiteral( "(\\$\\$)" ) ), QStringLiteral( "$" ) );

    helpPath = QStringLiteral( "%1/%2" ).arg( fullPath, key );

    QgsMessageLog::logMessage( QObject::tr( "Trying to open help using key '%1'. Full URI is '%2'…" ).arg( key ).arg( helpPath ), QObject::tr( "QGIS Help" ), Qgis::MessageLevel::Info );

    if ( helpPath.startsWith( QLatin1String( "http" ) ) )
    {
      if ( !QgsHelp::urlExists( helpPath ) )
      {
        continue;
      }
      helpUrl = QUrl( helpPath );
    }
    else
    {
      const QString filePath = helpPath.mid( 0, helpPath.lastIndexOf( QLatin1Char( '#' ) ) );
      if ( !QFileInfo::exists( filePath ) )
      {
        continue;
      }
      helpUrl = QUrl::fromLocalFile( filePath );
      const int pos = helpPath.lastIndexOf( QLatin1Char( '#' ) );
      if ( pos != -1 )
      {
        helpUrl.setFragment( helpPath.mid( helpPath.lastIndexOf( QLatin1Char( '#' ) ) + 1, -1 ) );
      }
    }

    helpFound = true;
    break;
  }

  return helpFound ? helpUrl : helpNotFound;
}

bool QgsHelp::urlExists( const QString &url )
{
  const QUrl helpUrl( url );
  QTcpSocket socket;

  const QgsSettings settings;
  const bool proxyEnabled = settings.value( QStringLiteral( "proxy/proxyEnabled" ), false ).toBool();
  if ( proxyEnabled )
  {
    QNetworkProxy proxy;
    const QString proxyHost = settings.value( QStringLiteral( "proxy/proxyHost" ), QString() ).toString();
    const int proxyPort = settings.value( QStringLiteral( "proxy/proxyPort" ), QString() ).toString().toInt();
    const QString proxyUser = settings.value( QStringLiteral( "proxy/proxyUser" ), QString() ).toString();
    const QString proxyPassword = settings.value( QStringLiteral( "proxy/proxyPassword" ), QString() ).toString();

    const QString proxyTypeString = settings.value( QStringLiteral( "proxy/proxyType" ), QString() ).toString();

    if ( proxyTypeString == QLatin1String( "DefaultProxy" ) )
    {
      QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
      if ( !proxies.isEmpty() )
      {
        proxy = proxies.first();
      }
    }
    else
    {
      QNetworkProxy::ProxyType proxyType = QNetworkProxy::DefaultProxy;
      if ( proxyTypeString == QLatin1String( "Socks5Proxy" ) )
      {
        proxyType = QNetworkProxy::Socks5Proxy;
      }
      else if ( proxyTypeString == QLatin1String( "HttpProxy" ) )
      {
        proxyType = QNetworkProxy::HttpProxy;
      }
      else if ( proxyTypeString == QLatin1String( "HttpCachingProxy" ) )
      {
        proxyType = QNetworkProxy::HttpCachingProxy;
      }
      else if ( proxyTypeString == QLatin1String( "FtpCachingProxy" ) )
      {
        proxyType = QNetworkProxy::FtpCachingProxy;
      }
      proxy = QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword );
    }
    socket.setProxy( proxy );
  }

  socket.connectToHost( helpUrl.host(), 80 );
  if ( socket.waitForConnected() )
  {
    socket.write( "HEAD " + helpUrl.path().toUtf8() + " HTTP/1.1\r\n"
                  "Host: " + helpUrl.host().toUtf8() + "\r\n\r\n" );
    if ( socket.waitForReadyRead() )
    {
      const QByteArray bytes = socket.readAll();
      if ( bytes.contains( "200 OK" ) ||  bytes.contains( "302 Found" ) ||  bytes.contains( "301 Moved" ) )
      {
        return true;
      }
    }
  }

  return false;
}
