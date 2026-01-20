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

#include <memory>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgssettings.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QUrl>

void QgsHelp::openHelp( const QString &key )
{
  QDesktopServices::openUrl( QgsHelp::helpUrl( key ) );
}

QUrl QgsHelp::helpUrl( const QString &key )
{
  QUrl helpNotFound = QUrl::fromLocalFile( QgsApplication::pkgDataPath() + "/doc/nohelp.html" );

  const QgsSettings settings;
  const QStringList paths = settings.value( u"help/helpSearchPath"_s ).toStringList();
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
    if ( path.endsWith( "\\"_L1 ) || path.endsWith( '/'_L1 ) )
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
      const QRegularExpression rx( u"(<!\\$\\$)*(\\$%1)"_s.arg( var ) );
      fullPath.replace( rx, scope->variable( var ).toString() );
    }
    const thread_local QRegularExpression pathRx( u"(\\$\\$)"_s );
    fullPath.replace( pathRx, u"$"_s );

    helpPath = u"%1/%2"_s.arg( fullPath, key );

    QgsMessageLog::logMessage( QObject::tr( "Trying to open help using key '%1'. Full URI is '%2'…" ).arg( key ).arg( helpPath ), QObject::tr( "QGIS Help" ), Qgis::MessageLevel::Info );

    if ( helpPath.startsWith( "http"_L1 ) )
    {
      if ( !QgsHelp::urlExists( helpPath ) )
      {
        continue;
      }
      helpUrl = QUrl( helpPath );
    }
    else
    {
      const QString filePath = helpPath.mid( 0, helpPath.lastIndexOf( '#'_L1 ) );
      if ( !QFileInfo::exists( filePath ) )
      {
        continue;
      }
      helpUrl = QUrl::fromLocalFile( filePath );
      const int pos = helpPath.lastIndexOf( '#'_L1 );
      if ( pos != -1 )
      {
        helpUrl.setFragment( helpPath.mid( helpPath.lastIndexOf( '#'_L1 ) + 1, -1 ) );
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

  QgsBlockingNetworkRequest request;
  QNetworkRequest req( helpUrl );
  QgsSetRequestInitiatorClass( req, u"QgsHelp"_s );

  QgsBlockingNetworkRequest::ErrorCode errCode = request.head( req );
  return errCode == QgsBlockingNetworkRequest::NoError;
}
