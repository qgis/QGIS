#include "qgsaimodels.h"

#include <QUuid>

QString qgsAiChatRoleToString( QgsAiChatRole role )
{
  switch ( role )
  {
    case QgsAiChatRole::System:
      return QStringLiteral( "system" );
    case QgsAiChatRole::Assistant:
      return QStringLiteral( "assistant" );
    case QgsAiChatRole::Tool:
      return QStringLiteral( "tool" );
    case QgsAiChatRole::User:
    default:
      return QStringLiteral( "user" );
  }
}

QgsAiChatRole qgsAiChatRoleFromString( const QString &value )
{
  if ( value == QLatin1String( "system" ) )
    return QgsAiChatRole::System;
  if ( value == QLatin1String( "assistant" ) )
    return QgsAiChatRole::Assistant;
  if ( value == QLatin1String( "tool" ) )
    return QgsAiChatRole::Tool;
  return QgsAiChatRole::User;
}

QJsonObject QgsAiChatMessage::toJson() const
{
  QJsonObject json;
  json.insert( QStringLiteral( "id" ), id );
  json.insert( QStringLiteral( "role" ), qgsAiChatRoleToString( role ) );
  json.insert( QStringLiteral( "content" ), content );
  json.insert( QStringLiteral( "timestamp" ), timestamp.toString( Qt::ISODateWithMs ) );
  json.insert( QStringLiteral( "metadata" ), QJsonObject::fromVariantMap( metadata ) );
  return json;
}

QgsAiChatMessage QgsAiChatMessage::fromJson( const QJsonObject &json )
{
  QgsAiChatMessage message;
  message.id = json.value( QStringLiteral( "id" ) ).toString( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  message.role = qgsAiChatRoleFromString( json.value( QStringLiteral( "role" ) ).toString() );
  message.content = json.value( QStringLiteral( "content" ) ).toString();
  message.timestamp = QDateTime::fromString( json.value( QStringLiteral( "timestamp" ) ).toString(), Qt::ISODateWithMs );
  message.metadata = json.value( QStringLiteral( "metadata" ) ).toObject().toVariantMap();
  return message;
}

QJsonObject QgsAiToolCall::toJson() const
{
  QJsonObject json;
  json.insert( QStringLiteral( "id" ), id );
  json.insert( QStringLiteral( "name" ), name );
  json.insert( QStringLiteral( "args" ), args );
  return json;
}

QgsAiToolCall QgsAiToolCall::fromJson( const QJsonObject &json )
{
  QgsAiToolCall call;
  call.id = json.value( QStringLiteral( "id" ) ).toString();
  call.name = json.value( QStringLiteral( "name" ) ).toString();
  call.args = json.value( QStringLiteral( "args" ) ).toObject();
  return call;
}

QJsonObject QgsAiPatchHunk::toJson() const
{
  QJsonObject json;
  json.insert( QStringLiteral( "filePath" ), filePath );
  json.insert( QStringLiteral( "originalText" ), originalText );
  json.insert( QStringLiteral( "replacementText" ), replacementText );
  json.insert( QStringLiteral( "priority" ), priority );
  return json;
}

QgsAiPatchHunk QgsAiPatchHunk::fromJson( const QJsonObject &json )
{
  QgsAiPatchHunk hunk;
  hunk.filePath = json.value( QStringLiteral( "filePath" ) ).toString();
  hunk.originalText = json.value( QStringLiteral( "originalText" ) ).toString();
  hunk.replacementText = json.value( QStringLiteral( "replacementText" ) ).toString();
  hunk.priority = json.value( QStringLiteral( "priority" ) ).toInt();
  return hunk;
}

QJsonObject QgsAiPatchProposal::toJson() const
{
  QJsonObject json;
  json.insert( QStringLiteral( "id" ), id );
  json.insert( QStringLiteral( "title" ), title );
  json.insert( QStringLiteral( "createdAt" ), createdAt.toString( Qt::ISODateWithMs ) );

  QJsonArray hunkArray;
  for ( const QgsAiPatchHunk &hunk : hunks )
    hunkArray.push_back( hunk.toJson() );
  json.insert( QStringLiteral( "hunks" ), hunkArray );
  return json;
}

QgsAiPatchProposal QgsAiPatchProposal::fromJson( const QJsonObject &json )
{
  QgsAiPatchProposal proposal;
  proposal.id = json.value( QStringLiteral( "id" ) ).toString( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  proposal.title = json.value( QStringLiteral( "title" ) ).toString();
  proposal.createdAt = QDateTime::fromString( json.value( QStringLiteral( "createdAt" ) ).toString(), Qt::ISODateWithMs );

  const QJsonArray hunkArray = json.value( QStringLiteral( "hunks" ) ).toArray();
  proposal.hunks.reserve( hunkArray.size() );
  for ( const QJsonValue &value : hunkArray )
    proposal.hunks.push_back( QgsAiPatchHunk::fromJson( value.toObject() ) );
  return proposal;
}

QJsonObject QgsAiReviewSuggestion::toJson() const
{
  QJsonObject json;
  json.insert( QStringLiteral( "id" ), id );
  json.insert( QStringLiteral( "summary" ), summary );
  json.insert( QStringLiteral( "proposal" ), proposal.toJson() );
  return json;
}

QgsAiReviewSuggestion QgsAiReviewSuggestion::fromJson( const QJsonObject &json )
{
  QgsAiReviewSuggestion suggestion;
  suggestion.id = json.value( QStringLiteral( "id" ) ).toString( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  suggestion.summary = json.value( QStringLiteral( "summary" ) ).toString();
  suggestion.proposal = QgsAiPatchProposal::fromJson( json.value( QStringLiteral( "proposal" ) ).toObject() );
  return suggestion;
}
