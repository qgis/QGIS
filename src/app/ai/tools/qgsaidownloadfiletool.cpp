/***************************************************************************
    qgsaidownloadfiletool.cpp
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaidownloadfiletool.h"

#include "qgsaiauditlog.h"
#include "qgsaifilecontextprovider.h"
#include "qgsaitoolschemautil.h"
#include "qgsaiworkspacetrust.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace
{
  QString humanBytes( qint64 bytes )
  {
    if ( bytes < 1024 )
      return u"%1 B"_s.arg( bytes );
    const double kb = bytes / 1024.0;
    if ( kb < 1024 )
      return u"%1 KiB"_s.arg( kb, 0, 'f', 1 );
    const double mb = kb / 1024.0;
    if ( mb < 1024 )
      return u"%1 MiB"_s.arg( mb, 0, 'f', 1 );
    return u"%1 GiB"_s.arg( mb / 1024.0, 0, 'f', 2 );
  }

  bool fullDownloadLogDetails()
  {
    QgsSettings settings;
    if ( settings.contains( u"strata/log/full_tool_details"_s ) )
      return settings.value( u"strata/log/full_tool_details"_s, false ).toBool();
    return settings.value( u"geoai/log/full_tool_details"_s, false ).toBool();
  }

  QString urlForLog( const QUrl &url )
  {
    if ( fullDownloadLogDetails() )
      return url.toString();

    QString path = url.path();
    if ( path.size() > 80 )
      path = path.left( 77 ) + u"..."_s;
    return u"%1://%2%3"_s.arg( url.scheme(), url.host(), path );
  }

  QString pathForLog( QgsAiFileContextProvider *contextProvider, const QString &absolutePath )
  {
    if ( fullDownloadLogDetails() || !contextProvider || contextProvider->workspaceRoot().isEmpty() )
      return absolutePath;

    return QDir( contextProvider->workspaceRoot() ).relativeFilePath( absolutePath );
  }

  bool isLocalHttpHost( const QString &host )
  {
    const QString normalized = host.trimmed().toLower();
    return normalized == "localhost"_L1 || normalized == "127.0.0.1"_L1 || normalized == "::1"_L1 || normalized.startsWith( "127."_L1 );
  }
} //namespace

QgsAiDownloadFileTool::QgsAiDownloadFileTool( QgsAiFileContextProvider *contextProvider, QWidget *dialogParent )
  : mContextProvider( contextProvider )
  , mDialogParent( dialogParent )
{}

bool QgsAiDownloadFileTool::isAvailable() const
{
  return mContextProvider && !mContextProvider->workspaceRoot().isEmpty() && QgsAiWorkspaceTrust::isTrusted( mContextProvider->workspaceRoot() );
}

QString QgsAiDownloadFileTool::availabilityReason() const
{
  if ( !mContextProvider || mContextProvider->workspaceRoot().isEmpty() )
    return u"download_file is not available because the AI workspace root is not configured. Save the QGIS project or set strata/workspace/root before downloading files."_s;
  return u"download_file is disabled because this workspace is not trusted. Trust the workspace from the AI provider settings to enable downloads."_s;
}

QString QgsAiDownloadFileTool::description() const
{
  return QStringLiteral(
    "Downloads a file from an HTTPS URL and saves it inside the workspace. The user "
    "must approve via a modal dialog showing the URL and destination before the request "
    "starts; refusal returns 'user_rejected'. Use this to fetch remote data (GeoJSON, "
    "Shapefile, Overpass/Nominatim/GADM responses, etc.) without installing extra Python "
    "packages. dest_path is relative to the workspace; writes outside the workspace are "
    "refused. By default the tool refuses to overwrite an existing file (set overwrite=true "
    "to override). Default size limit is 100 MiB. Returns sha256 and provenance metadata."
  );
}

QJsonObject QgsAiDownloadFileTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"url"_s, prop( u"string"_s, u"The HTTPS URL to fetch. Public HTTP URLs are refused; localhost HTTP is allowed for development."_s ) );
  properties.insert( u"dest_path"_s, prop( u"string"_s, u"Destination path, relative to the workspace root (e.g. 'data/pomponesco.geojson'). Writes outside the workspace are refused."_s ) );
  properties.insert( u"expected_sha256"_s, prop( u"string"_s, u"Optional expected SHA-256 hex digest. If supplied, the file is deleted and the tool fails on mismatch."_s ) );
  QJsonObject maxBytesProp;
  maxBytesProp.insert( u"type"_s, u"integer"_s );
  maxBytesProp.insert( u"description"_s, u"Optional cap on response size in bytes. Default 104857600 (100 MiB). Hard ceiling 2 GiB."_s );
  properties.insert( u"max_bytes"_s, maxBytesProp );
  QJsonObject overwriteProp;
  overwriteProp.insert( u"type"_s, u"boolean"_s );
  overwriteProp.insert( u"description"_s, u"If true, overwrite an existing destination file. Default false."_s );
  properties.insert( u"overwrite"_s, overwriteProp );
  return schemaObject( properties, QJsonArray { u"url"_s, u"dest_path"_s } );
}

QgsAiToolResult QgsAiDownloadFileTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( u"Workspace context provider is not available."_s );
  if ( !isAvailable() )
    return QgsAiToolResult::error( availabilityReason() );

  const QString urlString = args.value( u"url"_s ).toString().trimmed();
  if ( urlString.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'url' is required."_s );

  const QUrl url( urlString, QUrl::StrictMode );
  if ( !url.isValid() )
    return QgsAiToolResult::error( u"URL is not valid: '%1'."_s.arg( urlString ) );
  const QString scheme = url.scheme().toLower();
  if ( scheme != "http"_L1 && scheme != "https"_L1 )
    return QgsAiToolResult::error( u"Only http and https URLs are allowed (got '%1')."_s.arg( scheme ) );
  if ( scheme == "http"_L1 && !isLocalHttpHost( url.host() ) )
    return QgsAiToolResult::error( u"Public HTTP downloads are refused. Use an HTTPS URL or localhost for development."_s );

  const QString destRequest = args.value( u"dest_path"_s ).toString().trimmed();
  if ( destRequest.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'dest_path' is required."_s );

  if ( mContextProvider->workspaceRoot().isEmpty() )
    return QgsAiToolResult::error( u"AI workspace root is not configured. Save the QGIS project or set strata/workspace/root before downloading files."_s );

  const QString destPath = mContextProvider->normalizePath( destRequest, /*allowExternal=*/false );
  if ( destPath.isEmpty() )
    return QgsAiToolResult::error( u"Destination path is outside the workspace: '%1'."_s.arg( destRequest ) );

  const bool overwrite = args.value( u"overwrite"_s ).toBool( false );
  if ( !overwrite && QFileInfo::exists( destPath ) )
    return QgsAiToolResult::error( u"Destination file already exists: '%1'. Pass overwrite=true to replace it."_s.arg( destPath ) );

  const QString expectedSha256 = args.value( u"expected_sha256"_s ).toString().trimmed().toLower();
  if ( !expectedSha256.isEmpty() && expectedSha256.size() != 64 )
    return QgsAiToolResult::error( u"expected_sha256 must be a 64-character SHA-256 hex digest."_s );

  qint64 maxBytes = DEFAULT_MAX_BYTES;
  const QJsonValue maxBytesValue = args.value( u"max_bytes"_s );
  if ( maxBytesValue.isDouble() )
  {
    const qint64 requested = static_cast<qint64>( maxBytesValue.toDouble() );
    if ( requested <= 0 )
      return QgsAiToolResult::error( u"max_bytes must be a positive integer."_s );
    if ( requested > ABSOLUTE_MAX_BYTES )
      return QgsAiToolResult::error( u"max_bytes %1 exceeds the absolute ceiling %2."_s.arg( requested ).arg( ABSOLUTE_MAX_BYTES ) );
    maxBytes = requested;
  }

  // Modal approval. v1 uses a QMessageBox::question — not as polished as a dedicated
  // dialog but the pertinent fields (URL, dest, cap) are right there in the prompt.
  {
    QString question = QObject::tr(
                         "The AI assistant wants to download:\n\n"
                         "  URL: %1\n"
                         "  Destination: %2\n"
                         "  Max size: %3\n"
    )
                         .arg( url.toString( QUrl::FullyEncoded ), destPath, humanBytes( maxBytes ) );

    // Informative warning when the destination looks executable.
    static const QStringList executableSuffixes
      = { u"exe"_s, u"bat"_s, u"cmd"_s, u"sh"_s, u"ps1"_s, u"py"_s, u"pyw"_s, u"dylib"_s, u"so"_s, u"dll"_s, u"app"_s, u"jar"_s, u"com"_s, u"scr"_s, u"msi"_s, u"vbs"_s, u"command"_s };
    const QString suffix = QFileInfo( destPath ).suffix().toLower();
    if ( executableSuffixes.contains( suffix ) )
      question += QObject::tr( "\nWarning: the destination has an executable file extension (.%1). Only allow this if you expect to download a program.\n" ).arg( suffix );

    question += QObject::tr( "\nAllow the download?" );
    QMessageBox::StandardButton answer = QMessageBox::question( mDialogParent, QObject::tr( "AI: confirm file download" ), question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
    if ( answer != QMessageBox::Yes )
    {
      QgsMessageLog::logMessage( u"download_file rejected by user (url=%1)"_s.arg( urlForLog( url ) ), u"AI/Download"_s, Qgis::MessageLevel::Info, false );
      QJsonObject output;
      output.insert( u"status"_s, u"user_rejected"_s );
      return QgsAiToolResult::ok( output );
    }
    QgsAiAuditLog::append( u"download_file"_s, u"%1 -> %2"_s.arg( url.toString( QUrl::FullyEncoded ), destPath ) );
  }

  // Make sure the destination directory exists (relative paths into a fresh workspace
  // are common, e.g. data/foo.geojson when 'data/' doesn't exist yet).
  {
    QFileInfo destInfo( destPath );
    QDir parentDir = destInfo.dir();
    if ( !parentDir.exists() )
    {
      if ( !parentDir.mkpath( u"."_s ) )
        return QgsAiToolResult::error( u"Cannot create destination directory: '%1'."_s.arg( parentDir.absolutePath() ) );
    }
  }

  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  if ( !nam )
    return QgsAiToolResult::error( u"Network manager is not available."_s );

  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );
  request.setTransferTimeout( TRANSFER_TIMEOUT_MS );

  QgsMessageLog::
    logMessage( u"download_file: starting (url=%1, dest=%2, maxBytes=%3)"_s.arg( urlForLog( url ), pathForLog( mContextProvider, destPath ), QString::number( maxBytes ) ), u"AI/Download"_s, Qgis::MessageLevel::Info, false );

  QNetworkReply *reply = nam->get( request );
  if ( !reply )
    return QgsAiToolResult::error( u"Unable to start network request."_s );

  QFile outFile( destPath );
  if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    reply->abort();
    reply->deleteLater();
    return QgsAiToolResult::error( u"Cannot open destination for writing: '%1'."_s.arg( destPath ) );
  }

  qint64 bytesWritten = 0;
  bool exceededLimit = false;
  QCryptographicHash sha256( QCryptographicHash::Sha256 );

  // Stream the body to disk as it arrives so we never hold the full payload in RAM.
  QObject::connect( reply, &QNetworkReply::readyRead, reply, [&]() {
    if ( exceededLimit )
      return;
    const QByteArray chunk = reply->readAll();
    if ( bytesWritten + chunk.size() > maxBytes )
    {
      exceededLimit = true;
      reply->abort();
      return;
    }
    if ( !chunk.isEmpty() )
    {
      outFile.write( chunk );
      sha256.addData( chunk );
      bytesWritten += chunk.size();
    }
  } );

  // Overall timeout (in addition to setTransferTimeout, which is per-stall).
  QEventLoop loop;
  QTimer overallTimer;
  overallTimer.setSingleShot( true );
  bool overallTimedOut = false;
  QObject::connect( &overallTimer, &QTimer::timeout, &loop, [&]() {
    overallTimedOut = true;
    reply->abort();
    loop.quit();
  } );
  QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
  overallTimer.start( OVERALL_TIMEOUT_MS );
  loop.exec();
  if ( overallTimer.isActive() )
    overallTimer.stop();

  // Drain any tail bytes that may have arrived between the last readyRead and finished.
  if ( !exceededLimit && reply->bytesAvailable() > 0 )
  {
    const QByteArray tail = reply->readAll();
    if ( bytesWritten + tail.size() > maxBytes )
      exceededLimit = true;
    else
    {
      outFile.write( tail );
      sha256.addData( tail );
      bytesWritten += tail.size();
    }
  }

  outFile.close();

  const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QUrl finalUrl = reply->url();
  const QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
  const QNetworkReply::NetworkError networkError = reply->error();
  const QString networkErrorString = reply->errorString();
  reply->deleteLater();

  const bool httpOk = httpStatus >= 200 && httpStatus < 300;
  const bool success = !exceededLimit && !overallTimedOut && networkError == QNetworkReply::NoError && httpOk;

  if ( !success )
  {
    QFile::remove( destPath );
    QString reasonText;
    if ( exceededLimit )
      reasonText = u"size cap of %1 bytes exceeded"_s.arg( maxBytes );
    else if ( overallTimedOut )
      reasonText = u"overall timeout after %1 ms"_s.arg( OVERALL_TIMEOUT_MS );
    else if ( !httpOk && networkError == QNetworkReply::NoError )
      reasonText = u"HTTP %1"_s.arg( httpStatus );
    else
      reasonText = u"network error: %1"_s.arg( networkErrorString );

    QgsMessageLog::logMessage( u"download_file failed (url=%1, reason=%2)"_s.arg( urlForLog( url ), reasonText ), u"AI/Download"_s, Qgis::MessageLevel::Warning, false );

    return QgsAiToolResult::error( reasonText );
  }

  const QString sha256Hex = QString::fromLatin1( sha256.result().toHex() );
  if ( !expectedSha256.isEmpty() && sha256Hex != expectedSha256 )
  {
    QFile::remove( destPath );
    return QgsAiToolResult::error( u"Downloaded file SHA-256 mismatch: expected %1, got %2."_s.arg( expectedSha256, sha256Hex ) );
  }

  QgsMessageLog::
    logMessage( u"download_file ok (url=%1, dest=%2, bytes=%3)"_s.arg( urlForLog( url ), pathForLog( mContextProvider, destPath ), QString::number( bytesWritten ) ), u"AI/Download"_s, Qgis::MessageLevel::Info, false );

  QJsonObject output;
  output.insert( u"status"_s, u"ok"_s );
  output.insert( u"http_status"_s, httpStatus );
  output.insert( u"bytes_written"_s, bytesWritten );
  output.insert( u"dest_path"_s, destPath );
  output.insert( u"content_type"_s, contentType );
  output.insert( u"sha256"_s, sha256Hex );
  output.insert( u"source_host"_s, finalUrl.host().toLower() );
  output.insert( u"final_url"_s, finalUrl.toString( QUrl::FullyEncoded ) );
  output.insert( u"trust_level"_s, u"unverified"_s );
  return QgsAiToolResult::ok( output );
}
