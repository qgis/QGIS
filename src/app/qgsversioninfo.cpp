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

#include "qgsconfig.h"
#include "qgsversioninfo.h"

#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QString>
#include <QUrl>

#include "moc_qgsversioninfo.cpp"

using namespace Qt::StringLiterals;

QgsVersionInfo::QgsVersionInfo( QObject *parent )
  : QObject( parent )
{}

void QgsVersionInfo::checkVersion()
{
  QNetworkRequest request( QUrl( u"https://api.github.com/repos/francemazzi/strata/releases?per_page=20"_s ) );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  request.setRawHeader( "Accept", "application/vnd.github+json" );
  request.setHeader( QNetworkRequest::UserAgentHeader, u"Strata/%1"_s.arg( QString::fromUtf8( STRATA_VERSION ) ) );
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, &QNetworkReply::finished, this, &QgsVersionInfo::versionReplyFinished );
}

bool QgsVersionInfo::newVersionAvailable() const
{
  return mLatestVersion > STRATA_VERSION_INT;
}

bool QgsVersionInfo::isDevelopmentVersion() const
{
  return mLatestVersion > 0 && STRATA_VERSION_INT > mLatestVersion;
}

int QgsVersionInfo::versionCodeFromString( const QString &versionString, bool *ok )
{
  static const QRegularExpression versionRegex( u"^([0-9]+)\\.([0-9]+)\\.([0-9]+)$"_s );
  const QRegularExpressionMatch match = versionRegex.match( versionString.trimmed() );
  if ( !match.hasMatch() )
  {
    if ( ok )
      *ok = false;
    return 0;
  }

  if ( ok )
    *ok = true;

  return match.captured( 1 ).toInt() * 10000 + match.captured( 2 ).toInt() * 100 + match.captured( 3 ).toInt();
}

int QgsVersionInfo::versionCodeFromTag( const QString &tagName, bool *ok )
{
  static const QRegularExpression tagRegex( u"^strata-v([0-9]+\\.[0-9]+\\.[0-9]+)$"_s );
  const QRegularExpressionMatch match = tagRegex.match( tagName.trimmed() );
  if ( !match.hasMatch() )
  {
    if ( ok )
      *ok = false;
    return 0;
  }

  return versionCodeFromString( match.captured( 1 ), ok );
}

bool QgsVersionInfo::releaseDetailsFromGitHubReleases( const QByteArray &content, ReleaseDetails &details, QString *errorString )
{
  details = ReleaseDetails();

  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson( content, &parseError );
  if ( parseError.error != QJsonParseError::NoError || !document.isArray() )
  {
    if ( errorString )
      *errorString = parseError.error != QJsonParseError::NoError ? parseError.errorString() : tr( "GitHub releases response was not a JSON array." );
    return false;
  }

  const QJsonArray releases = document.array();
  for ( const QJsonValue &value : releases )
  {
    const QJsonObject release = value.toObject();
    if ( release.value( u"draft"_s ).toBool() || release.value( u"prerelease"_s ).toBool() )
      continue;

    const QString tagName = release.value( u"tag_name"_s ).toString();
    bool ok = false;
    const int versionCode = versionCodeFromTag( tagName, &ok );
    if ( !ok || versionCode <= details.versionCode )
      continue;

    details.versionCode = versionCode;
    details.version = tagName.mid( u"strata-v"_s.size() );
    details.url = release.value( u"html_url"_s ).toString();
    details.body = release.value( u"body"_s ).toString();
  }

  if ( details.versionCode == 0 )
  {
    if ( errorString )
      *errorString = tr( "No stable Strata releases were found on GitHub." );
    return false;
  }

  return true;
}

void QgsVersionInfo::versionReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  Q_ASSERT( reply );

  mError = reply->error();
  mErrorString = reply->errorString();

  if ( mError == QNetworkReply::NoError )
  {
    ReleaseDetails details;
    QString parseError;
    if ( releaseDetailsFromGitHubReleases( reply->readAll(), details, &parseError ) )
    {
      mLatestVersion = details.versionCode;
      mLatestVersionString = details.version;
      mReleaseUrl = details.url;
      mDownloadInfo = tr( "Download Strata %1 from %2" ).arg( mLatestVersionString, mReleaseUrl );
      mAdditionalHtml = details.body.toHtmlEscaped().replace( '\n', "<br>"_L1 );
    }
    else
    {
      mError = QNetworkReply::UnknownContentError;
      mErrorString = parseError;
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
      if ( mErrorString.isEmpty() )
        mErrorString = reply->errorString();
      break;
  }

  reply->deleteLater();

  emit versionInfoAvailable();
}
