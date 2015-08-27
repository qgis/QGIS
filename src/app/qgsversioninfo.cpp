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

#include "qgsnetworkaccessmanager.h"

QgsVersionInfo::QgsVersionInfo( QObject *parent ) : QObject( parent )
{

}

void QgsVersionInfo::checkVersion()
{
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( QUrl( "https://qgis.org/version.txt" ) ) );
  connect( reply, SIGNAL( finished() ), this, SLOT( versionReplyFinished() ) );
}

bool QgsVersionInfo::newVersionAvailable() const
{
  return mLatestVersion > QGis::QGIS_VERSION_INT;
}

bool QgsVersionInfo::isDevelopmentVersion() const
{
  return QGis::QGIS_VERSION_INT > mLatestVersion;
}

void QgsVersionInfo::versionReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );
  Q_ASSERT( reply );

  mError = reply->error();
  mErrorString = reply->errorString();

  if ( mError == QNetworkReply::NoError )
  {
    QString versionMessage = reply->readAll();

    // strip the header
    QString contentFlag = "#QGIS Version";
    int pos = versionMessage.indexOf( contentFlag );

    if ( pos > -1 )
    {
      pos += contentFlag.length();

      versionMessage = versionMessage.mid( pos );
      QStringList parts = versionMessage.split( "|", QString::SkipEmptyParts );
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
      mErrorString = tr( "The host name qgis.org could not be resolved. Check your DNS settings or contact your system administrator." );
      break;
    case QNetworkReply::NoError:
      mErrorString = "";
      break;
    default:
      mErrorString = reply->errorString();
      break;
  }

  reply->deleteLater();

  emit versionInfoAvailable();
}
