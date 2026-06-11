/***************************************************************************
    qgsaimodels.h
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
    QString id;       // tool_use_id (Anthropic) / call_id (OpenAI)
    QString name;     // tool name
    QJsonObject args; // parsed input arguments

    QJsonObject toJson() const;
    static QgsAiToolCall fromJson( const QJsonObject &json );
};

/**
 * Token and cost accounting for a single model response. OpenRouter always
 * includes usage in its responses; other providers report a subset of fields.
 */
struct APP_EXPORT QgsAiUsage
{
    qint64 promptTokens = 0;
    qint64 completionTokens = 0;
    qint64 totalTokens = 0;
    qint64 cachedTokens = 0;    // prompt tokens served from the provider prompt cache
    qint64 reasoningTokens = 0; // thinking tokens for reasoning models
    double costUsd = 0.0;       // total amount charged (OpenRouter `cost` field)

    bool isValid() const { return promptTokens > 0 || completionTokens > 0 || totalTokens > 0; }

    void add( const QgsAiUsage &other )
    {
      promptTokens += other.promptTokens;
      completionTokens += other.completionTokens;
      totalTokens += other.totalTokens;
      cachedTokens += other.cachedTokens;
      reasoningTokens += other.reasoningTokens;
      costUsd += other.costUsd;
    }
};

struct APP_EXPORT QgsAiPatchHunk
{
    QString filePath;
    QString originalText;
    QString replacementText;
    int priority = 0;

    /**
   * If true, the hunk represents a new file creation. \a originalText must be empty,
   * \a replacementText is the full content of the new file. The file at filePath must
   * not exist when the hunk is applied.
   */
    bool isCreate = false;

    /**
   * If true, the hunk represents a file deletion. Both \a originalText and
   * \a replacementText are ignored; the file at filePath is removed (with backup).
   */
    bool isDelete = false;

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
