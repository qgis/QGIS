/***************************************************************************
    qgsaicloudindexclient.cpp
    -------------------------
    begin                : July 2026
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

#include "qgsaicloudindexclient.h"

#include <algorithm>

#include "qgsnetworkaccessmanager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QUrl>

#include "moc_qgsaicloudindexclient.cpp"

using namespace Qt::StringLiterals;

namespace
{
  constexpr int FETCH_TIMEOUT_MS = 30000;
  constexpr int MAX_CONTEXT_BATCH = 128;
  constexpr qint64 MAX_RULE_OR_SKILL_BYTES = 16 * 1024;

  QString normalizedRelativePath( const QString &path )
  {
    QString out = QDir::cleanPath( path.trimmed() );
    out.replace( '\\', '/' );
    while ( out.startsWith( "./"_L1 ) )
      out = out.mid( 2 );
    return out;
  }

  bool isRelativeMetadataPath( const QString &path )
  {
    if ( path.isEmpty() )
      return true;
    if ( QDir::isAbsolutePath( path ) )
      return false;
    return !path.contains( "://"_L1 ) && !path.startsWith( "../"_L1 ) && path != ".."_L1;
  }

  QString responseErrorMessage( QNetworkReply *reply, const QByteArray &body )
  {
    const int httpStatus = reply ? reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() : 0;
    const QJsonObject root = QJsonDocument::fromJson( body ).object();
    const QJsonValue error = root.value( u"error"_s );
    QString message = root.value( u"message"_s ).toString();
    if ( message.isEmpty() && error.isObject() )
      message = error.toObject().value( u"message"_s ).toString();
    if ( message.isEmpty() && error.isString() )
      message = error.toString();
    if ( message.isEmpty() && reply )
      message = reply->errorString();
    return httpStatus > 0 ? QObject::tr( "Strata Cloud index request failed (HTTP %1): %2" ).arg( httpStatus ).arg( message ) : message;
  }

  QUrl apiUrl( const QString &apiBase, const QString &path )
  {
    return QUrl( apiBase + path );
  }

  void setJsonHeaders( QNetworkRequest &request, const QString &sessionToken )
  {
    request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
    request.setRawHeader( "Accept", "application/json" );
    request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
    request.setTransferTimeout( FETCH_TIMEOUT_MS );
  }

  QString classifyFileSourceType( const QString &relativePath )
  {
    const QString path = normalizedRelativePath( relativePath );
    const QString lower = path.toLower();
    if ( lower.startsWith( ".strata/rules/"_L1 ) )
      return u"rule"_s;
    if ( lower.startsWith( ".strata/skills/"_L1 ) )
      return u"skill"_s;

    const QString suffix = QFileInfo( path ).suffix().toLower();
    if ( suffix == "pdf"_L1 )
      return u"pdf"_s;
    static const QSet<QString> imageSuffixes = { u"png"_s, u"jpg"_s, u"jpeg"_s, u"webp"_s, u"tif"_s, u"tiff"_s };
    if ( imageSuffixes.contains( suffix ) )
      return u"image"_s;
    return QString();
  }

  QString mimeTypeForPath( const QString &relativePath )
  {
    const QString suffix = QFileInfo( relativePath ).suffix().toLower();
    if ( suffix == "pdf"_L1 )
      return u"application/pdf"_s;
    if ( suffix == "png"_L1 )
      return u"image/png"_s;
    if ( suffix == "jpg"_L1 || suffix == "jpeg"_L1 )
      return u"image/jpeg"_s;
    if ( suffix == "webp"_L1 )
      return u"image/webp"_s;
    if ( suffix == "tif"_L1 || suffix == "tiff"_L1 )
      return u"image/tiff"_s;
    return QString();
  }

  QString sanitizedLayerText( const QgsAiWorkspaceIndex::Chunk &chunk )
  {
    QStringList lines;
    lines << u"Layer metadata snapshot; sensitive spatial payload omitted."_s;
    const QStringList rawLines = chunk.text.split( '\n' );
    for ( QString line : rawLines )
    {
      line = line.trimmed();
      if ( line.isEmpty() )
        continue;
      const QString lower = line.toLower();
      if ( lower.startsWith( '[' ) )
        continue;
      if ( lower.contains( "bbox="_L1 ) || lower.contains( "extent="_L1 ) )
        continue;
      lines << line;
    }
    if ( !chunk.relativePath.isEmpty() )
      lines << u"title=%1"_s.arg( chunk.relativePath );
    if ( chunk.firstFeatureId >= 0 || chunk.lastFeatureId >= 0 )
      lines << u"feature_id_range=%1..%2"_s.arg( chunk.firstFeatureId ).arg( chunk.lastFeatureId );
    return lines.join( '\n' ).trimmed();
  }

  bool textLooksLikeSensitiveSpatialPayload( const QString &text )
  {
    const QString value = text.trimmed();
    if ( value.isEmpty() )
      return false;

    const QString lower = value.toLower();
    if ( lower.contains( "datasourceuri"_L1 ) || lower.contains( "datasource uri"_L1 ) || lower.contains( "sourceuri"_L1 ) || lower.contains( "source uri"_L1 ) )
      return true;
    if ( lower.contains( "wkt="_L1 ) || lower.contains( "wkt:"_L1 ) || lower.contains( "bbox="_L1 ) || lower.contains( "extent="_L1 ) )
      return true;

    static const QRegularExpression wktRe( u"\\b(point|linestring|polygon|multipoint|multilinestring|multipolygon|geometrycollection)\\s*\\("_s, QRegularExpression::CaseInsensitiveOption );
    if ( wktRe.match( value ).hasMatch() )
      return true;

    static const QRegularExpression coordinateTupleRe( u"\\([-+]?\\d{1,3}(?:\\.\\d+)?\\s*,\\s*[-+]?\\d{1,3}(?:\\.\\d+)?(?:\\s*,\\s*[-+]?\\d{1,3}(?:\\.\\d+)?){0,2}\\)"_s );
    return coordinateTupleRe.match( value ).hasMatch();
  }

  QJsonObject itemJson( const QgsAiCloudIndexClient::ContextItem &item )
  {
    QJsonObject out;
    out.insert( u"sourceType"_s, item.sourceType );
    if ( !item.path.isEmpty() )
      out.insert( u"path"_s, normalizedRelativePath( item.path ) );
    if ( !item.layerId.isEmpty() )
      out.insert( u"layerId"_s, item.layerId );
    if ( !item.title.isEmpty() )
      out.insert( u"title"_s, item.title );
    if ( !item.mimeType.isEmpty() )
      out.insert( u"mimeType"_s, item.mimeType );
    if ( !item.text.isEmpty() )
      out.insert( u"text"_s, item.text );
    if ( !item.ocrText.isEmpty() )
      out.insert( u"ocrText"_s, item.ocrText );
    if ( !item.caption.isEmpty() )
      out.insert( u"caption"_s, item.caption );
    if ( item.chunkIndex >= 0 )
      out.insert( u"chunkIndex"_s, item.chunkIndex );
    return out;
  }

  void appendFolderItems( QList<QgsAiCloudIndexClient::ContextItem> &items, const QString &workspaceRoot, const QString &relativeDir, const QString &sourceType )
  {
    const QString root = workspaceRoot.trimmed().isEmpty() ? QString() : QDir( workspaceRoot ).absolutePath();
    if ( root.isEmpty() || relativeDir.trimmed().isEmpty() )
      return;

    const QDir rootDir( root );
    const QString absolutePath = QDir::cleanPath( rootDir.filePath( relativeDir ) );
    const QString relativeBase = normalizedRelativePath( rootDir.relativeFilePath( absolutePath ) );
    if ( !isRelativeMetadataPath( relativeBase ) )
      return;

    const QFileInfo dirInfo( absolutePath );
    if ( !dirInfo.exists() || !dirInfo.isDir() )
      return;

    QDir dir( absolutePath );
    const QStringList filters = { u"*.md"_s, u"*.markdown"_s, u"*.txt"_s };
    const QFileInfoList files = dir.entryInfoList( filters, QDir::Files | QDir::Readable, QDir::Name );
    for ( const QFileInfo &fileInfo : files )
    {
      QFile file( fileInfo.absoluteFilePath() );
      if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        continue;

      QByteArray raw = file.read( MAX_RULE_OR_SKILL_BYTES + 1 );
      if ( raw.size() > MAX_RULE_OR_SKILL_BYTES )
        raw.truncate( MAX_RULE_OR_SKILL_BYTES );
      QString text = QString::fromUtf8( raw ).trimmed();
      if ( text.isEmpty() )
        continue;

      QgsAiCloudIndexClient::ContextItem item;
      item.sourceType = sourceType;
      item.path = normalizedRelativePath( rootDir.relativeFilePath( fileInfo.absoluteFilePath() ) );
      item.title = fileInfo.fileName();
      item.text = text;
      items << item;
    }
  }
} //namespace

QgsAiCloudIndexClient::QgsAiCloudIndexClient( QObject *parent )
  : QObject( parent )
{
  qRegisterMetaType<QgsAiCloudIndexClient::ContextItem>();
  qRegisterMetaType<QgsAiCloudIndexClient::SyncResult>();
}

QString QgsAiCloudIndexClient::apiBaseForChatEndpoint( const QString &chatEndpoint )
{
  QUrl url( chatEndpoint.trimmed() );
  if ( !url.isValid() || url.scheme().isEmpty() || url.host().isEmpty() )
    return QString();

  url.setPath( QString() );
  url.setQuery( QString() );
  url.setFragment( QString() );
  QString base = url.toString();
  if ( base.endsWith( '/' ) )
    base.chop( 1 );
  return base;
}

QString QgsAiCloudIndexClient::workspaceFingerprint( const QString &workspaceRoot )
{
  const QString root = workspaceRoot.trimmed().isEmpty() ? QString() : QDir( workspaceRoot ).absolutePath();
  if ( root.isEmpty() )
    return QString();
  return QString::fromLatin1( QCryptographicHash::hash( root.toUtf8(), QCryptographicHash::Sha1 ).toHex() );
}

QList<QgsAiCloudIndexClient::ContextItem> QgsAiCloudIndexClient::contextItemsFromChunks( const QList<QgsAiWorkspaceIndex::Chunk> &chunks )
{
  QList<ContextItem> items;
  items.reserve( chunks.size() );
  for ( const QgsAiWorkspaceIndex::Chunk &chunk : chunks )
  {
    if ( chunk.sourceType == QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER ) )
    {
      ContextItem item;
      item.sourceType = u"layer"_s;
      item.layerId = chunk.layerId;
      item.title = chunk.relativePath;
      item.chunkIndex = chunk.chunkIndex;
      item.text = sanitizedLayerText( chunk );
      if ( !item.text.isEmpty() )
        items << item;
      continue;
    }

    const QString path = normalizedRelativePath( chunk.relativePath );
    const QString sourceType = classifyFileSourceType( path );
    if ( sourceType.isEmpty() )
      continue;

    ContextItem item;
    item.sourceType = sourceType;
    item.path = path;
    item.title = QFileInfo( path ).fileName();
    item.mimeType = mimeTypeForPath( path );
    item.chunkIndex = chunk.chunkIndex;
    item.text = chunk.text;
    if ( !item.text.isEmpty() )
      items << item;
  }
  return deduplicateContextItems( items );
}

QList<QgsAiCloudIndexClient::ContextItem> QgsAiCloudIndexClient::contextItemsFromWorkspaceFolders( const QString &workspaceRoot, const QString &rulesPath, const QString &skillsPath )
{
  QList<ContextItem> items;
  appendFolderItems( items, workspaceRoot, rulesPath, u"rule"_s );
  appendFolderItems( items, workspaceRoot, skillsPath, u"skill"_s );
  return deduplicateContextItems( items );
}

QList<QgsAiCloudIndexClient::ContextItem> QgsAiCloudIndexClient::deduplicateContextItems( const QList<ContextItem> &items )
{
  QList<ContextItem> out;
  QSet<QString> seen;
  for ( const ContextItem &item : items )
  {
    const QString key = item.sourceType + QChar( 0x1f ) + item.path + QChar( 0x1f ) + item.layerId + QChar( 0x1f ) + QString::number( item.chunkIndex );
    if ( seen.contains( key ) )
      continue;
    seen.insert( key );
    out << item;
  }
  return out;
}

bool QgsAiCloudIndexClient::containsForbiddenPayload( const ContextItem &item )
{
  if ( item.path.contains( "://"_L1 ) )
    return true;
  if ( !isRelativeMetadataPath( normalizedRelativePath( item.path ) ) )
    return true;

  const QStringList textFields = { item.path, item.title, item.mimeType, item.text, item.ocrText, item.caption };
  for ( const QString &field : textFields )
  {
    if ( textLooksLikeSensitiveSpatialPayload( field ) )
      return true;
  }
  return false;
}

bool QgsAiCloudIndexClient::validateContextItems( const QList<ContextItem> &items, QString *errorMessage )
{
  if ( items.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = tr( "No safe context items are available to sync." );
    return false;
  }

  static const QSet<QString> allowedSources = { u"layer"_s, u"pdf"_s, u"image"_s, u"rule"_s, u"skill"_s };
  for ( const ContextItem &item : items )
  {
    if ( !allowedSources.contains( item.sourceType ) )
    {
      if ( errorMessage )
        *errorMessage = tr( "Unsupported cloud context source type: %1" ).arg( item.sourceType );
      return false;
    }
    if ( item.text.trimmed().isEmpty() && item.ocrText.trimmed().isEmpty() && item.caption.trimmed().isEmpty() )
    {
      if ( errorMessage )
        *errorMessage = tr( "Cloud context item %1 has no text, OCR text, or caption." ).arg( item.path.isEmpty() ? item.title : item.path );
      return false;
    }
    if ( containsForbiddenPayload( item ) )
    {
      if ( errorMessage )
        *errorMessage = tr( "Cloud context item %1 contains geometry, coordinates, WKT, or datasource URI data." ).arg( item.path.isEmpty() ? item.title : item.path );
      return false;
    }
  }
  return true;
}

void QgsAiCloudIndexClient::syncWorkspaceContext(
  const QString &chatEndpoint, const QString &sessionToken, const QString &workspaceRoot, const QString &workspaceName, const QList<ContextItem> &items, bool contentOptIn
)
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  if ( apiBase.isEmpty() )
  {
    emit requestFailed( tr( "Plan backend endpoint is not configured." ) );
    return;
  }
  if ( sessionToken.trimmed().isEmpty() )
  {
    emit requestFailed( tr( "Plan Account session token is missing." ) );
    return;
  }
  if ( !contentOptIn )
  {
    emit requestFailed( tr( "Cloud context indexing requires explicit content opt-in." ) );
    return;
  }

  const QString fingerprint = workspaceFingerprint( workspaceRoot );
  if ( fingerprint.isEmpty() )
  {
    emit requestFailed( tr( "Workspace root is unset; cannot sync cloud context." ) );
    return;
  }

  QString validationError;
  const QList<ContextItem> safeItems = deduplicateContextItems( items );
  if ( !validateContextItems( safeItems, &validationError ) )
  {
    emit requestFailed( validationError );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  const QString normalizedRoot = QDir( workspaceRoot ).absolutePath();
  QString name = workspaceName.trimmed();
  if ( name.isEmpty() )
    name = QFileInfo( normalizedRoot ).fileName();
  if ( name.isEmpty() )
    name = tr( "Strata workspace" );

  QJsonObject settings;
  settings.insert( u"client"_s, u"strata-desktop"_s );
  settings.insert( u"contextSync"_s, true );

  QJsonObject body;
  body.insert( u"fingerprint"_s, fingerprint );
  body.insert( u"name"_s, name );
  body.insert( u"settings"_s, settings );

  QNetworkRequest request( apiUrl( apiBase, u"/v1/workspaces"_s ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud workspace request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, apiBase, token = sessionToken.trimmed(), safeItems, contentOptIn]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }

    const QString workspaceId = QJsonDocument::fromJson( responseBody ).object().value( u"id"_s ).toString();
    if ( workspaceId.isEmpty() )
    {
      emit requestFailed( tr( "Strata Cloud workspace response did not include an id." ) );
      return;
    }

    postContextBatch( apiBase, token, workspaceId, safeItems, 0, 0, 0, contentOptIn );
  } );
}

void QgsAiCloudIndexClient::postContextBatch(
  const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const QList<ContextItem> &items, int offset, int accumulatedUpserted, int accumulatedQueued, bool contentOptIn
)
{
  if ( offset >= items.size() )
  {
    emit contextSynced( SyncResult { workspaceId, accumulatedUpserted, accumulatedQueued } );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QJsonArray batchItems;
  const int end = std::min<int>( offset + MAX_CONTEXT_BATCH, items.size() );
  for ( int i = offset; i < end; ++i )
    batchItems << itemJson( items.at( i ) );

  QJsonObject body;
  body.insert( u"contentOptIn"_s, contentOptIn );
  body.insert( u"items"_s, batchItems );

  const QString encodedWorkspaceId = QString::fromLatin1( QUrl::toPercentEncoding( workspaceId ) );
  QNetworkRequest request( apiUrl( apiBase, u"/v1/index/%1/context"_s.arg( encodedWorkspaceId ) ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud context index request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, apiBase, sessionToken, workspaceId, items, end, accumulatedUpserted, accumulatedQueued, contentOptIn]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }

    const QJsonObject root = QJsonDocument::fromJson( responseBody ).object();
    const int upserted = accumulatedUpserted + root.value( u"upserted"_s ).toInt();
    const int queued = accumulatedQueued + root.value( u"queued"_s ).toInt();
    postContextBatch( apiBase, sessionToken, workspaceId, items, end, upserted, queued, contentOptIn );
  } );
}
