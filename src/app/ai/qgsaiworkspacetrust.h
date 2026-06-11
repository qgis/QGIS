/***************************************************************************
    qgsaiworkspacetrust.h
    ---------------------
    begin                : June 2026
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

#ifndef QGSAIWORKSPACETRUST_H
#define QGSAIWORKSPACETRUST_H

#include "qgis_app.h"

#include <QString>

/**
 * Per-workspace trust state for the AI assistant (VS Code-style workspace trust).
 *
 * Untrusted (or not-yet-decided) workspaces:
 *  - do not load `.strata/rules` / `.strata/skills` files into the system prompt
 *    (defense against indirect prompt injection from shared/downloaded projects);
 *  - do not expose the risky tools (run_python, install_python_package,
 *    download_file) to the model.
 *
 * The decision is persisted per workspace-root hash in QgsSettings and can be
 * changed any time from the AI provider settings dialog.
 */
class APP_EXPORT QgsAiWorkspaceTrust
{
  public:
    enum class State
    {
      Unknown,  //!< Never decided — treated as NOT trusted (restricted until answered)
      Trusted,
      Untrusted
    };

    //! Stable hash for \a root (same convention as the index/chat stores: SHA1 hex, first 16 chars).
    static QString workspaceHash( const QString &root );

    //! Persisted trust state for \a root. An empty root has nothing to trust and reports Trusted.
    static State state( const QString &root );

    //! Persists the trust decision for \a root.
    static void setState( const QString &root, State state );

    //! True only when the user explicitly trusted \a root (Unknown counts as NOT trusted).
    static bool isTrusted( const QString &root );

    //! Convenience: trust state of the currently configured AI workspace root.
    static bool isCurrentWorkspaceTrusted();

    //! Currently configured AI workspace root (reads the same settings as the file context provider).
    static QString currentWorkspaceRoot();
};

#endif // QGSAIWORKSPACETRUST_H
