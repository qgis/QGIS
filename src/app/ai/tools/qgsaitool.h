#ifndef QGSAITOOL_H
#define QGSAITOOL_H

#include "qgis_app.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

struct APP_EXPORT QgsAiToolResult
{
  bool success = false;
  QJsonValue output;
  QString errorMessage;

  static QgsAiToolResult ok( const QJsonValue &output )
  {
    QgsAiToolResult result;
    result.success = true;
    result.output = output;
    return result;
  }

  static QgsAiToolResult error( const QString &message )
  {
    QgsAiToolResult result;
    result.success = false;
    result.errorMessage = message;
    return result;
  }
};

class APP_EXPORT QgsAiTool
{
  public:
    virtual ~QgsAiTool() = default;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QJsonObject schema() const = 0;
    virtual QgsAiToolResult execute( const QJsonObject &args ) = 0;
    virtual bool requiresApproval() const { return false; }
};

#endif // QGSAITOOL_H
