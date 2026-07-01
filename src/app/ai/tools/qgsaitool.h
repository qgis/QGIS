/***************************************************************************
    qgsaitool.h
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

#ifndef QGSAITOOL_H
#define QGSAITOOL_H

#include "qgis_app.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::StringLiterals;

enum class QgsAiToolRiskLevel
{
  Low,
  Medium,
  High,
  Critical
};

inline QString QgsAiToolRiskLevelName( QgsAiToolRiskLevel level )
{
  switch ( level )
  {
    case QgsAiToolRiskLevel::Low:
      return u"low"_s;
    case QgsAiToolRiskLevel::Medium:
      return u"medium"_s;
    case QgsAiToolRiskLevel::High:
      return u"high"_s;
    case QgsAiToolRiskLevel::Critical:
      return u"critical"_s;
  }
  return u"low"_s;
}

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
    virtual QgsAiToolRiskLevel riskLevel() const { return QgsAiToolRiskLevel::Low; }
    virtual bool isAvailable() const { return true; }
    virtual QString availabilityReason() const { return QString(); }
};

#endif // QGSAITOOL_H
