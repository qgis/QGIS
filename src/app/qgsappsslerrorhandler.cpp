/***************************************************************************
    qgsappsslerrorhandler.cpp
    ---------------------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappsslerrorhandler.h"
#include "qgslogger.h"
#include "qgsauthcertutils.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthsslerrorsdialog.h"
#include "qgisapp.h"

void QgsAppSslErrorHandler::handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

  const QString hostport( QStringLiteral( "%1:%2" )
                            .arg( reply->url().host() )
                            .arg( reply->url().port() != -1 ? reply->url().port() : 443 )
                            .trimmed() );
  const QString digest( QgsAuthCertUtils::shaHexForCert( reply->sslConfiguration().peerCertificate() ) );
  const QString dgsthostport( QStringLiteral( "%1:%2" ).arg( digest, hostport ) );

  const QHash<QString, QSet<QSslError::SslError>> &errscache( QgsApplication::authManager()->ignoredSslErrorCache() );

  if ( errscache.contains( dgsthostport ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Ignored SSL errors cached item found, ignoring errors if they match for %1" ).arg( hostport ), 2 );
    const QSet<QSslError::SslError> &errenums( errscache.value( dgsthostport ) );
    bool ignore = !errenums.isEmpty();
    if ( ignore )
    {
      for ( const QSslError &error : errors )
      {
        if ( error.error() == QSslError::NoError )
          continue;

        const bool errmatch = errenums.contains( error.error() );
        ignore = ignore && errmatch;
      }
    }

    if ( ignore )
    {
      QgsDebugMsgLevel( QStringLiteral( "Errors matched cached item's, ignoring all for %1" ).arg( hostport ), 2 );
      reply->ignoreSslErrors();
      return;
    }

    QgsDebugMsgLevel( QStringLiteral( "Errors %1 for cached item for %2" ).arg( errenums.isEmpty() ? QStringLiteral( "not found" ) : QStringLiteral( "did not match" ), hostport ), 2 );
  }

  QgsDebugError( QStringLiteral( "SSL errors occurred accessing URL:\n%1" ).arg( reply->request().url().toString() ) );

  QgsAuthSslErrorsDialog *dlg = new QgsAuthSslErrorsDialog( reply, errors, QgisApp::instance(), digest, hostport );
  dlg->setWindowModality( Qt::ApplicationModal );
  dlg->resize( 580, 512 );
  if ( dlg->exec() )
  {
    QgsDebugMsgLevel( QStringLiteral( "All SSL errors ignored for %1" ).arg( hostport ), 2 );
    reply->ignoreSslErrors();
  }
  dlg->deleteLater();
}
