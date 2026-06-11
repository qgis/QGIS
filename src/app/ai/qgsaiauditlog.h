/***************************************************************************
    qgsaiauditlog.h
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

#ifndef QGSAIAUDITLOG_H
#define QGSAIAUDITLOG_H

#include "qgis_app.h"

#include <QString>

/**
 * Append-only audit trail for the risky AI tool executions the user approved
 * (run_python, install_python_package, download_file).
 *
 * Each entry is one line in `<qgisSettingsDirPath>/ai_audit.log` and records a
 * redacted summary plus a SHA-256 hash of the original detail. Raw code, full
 * URLs, query strings and absolute paths are not written.
 * Best-effort: an unwritable log never blocks tool execution (one warning per
 * session in the message log).
 */
class APP_EXPORT QgsAiAuditLog
{
  public:
    //! Appends an audit entry for \a tool with free-form \a detail (code, package list, URL…).
    static void append( const QString &tool, const QString &detail );

    //! Resolved audit log file path. Honors setFilePathOverride() (tests).
    static QString filePath();

    //! Overrides the audit log path (unit tests only). Empty restores the default.
    static void setFilePathOverride( const QString &path );
};

#endif // QGSAIAUDITLOG_H
