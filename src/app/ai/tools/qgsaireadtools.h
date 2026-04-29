#ifndef QGSAIREADTOOLS_H
#define QGSAIREADTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

class QgsAiFileContextProvider;
class QgsMapCanvas;
class QgsProject;

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

    QString name() const override { return QStringLiteral( "read_file" ); }
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

    QString name() const override { return QStringLiteral( "search_files" ); }
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

    QString name() const override { return QStringLiteral( "list_files" ); }
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

    QString name() const override { return QStringLiteral( "list_project_layers" ); }
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

    QString name() const override { return QStringLiteral( "get_active_canvas_extent" ); }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsMapCanvas *mCanvas = nullptr;
};

#endif // QGSAIREADTOOLS_H
