#ifndef QGSAIMODELS_H
#define QGSAIMODELS_H

#include "qgis_app.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QVariantMap>

enum class QgsAiChatRole
{
  System,
  User,
  Assistant,
  Tool
};

struct APP_EXPORT QgsAiChatMessage
{
  QString id;
  QgsAiChatRole role = QgsAiChatRole::User;
  QString content;
  QDateTime timestamp = QDateTime::currentDateTimeUtc();
  QVariantMap metadata;

  QJsonObject toJson() const;
  static QgsAiChatMessage fromJson( const QJsonObject &json );
};

/**
 * Represents a single tool invocation requested by the LLM (Anthropic `tool_use` block
 * or OpenAI Responses `function_call` event). Same struct works for both providers,
 * the router normalizes their formats.
 */
struct APP_EXPORT QgsAiToolCall
{
  QString id;          // tool_use_id (Anthropic) / call_id (OpenAI)
  QString name;        // tool name
  QJsonObject args;    // parsed input arguments

  QJsonObject toJson() const;
  static QgsAiToolCall fromJson( const QJsonObject &json );
};

struct APP_EXPORT QgsAiPatchHunk
{
  QString filePath;
  QString originalText;
  QString replacementText;
  int priority = 0;

  QJsonObject toJson() const;
  static QgsAiPatchHunk fromJson( const QJsonObject &json );
};

struct APP_EXPORT QgsAiPatchProposal
{
  QString id;
  QString title;
  QList<QgsAiPatchHunk> hunks;
  QDateTime createdAt = QDateTime::currentDateTimeUtc();

  QJsonObject toJson() const;
  static QgsAiPatchProposal fromJson( const QJsonObject &json );
};

struct APP_EXPORT QgsAiReviewSuggestion
{
  QString id;
  QString summary;
  QgsAiPatchProposal proposal;

  QJsonObject toJson() const;
  static QgsAiReviewSuggestion fromJson( const QJsonObject &json );
};

APP_EXPORT QString qgsAiChatRoleToString( QgsAiChatRole role );
APP_EXPORT QgsAiChatRole qgsAiChatRoleFromString( const QString &value );

#endif // QGSAIMODELS_H
