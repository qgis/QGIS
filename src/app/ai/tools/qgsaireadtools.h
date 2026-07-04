/***************************************************************************
    qgsaireadtools.h
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

#ifndef QGSAIREADTOOLS_H
#define QGSAIREADTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsAiFileContextProvider;
class QgsMapCanvas;
class QgsProject;
class QWidget;

/**
 * read_file: returns text content of a workspace file. Optional start_line / end_line
 * select a range (1-based, inclusive). Truncation is reported as `truncated=true`.
 * The path is normalized via the file context provider, so paths outside the workspace
 * are rejected with a clear error.
 */
class APP_EXPORT QgsAiReadFileTool : public QgsAiTool
{
  public:
    explicit QgsAiReadFileTool( QgsAiFileContextProvider *contextProvider );

    QString name() const override { return u"read_file"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiFileContextProvider *mContextProvider = nullptr;
};

/**
 * search_files: substring (case-insensitive) grep across the workspace, optionally
 * filtered by glob on the relative path. Returns up to `max_results` matches as
 * `[{path, line, text}]`.
 */
class APP_EXPORT QgsAiSearchFilesTool : public QgsAiTool
{
  public:
    explicit QgsAiSearchFilesTool( QgsAiFileContextProvider *contextProvider );

    QString name() const override { return u"search_files"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiFileContextProvider *mContextProvider = nullptr;
};

/**
 * list_files: returns workspace file paths matching an optional glob substring.
 * Skips well-known noise directories (.git, build, external, …).
 */
class APP_EXPORT QgsAiListFilesTool : public QgsAiTool
{
  public:
    explicit QgsAiListFilesTool( QgsAiFileContextProvider *contextProvider );

    QString name() const override { return u"list_files"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiFileContextProvider *mContextProvider = nullptr;
};

/**
 * list_project_layers: enumerates the layers loaded in the current QgsProject.
 * Returns id, name, type, geometry type (for vectors), CRS, source path and feature count.
 */
class APP_EXPORT QgsAiListProjectLayersTool : public QgsAiTool
{
  public:
    explicit QgsAiListProjectLayersTool( QgsProject *project );

    QString name() const override { return u"list_project_layers"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsProject *mProject = nullptr;
};

/**
 * get_active_canvas_extent: returns the visible extent of the main map canvas plus
 * its destination CRS. Useful when the model needs to bound spatial queries.
 */
class APP_EXPORT QgsAiGetCanvasExtentTool : public QgsAiTool
{
  public:
    explicit QgsAiGetCanvasExtentTool( QgsMapCanvas *canvas );

    QString name() const override { return u"get_active_canvas_extent"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsMapCanvas *mCanvas = nullptr;
};

/**
 * set_canvas_extent: changes the main map canvas view by explicit extent, scale,
 * zoom-to-layer, or zoom-to-selection. Returns a rollback token that restores
 * the previous extent and destination CRS.
 */
class APP_EXPORT QgsAiSetCanvasExtentTool : public QgsAiTool
{
  public:
    QgsAiSetCanvasExtentTool( QgsMapCanvas *canvas, QgsProject *project );

    QString name() const override { return u"set_canvas_extent"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::Low; }

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QgsProject *mProject = nullptr;
};

/**
 * capture_map_canvas: renders the current 2D map canvas offscreen to a temporary PNG
 * and returns the image path plus canvas metadata. The image is intended for visual
 * context and may be sent to vision-capable model providers after user consent.
 */
class APP_EXPORT QgsAiCaptureMapCanvasTool : public QgsAiTool
{
  public:
    QgsAiCaptureMapCanvasTool( QgsMapCanvas *canvas, QWidget *consentParent = nullptr );

    QString name() const override { return u"capture_map_canvas"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QWidget *mConsentParent = nullptr;
};

#endif // QGSAIREADTOOLS_H
