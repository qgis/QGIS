/***************************************************************************
    qgsaimodels.cpp
    ---------------------
    begin                : April 2026
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

#include "qgsaimodels.h"

#include <QString>
#include <QUuid>

using namespace Qt::StringLiterals;

QString qgsAiChatRoleToString( QgsAiChatRole role )
{
  switch ( role )
  {
    case QgsAiChatRole::System:
      return u"system"_s;
    case QgsAiChatRole::Assistant:
      return u"assistant"_s;
    case QgsAiChatRole::Tool:
      return u"tool"_s;
    case QgsAiChatRole::User:
    default:
      return u"user"_s;
  }
}

QgsAiChatRole qgsAiChatRoleFromString( const QString &value )
{
  if ( value == "system"_L1 )
    return QgsAiChatRole::System;
  if ( value == "assistant"_L1 )
    return QgsAiChatRole::Assistant;
  if ( value == "tool"_L1 )
    return QgsAiChatRole::Tool;
  return QgsAiChatRole::User;
}

QJsonObject QgsAiChatMessage::toJson() const
{
  QJsonObject json;
  json.insert( u"id"_s, id );
  json.insert( u"role"_s, qgsAiChatRoleToString( role ) );
  json.insert( u"content"_s, content );
  json.insert( u"timestamp"_s, timestamp.toString( Qt::ISODateWithMs ) );
  json.insert( u"metadata"_s, QJsonObject::fromVariantMap( metadata ) );
  return json;
}

QgsAiChatMessage QgsAiChatMessage::fromJson( const QJsonObject &json )
{
  QgsAiChatMessage message;
  message.id = json.value( u"id"_s ).toString( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  message.role = qgsAiChatRoleFromString( json.value( u"role"_s ).toString() );
  message.content = json.value( u"content"_s ).toString();
  message.timestamp = QDateTime::fromString( json.value( u"timestamp"_s ).toString(), Qt::ISODateWithMs );
  message.metadata = json.value( u"metadata"_s ).toObject().toVariantMap();
  return message;
}

QJsonObject QgsAiToolCall::toJson() const
{
  QJsonObject json;
  json.insert( u"id"_s, id );
  json.insert( u"name"_s, name );
  json.insert( u"args"_s, args );
  return json;
}

QgsAiToolCall QgsAiToolCall::fromJson( const QJsonObject &json )
{
  QgsAiToolCall call;
  call.id = json.value( u"id"_s ).toString();
  call.name = json.value( u"name"_s ).toString();
  call.args = json.value( u"args"_s ).toObject();
  return call;
}

QJsonObject QgsAiPatchHunk::toJson() const
{
  QJsonObject json;
  json.insert( u"filePath"_s, filePath );
  json.insert( u"originalText"_s, originalText );
  json.insert( u"replacementText"_s, replacementText );
  json.insert( u"priority"_s, priority );
  json.insert( u"isCreate"_s, isCreate );
  json.insert( u"isDelete"_s, isDelete );
  return json;
}

QgsAiPatchHunk QgsAiPatchHunk::fromJson( const QJsonObject &json )
{
  QgsAiPatchHunk hunk;
  hunk.filePath = json.value( u"filePath"_s ).toString();
  hunk.originalText = json.value( u"originalText"_s ).toString();
  hunk.replacementText = json.value( u"replacementText"_s ).toString();
  hunk.priority = json.value( u"priority"_s ).toInt();
  hunk.isCreate = json.value( u"isCreate"_s ).toBool();
  hunk.isDelete = json.value( u"isDelete"_s ).toBool();
  return hunk;
}

QJsonObject QgsAiPatchProposal::toJson() const
{
  QJsonObject json;
  json.insert( u"id"_s, id );
  json.insert( u"title"_s, title );
  json.insert( u"createdAt"_s, createdAt.toString( Qt::ISODateWithMs ) );

  QJsonArray hunkArray;
  for ( const QgsAiPatchHunk &hunk : hunks )
    hunkArray.push_back( hunk.toJson() );
  json.insert( u"hunks"_s, hunkArray );
  return json;
}

QgsAiPatchProposal QgsAiPatchProposal::fromJson( const QJsonObject &json )
{
  QgsAiPatchProposal proposal;
  proposal.id = json.value( u"id"_s ).toString( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  proposal.title = json.value( u"title"_s ).toString();
  proposal.createdAt = QDateTime::fromString( json.value( u"createdAt"_s ).toString(), Qt::ISODateWithMs );

  const QJsonArray hunkArray = json.value( u"hunks"_s ).toArray();
  proposal.hunks.reserve( hunkArray.size() );
  for ( const QJsonValue &value : hunkArray )
    proposal.hunks.push_back( QgsAiPatchHunk::fromJson( value.toObject() ) );
  return proposal;
}

QJsonObject QgsAiReviewSuggestion::toJson() const
{
  QJsonObject json;
  json.insert( u"id"_s, id );
  json.insert( u"summary"_s, summary );
  json.insert( u"proposal"_s, proposal.toJson() );
  return json;
}

QgsAiReviewSuggestion QgsAiReviewSuggestion::fromJson( const QJsonObject &json )
{
  QgsAiReviewSuggestion suggestion;
  suggestion.id = json.value( u"id"_s ).toString( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  suggestion.summary = json.value( u"summary"_s ).toString();
  suggestion.proposal = QgsAiPatchProposal::fromJson( json.value( u"proposal"_s ).toObject() );
  return suggestion;
}
