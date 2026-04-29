#ifndef QGSAIECHOTOOL_H
#define QGSAIECHOTOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

/**
 * Smoke-test tool that returns its input under `echoed`. Used to validate the
 * full tool-use loop (schema in payload, model invocation, execute, tool_result
 * back to the model) without depending on any GIS state. Always present in the
 * registry so the loop can be exercised end-to-end on a fresh install.
 */
class APP_EXPORT QgsAiEchoTool : public QgsAiTool
{
  public:
    QgsAiEchoTool() = default;

    QString name() const override { return QStringLiteral( "echo" ); }

    QString description() const override
    {
      return QStringLiteral(
        "Returns the provided text unchanged. Use this only when the user explicitly asks to test the tool loop."
      );
    }

    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

#endif // QGSAIECHOTOOL_H
