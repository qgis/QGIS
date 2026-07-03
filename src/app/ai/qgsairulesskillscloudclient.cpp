/***************************************************************************
    qgsairulesskillscloudclient.cpp
    ---------------------
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

#include "qgsairulesskillscloudclient.h"

#include "index/qgsaicloudindexclient.h"
#include "qgsnetworkaccessmanager.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

#include "moc_qgsairulesskillscloudclient.cpp"

using namespace Qt::StringLiterals;

namespace
{
  constexpr int REQUEST_TIMEOUT_MS = 20000;

  QUrl apiUrl( const QString &apiBase, const QString &path )
  {
    return QUrl( apiBase + path );
  }

  void setJsonHeaders( QNetworkRequest &request, const QString &sessionToken )
  {
    request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
    request.setRawHeader( "Accept", "application/json" );
    request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
    request.setTransferTimeout( REQUEST_TIMEOUT_MS );
  }

  QString responseErrorMessage( QNetworkReply *reply, const QByteArray &body )
  {
    const int httpStatus = reply ? reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() : 0;
    const QJsonObject root = QJsonDocument::fromJson( body ).object();
    QString message = root.value( u"message"_s ).toString();
    if ( message.isEmpty() && reply )
      message = reply->errorString();
    return httpStatus > 0 ? QObject::tr( "Strata Cloud rules/skills request failed (HTTP %1): %2" ).arg( httpStatus ).arg( message ) : message;
  }

  QString encodedId( const QString &id )
  {
    return QString::fromLatin1( QUrl::toPercentEncoding( id ) );
  }

  QJsonObject ruleJson( const QgsAiRulesSkillsCloudClient::RemoteRule &rule )
  {
    QJsonObject out;
    out.insert( u"slug"_s, rule.slug );
    out.insert( u"name"_s, rule.name );
    if ( !rule.description.isEmpty() )
      out.insert( u"description"_s, rule.description );
    if ( !rule.globs.isEmpty() )
      out.insert( u"globs"_s, QJsonArray::fromStringList( rule.globs ) );
    out.insert( u"alwaysApply"_s, rule.alwaysApply );
    out.insert( u"enabled"_s, rule.enabled );
    out.insert( u"content"_s, rule.content );
    return out;
  }

  QgsAiRulesSkillsCloudClient::RemoteRule ruleFromJson( const QJsonObject &obj )
  {
    QgsAiRulesSkillsCloudClient::RemoteRule rule;
    rule.id = obj.value( u"id"_s ).toString();
    rule.slug = obj.value( u"slug"_s ).toString();
    rule.name = obj.value( u"name"_s ).toString();
    rule.description = obj.value( u"description"_s ).toString();
    for ( const QJsonValue &value : obj.value( u"globs"_s ).toArray() )
      rule.globs << value.toString();
    rule.alwaysApply = obj.value( u"alwaysApply"_s ).toBool( true );
    rule.enabled = obj.value( u"enabled"_s ).toBool( true );
    rule.content = obj.value( u"content"_s ).toString();
    return rule;
  }

  QJsonObject skillJson( const QgsAiRulesSkillsCloudClient::RemoteSkill &skill )
  {
    QJsonObject out;
    out.insert( u"slug"_s, skill.slug );
    out.insert( u"name"_s, skill.name );
    out.insert( u"description"_s, skill.description );
    out.insert( u"enabled"_s, skill.enabled );
    out.insert( u"content"_s, skill.content );
    return out;
  }

  QgsAiRulesSkillsCloudClient::RemoteSkill skillFromJson( const QJsonObject &obj )
  {
    QgsAiRulesSkillsCloudClient::RemoteSkill skill;
    skill.id = obj.value( u"id"_s ).toString();
    skill.slug = obj.value( u"slug"_s ).toString();
    skill.name = obj.value( u"name"_s ).toString();
    skill.description = obj.value( u"description"_s ).toString();
    skill.enabled = obj.value( u"enabled"_s ).toBool( true );
    skill.content = obj.value( u"content"_s ).toString();
    return skill;
  }
} // namespace

QgsAiRulesSkillsCloudClient::QgsAiRulesSkillsCloudClient( QObject *parent )
  : QObject( parent )
{
  qRegisterMetaType<QgsAiRulesSkillsCloudClient::RemoteRule>();
  qRegisterMetaType<QgsAiRulesSkillsCloudClient::RemoteSkill>();
}

QgsAiRulesSkillsCloudClient::RemoteRule QgsAiRulesSkillsCloudClient::toRemoteRule( const QgsAiRuleInfo &info, const QString &content )
{
  RemoteRule rule;
  rule.slug = info.slug;
  rule.name = info.name;
  rule.description = info.description;
  rule.globs = info.globs;
  rule.alwaysApply = info.alwaysApply;
  rule.enabled = info.enabled;
  rule.content = content;
  return rule;
}

QgsAiRulesSkillsCloudClient::RemoteSkill QgsAiRulesSkillsCloudClient::toRemoteSkill( const QgsAiSkillInfo &info, const QString &content )
{
  RemoteSkill skill;
  skill.slug = info.slug;
  skill.name = info.name;
  skill.description = info.description;
  skill.enabled = info.enabled;
  skill.content = content;
  return skill;
}

void QgsAiRulesSkillsCloudClient::ensureWorkspace( const QString &apiBase, const QString &sessionToken, const QString &workspaceRoot, const QString &workspaceName )
{
  const QString fingerprint = QgsAiCloudIndexClient::workspaceFingerprint( workspaceRoot );
  if ( apiBase.isEmpty() || sessionToken.trimmed().isEmpty() || fingerprint.isEmpty() )
  {
    emit requestFailed( tr( "Strata Cloud sync requires a signed-in Plan Account and a configured workspace." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QString name = workspaceName.trimmed();
  if ( name.isEmpty() )
    name = QFileInfo( QDir( workspaceRoot ).absolutePath() ).fileName();
  if ( name.isEmpty() )
    name = tr( "Strata workspace" );

  QJsonObject body;
  body.insert( u"fingerprint"_s, fingerprint );
  body.insert( u"name"_s, name );

  QNetworkRequest request( apiUrl( apiBase, u"/v1/workspaces"_s ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud workspace request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
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
    emit workspaceReady( workspaceId );
  } );
}

void QgsAiRulesSkillsCloudClient::fetchRules( const QString &apiBase, const QString &sessionToken, const QString &workspaceId )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/workspaces/%1/rules"_s.arg( encodedId( workspaceId ) ) ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud rules request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }
    QList<RemoteRule> rules;
    for ( const QJsonValue &value : QJsonDocument::fromJson( responseBody ).object().value( u"items"_s ).toArray() )
      rules << ruleFromJson( value.toObject() );
    emit rulesFetched( rules );
  } );
}

void QgsAiRulesSkillsCloudClient::fetchSkills( const QString &apiBase, const QString &sessionToken, const QString &workspaceId )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/workspaces/%1/skills"_s.arg( encodedId( workspaceId ) ) ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud skills request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }
    QList<RemoteSkill> skills;
    for ( const QJsonValue &value : QJsonDocument::fromJson( responseBody ).object().value( u"items"_s ).toArray() )
      skills << skillFromJson( value.toObject() );
    emit skillsFetched( skills );
  } );
}

void QgsAiRulesSkillsCloudClient::pushRule( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const RemoteRule &rule )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  const bool isUpdate = !rule.id.isEmpty();
  const QString path = isUpdate ? u"/v1/workspaces/%1/rules/%2"_s.arg( encodedId( workspaceId ), encodedId( rule.id ) ) : u"/v1/workspaces/%1/rules"_s.arg( encodedId( workspaceId ) );
  QNetworkRequest request( apiUrl( apiBase, path ) );
  setJsonHeaders( request, sessionToken );
  const QByteArray payload = QJsonDocument( ruleJson( rule ) ).toJson( QJsonDocument::Compact );
  QNetworkReply *reply = isUpdate ? networkManager->sendCustomRequest( request, "PATCH", payload ) : networkManager->post( request, payload );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud rule sync request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }
    emit ruleSynced( ruleFromJson( QJsonDocument::fromJson( responseBody ).object() ) );
  } );
}

void QgsAiRulesSkillsCloudClient::pushSkill( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const RemoteSkill &skill )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  const bool isUpdate = !skill.id.isEmpty();
  const QString path = isUpdate ? u"/v1/workspaces/%1/skills/%2"_s.arg( encodedId( workspaceId ), encodedId( skill.id ) ) : u"/v1/workspaces/%1/skills"_s.arg( encodedId( workspaceId ) );
  QNetworkRequest request( apiUrl( apiBase, path ) );
  setJsonHeaders( request, sessionToken );
  const QByteArray payload = QJsonDocument( skillJson( skill ) ).toJson( QJsonDocument::Compact );
  QNetworkReply *reply = isUpdate ? networkManager->sendCustomRequest( request, "PATCH", payload ) : networkManager->post( request, payload );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud skill sync request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }
    emit skillSynced( skillFromJson( QJsonDocument::fromJson( responseBody ).object() ) );
  } );
}

void QgsAiRulesSkillsCloudClient::deleteRuleRemote( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const QString &remoteRuleId )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/workspaces/%1/rules/%2"_s.arg( encodedId( workspaceId ), encodedId( remoteRuleId ) ) ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->deleteResource( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud rule delete request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, remoteRuleId]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }
    emit ruleDeleted( remoteRuleId );
  } );
}

void QgsAiRulesSkillsCloudClient::deleteSkillRemote( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const QString &remoteSkillId )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/workspaces/%1/skills/%2"_s.arg( encodedId( workspaceId ), encodedId( remoteSkillId ) ) ) );
  setJsonHeaders( request, sessionToken );
  QNetworkReply *reply = networkManager->deleteResource( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Strata Cloud skill delete request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, remoteSkillId]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, responseBody ) );
      return;
    }
    emit skillDeleted( remoteSkillId );
  } );
}
