/***************************************************************************
    qgsaigissuggestionengine.h
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

#ifndef QGSAIGISSUGGESTIONENGINE_H
#define QGSAIGISSUGGESTIONENGINE_H

#include "qgis_app.h"

#include <QList>
#include <QString>

class QgsProject;

struct APP_EXPORT QgsAiGisSuggestion
{
    QString id;
    QString title;
    QString detail;
    QString actionPrompt;
    QString risk = QStringLiteral( "low" ); //!< low | medium | high
};

/**
 * Rule-based project health checks feeding the AI chat: automatic system-prompt
 * context, the in-chat suggestion card and the \c @gis mention.
 */
class APP_EXPORT QgsAiGisSuggestionEngine
{
  public:
    static QList<QgsAiGisSuggestion> suggestionsForProject( QgsProject *project );

    /**
     * Markdown block "## Current project GIS health", capped at \a maxSuggestions
     * entries. \a full adds each suggestion's action prompt (used by the \c @gis
     * mention); the compact form is meant for automatic prompt injection.
     */
    static QString formatHealthBlock( const QList<QgsAiGisSuggestion> &suggestions, bool full, int maxSuggestions = 10 );

    //! Compact health block for the system prompt; empty when disabled or healthy.
    static QString promptHealthBlockForProject( QgsProject *project );

    static QString globalEnabledSettingsKey();
    static QString projectEnabledSettingsKey( const QString &projectFilePath );
    //! QStringList of suggestion ids dismissed from the in-chat card for this project.
    static QString dismissedSettingsKey( const QString &projectFilePath );
    static bool suggestionsEnabledForProject( QgsProject *project );
};

#endif // QGSAIGISSUGGESTIONENGINE_H
