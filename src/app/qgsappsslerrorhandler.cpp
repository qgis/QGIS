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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgsauthsslerrorsdialog.h"
#include "qgslogger.h"

void QgsAppSslErrorHandler::handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

  const QString hostport( u"%1:%2"_s
                            .arg( reply->url().host() )
                            .arg( reply->url().port() != -1 ? reply->url().port() : 443 )
                            .trimmed() );
  const QString digest( QgsAuthCertUtils::shaHexForCert( reply->sslConfiguration().peerCertificate() ) );
  const QString dgsthostport( u"%1:%2"_s.arg( digest, hostport ) );

  const QHash<QString, QSet<QSslError::SslError>> &errscache( QgsApplication::authManager()->ignoredSslErrorCache() );

  if ( errscache.contains( dgsthostport ) )
  {
    QgsDebugMsgLevel( u"Ignored SSL errors cached item found, ignoring errors if they match for %1"_s.arg( hostport ), 2 );
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
      QgsDebugMsgLevel( u"Errors matched cached item's, ignoring all for %1"_s.arg( hostport ), 2 );
      reply->ignoreSslErrors();
      return;
    }

    QgsDebugMsgLevel( u"Errors %1 for cached item for %2"_s.arg( errenums.isEmpty() ? u"not found"_s : u"did not match"_s, hostport ), 2 );
  }

  QgsDebugError( u"SSL errors occurred accessing URL:\n%1"_s.arg( reply->request().url().toString() ) );

  QgsAuthSslErrorsDialog *dlg = new QgsAuthSslErrorsDialog( reply, errors, QgisApp::instance(), digest, hostport );
  dlg->setWindowModality( Qt::ApplicationModal );
  dlg->resize( 580, 512 );
  if ( dlg->exec() )
  {
    QgsDebugMsgLevel( u"All SSL errors ignored for %1"_s.arg( hostport ), 2 );
    reply->ignoreSslErrors();
  }
  dlg->deleteLater();
}
