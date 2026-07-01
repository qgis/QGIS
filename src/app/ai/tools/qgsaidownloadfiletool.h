/***************************************************************************
    qgsaidownloadfiletool.h
    ---------------------
    begin                : May 2026
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

#ifndef QGSAIDOWNLOADFILETOOL_H
#define QGSAIDOWNLOADFILETOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QWidget;
class QgsAiFileContextProvider;

/**
 * download_file: lets the model fetch a remote file (HTTP/HTTPS) and save it inside
 * the workspace. The user MUST approve via a confirmation dialog before the request
 * starts; cancellation returns "user_rejected".
 *
 * Use this when the user needs remote data (e.g. Overpass/Nominatim/GADM GeoJSON,
 * shapefiles in zip, etc.) — it is one approval and stays inside the workspace
 * boundary. Prefer this over installing `requests` + writing run_python.
 *
 * Path safety: dest_path is validated against QgsAiFileContextProvider::normalizePath
 * with allowExternal=false, so writes outside the workspace are refused before the
 * dialog is shown. Existing files are not overwritten unless overwrite=true.
 *
 * Streaming: the body is written incrementally to disk via QNetworkReply::readyRead
 * to avoid loading large payloads in RAM. The reply is aborted (and the partial file
 * removed) if cumulative bytes exceed max_bytes or HTTP status is not 2xx.
 *
 * Hard caps:
 *
 * - URL must use http or https.
 * - Default max_bytes = 100 MiB; caller may pass a smaller value.
 * - Redirect policy = NoLessSafeRedirectPolicy (HTTPS→HTTPS ok, HTTPS→HTTP refused).
 * - Transfer timeout 60s, overall request timeout 600s.
 */
class APP_EXPORT QgsAiDownloadFileTool : public QgsAiTool
{
  public:
    static constexpr qint64 DEFAULT_MAX_BYTES = 100LL * 1024 * 1024;       // 100 MiB
    static constexpr qint64 ABSOLUTE_MAX_BYTES = 2LL * 1024 * 1024 * 1024; // 2 GiB hard ceiling
    static constexpr int TRANSFER_TIMEOUT_MS = 60'000;
    static constexpr int OVERALL_TIMEOUT_MS = 600'000;

    QgsAiDownloadFileTool( QgsAiFileContextProvider *contextProvider, QWidget *dialogParent );

    QString name() const override { return u"download_file"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::High; }
    bool isAvailable() const override;
    QString availabilityReason() const override;

  private:
    QgsAiFileContextProvider *mContextProvider = nullptr;
    QWidget *mDialogParent = nullptr;
};

#endif // QGSAIDOWNLOADFILETOOL_H
