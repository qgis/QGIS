/***************************************************************************
    qgsaitoolregistry.h
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

#ifndef QGSAITOOLREGISTRY_H
#define QGSAITOOLREGISTRY_H

#include <map>
#include <memory>

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QJsonArray>
#include <QObject>
#include <QStringList>

class APP_EXPORT QgsAiToolRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiToolRegistry( QObject *parent = nullptr );
    ~QgsAiToolRegistry() override;

    //! Registers \a tool. Takes ownership. Returns false if a tool with same name already exists.
    bool registerTool( std::unique_ptr<QgsAiTool> tool );

    //! Returns pointer to registered tool, or nullptr.
    QgsAiTool *find( const QString &name ) const;

    //! Returns all registered tool names.
    QStringList toolNames() const;

    //! Returns registered tool names whose runtime dependencies are currently available.
    QStringList availableToolNames() const;

    int count() const { return mTools.size(); }

    /**
     * Returns a JSON array describing registered tools in Anthropic Claude tool-use format:
     * `[{ "name": "...", "description": "...", "input_schema": {...} }, ...]`.
     * If \a allowedTools is non-empty, only tools whose name is in the set are included.
     * The router converts this format to provider-specific payloads.
     */
    QJsonArray schemasJson( const QStringList &allowedTools = QStringList() ) const;

    enum class WireFormat
    {
      AnthropicTools,       //!< `[{name, description, input_schema}]` for Anthropic Messages API
      OpenAiResponses,      //!< `[{type:"function", name, description, parameters}]` for OpenAI Responses API
      OpenAiChatCompletions //!< `[{type:"function", function:{name, description, parameters}}]` for OpenAI Chat Completions API
    };

    /**
     * Returns the registered tool schemas in the wire format expected by \a format.
     * Use this when building the request payload for a specific provider.
     */
    QJsonArray schemasJsonForFormat( WireFormat format, const QStringList &allowedTools = QStringList() ) const;

    /**
     * Looks up the tool by \a name and runs it with \a args. If the tool is missing
     * the result is `success=false` with an actionable error message.
     */
    QgsAiToolResult execute( const QString &name, const QJsonObject &args ) const;

    //! Removes all registered tools.
    void clear();

  signals:
    void toolRegistered( const QString &name );

  private:
    std::map<QString, std::unique_ptr<QgsAiTool>> mTools;
};

#endif // QGSAITOOLREGISTRY_H
