/***************************************************************************
    qgsailayertools.h
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

#ifndef QGSAILAYERTOOLS_H
#define QGSAILAYERTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsAiFileContextProvider;
class QgsMapCanvas;
class QgsProject;

/**
 * add_layer_from_file: loads a vector or raster file as a new map layer in the
 * current QGIS project. The path may be relative to the workspace root or an
 * absolute path to an existing file on disk. The kind is auto-detected from the
 * file extension unless explicitly forced via the `kind` argument.
 *
 * No approval gate: adding a layer is a reversible action (the user can remove
 * it from the layer panel). The tool returns the created layer's id, feature
 * count, CRS and extent so the model can chain follow-up calls.
 */
class APP_EXPORT QgsAiAddLayerFromFileTool : public QgsAiTool
{
  public:
    QgsAiAddLayerFromFileTool( QgsAiFileContextProvider *contextProvider, QgsProject *project );

    QString name() const override { return u"add_layer_from_file"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsProject *mProject = nullptr;
};

/**
 * describe_layer: returns the schema and a small attribute sample for a vector
 * layer already loaded in the project. For raster layers, returns dimensions
 * and band count. Use before writing run_python code that filters or aggregates
 * by attribute, so the model does not need a preliminary inspection round.
 */
class APP_EXPORT QgsAiDescribeLayerTool : public QgsAiTool
{
  public:
    explicit QgsAiDescribeLayerTool( QgsProject *project );

    QString name() const override { return u"describe_layer"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsProject *mProject = nullptr;
};

/**
 * run_processing_algorithm: executes a QGIS Processing algorithm with JSON
 * parameters. This gives the agent a typed native path for common GIS
 * transformations without falling back to run_python.
 */
class APP_EXPORT QgsAiRunProcessingAlgorithmTool : public QgsAiTool
{
  public:
    explicit QgsAiRunProcessingAlgorithmTool( QgsProject *project );

    QString name() const override { return u"run_processing_algorithm"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    bool isAvailable() const override;
    QString availabilityReason() const override;

  private:
    QgsProject *mProject = nullptr;
};

/**
 * style_layer: applies common map styling changes directly through QGIS APIs.
 */
class APP_EXPORT QgsAiStyleLayerTool : public QgsAiTool
{
  public:
    explicit QgsAiStyleLayerTool( QgsProject *project );

    QString name() const override { return u"style_layer"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }

  private:
    QgsProject *mProject = nullptr;
};

/**
 * create_print_layout: creates a simple print layout with a map frame and
 * optional title, using the current canvas extent where available.
 */
class APP_EXPORT QgsAiCreatePrintLayoutTool : public QgsAiTool
{
  public:
    QgsAiCreatePrintLayoutTool( QgsProject *project, QgsMapCanvas *canvas );

    QString name() const override { return u"create_print_layout"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }

  private:
    QgsProject *mProject = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
};

/**
 * export_map: exports an existing print layout to PDF/PNG, or the current map
 * canvas to PNG, into the trusted workspace.
 */
class APP_EXPORT QgsAiExportMapTool : public QgsAiTool
{
  public:
    QgsAiExportMapTool( QgsAiFileContextProvider *contextProvider, QgsProject *project, QgsMapCanvas *canvas );

    QString name() const override { return u"export_map"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }

  private:
    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsProject *mProject = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
};

#endif // QGSAILAYERTOOLS_H
