/***************************************************************************

               ----------------------------------------------------
              date                 : 18.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsversioninfo.h"
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include <QUrl>

QgsVersionInfo::QgsVersionInfo( QObject *parent )
  : QObject( parent )
{

}

void QgsVersionInfo::checkVersion()
{
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( QUrl( QStringLiteral( "https://version.qgis.org/version.txt" ) ) ) );
  connect( reply, &QNetworkReply::finished, this, &QgsVersionInfo::versionReplyFinished );
}

bool QgsVersionInfo::newVersionAvailable() const
{
  return mLatestVersion > Qgis::versionInt();
}

bool QgsVersionInfo::isDevelopmentVersion() const
{
  return Qgis::versionInt() > mLatestVersion;
}

void QgsVersionInfo::versionReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  Q_ASSERT( reply );

  mError = reply->error();
  mErrorString = reply->errorString();

  if ( mError == QNetworkReply::NoError )
  {
    QString versionMessage = reply->readAll();

    // strip the header
    const QString contentFlag = QStringLiteral( "#QGIS Version" );
    int pos = versionMessage.indexOf( contentFlag );

    if ( pos > -1 )
    {
      pos += contentFlag.length();

      versionMessage = versionMessage.mid( pos );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      QStringList parts = versionMessage.split( '|', QString::SkipEmptyParts );
#else
      QStringList parts = versionMessage.split( '|', Qt::SkipEmptyParts );
#endif
      // check the version from the  server against our version
      mLatestVersion = parts[0].toInt();
      mDownloadInfo = parts.value( 1 );
      mAdditionalHtml = parts.value( 2 );
    }
  }

  // get error type
  switch ( mError )
  {
    case QNetworkReply::ConnectionRefusedError:
      mErrorString = tr( "Connection refused - server may be down" );
      break;
    case QNetworkReply::HostNotFoundError:
      mErrorString = tr( "The host name %1 could not be resolved. Check your DNS settings or contact your system administrator." ).arg( reply->request().url().host() );
      break;
    case QNetworkReply::NoError:
      mErrorString.clear();
      break;
    default:
      mErrorString = reply->errorString();
      break;
  }

  reply->deleteLater();

  emit versionInfoAvailable();
}
